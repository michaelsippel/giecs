
#pragma once

#include <istream>
#include <cstring>
#include <giecs/memory/context.h>

#include "language.h"

using namespace giecs;

namespace repl
{

namespace lang
{

template <int page_size, typename align_t, typename addr_t=align_t, typename word_t=addr_t>
class Brainfuck : public Language
{
        typedef memory::accessors::Stack<page_size, align_t, word_t, addr_t> Stack;

    public:
        Brainfuck(memory::Context<page_size, align_t> const& context_, int limit_)
            : context(context_), limit(limit_),
              mem(this->context.template createLinear<addr_t, word_t>()),
              def_stack(memory::accessors::Stack<page_size, align_t, addr_t, word_t>(context_, (limit_*bitsize<word_t>())/bitsize<align_t>()))
        {
            this->def_limit = 0;
            this->memptr = addr_t();

            for(int i = 0; i < 8; ++i)
                this->ptrs[i] = this->limit + this->def_limit++;

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
                word_t a = this->mem[this->memptr];
                if(a > word_t())
                    this->core.eval(stack);
                else
                    stack.move(-1);
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

            this->def_stack.pos = this->def_limit;
            addr_t addr = this->parse_def(str, str+strlen(str));
            if(addr > this->limit)
            {
                Stack stack = this->context.template createStack<addr_t, word_t>();

                for(int i = 0; i < this->limit; ++i)
                    this->mem[i] = word_t();

                stack.move(this->limit + this->def_stack.pos);
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
        Core<page_size, align_t, addr_t> core;
        memory::accessors::Stack<page_size, align_t, addr_t, word_t> def_stack;
        memory::accessors::Linear<page_size, align_t, addr_t, word_t> mem;
        addr_t ptrs[8];
        int const limit;
        int def_limit;
        addr_t memptr;

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

                            pv[j++] = addr_t(this->limit + this->def_stack.pos);
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
                addr_t addr = addr_t(this->limit + this->def_stack.pos);

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

