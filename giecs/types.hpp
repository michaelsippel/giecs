
#pragma once

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

#include <giecs/bits.hpp>

namespace giecs
{

#define OPERATOR(op, T) \
template <typename T2> \
Bits<N> operator op (T2 const v) const \
{ \
    return Bits<N>(T(this->getValue()) op T(v)); \
} \
Bits<N> operator op (Bits<N> const v) const \
{ \
    return Bits<N>(T(this->getValue()) op T(v.getValue())); \
}

#define OPERATOR_EQ(op, T) \
template <typename T2> \
Bits<N>& operator op (T2 const v) \
{ \
    this->value op v; \
    return *this; \
} \
Bits<N>& operator op (Bits<N> const v) \
{ \
    this->value op T(v.getValue()); \
    return *this; \
}

#define DEF_OPERATOR(r, data, elem) OPERATOR(elem, data)
#define DEF_OPERATOR_EQ(r, data, elem) OPERATOR_EQ(elem, data)

#define TYPE_BITS(name, T, op_seq, eq_op_seq) \
template <int N> \
class name : public Bits<N> \
{ \
    public: \
        using Bits<N>::Bits; \
        BOOST_PP_SEQ_FOR_EACH(DEF_OPERATOR, T, op_seq) \
        BOOST_PP_SEQ_FOR_EACH(DEF_OPERATOR_EQ, T, eq_op_seq)

#define NUMBER_TYPE_BITS(name, T) \
TYPE_BITS(name, T, \
        (+)(-)(*)(/)(>)(>=)(<)(<=), \
        (+=)(-=)(*=)(/=))

NUMBER_TYPE_BITS(Int, int)
Bits<N>& operator ++ ()
{
    ++this->value;
    return *this;
}
Bits<N>& operator -- ()
{
    --this->value;
    return *this;
}
};

NUMBER_TYPE_BITS(Float, float)
};

} // namespace giecs

