
#include <cstddef>
#include <cstdlib>
#include <memory/block.h>
#include <memory/context.h>

namespace giecs
{
namespace memory
{

size_t hash_value(BlockKey const& block)
{
    boost::hash<int> hasher;
    return hasher(block.page_id);
}

bool Context::CheckOverlapBlocks::operator() (BlockKey const& blk1, BlockKey const& blk2) const
{
    return (blk1.page_id == blk2.page_id);
}

Context::Context()
{
    this->blocks = new BlockMap();
    this->masters = new MasterMap();
}

Context::~Context()
{
    for(auto const p : *this->blocks)
        delete p.second;

    delete this->blocks;
    delete this->masters;
}

void Context::addBlock(Block* const block, BlockKey const key) const
{
    this->blocks->insert(std::pair<BlockKey, Block*>(key, block));
    this->syncPage(key.page_id, block);
}

void Context::addBlock(Block* const block, std::vector<BlockKey> const& keys) const
{
    for(auto const k : keys)
        this->addBlock(block, k);
}

Block* Context::getBlock(BlockKey const key) const
{
//    auto const p = this->blocks->find(key, BlockMap::hasher(), std::equal_to<BlockKey>());
//	if(p != this->blocks->end())

    // overloading find doesn't work..
    auto const p = this->blocks->equal_range(key);
    for(auto it = p.first; it != p.second; ++it)
    {
        if(key == it->first)
        {
            this->syncPage(key.page_id, it->second);
            return it->second;
        }
    }

    return NULL;
}

void Context::markPageDirty(int const page_id, Block* const block) const
{
    this->masters->insert(std::pair<int, Block*>(page_id, block));
}

void Context::syncPage(int const page_id, Block* const req_block) const
{
    auto const masterp = this->masters->find(page_id);
    if(masterp != this->masters->end())
    {
        Block* const master = masterp->second;
        if(master != req_block)
        {
            this->masters->erase(masterp);

            if(this->blocks->count({page_id}) > 1)
            {
                auto const itp = this->blocks->equal_range({page_id});
                uint8_t* page = (uint8_t*) malloc(this->page_size);

                ContextSync* sync = master->getSync(this);
                sync->read_page(page_id, page);
                delete sync;

                for(auto it = itp.first; it != itp.second; it++)
                {
                    Block* block = it->second;
                    if(block != master)
                    {
                        ContextSync* sync = block->getSync(this);
                        sync->write_page(page_id, page);
                        delete sync;
                    }
                }

                free(page);
            }
        }
    }
}

} // namespace memory

} // namespace giecs

