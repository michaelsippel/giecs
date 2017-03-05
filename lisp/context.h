
#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#include <giecs/memory/context.h>
#include <giecs/memory/accessors/stack.h>
#include <giecs/core.h>
#include <giecs/ll/arithmetic.h>
#include <giecs/ll/io.h>

namespace lisp
{

/**
 * Contains symbol table and stack access
 */
template <int page_size, typename align_t, typename addr_t, typename val_t>
class Context
{
    private:
        static void printi(giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)
        {
            giecs::ll::ConsoleIO<int>::print(stack);
            std::cout << std::endl;
        }

    public:
        Context(giecs::memory::Context<page_size, align_t> const& mem_context_, int limit_)
            : mem_context(mem_context_),
              limit(limit_), def_limit(0),
              stack(mem_context_.template createStack<addr_t, val_t>()),
              def_stack(giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>(mem_context_, (limit_*giecs::bitsize<val_t>())/giecs::bitsize<align_t>()))
        {
            this->add_llfn("printi", printi);
            this->add_llfn("+", giecs::ll::Arithmetic<int>::add);
            this->add_llfn("-", giecs::ll::Arithmetic<int>::sub);
            this->add_llfn("*", giecs::ll::Arithmetic<int>::mul);
            this->add_llfn("/", giecs::ll::Arithmetic<int>::div);
            this->add_llfn("=", giecs::ll::Arithmetic<int>::eq);
            this->add_llfn("!=", giecs::ll::Arithmetic<int>::neq);
            this->add_llfn(">", giecs::ll::Arithmetic<int>::gt);
            this->add_llfn(">=", giecs::ll::Arithmetic<int>::get);
            this->add_llfn("<", giecs::ll::Arithmetic<int>::lt);
            this->add_llfn("<=", giecs::ll::Arithmetic<int>::let);
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
            // for testing always printi
            this->def_stack.push(val_t(2));
            this->def_stack.push(this->symbols["printi"]);
            this->def_stack.push(v);
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

        void add_llfn(std::string name, void (*fn)(giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack))
        {
            this->add_llfn(name, std::function<void (giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)>(fn));
        }

        void add_llfn(std::string name, std::function<void (giecs::memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)> fn)
        {
            this->core.template addOperation<val_t>(this->limit + this->def_stack.pos, fn);
            this->def_stack.move(1);

            this->save_symbol(name);
        }
}; // class Context

} // namespace lisp

