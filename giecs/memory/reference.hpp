
#pragma once

#include <vector>

#include <giecs/memory/context.h>

namespace giecs
{

namespace memory
{

template <size_t page_size, typename align_t>
class Reference
{
    public:
        Reference(Context<page_size, align_t> const& context_)
            : context(context_)
        {
        }

  unsigned int page_id;
  unsigned int align_offset;

  Context<page_size, align_t> const& context;
};

} // namespace memory

} // namespace giecs

