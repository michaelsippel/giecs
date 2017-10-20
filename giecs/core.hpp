
#pragma once

#include <queue>

#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

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

template <typename Instruction, typename Operator>
struct Core
{
    using InstructionType = Instruction;

    template <typename Container>
    static inline void eval(std::queue<Instruction, Container>& code, typename Instruction::Data& data)
    {
        while(! code.empty())
        {
            typename Instruction::Opcode opcode = code.front().fetch(data);
            Operator::template operate<typename Instruction::Opcode, typename Instruction::Data&>(opcode, data);
            code.pop();
        }
    }
}; // struct Core

} // namespace giecs

