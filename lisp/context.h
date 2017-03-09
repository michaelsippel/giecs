
#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#include <giecs/memory/context.h>
#include <giecs/memory/accessors/stack.h>
#include <giecs/core.h>
#include <giecs/ll/arithmetic.h>
#include <giecs/ll/io.h>
#include <giecs/ll/system.h>

#include <lisp/ast_write.h>

namespace lisp
{

/**
 * Contains symbol table and stack access
 */
template <int page_size, typename align_t, typename addr_t, typename val_t>
class Context
{
    private:
        static void ll_printi(giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
        {
            giecs::ll::ConsoleIO<int>::print(stack);
            std::cout << std::endl;
        }

        static void ll_nop(giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
        {
        }

        void eval_param(int l, giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
        {
            int i = 0;
            val_t buffer[l];

            while(i < l)
            {
                auto root = read_ast<page_size, align_t, addr_t, val_t>(stack);

                int start = this->def_stack.pos;
                parse(root, *this);
                int end = this->def_stack.pos;

                int sb = stack.pos;

                size_t len = end - start;
                size_t nl = 0;
                if(root->getType() == ast::NodeType::list)
                {
                    val_t buf[len];
                    this->def_stack.read(addr_t(start), len, buf);

                    stack.push(len, buf);
                    stack.pop();
                    this->core.eval(stack);

                    nl = stack.pos - sb;
                    stack.read(addr_t(sb), nl, &buffer[i]);
                }
                else
                {
                    this->def_stack.read(addr_t(start), len, &buffer[i]);
                    nl = len;
                }

                stack.pos = sb;
                this->def_stack.pos = start;
                i += nl;
            }

            stack.push(l, buffer);
        }

    public:
        Context(giecs::memory::Context<page_size, align_t> const& mem_context_, int limit_)
            : mem_context(mem_context_),
              limit(limit_), def_limit(0),
              stack(mem_context_.template createStack<addr_t, val_t>()),
              def_stack(giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>(mem_context_, (limit_*giecs::bitsize<val_t>())/giecs::bitsize<align_t>()))
        {
            std::function<void (giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)> eval =
                [this](giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
            {
                this->core.eval(stack);
            };
            std::function<void (giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)> deval =
                [this](giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
            {
                int llen = stack.template pop<val_t>();
                addr_t laddr = stack.template pop<addr_t>();

                addr_t list_index[llen];

                for(int i = 0; i < llen; i++)
                {
                    list_index[i] = addr_t(laddr + i);
                    val_t attr = this->stack[addr_t(laddr + i)];
                    laddr = laddr + ((attr == val_t(-1)) ? val_t(1) : attr);
                }

                for(int i = llen-1; i >= 0; i--)
                {
                    addr_t list_addr = list_index[i];

                    val_t attr = this->stack[list_addr];
                    if(attr == val_t(-1))
                    {
                        // execute
                        addr_t w = this->stack[++list_addr];
                        stack.push(w);

                        this->core.eval(stack);
                    }
                    else
                    {
                        val_t buf[int(attr)];
                        this->stack.read(++list_addr, attr, buf);
                        stack.push(attr, buf);
                    }
                }

                this->core.eval(stack);
            };
            std::function<void (giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)> quote =
                [this](giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
            {
                auto root = read_ast<page_size, align_t, addr_t, val_t>(stack);

                int start = this->def_stack.pos;
                parse(root, *this);
                int end = this->def_stack.pos;

                if(root->getType() != ast::NodeType::list)
                {
                    int len = end - start;
                    val_t buf[len];
                    this->def_stack.read(addr_t(start), len, buf);
                    stack.push(len, buf);
                    this->def_stack.pos = start;
                }
                else
                    stack.push(addr_t(this->limit + start));
            };
            std::function<void (giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)> define =
                [this](giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
            {
                std::cout << "Define" << std::endl;
            };
            std::function<void (giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)> peval =
                [this](giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
            {
                addr_t fn = stack.pop();
                int len = stack.template pop<val_t>();
                this->eval_param(len, stack);
                stack.push(fn);
                this->core.eval(stack);
            };

            this->add_macro("nop", ll_nop);
            this->add_macro("quote", quote);
            this->add_macro("define", define);
            this->add_macro("peval", peval);

            this->add_function("eval", 1, eval);
            this->add_function("deval", 2, deval);
            this->add_function("syscall", 6, giecs::ll::System::syscall);
            this->add_function("printi", 1, ll_printi);
            this->add_function("+", 2, giecs::ll::Arithmetic<int>::add);
            this->add_function("-", 2, giecs::ll::Arithmetic<int>::sub);
            this->add_function("*", 2, giecs::ll::Arithmetic<int>::mul);
            this->add_function("/", 2, giecs::ll::Arithmetic<int>::div);
            this->add_function("=", 2, giecs::ll::Arithmetic<int>::eq);
            this->add_function("!=", 2, giecs::ll::Arithmetic<int>::neq);
            this->add_function(">", 2, giecs::ll::Arithmetic<int>::gt);
            this->add_function(">=", 2, giecs::ll::Arithmetic<int>::get);
            this->add_function("<", 2, giecs::ll::Arithmetic<int>::lt);
            this->add_function("<=", 2, giecs::ll::Arithmetic<int>::let);
        }

        void reset(void)
        {
            this->def_stack.pos = this->def_limit;
        }

        void eval(void)
        {
            this->stack.push(addr_t(this->limit + this->def_limit)); // startig address of last definition
            this->core.eval(this->stack);
        }

        void push(val_t v)
        {
            this->def_stack.push(v);
        }

        addr_t resolve_symbol(std::string name)
        {
            auto it = this->symbols.find(name);
            if(it != this->symbols.end())
                return it->second;
            else
                std::cout << "lisp: undefined symbol \"" << name << "\"." << std::endl;

            return addr_t();
        }
        void push(std::string name)
        {
            this->def_stack.push(resolve_symbol(name));
        }

        addr_t def_ptr(void) const
        {
            return addr_t(this->limit + this->def_stack.pos);
        }

        void write_def(addr_t addr, val_t val)
        {
            this->def_stack[addr-this->limit] = val;
        }

        struct List : public std::vector<val_t>
        {
            addr_t start;
        };

        List create_list(int len)
        {
            List l;
            l.start = addr_t(this->def_stack.pos);
            this->def_stack.pos += len;
            return l;
        }

        void push(List& l)
        {
            for(addr_t const& a : l)
            {
                this->def_stack[l.start] = a;
                ++l.start;
            }
        }

        template <typename Node>
        size_t push_ast(std::shared_ptr<Node> node)
        {
            return write_ast<page_size, align_t, addr_t, val_t>(node, this->def_stack);
        }

    private:
        std::unordered_map<std::string, addr_t> symbols;
        giecs::memory::Context<page_size, align_t> const& mem_context;
        giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t> stack;
        giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t> def_stack;
        giecs::Core<page_size, align_t, addr_t> core;

        int def_limit;
        int limit;

        void save_symbol(std::string name)
        {
            this->symbols[name] = this->limit + this->def_limit;
            this->def_limit = this->def_stack.pos;

            std::cout << "saved " << name << " as " << int(this->symbols[name]) << std::endl;
        }

        void add_macro(std::string name, void (*fn)(giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack))
        {
            this->add_macro(name, std::function<void (giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)>(fn));
        }

        void add_macro(std::string name, std::function<void (giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)> fn)
        {
            this->core.template addOperation<val_t>(this->def_ptr(), fn);
            this->def_stack.move(1);
            this->save_symbol(name);
        }

        void add_function(std::string name, int l, void (*fn)(giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack))
        {
            this->add_function(name, l, std::function<void (giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)>(fn));
        }

        void add_function(std::string name, int l, std::function<void (giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)> fn)
        {
            this->core.template addOperation<val_t>(this->def_ptr(), fn);
            addr_t addr = this->def_ptr();
            this->def_stack.move(1);
            this->def_limit = this->def_stack.pos;

            this->push(3);
            this->push(std::string("peval"));
            this->push(addr);
            this->push(l);

            this->save_symbol(name);
        }

}; // class Context

} // namespace lisp

