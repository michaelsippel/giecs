
#include <cstddef>
#include <cstdlib>
#include <memory/block.h>

namespace giecs
{
namespace memory
{

Block::Block(size_t l)
{
    this->length = l;
    this->ptr = malloc(this->length);
}

Block::Block(const Block& b)
{
    this->length = b.length;
    this->ptr = b.ptr;
}

Block::~Block()
{
    free(this->ptr);
}

} // namespace memory

} // namespace giecs

