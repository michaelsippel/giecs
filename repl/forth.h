
#pragma once

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
        Forth(memory::Context<page_size, align_t> const& context_)
            : context(context_)
        {
            std::function<void (memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)> ev = [this](memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack)
            {
                return this->forth_eval(stack);
            };
            this->core.addOperation(addr_t(0), ev);
            this->addOperation("nop", nop);
            this->addOperation(".", ll::ConsoleIO<int>::print);
//			this->addOperation("@", read);
//			this->addOperation("!", write);
            this->addOperation("dup", dup);
            this->addOperation("swap", swap);
            this->addOperation("+", ll::Arithmetic<int>::add);
            this->addOperation("-", ll::Arithmetic<int>::sub);
            this->addOperation("*", ll::Arithmetic<int>::mul);
            this->addOperation("/", ll::Arithmetic<int>::div);

            this->addOperation("if", ll::cond);
            this->addOperation("=", ll::Arithmetic<int>::eq);
            this->addOperation("!=", ll::Arithmetic<int>::neq);
            this->addOperation(">", ll::Arithmetic<int>::gt);
            this->addOperation(">=", ll::Arithmetic<int>::get);
            this->addOperation("<", ll::Arithmetic<int>::lt);
            this->addOperation("<=", ll::Arithmetic<int>::let);
        }

        ~Forth()
        {
        }

        Language* parse(char* str)
        {
            if(strcmp(str, "exit") == 0)
            {
                delete this;
                return NULL;
            }

            memory::accessors::Stack<page_size, align_t, addr_t, word_t> stack = this->context.template createStack<addr_t, word_t>();

            std::vector<std::string> lines;
            boost::split(lines, str, boost::is_any_of(";"));

            for(std::string line : lines)
            {
                boost::trim(line);
                if(line.empty())
                    continue;

                std::vector<std::string> list;
                boost::split(list, line, boost::is_any_of("\n\t "));

                bool name = false;
                int i = 0;
                int llen = list.size();
                word_t ptrs[llen];
                int p = 0;

                for(std::string a : list)
                {
                    boost::trim(a);
                    if(a.empty())
                        continue;

                    if(a == ":" && i == 0)
                    {
                        name = true;
                    }
                    else if(name && i == 1)
                    {
                        this->symbols[a] = ++this->fnid;

                        stack[this->fnid] = word_t(llen); // total length
                        stack[++this->fnid] = word_t(0); // forth_eval
                        stack[++this->fnid] = word_t(llen-2); // length for forth_eval

                        stack.pos = this->fnid+1;
                        this->fnid += llen-2;
                    }
                    else if((a[0] >= '0' && a[0] <= '9') ||
                            (a[0] == '-' && a[1] >= '0' && a[1] <= '9'))
                    {
                        int n = std::stoi(a);
                        ptrs[p++] = word_t(++this->fnid);
                        stack[this->fnid] = word_t(2); // length
                        stack[++this->fnid] = word_t(this->symbols["nop"]);
                        stack[++this->fnid] = word_t(n);
                    }
                    else
                    {
                        auto it = this->symbols.find(a);
                        if(it != this->symbols.end())
                        {
                            addr_t addr = it->second;
                            ptrs[p++] = word_t(addr);
                        }
                        else
                            std::cout << "Undefined symbol!\n";
                    }

                    ++i;
                }

                if(name)
                {
                    stack.push(p, ptrs);
                }
                else
                {
                    stack.pos = this->fnid+1;

                    stack.push(p, ptrs);
                    stack.push(word_t(llen));
                    this->forth_eval(stack);
                    std::cout << "\n";
                }
            }

            return this;
        }

        void name(char* buf)
        {
            strcpy(buf, "Forth");
        }

    private:
        memory::Context<page_size, align_t> const& context;
        Core<page_size, align_t, addr_t> core;
        std::map<std::string, addr_t> symbols;
        addr_t fnid;

        void addOperation(std::string name, void (*fn)(memory::accessors::Stack<page_size, align_t, addr_t, word_t>& stack))
        {
            this->core.template addOperation<word_t>(++this->fnid, fn);
            this->symbols[name] = this->fnid;
        }
};

} // namespace lang

} // namespace repl

