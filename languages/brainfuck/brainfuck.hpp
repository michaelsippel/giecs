
#pragma once

#include <istream>
#include <cstddef>
#include <cstring>
#include <boost/circular_buffer.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#include <giecs/core.hpp>
#include <giecs/eval.hpp>

namespace brainfuck
{
enum Opcode
{
    in,
    out,
    inc,
    dec,
    next,
    prev,
    jmp,
    jz,
};

template <typename TapeIterator, typename ProgramIterator>
struct VM
{
        struct Instruction
        {
            using Opcode = ::brainfuck::Opcode;
            struct Data
            {
                Data(TapeIterator const& tape_, ProgramIterator const& pc_)
                    :tape(tape_), pc(pc_)
                {}

                // Registers
                int arg1;
                ProgramIterator pc;
                TapeIterator tape;
            };

            Opcode op;
            int arg1;

            Opcode fetch(Data& data)
            {
                data.arg1 = this->arg1;
                return this->op;
            }
        }; // struct Instruction

        static inline void _jmp(typename Instruction::Data& d)
        {
            for(int i = 0; i < d.arg1; ++i, ++d.pc);
            for(int i = d.arg1; i < 0; ++i, --d.pc);
        }

#define TAPE_FN(def) ([](typename Instruction::Data& d){ auto tape = d.tape; def ; })
        GIECS_CORE_OPERATOR(Operator,
                            ((Opcode::in, TAPE_FN(std::cin >> *tape)))
                            ((Opcode::out, TAPE_FN(std::cout << *tape)))
                            ((Opcode::inc, TAPE_FN(++(*tape))))
                            ((Opcode::dec, TAPE_FN(--(*tape))))
                            ((Opcode::next, TAPE_FN(++tape)))
                            ((Opcode::prev, TAPE_FN(--tape)))
                            ((Opcode::jmp, TAPE_FN(_jmp(d))))
                            ((Opcode::jz, TAPE_FN(if(*tape == 0) _jmp(d))))
                           );

        class CodeAccessor : public boost::circular_buffer<Instruction>
        {
            private:
                ProgramIterator& pc;
                ProgramIterator& end;

            public:
                CodeAccessor(ProgramIterator& begin_, ProgramIterator& end_)
                    : boost::circular_buffer<Instruction>(16), pc(begin_), end(end_)
                {}

                Instruction& front(void)
                {
                    if(this->boost::circular_buffer<Instruction>::empty())
                    {
                        // prepare instructions
                        Instruction inst;
                        do
                        {
                            inst.op = (Opcode)*pc;
                            ++pc;
                            // extended instructions
                            if(inst.op == Opcode::jmp || inst.op == Opcode::jz)
                            {
                                inst.arg1 = static_cast<int8_t>(*pc);
                                ++pc;
                            }

                            this->push_back(inst);
                        }
                        while(this->pc != this->end && inst.op != Opcode::jz && inst.op != Opcode::jmp && !this->full());
                    }
                    return this->boost::circular_buffer<Instruction>::front();
                }

                bool empty(void) const
                {
                    return (this->pc == this->end) && this->boost::circular_buffer<Instruction>::empty();
                }
        }; // class CodeAccessor
}; // struct VM
/*
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
}; */

} // namespace brainfuck

