
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
                TapeIterator tape;
                ProgramIterator pc;
                int arg1;
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

#define DATA_FN(def) ([](typename Instruction::Data& d){ def ; })
#define TAPE_FN(def) DATA_FN( auto tape = d.tape; def )
        GIECS_CORE_OPERATOR(Operator,
                            ((Opcode::in, TAPE_FN(std::cin >> *tape)))
                            ((Opcode::out, TAPE_FN(std::cout << *tape)))
                            ((Opcode::inc, TAPE_FN(++(*tape))))
                            ((Opcode::dec, TAPE_FN(--(*tape))))
                            ((Opcode::next, TAPE_FN(++tape)))
                            ((Opcode::prev, TAPE_FN(--tape)))
                            ((Opcode::jmp, DATA_FN(_jmp(d))))
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

} // namespace brainfuck

