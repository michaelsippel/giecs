
#pragma once

#include <giecs/memory/accessors/stack.h>

namespace giecs
{

#define LLFN(name) \
template <size_t page_size, typename align_t, typename addr_t, typename val_t> \
static void name (memory::accessors::Stack<page_size, align_t, addr_t, val_t>& stack)

} // namespace giecs

