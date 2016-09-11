
#include <cstddef>
#include <cstdlib>
#include <memory/block.h>

namespace giecs
{
namespace memory
{

ContextSync::ContextSync(Context const* const context_)
    : context(context_)
{
}

Block::Block(size_t const l, std::function<ContextSync* (Context const* const)> const createSync_)
    : length(l), createSync(createSync_)
{
    this->ptr = malloc(this->length);
}

Block::Block(Block const& b)
{
    this->length = b.length;
    this->ptr = b.ptr;
}

Block::~Block()
{
    free(this->ptr);
}

ContextSync* Block::getSync(Context const* const context) const
{
    return this->createSync(context);
}

} // namespace memory

} // namespace giecs

