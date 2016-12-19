
#pragma once

#include <cstddef>

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/type_index.hpp>

namespace giecs
{
namespace memory
{

class Block;

struct BlockKey
{
    int page_id;
    int block_id;
    boost::typeindex::type_index accessor_type;

    bool operator==(BlockKey const& b) const
    {
        return (this->page_id == b.page_id &&
                this->block_id == b.block_id &&
                this->accessor_type == b.accessor_type);
    }
};

size_t hash_value(const BlockKey& block);

class Context
{
    public:
        struct CheckOverlapBlocks
        {
            bool operator() (const BlockKey& key1, const BlockKey& key2) const;
        };

        static size_t const page_size = 0x1000; // in size of uint8_t

        typedef boost::unordered_multimap<BlockKey, Block*, boost::hash<BlockKey>, CheckOverlapBlocks> BlockMap;
        typedef boost::unordered_map<int, Block*> MasterMap;

        Context();
        ~Context();

        void addBlock(Block* const block, BlockKey const key) const;
        void addBlock(Block* const block, std::vector<BlockKey> const& keys) const;
        Block* getBlock(BlockKey const key) const;

        void markPageDirty(int const page_id, Block* const block) const;

    private:
        BlockMap* blocks;
        MasterMap* masters;

        void syncPage(int const page_id, Block* const req_block) const;
};

} // namespace memory

} // namespace giecs

