
#pragma once

#include <map>
#include <stack>
#include <boost/circular_buffer.hpp>
#include <giecs/ll/arithmetic.hpp>
#include <giecs/ll/io.hpp>

#include <giecs/core.hpp>
#include <giecs/eval.hpp>

namespace forth
{

/**
 * Implements the complete Virtual-Forth-Machine
 */
template <typename MemWord, typename MemContainer, typename DataStackContainer, typename ReturnStackContainer>
class VM : public std::stack<MemWord, DataStackContainer>
{
    public:
        MemWord pc; /// Instruction pointer
        MemContainer mem;
        std::stack<MemWord, ReturnStackContainer> return_stack; /// return stack
        std::map<MemWord, giecs::ProgramBase*> programs; /// external programs

        struct Instruction
        {
            enum Opcode
            {
                compose, branch, exit, // flow
                push, drop, dup, over, swap, pushr, popr, // stack
                load, store, // memory
                noti, andi, ori, xori, // bitwise logic
                addi, subi, muli, divi, // integer arithmetic
                gti, lti, eq, // integer relation
                printi, emit, // out
            };

            using Data = VM<MemWord, MemContainer, DataStackContainer, ReturnStackContainer>;

            Opcode op;
            MemWord pc;

            Opcode fetch(Data& data)
            {
                data.pc = this->pc;
                return op;
            }
        }; // struct Instruction

        giecs::ProgramBase* get_program(MemWord addr)
        {
            auto it = this->programs.find(addr);
            if(it == this->programs.end())
                return nullptr;
            else
                return it->second;
        }

        MemWord pop()
        {
            MemWord a = this->top();
            this->std::stack<MemWord, DataStackContainer>::pop();
            return a;
        }

        /**
         * Executes Opcodes
         */
#define FN(def) ([](VM<MemWord, MemContainer, DataStackContainer, ReturnStackContainer>& state){ def ;})
        GIECS_CORE_OPERATOR(Operator,
                            ((Instruction::Opcode::compose, FN(state.return_stack.push(state.pc); state.pc = state.mem[state.pc];)))
                            ((Instruction::Opcode::branch, FN(MemWord off = state.mem[++state.pc]-1; if(state.pop() == 0) state.pc += off;)))
                            ((Instruction::Opcode::exit, FN(state.pc = state.return_stack.top(); state.return_stack.pop();)))

                            ((Instruction::Opcode::load, FN(MemWord addr=state.pop(); state.push(state.mem[addr]);)))
                            ((Instruction::Opcode::store, FN(MemWord addr=state.pop(); state.mem[addr] = state.pop();)))
                            ((Instruction::Opcode::push, FN(state.push(state.mem[++state.pc]))))
                            ((Instruction::Opcode::drop, FN(state.pop())))
                            ((Instruction::Opcode::dup, FN(state.push(state.top()))))
                            ((Instruction::Opcode::over, FN(MemWord a = state.pop(); MemWord b = state.top(); state.push(a); state.push(b);)))
                            ((Instruction::Opcode::swap, FN(MemWord a = state.pop(); MemWord b = state.pop(); state.push(a); state.push(b);)))
                            ((Instruction::Opcode::pushr, FN(state.return_stack.push(state.pop()))))
                            ((Instruction::Opcode::popr, FN(state.push(state.return_stack.top()); state.return_stack.pop();)))

                            ((Instruction::Opcode::noti, giecs::ll::Bitwise<int>::op_not))
                            ((Instruction::Opcode::andi, giecs::ll::Bitwise<int>::op_and))
                            ((Instruction::Opcode::ori, giecs::ll::Bitwise<int>::op_or))
                            ((Instruction::Opcode::xori, giecs::ll::Bitwise<int>::op_xor))
                            ((Instruction::Opcode::addi, giecs::ll::Arithmetic<int>::add))
                            ((Instruction::Opcode::subi, giecs::ll::Arithmetic<int>::sub))
                            ((Instruction::Opcode::muli, giecs::ll::Arithmetic<int>::mul))
                            ((Instruction::Opcode::divi, giecs::ll::Arithmetic<int>::div))
                            ((Instruction::Opcode::gti, giecs::ll::Relation<int>::gt))
                            ((Instruction::Opcode::lti, giecs::ll::Relation<int>::lt))
                            ((Instruction::Opcode::eq, giecs::ll::Relation<int>::eq))

                            ((Instruction::Opcode::emit, giecs::ll::ConsoleIO<char>::print))
                            ((Instruction::Opcode::printi, giecs::ll::ConsoleIO<int>::print))
                           ); // Operator

        /**
         * Evaluated by giecs::eval
         */
        class Program : public giecs::Program<giecs::Core<Instruction, Operator>, Program>
        {
            private:
                class InstructionDecoder : public boost::circular_buffer<Instruction>
                {
                    private:
                        VM& state;

                    public:
                        InstructionDecoder(VM& s)
                            : boost::circular_buffer<Instruction>(16), state(s)
                        {
                        }

                        Instruction& front(void)
                        {
                            if(this->boost::circular_buffer<Instruction>::empty())
                            {
                                MemWord pc = state.pc;
                                for(Instruction inst; inst.op != Instruction::Opcode::compose && inst.op != Instruction::Opcode::exit && inst.op != Instruction::Opcode::branch && !this->full();)
                                {
                                    ++pc;
                                    inst.op = (typename Instruction::Opcode) this->state.mem[this->state.mem[pc]];
                                    inst.pc = pc;
                                    // We know which instructions take more
                                    if(inst.op == Instruction::Opcode::push)
                                        ++pc;
                                    this->push_back(inst);
                                }
                            }
                            return this->boost::circular_buffer<Instruction>::front();
                        }

                        bool empty(void) const
                        {
                            return (this->state.programs.count(this->state.pc) > 0) || this->state.return_stack.empty();
                        }
                }; // class InstructionDecoder

                VM& state;
                std::queue<Instruction, InstructionDecoder> queue; // could be multiple objects for multithreaded operation

            public:
                Program(VM& vm_, giecs::ProgramBase* ret=nullptr)
                    : state(vm_), queue(InstructionDecoder(vm_))
                {
                }

                std::queue<Instruction, InstructionDecoder>& program(void)
                {
                    return this->queue;
                }

                VM& data(void)
                {
                    return this->state;
                }

                giecs::ProgramBase* next(void)
                {
                    return this->state.get_program(this->state.pc);
                }
        }; // class Word
}; // class VM

} // namespace forth

