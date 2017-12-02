
#pragma once

#include <istream>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <boost/circular_buffer.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#include <giecs/core.hpp>
#include <giecs/eval.hpp>

namespace brainfuck
{

/**
 * Opcodes for the Virtual-Brainfuck-Machine
 */
enum Opcode
{
    in, ///< Get character from input and write it to current cell.
    out, ///< Write current cell as character to output.
    inc, ///< Increment the current cell.
    dec, ///< Decrement the current cell.
    next, ///< Move cell-pointer right.
    prev, ///< Move cell-pointer left.
    jmp, ///< Jump unconditionally
    jz, ///< Jump, if current cell is zero.
};

/**
 * Stores the state of the virtual-machine and provides all operations.
 */
template <typename TapeIterator, typename ProgramIterator>
class VM
{
    public:
        TapeIterator tape; ///< Pointer to current memory-cell
        ProgramIterator pc; ///< Program counter
        typename std::iterator_traits<ProgramIterator>::difference_type jmp_off; ///< Register (used by jmp and jz)

        /**
         * Instruction to be executed, gets decoded by CodeAccessor from program-stream
         */
        struct Instruction
        {
            using Opcode = ::brainfuck::Opcode;
            using Data = VM<TapeIterator, ProgramIterator>;

            Opcode op;
            typename std::iterator_traits<ProgramIterator>::difference_type jmp_off;

            /**
             * Get the opcode for this instruction and update the vm-state (load register).
             */
            Opcode fetch(Data& data)
            {
                data.jmp_off = this->jmp_off;
                return this->op;
            }
        }; // struct Instruction


        /**
         * Implements all brainfuck-operations
         */
#define DATA_FN(def) ([](typename Instruction::Data& d){ def ; })
#define TAPE_FN(def) DATA_FN( auto& tape = d.tape; def )
        GIECS_CORE_OPERATOR(Operator,
                            ((Opcode::in, TAPE_FN(std::cin >> *tape)))
                            ((Opcode::out, TAPE_FN(std::cout << (char)*tape)))
                            ((Opcode::inc, TAPE_FN(++(*tape))))
                            ((Opcode::dec, TAPE_FN(--(*tape))))
                            ((Opcode::next, TAPE_FN(++tape)))
                            ((Opcode::prev, TAPE_FN(--tape)))
                            ((Opcode::jmp, DATA_FN(std::advance(d.pc, d.jmp_off))))
                            ((Opcode::jz, TAPE_FN(if(*tape == 0) std::advance(d.pc, d.jmp_off))))
                           );

        /**
         * Decodes the program-stream to instructions and buffers them. Emulates a queue.
         */
        class CodeAccessor : public boost::circular_buffer<Instruction>
        {
            private:
                ProgramIterator& pc;
                ProgramIterator const& end;

            public:
                CodeAccessor(ProgramIterator& begin_, ProgramIterator const& end_)
                    : boost::circular_buffer<Instruction>(16), pc(begin_), end(end_)
                {}

                /// @return the next instruction
                Instruction& front(void)
                {
                    // check if we need to fill the buffer
                    if(this->boost::circular_buffer<Instruction>::empty())
                    {
                        Instruction inst;
                        do
                        {
                            // Get Opcode
                            inst.op = (Opcode)*pc++;

                            // Jump-Instructions are two cells long, because they are followed with their jump offset.
                            if(inst.op == Opcode::jmp || inst.op == Opcode::jz)
                                inst.jmp_off = static_cast<int8_t>(*pc++);

                            // push instruction to buffer
                            this->push_back(inst);
                        }
                        while(this->pc != this->end && inst.op != Opcode::jz && inst.op != Opcode::jmp && !this->full());
                    }

                    // buffer is not empty. return buffered instructions
                    return this->boost::circular_buffer<Instruction>::front();
                }

                /// @return if end-of-program is reached
                bool empty(void) const
                {
                    // If everything is decoded and the buffer is empty, we're done.
                    return (this->pc == this->end) && this->boost::circular_buffer<Instruction>::empty();
                }
        }; // class CodeAccessor
}; // struct VM

} // namespace brainfuck

