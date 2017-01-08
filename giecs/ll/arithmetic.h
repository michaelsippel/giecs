
#pragma once

#include <giecs/ll.h>

namespace giecs
{

namespace ll
{

template <typename cast_t>
struct Arithmetic
{

#define A_OP(name, op) \
	LLFN( name ) \
    { \
        val_t a = stack.pop(); \
		val_t b = stack.pop(); \
		cast_t r = cast_t(a) op cast_t(b); \
		stack.push(val_t(r)); \
    }

    A_OP(add, +)
    A_OP(sub, -)
    A_OP(mul, *)
    A_OP(div, /)
};

} // namespace ll

} // namespace giecs

