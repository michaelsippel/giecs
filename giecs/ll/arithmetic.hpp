
#pragma once

#include <giecs/ll.hpp>

namespace giecs
{

namespace ll
{

template <typename cast_t>
struct Arithmetic
{

#define OP(name, op) \
    LLFN( name ) \
    { \
        val_t a = stack.top(); \
        stack.pop(); \
        val_t b = stack.top(); \
        stack.pop(); \
        cast_t r = cast_t(a) op cast_t(b); \
        stack.push(val_t(r)); \
    }

    OP(add, +)
    OP(sub, -)
    OP(mul, *)
    OP(div, /)
#undef OP

#define SINGLE_OP(name, op) \
    LLFN( name ) \
    { \
        val_t a = stack.top(); \
        stack.pop(); \
        cast_t r = cast_t(a); \
        op r; \
        stack.push(val_t(r)); \
    }

    SINGLE_OP(inc, ++)
    SINGLE_OP(dec, --)

#define COMP(name, op) \
    LLFN( name ) \
    { \
        val_t a = stack.top(); \
        stack.pop(); \
        val_t b = stack.top(); \
        stack.pop(); \
        bool r = cast_t(a) op cast_t(b); \
        stack.push(val_t(r)); \
    }

    COMP(eq, ==)
    COMP(neq, !=)
    COMP(gt, >)
    COMP(lt, <)
    COMP(get, >=)
    COMP(let, <=)
#undef COMP

};

} // namespace ll

} // namespace giecs

