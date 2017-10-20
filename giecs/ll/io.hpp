
#pragma once

#include <iostream>
#include <giecs/ll.hpp>

namespace giecs
{

namespace ll
{

template <typename cast_t>
struct ConsoleIO
{
    LLFN(print)
    {
        val_t a = stack.top();
        stack.pop();
        std::cout << cast_t(a);
    }

    LLFN(get)
    {
        cast_t a;
        std::cin >> a;
        stack.push(val_t(a));
    }
};

} // namespace ll

} // namespace giecs

