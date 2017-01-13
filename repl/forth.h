
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

    public:
        Forth(memory::Context<page_size, align_t> const& context_)
            : context(context_)
        {
            this->addOperation(".", ll::ConsoleIO<int>::print);
//			this->addOperation("@", read);
//			this->addOperation("!", write);
            this->addOperation("dup", dup);
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

            std::vector<std::string> list;
            boost::split(list, str, boost::is_any_of("\n\t "));

            bool name = false;

            for(std::string a : list)
            {
                if(name)
                {
                    // TODO
                }
                else if(a == ":")
                {
                    name = true;
                }
                else if((a[0] >= '0' && a[0] <= '9') ||
                        (a[0] == '-' && a[1] >= '0' && a[1] <= '9'))
                {
                    int n = std::stoi(a);
                    stack.push(word_t(n));
                }
                else
                {
                    auto it = this->symbols.find(a);
                    if(it != this->symbols.end())
                    {
                        addr_t addr = it->second;
                        stack.push(word_t(addr));
                        this->core.eval(stack);
                    }
                    else
                        std::cout << "Undefined symbol!\n";
                }
            }

            std::cout << "\n";

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
            this->core.template addOperation<word_t>(++fnid, fn);
            this->symbols[name] = fnid;
        }
};

} // namespace lang

} // namespace repl

