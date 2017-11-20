
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
template <typename MemWord, typename DataStackContainer, typename ReturnStackContainer, typename MemContainer>
class VM
{
    public:
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

                Opcode op;
                MemWord pc;

                /**
                 * Represents the VM-State with Memory & Registers
                 */
                struct Data : public std::stack<MemWord, DataStackContainer>, public MemContainer
                {
                    MemWord pc;
                    std::stack<MemWord, ReturnStackContainer> return_stack;
                    std::map<MemWord, giecs::ProgramBase*> programs;
                };

                Opcode fetch(Data& data)
                {
                    data.pc = this->pc;
                    return op;
                }
        }; // struct Instruction

        using State = typename Instruction::Data;
        State state;

        giecs::ProgramBase* get_program(MemWord addr)
        {
            auto it = this->state.programs.find(addr);
            if(it == this->state.programs.end())
                return nullptr;
            else
                return it->second;
        }

        template <typename T, typename Container>
        static inline T pop(std::stack<T, Container>& stack)
        {
            T a = stack.top();
            stack.pop();
            return a;
        }

        /**
         * Executes Opcodes
         */
#define FN(def) ([](State& data){ def ;})
        GIECS_CORE_OPERATOR(Operator,
                            ((Instruction::Opcode::compose, FN(data.return_stack.push(data.pc); data.pc = data[data.pc];)))
                            ((Instruction::Opcode::branch, FN(if(data.top() == 0) data.pc += data[++data.pc];)))
                            ((Instruction::Opcode::exit, FN(data.pc = pop(data.return_stack);)))

                            ((Instruction::Opcode::load, FN(MemWord addr=pop(data); data.push(data[addr]);)))
                            ((Instruction::Opcode::store, FN(MemWord addr=pop(data); data[addr] = pop(data);)))
                            ((Instruction::Opcode::push, FN(data.push(data[++data.pc]);)))
                            ((Instruction::Opcode::drop, FN(data.pop())))
                            ((Instruction::Opcode::dup, FN(data.push(data.top()))))
                            ((Instruction::Opcode::over, FN(MemWord a = pop(data); MemWord b = data.top(); data.push(a); data.push(b);)))
                            ((Instruction::Opcode::swap, FN(MemWord a = pop(data); MemWord b = pop(data); data.push(a); data.push(b);)))
                            ((Instruction::Opcode::pushr, FN(data.return_stack.push(pop(data)))))
                            ((Instruction::Opcode::popr, FN(data.push(pop(data.return_stack)))))

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
                        State& state;

                    public:
                        InstructionDecoder(State& s)
                            : boost::circular_buffer<Instruction>(16), state(s)
                        {
                        }

                        Instruction& front(void)
                        {
                            if(this->boost::circular_buffer<Instruction>::empty())
                            {
                                //std::cout << "BEGIN DECODE" << std::endl;
                                MemWord pc = state.pc;
                                for(Instruction inst; inst.op != Instruction::Opcode::compose && inst.op != Instruction::Opcode::exit && !this->full();)
                                {
                                    ++pc;
                                    inst.op = (typename Instruction::Opcode) this->state[this->state[pc]];
                                    inst.pc = pc;
                                    // We know which instructions take more
                                    if(inst.op == Instruction::Opcode::push)
                                        ++pc;
                                    this->push_back(inst);
                                    //std::cout << "Instruction " << inst.op << std::endl;
                                }
                                //  std::cout << "END DECODE" << std::endl;
                            }
                            return this->boost::circular_buffer<Instruction>::front();
                        }

                        bool empty(void) const
                        {
                            return (this->state.programs.count(this->state.pc) > 0) || this->state.return_stack.empty();
                        }
                }; // class InstructionDecoder

                VM& vm;
                std::queue<Instruction, InstructionDecoder> queue; // could be multiple objects for multithreaded operation

            public:
                Program(VM& vm_, giecs::ProgramBase* ret=nullptr)
                    : vm(vm_), queue(InstructionDecoder(vm_.state))
                {
                }

                std::queue<Instruction, InstructionDecoder>& program(void)
                {
                    return this->queue;
                }

                State& data(void)
                {
                    return this->vm.state;
                }

                giecs::ProgramBase* next(void)
                {
                    return this->vm.get_program(this->vm.state.pc);
                }
        }; // class Word
}; // class VM

} // namespace forth

