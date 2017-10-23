
#pragma once

#include <queue>
#include <istream>
#include <sstream>

namespace brainfuck
{

template <typename Container>
class SyntaxAccessor : public std::queue<uint8_t, Container>
{
    public:
        void parse(std::istream& stream)
        {
            int level = 0;
            int n_brackets = 0;
            std::stringbuf buf;

            char cmd;
            while(stream.get(cmd))
            {
                if(cmd == ']')
                {
                    ++n_brackets;
                    --level;
                }
                if(level == 0)
                {
                    switch(cmd)
                    {
#define BF_CASE(r, data, elem) case BOOST_PP_TUPLE_ELEM(2, 0, elem) : this->push(BOOST_PP_TUPLE_ELEM(2, 1, elem)); break;
                            BOOST_PP_SEQ_FOR_EACH(BF_CASE, 0,
                                                  ((',', Opcode::in))
                                                  (('.', Opcode::out))
                                                  (('+', Opcode::inc))
                                                  (('-', Opcode::dec))
                                                  (('>', Opcode::next))
                                                  (('<', Opcode::prev))
                                                  (('[', Opcode::jz))
                                                 );

                        case ']':
                        {
                            int jmpoff = buf.str().length() + 2*n_brackets;
                            n_brackets = 0;
                            this->push(jmpoff);

                            buf.sputc(0);
                            std::istream substream(&buf);
                            this->parse(substream);

                            this->push(Opcode::jmp);
                            this->push(-jmpoff-2);

                            buf = std::stringbuf();
                        }
                        break;
                    }
                }
                else
                {
                    switch(cmd)
                    {
                        case '.':
                        case ',':
                        case '+':
                        case '-':
                        case '>':
                        case '<':
                        case '[':
                        case ']':
                            buf.sputc(cmd);
                            break;
                    }
                }
                if(cmd == '[') ++level;
            }
        }
}; // class SyntaxAccessor

} // namespace brainfuck

