#include <boost/test/unit_test.hpp>

#include <iostream>
#include <memory>
#include <stack>
#include <queue>

#include <giecs/core.hpp>

BOOST_AUTO_TEST_SUITE(core);

struct Instruction
{
        enum Opcode
        {
            push,
            jmp,
            addi,
            printi,
        };

        struct Data : public std::stack<int>
        {
            int data;
        };

        Opcode fetch(Data& d)
        {
            d.data = this->data;
            return this->op;
        }

        Opcode op;
        int data;
};

inline void f_addi(std::stack<int>& stack)
{
    int a = stack.top();
    stack.pop();
    int b = stack.top();
    stack.pop();
    int c = a + b;
    stack.push(c);
}

inline void f_printi(std::stack<int>& stack)
{
    int a = stack.top();
    stack.pop();
    std::cout << int(a) << std::endl;
}

inline void f_push(Instruction::Data& stack)
{
    stack.push(stack.data);
}

GIECS_CORE_OPERATOR(TestOperator,
                    ((Instruction::Opcode::push, f_push))
                    ((Instruction::Opcode::addi, f_addi))
                    ((Instruction::Opcode::printi, f_printi))
                   );

using TestCore = giecs::Core<Instruction, TestOperator>;

BOOST_AUTO_TEST_CASE(eval)
{
    TestCore core;

    Instruction::Data data;
    std::queue<Instruction> code = std::queue<Instruction>();

    code.push({Instruction::Opcode::push,2});
    code.push({Instruction::Opcode::push,3});
    code.push({Instruction::Opcode::addi});
    code.push({Instruction::Opcode::printi});

    core.eval(code, data);
}

BOOST_AUTO_TEST_SUITE_END();

