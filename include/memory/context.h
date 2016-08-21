
#pragma once

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

namespace giecs
{
namespace memory
{

class Block;

class Context
{
    public:
        struct overlapBlocks
        {
            bool operator() (const int& key1, const int& key2) const
            {
                return (key1 == key2);
            }
        };

        typedef boost::unordered_multimap<int, Block*, boost::hash<int>, overlapBlocks> BlockMap;

        Context()
        {
            this->blocks = new BlockMap();
        }

        Block* getBlock(int key)
        {
            auto p = this->blocks->find(key);
            if(p != this->blocks->end())
                return p->second;
            else
                return NULL;
        }

        void addBlock(int key, Block* block)
        {
            this->blocks->insert(std::pair<int, Block*>(key, block));
        }

        ~Context()
        {
            for(const auto& p : *this->blocks)
                delete &p;

            delete this->blocks;
        }

    private:
        BlockMap* blocks;
};

} // namespace memory

} // namespace giecs

