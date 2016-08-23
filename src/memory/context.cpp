
#include <cstddef>
#include <memory/block.h>
#include <memory/context.h>

namespace giecs
{
namespace memory
{

size_t hash_value(const BlockKey& block)
{
    boost::hash<int> hasher;
    return hasher(block.page_id);
}

bool Context::CheckOverlapBlocks::operator() (const BlockKey& blk1, const BlockKey& blk2) const
{
    return (blk1.page_id == blk2.page_id);
}

Context::Context()
{
    this->blocks = new BlockMap();
}

Context::~Context()
{
    for(const auto p : *this->blocks)
        delete p.second;

    delete this->blocks;
}

void Context::addBlock(Block* const block, const BlockKey key) const
{
    this->blocks->insert(std::pair<BlockKey, Block*>(key, block));
}

void Context::addBlock(Block* const block, const std::vector<BlockKey>& keys) const
{
    for(const auto k : keys)
        this->addBlock(block, k);
}

Block* Context::getBlock(const BlockKey key) const
{
    auto p = this->blocks->find(key);
    if(p != this->blocks->end())
        return p->second;
    else
        return NULL;
}

} // namespace memory

} // namespace giecs

