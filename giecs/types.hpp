
#pragma once

#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

#include <giecs/bits.hpp>

namespace giecs
{

#define OPERATOR(name, T, op) \
template <typename T2> \
name<N> operator op (T2 const v) const \
{ \
    return name<N>(T(this->getValue()) op T(v)); \
} \
name<N> operator op (name<N> const v) const \
{ \
    return name<N>(T(this->getValue()) op T(v.getValue())); \
}

#define OPERATOR_EQ(name, T, op) \
template <typename T2> \
name<N>& operator op (T2 const v) \
{ \
    this->value op v; \
    return *this; \
} \
name<N>& operator op (name<N> const v) \
{ \
    this->value op T(v.getValue()); \
    return *this; \
}

#define DEF_OPERATOR(r, data, elem) OPERATOR(BOOST_PP_SEQ_ELEM(0,data), BOOST_PP_SEQ_ELEM(1,data), elem)
#define DEF_OPERATOR_EQ(r, data, elem) OPERATOR_EQ(BOOST_PP_SEQ_ELEM(0,data), BOOST_PP_SEQ_ELEM(1,data), elem)

#define TYPE_BITS(name, T, op_seq, eq_op_seq) \
template <int N> \
class name : public Bits<N> \
{ \
    public: \
        using Bits<N>::Bits; \
        BOOST_PP_SEQ_FOR_EACH(DEF_OPERATOR, (name)(T), op_seq)  \
        BOOST_PP_SEQ_FOR_EACH(DEF_OPERATOR_EQ, (name)(T), eq_op_seq)

#define NUMBER_TYPE_BITS(name, T) \
TYPE_BITS(name, T, \
        (+)(-)(*)(/)(>)(>=)(<)(<=), \
        (+=)(-=)(*=)(/=))

NUMBER_TYPE_BITS(Int, int)
Int<N>& operator ++ ()
{
    ++this->value;
    return *this;
}
Int<N>& operator -- ()
{
    --this->value;
    return *this;
}
};

NUMBER_TYPE_BITS(Float, float)
};

} // namespace giecs

