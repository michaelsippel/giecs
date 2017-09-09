
#pragma once

#include <cstddef>

namespace giecs
{

namespace memory
{

struct Reference
{
    std::size_t page_id;
    std::size_t offset;
};

} // namespace memory

} // namespace giecs

