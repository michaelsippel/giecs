
#pragma once

#include <giecs/ll.hpp>

namespace giecs
{

namespace ll
{

#define OP(name, op)                            \
  LLFN( name )                                  \
  {                                             \
    val_t a = stack.top();                      \
    stack.pop();                                \
    val_t b = stack.top();                      \
    stack.pop();                                \
    cast_t r = cast_t(a) op cast_t(b);          \
    stack.push(val_t(r));                       \
  }


#define SINGLE_OP(name, op)                     \
  LLFN( name )                                  \
  {                                             \
    val_t a = stack.top();                      \
    stack.pop();                                \
    cast_t r = cast_t(a);                       \
    stack.push(val_t(op r));                    \
  }


template <typename cast_t>
struct Arithmetic
{
    OP(add, +)
    OP(sub, -)
    OP(mul, *)
    OP(div, /)

    SINGLE_OP(inc, ++)
    SINGLE_OP(dec, --)
}; // struct Arithmetic

  template <typename cast_t=bool>
struct Logic
{
    SINGLE_OP(op_not, not);
    OP(op_and, and);
    OP(op_or, or);
}; // struct Logic

template <typename cast_t>
struct Bitwise
{
    SINGLE_OP(op_not, compl);
    OP(op_and, bitand);
    OP(op_or, bitor);
    OP(op_xor, xor);
    OP(leftshift, <<);
    OP(rightshift, >>);
}; // struct Bitwise

#undef OP
#undef SINGLE_OP

template <typename cast_t>
struct Relation
{
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
}; // struct Relation

} // namespace ll

} // namespace giecs

