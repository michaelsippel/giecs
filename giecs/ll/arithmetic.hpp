
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
        val_t a = stack.pop(); \
        val_t b = stack.pop(); \
        cast_t r = cast_t(a) op cast_t(b); \
        stack.push(val_t(r)); \
    }

    OP(add, +)
    OP(sub, -)
    OP(mul, *)
    OP(div, /)
#undef OP

#define COMP(name, op) \
    LLFN( name ) \
    { \
        val_t a = stack.pop(); \
        val_t b = stack.pop(); \
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

