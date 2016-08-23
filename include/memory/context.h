
#pragma once

#include <cstddef>

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

namespace giecs
{
namespace memory
{

class Block;

struct BlockKey
{
    int page_id;
    int block_id;
};

size_t hash_value(const BlockKey& block);

class Context
{
    public:
        struct CheckOverlapBlocks
        {
            bool operator() (const BlockKey& key1, const BlockKey& key2) const;
        };

        typedef boost::unordered_multimap<BlockKey, Block*, boost::hash<BlockKey>, CheckOverlapBlocks> BlockMap;

        Context();
        ~Context();

        void addBlock(Block* const block, const BlockKey key) const;
        void addBlock(Block* const block, const std::vector<BlockKey>& keys) const;
        Block* getBlock(const BlockKey key) const;

    private:
        BlockMap* blocks;
};

} // namespace memory

} // namespace giecs

