
#pragma once

namespace brainfuck
{

class SyntaxAccessor : public giecs::memory::Linear<char>
{
    void parse(char* cmd, char* end)
    {
        int m = 0; // loop level
        char* start = nullptr;

        while(cmd < end)
        {
            switch(*cmd)
            {
#define BF_CASE(r, data, elem) case BOOST_PP_TUPLE_ELEM(2, 0, elem) : if(m == 0) this->push( BOOST_PP_TUPLE_ELEM(2, 1, elem) ); break;

                    BOOST_PP_SEQ_FOR_EACH(BF_CASE, 0,
                                          ((',', Opcode::in))
                                          (('.', Opcode::out))
                                          (('+', Opcode::inc))
                                          (('-', Opcode::dec))
                                          (('<', Opcode::left))
                                          (('>', Opcode::right))
                                         );

                case '[':
                    if(m == 0)
                        start = cmd+1;
                    // PUSH jz ...
                    m++;
                    break;

                case ']':
                    if(m == 1)
                    {
                        // n = cmd - start
                        // PUSH ... n
                        // this->parse(start, cmd);
                        // PUSH jmp -n

                        SyntaxAccessor sub;
                        sub->parse(start, cmd);
                        start = nullptr;
                    }
                    m--;
                    break;
            }
            cmd++;
        }
    }
}
}; // class SyntaxAccessor

} // namespace brainfuck

