
#pragma once

#include <array>
#include <memory>
#include <cassert>
#include <cstddef>

#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#include <giecs/memory/accessor.hpp>
#include <giecs/memory/accessors/queue.hpp>

namespace giecs
{

#define GIECS_CORE_OP_CASE(r, data, elem) case BOOST_PP_TUPLE_ELEM(2, 0, elem) : BOOST_PP_TUPLE_ELEM(2, 1, elem) (data); break;
#define GIECS_CORE_OPERATOR(name, OpcodeList) \
struct name \
{ \
    template <typename Opcode, typename Data> \
    static inline void operate (Opcode opcode, Data data) \
    { \
        switch(opcode) \
        { \
            BOOST_PP_SEQ_FOR_EACH(GIECS_CORE_OP_CASE, data, OpcodeList); \
        } \
    } \
};

struct CoreBase
{
    // virtual void eval(Code) {}
}; // struct CoreBase

template <typename Instruction, typename Operator>
struct Core : public CoreBase
{
    template <typename Container>
    void eval(std::queue<Instruction, Container>& code, typename Instruction::Data& data)
    {
        while(! code.empty())
        {
            typename Instruction::Opcode opcode = code.front().fetch(data);
            Operator::template operate<typename Instruction::Opcode, typename Instruction::Data&>(opcode, data);
            code.pop();
        }
    }
}; // struct Core
/*
template <std::size_t page_size, typename align_t>
void eval(memory::accessors::Code<page_size, align_t> code)
{
    Core* core = code->createCore();
    while(! code->empty())
    {
        CodeBlock c = code->top();
        core->eval();

        if(code->jmp())
        {
          code = code->jmp_code();
          core = code->createCore();
        }
        else
          code->pop();
    }
}
*/
} // namespace giecs

