
#pragma once

#include <stack>
#include <giecs/ll/arithmetic.hpp>
#include <giecs/ll/io.hpp>

#include <giecs/core.hpp>

namespace forth
{

enum WordType
{
    Lit,
    SubWord,
};

enum Primitive
{
    noop,
    // flow
    branch,
    exit,
    // meta
    compile,
    execute,
    create,
    // stack
    push,
    drop,
    dup,
    // memory
    load,
    store,
    // logic
    andb,
    orb,
    xorb,
    // integer
    addi,
    subi,
    muli,
    divi,
    printi,
};

template <typename word_t, typename Container>
struct VM
{
        struct Instruction
        {
                using Opcode = enum Primitive;
                Opcode op;
                word_t arg[2];

                struct Data : public std::stack<word_t, Container>
                {
                    word_t r[2];
                    /*   Container& container(void)
                    {
                      return this->c;
                      }*/
                };

                Primitive fetch(Data& data)
                {
                    data.r[0] = this->arg[0];
                    data.r[1] = this->arg[1];
                    return op;
                }
        }; // struct Instruction

#define FN(def) ([](typename Instruction::Data& data){ def ;})
        GIECS_CORE_OPERATOR(Operator,
                            ((Primitive::noop, FN()))
                            //((Primitive::branch, FN(jmp(data.r0))))
                            //((Primitive::load, FN(data.push(data.mem[data.r0]))))
                            //((Primitive::store, FN(data.mem[])))

                            ((Primitive::push, FN(data.push(data.r[0]))))
                            ((Primitive::drop, FN(data.pop())))
                            ((Primitive::dup, FN(data.push(data.top()))))
                            ((Primitive::addi, giecs::ll::Arithmetic<int>::add))
                            ((Primitive::subi, giecs::ll::Arithmetic<int>::sub))
                            ((Primitive::muli, giecs::ll::Arithmetic<int>::mul))
                            ((Primitive::divi, giecs::ll::Arithmetic<int>::div))

                            ((Primitive::printi, giecs::ll::ConsoleIO<int>::print))
                           );
}; // struct VM

} // namespace forth

