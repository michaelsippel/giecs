
#pragma once

#include <istream>
#include <cstddef>
#include <cstring>
#include <giecs/core.hpp>
#include <giecs/memory/context.hpp>

#include "language.hpp"

using namespace giecs;

namespace repl
{

namespace lang
{

template <std::size_t page_size, typename align_t, typename addr_t=align_t, typename word_t=addr_t>
class Brainfuck : public Language
{
        typedef memory::accessors::Stack<page_size, align_t, word_t, addr_t> Stack;

    public:
        Brainfuck(memory::Context<page_size, align_t> const& context_, addr_t limit_)
            : context(context_), limit(limit_), def_limit(limit_),
              mem(this->context.template createLinear<addr_t, word_t>()),
              def_stack(memory::accessors::Stack<page_size, align_t, addr_t, word_t>(this->context,
        {
            0,0
        }, this->def_limit))
        {
            for(int i = 0; i < 8; ++i, ++this->def_limit)
                this->ptrs[i] = this->def_limit;

            this->core.addOperation(this->ptrs[0],
                                    std::function<void (Stack&)>([this](Stack& stack)
            {
                // could be (map eval ())
                int len = stack.template pop<word_t>();
                for(int i = 0; i < len; ++i)
                    this->core.eval(stack);
            }));
            this->core.addOperation(this->ptrs[1],
                                    std::function<void (Stack&)>([this](Stack& stack)
            {
                char a;
                std::cin >> a;
                this->mem[this->memptr] = word_t(a);
            }));
            this->core.addOperation(this->ptrs[2],
                                    std::function<void (Stack&)>([this](Stack& stack)
            {
                word_t a = this->mem[this->memptr];
                std::cout << char(a);
            }));
            this->core.addOperation(this->ptrs[3],
                                    std::function<void (Stack&)>([this](Stack& stack)
            {
                ++this->memptr;
            }));
            this->core.addOperation(this->ptrs[4],
                                    std::function<void (Stack&)>([this](Stack& stack)
            {
                --this->memptr;
            }));
            this->core.addOperation(this->ptrs[5],
                                    std::function<void (Stack&)>([this](Stack& stack)
            {
                word_t a = this->mem[this->memptr];
                ++a;
                this->mem[this->memptr] = a;
            }));
            this->core.addOperation(this->ptrs[6],
                                    std::function<void (Stack&)>([this](Stack& stack)
            {
                word_t a = this->mem[this->memptr];
                --a;
                this->mem[this->memptr] = a;
            }));
            this->core.addOperation(this->ptrs[7],
                                    std::function<void (Stack&)>([this](Stack& stack)
            {
                word_t ptr = stack.pop();

                word_t a = this->mem[this->memptr];
                while(a > word_t())
                {
                    stack.push(ptr);
                    this->core.eval(stack);
                    a = this->mem[this->memptr];
                }
            }));
        }

        ~Brainfuck()
        {
        }

        int parse(std::istream& stream)
        {
            char str[512];
            stream.get(str, 512);

            if(strcmp(str, "exit") == 0)
                return 1;

            this->def_stack.pos = this->def_limit; // erase
            addr_t addr = this->parse_def(str, str+strlen(str));
            if(addr > this->limit)
            {
                this->mem.operate(addr_t(), this->limit, [](word_t& v)
                {
                    v=word_t();
                }, true);

                Stack stack = memory::accessors::Stack<page_size, align_t, addr_t, word_t>(this->def_stack);
                stack << word_t(addr);
                this->core.eval(stack);

                std::cout << std::endl;
            }

            return 0;
        }

        void name(char* buf)
        {
            strcpy(buf, "Brainfuck");
        }

    private:
        memory::Context<page_size, align_t> const& context;
        addr_t const limit;
        addr_t def_limit;
        memory::accessors::Linear<page_size, align_t, addr_t, word_t> mem;
        memory::accessors::Stack<page_size, align_t, addr_t, word_t> def_stack;
        Core<page_size, align_t, addr_t> core;
        addr_t memptr;
        addr_t ptrs[8];

        addr_t parse_def(char* prg, char* end)
        {
            size_t n = (size_t)end - (size_t)prg;
            addr_t pv[n];
            int m = 0;
            int j = 0;
            char* start = NULL;

            while(prg < end)
            {
                switch(*prg)
                {
                    case ',':
                        if(m == 0)
                            pv[j++] = word_t(this->ptrs[1]);
                        break;
                    case '.':
                        if(m == 0)
                            pv[j++] = word_t(this->ptrs[2]);
                        break;
                    case '>':
                        if(m == 0)
                            pv[j++] = word_t(this->ptrs[3]);
                        break;
                    case '<':
                        if(m == 0)
                            pv[j++] = word_t(this->ptrs[4]);
                        break;
                    case '+':
                        if(m == 0)
                            pv[j++] = word_t(this->ptrs[5]);
                        break;
                    case '-':
                        if(m == 0)
                            pv[j++] = word_t(this->ptrs[6]);
                        break;
                    case '[':
                        if(m == 0)
                            start = prg+1;
                        m++;
                        break;
                    case ']':
                        if(m == 1)
                        {
                            addr_t addr = this->parse_def(start, prg);

                            pv[j++] = this->def_stack.pos;
                            this->def_stack << word_t(2);
                            this->def_stack << word_t(this->ptrs[7]);
                            this->def_stack << word_t(addr);

                            start = NULL;
                        }
                        m--;
                        break;
                }
                prg++;
            }

            if(j > 0)
            {
                addr_t addr = this->def_stack.pos;

                def_stack << word_t(j+2);
                def_stack << word_t(ptrs[0]);
                def_stack << word_t(j);
                for(int i = 0; i < j; ++i)
                    def_stack << word_t(pv[i]);

                return addr;
            }
            else return addr_t();
        }
};

} // namespace lang

} // namespace repl

