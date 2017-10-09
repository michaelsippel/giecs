#include <boost/test/unit_test.hpp>

#include <iostream>
#include <memory>
#include <stack>
#include <queue>

#include <giecs/core.hpp>

BOOST_AUTO_TEST_SUITE(core);

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

enum Opcode
{
    push,
    addi,
    printi,
};

GIECS_CORE_OPERATOR(TestOperator,
                    ((Opcode::addi, f_addi))
                    ((Opcode::printi, f_printi))
                   );
using TestCore = giecs::Core<Opcode, std::stack<int>&, TestOperator>;

BOOST_AUTO_TEST_CASE(eval)
{
    TestCore core;

    std::stack<int> data = std::stack<int>();
    std::queue<Opcode> code = std::queue<Opcode>();

    data.push(2);
    data.push(3);
    code.push(addi);
    code.push(printi);

    core.eval(code, data);
}

BOOST_AUTO_TEST_SUITE_END();

