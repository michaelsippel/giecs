
#pragma once

#include <giecs/ll.hpp>

namespace giecs
{

namespace ll
{
LLFN(cond)
{
    val_t c = stack.pop();
    val_t a = stack.pop();
    if(bool(c))
    {
        stack.pop();
        stack.push(a);
    }
}

} // namespace ll

} // namespace giecs

