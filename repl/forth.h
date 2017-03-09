
#pragma once

#include <istream>
#include <string>
#include <map>
#include <boost/algorithm/string.hpp>
#include <giecs/memory/context.h>
#include <giecs/core.h>
#include <giecs/ll/arithmetic.h>
#include <giecs/ll/cond.h>
#include <giecs/ll/io.h>

#include "language.h"

using namespace giecs;

namespace repl
{

namespace lang
{

template <int page_size, typename align_t, typename addr_t, typename word_t=addr_t>
class Forth : public Language
{
        static void dup(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
        {
            word_t a = stack[-1];
            stack.push(a);
        }

        // effectively a push
        static void nop(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
        {
        }

        static void swap(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
        {
            word_t a = stack.pop();
            word_t b = stack.pop();
            stack.push(a);
            stack.push(b);
        }

        static void out(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
        {
            ll::ConsoleIO<int>::print(stack);
            std::cout << "\n";
        }

        static void fif(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
        {
        }

        static void felse(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
        {
        }

        static void fthen(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
        {
        }

        static void read(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
        {
            addr_t addr = stack.pop();

            memory::accessors::Linear<page_size, align_t, addr_t, word_t> abs = memory::accessors::Linear<page_size, align_t, addr_t, word_t>(stack, -stack.getOffset());
            word_t v = abs.read(addr);

            stack.push(v);
        }

        static void write(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
        {
            addr_t addr = stack.pop();
            word_t v = stack.pop();

            memory::accessors::Linear<page_size, align_t, addr_t, word_t> abs = memory::accessors::Linear<page_size, align_t, addr_t, word_t>(stack, -stack.getOffset());
            abs.write(addr, v);
        }

        void forth_eval(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack) const
        {
            int len = stack.template pop<word_t>();
            word_t ptrs[len];
            for(int i = 0; i < len; i++)
                ptrs[i] = stack.pop();

            for(int i = 0; i < len; i++)
            {
                stack.push(ptrs[i]);
                this->core.eval(stack);
            }
        }

    public:
        Forth(memory::Context<page_size, align_t> const& context_, Core<page_size, align_t, addr_t>& core_, int limit_)
            : context(context_), core(core_), limit(limit_),
              stack(this->context.template createStack<addr_t, word_t>()),
              def_stack(memory::accessors::Stack<page_size, align_t, addr_t, word_t>(context_, (limit_*bitsize<word_t>())/bitsize<align_t>()))
        {
            std::function<void (memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)> fev =
                [this](memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
            {
                this->forth_eval(stack);
            };
            std::function<void (memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)> eval =
                [this](memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
            {
                this->core.eval(stack);
            };

            this->def_limit = 0;
            this->core.addOperation(addr_t(this->def_limit + this->limit), fev);
            ++this->def_limit;

            this->addOperation("EXECUTE", eval);
            this->addOperation("NOP", nop);

            this->addOperation("IF", fif);
            this->addOperation("ELSE", felse);
            this->addOperation("THEN", fthen);

            this->addOperation(".", out);
            this->addOperation("EMIT", ll::ConsoleIO<char>::print);
            this->addOperation("@", read);
            this->addOperation("!", write);

            this->addOperation("DUP", dup);
            this->addOperation("SWAP", swap);

            this->addOperation("+", ll::Arithmetic<int>::add);
            this->addOperation("-", ll::Arithmetic<int>::sub);
            this->addOperation("*", ll::Arithmetic<int>::mul);
            this->addOperation("/", ll::Arithmetic<int>::div);

            this->addOperation("=", ll::Arithmetic<int>::eq);
            this->addOperation("!=", ll::Arithmetic<int>::neq);
            this->addOperation(">", ll::Arithmetic<int>::gt);
            this->addOperation(">=", ll::Arithmetic<int>::get);
            this->addOperation("<", ll::Arithmetic<int>::lt);
            this->addOperation("<=", ll::Arithmetic<int>::let);

            // a tiny stdlib
            this->parse("\
				: ? @ . ; \
				: CR 10 EMIT ; \
				: SPACE 32 EMIT ; \
            ");

            std::cout << "To exit type \"BYE\"." << std::endl;
        }

        ~Forth()
        {
        }

        using Language::parse;
        int parse(std::istream& stream)
        {
            char linebuf[512];
            while(stream.get(linebuf, 512, ';'))
            {
                stream.ignore(1);
                std::string line(linebuf);
                boost::trim(line);
                if(line.empty())
                    continue;

                std::vector<std::string> list;
                boost::split(list, line, boost::is_any_of("\n\t "));

                bool name = false;
                bool var = false;
                bool quote = false;
                int i = 0;
                int llen = list.size();
                addr_t ptrs[llen];
                int p = 0;
                int tmp;

//                this->def_stack.pos = this->def_limit; // erase all temporary allocations

                for(std::string a : list)
                {
                    boost::trim(a);
                    boost::to_upper(a);

                    if(a.empty())
                        continue;

                    if(a == "BYE")
                        return 1;

                    if(a == "'")
                        quote = true;
                    else if(a == ":" && i == 0)
                        name = true;
                    else if(a == "VARIABLE" && i == 0)
                    {
                        name = true;
                        var = true;
                    }
                    else if(name && i == 1)
                    {
                        if(this->symbols.count(a) > 0)
                        {
                            std::cout << "Symbol \"" << a << "\" already defined!" << std::endl;
                            goto abort;
                        }
                        tmp = this->def_stack.pos;
                        this->symbols[a] = addr_t(this->limit + tmp);
                        this->def_stack.move(llen-2 + 3);

                        std::cout << "defined " << a << " as " << int(this->limit + tmp) << std::endl;
                    }
                    else if(var && i == 1)
                    {
                        word_t addr = word_t(this->limit + this->def_stack.pos);
                        this->def_stack << word_t(42); // initialize

                        this->symbols[a] = this->push_num(addr);
                        goto run;
                    }
                    else if((a[0] >= '0' && a[0] <= '9') ||
                            (a[0] == '-' && a[1] >= '0' && a[1] <= '9'))
                    {
                        int n = std::stoi(a);
                        ptrs[p++] = this->push_num(word_t(n));
                    }
                    else
                    {
                        auto it = this->symbols.find(a);
                        if(it != this->symbols.end())
                        {
                            addr_t addr = it->second;

                            if(quote)
                            {
                                ptrs[p++] = this->push_num(word_t(addr));
                                quote = false;
                            }
                            else
                                ptrs[p++] =	addr;
                        }
                        else
                        {
                            std::cout << "undefined symbol \"" << a << "\"!\n";
                            goto abort;
                        }
                    }

                    ++i;
                }

run:
                if(name)
                {
                    this->def_limit = this->def_stack.pos; // save allocations

                    this->def_stack.pos = tmp;
                    this->def_stack << word_t(p+2); // total length
                    this->def_stack << word_t(this->limit+0); // forth_eval
                    this->def_stack << word_t(p); // length for forth_eval
                    for(int i = 0; i < p; ++i)
                        this->def_stack.push(word_t(ptrs[i]));

                    this->def_stack.pos = this->def_limit;
                }
                else if(p > 0)
                {
                    this->stack.push(p, ptrs);
                    this->stack.push(word_t(p));
                    this->forth_eval(this->stack);

                    if(this->stack.pos < 0)
                    {
                        this->stack.pos = 0;
                        std::cout << "stack underflow!" << std::endl;
                        goto abort;
                    }
                    else if(this->stack.pos > this->limit)
                    {
                        this->stack.pos = this->limit;
                        std::cout << "stack overflow!" << std::endl;
                        goto abort;
                    }
                }
            }

            std::cout << "ok" << std::endl;

abort:
            return 0;
        }

        void name(char* buf)
        {
            strcpy(buf, "Forth");
        }

    private:
        memory::Context<page_size, align_t> const& context;
        memory::accessors::Stack<page_size, align_t, addr_t, word_t> stack;
        memory::accessors::Stack<page_size, align_t, addr_t, word_t> def_stack;
        Core<page_size, align_t, addr_t>& core;
        std::map<std::string, addr_t> symbols;
        int def_limit;
        int const limit;

        void addOperation(std::string name, void (*fn)(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack))
        {
            this->addOperation(name, std::function<void (memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)>(fn));
        }

        void addOperation(std::string name, std::function<void (memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)> fn)
        {
            this->core.template addOperation<word_t>(this->limit + this->def_stack.pos, fn);
            this->symbols[name] = this->limit + this->def_stack.pos;

            this->def_stack.move(1);
            this->def_limit = this->def_stack.pos;
        }

        addr_t push_num(word_t n)
        {
            addr_t p = addr_t(this->limit + this->def_stack.pos);
            this->def_stack << word_t(2); // length
            this->def_stack << word_t(this->symbols["NOP"]);
            this->def_stack << word_t(n);
            return p;
        }
};

} // namespace lang

} // namespace repl

