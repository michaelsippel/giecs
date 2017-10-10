#include <boost/test/unit_test.hpp>

#include <iostream>
#include <memory>
#include <stack>
#include <queue>

#include <giecs/core.hpp>
#include <giecs/eval.hpp>

BOOST_AUTO_TEST_SUITE(core);

struct Instruction
{
        enum Opcode
        {
            push,
            addi,
            printi,
            call,
            ret,
        };

        struct Data : public std::stack<int>
        {
            int data;
            giecs::ProgramBase* cs_push = nullptr;
            giecs::ProgramBase* cs_pop = nullptr;
            std::stack<giecs::ProgramBase*> call_stack;
        };

        Opcode fetch(Data& d)
        {
            d.data = this->data;
            d.cs_push = jmp;
            return this->op;
        }

        Opcode op;
        int data;
        giecs::ProgramBase* jmp;
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

inline void f_call(Instruction::Data& data)
{
    data.call_stack.push(data.cs_push);
}

inline void f_ret(Instruction::Data& data)
{
    data.cs_pop = data.call_stack.top();
    data.call_stack.pop();
}

GIECS_CORE_OPERATOR(TestOperator,
                    ((Instruction::Opcode::push, f_push))
                    ((Instruction::Opcode::addi, f_addi))
                    ((Instruction::Opcode::printi, f_printi))
                    ((Instruction::Opcode::call, f_call))
                    ((Instruction::Opcode::ret, f_ret))
                   );

using TestCore = giecs::Core<Instruction, TestOperator>;

BOOST_AUTO_TEST_CASE(core)
{
    Instruction::Data data;
    std::queue<Instruction> prg = std::queue<Instruction>();

    prg.push({Instruction::Opcode::push,2});
    prg.push({Instruction::Opcode::push,3});
    prg.push({Instruction::Opcode::addi});
    prg.push({Instruction::Opcode::printi});

    TestCore::eval(prg, data);
}

BOOST_AUTO_TEST_CASE(eval)
{
    Instruction::Data g_data;
    struct Seq : public giecs::Program<TestCore, Seq>, public std::queue<Instruction>
    {
        std::queue<Instruction>& program(void)
        {
            return *this;
        }

        Instruction::Data* m_data;
        Instruction::Data& data(void)
        {
            return *m_data;
        }

        ProgramBase* m_next = nullptr;
        ProgramBase* next(void)
        {
            if(m_data->cs_pop)
            {
                m_next = m_data->cs_pop;
                m_data->cs_pop = nullptr;
            }
            return m_next;
        }
    };

    Seq ret;
    ret.m_data = &g_data;
    ret.push({Instruction::Opcode::ret});

    Seq add10;
    add10.m_data = &g_data;
    add10.m_next = &ret;
    add10.push({Instruction::Opcode::push,10});
    add10.push({Instruction::Opcode::addi});

    Seq prg1,prg2;
    prg1.m_data = &g_data;
    prg2.m_data = &g_data;
    prg1.m_next = &add10;
    prg1.push({Instruction::Opcode::push,2});
    prg1.push({Instruction::Opcode::call, 0, &prg2});
    prg2.push({Instruction::Opcode::printi});

    giecs::eval(&prg1);
}

BOOST_AUTO_TEST_SUITE_END();

