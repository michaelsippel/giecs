
#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

#include <memory/block.h>

namespace giecs
{
namespace memory
{

namespace accessors
{
template <size_t page_size, typename align_t, typename addr_t, typename val_t, typename buf_t, typename index_t>
class Linear;
template <size_t page_size, typename align_t, typename addr_t, typename val_t>
class Stack;
};

template <size_t page_size, typename align_t>
class Context
{
    public:
        struct CheckOverlapBlocks
        {
            bool operator() (const BlockKey& key1, const BlockKey& key2) const
            {
                return (key1.page_id == key2.page_id);
            }
        };

        typedef boost::unordered_multimap<BlockKey, Block<page_size, align_t>*, boost::hash<BlockKey>, CheckOverlapBlocks> BlockMap;
        typedef boost::unordered_map<BlockKey, Block<page_size, align_t>*, boost::hash<BlockKey>, CheckOverlapBlocks > MasterMap;

        Context()
        {
            this->blocks = new BlockMap();
            this->masters = new MasterMap();
        }

        ~Context()
        {
            for(auto const p : *this->blocks)
                delete p.second;

            delete this->blocks;
            delete this->masters;
        }

        template <typename addr_t, typename val_t, typename buf_t=val_t*, typename index_t=size_t>
        inline accessors::Linear<page_size, align_t, addr_t, val_t, buf_t, index_t> createLinear() const
        {
            return accessors::Linear<page_size, align_t, addr_t, val_t, buf_t, index_t>(this);
        }

        template <typename addr_t, typename val_t>
        inline accessors::Stack<page_size, align_t, addr_t, val_t> createStack() const
        {
            return accessors::Stack<page_size, align_t, addr_t, val_t>(this);
        }

        void addBlock(Block<page_size, align_t>* const block, BlockKey const key) const
        {
            this->blocks->insert(std::make_pair(key, block));
            this->syncPage(key.page_id, key);
        }

        void addBlock(Block<page_size, align_t>* const block, std::vector<BlockKey> const& keys) const
        {
            for(auto const k : keys)
                this->addBlock(block, k);
        }

        std::vector< std::pair< BlockKey, Block<page_size, align_t>* > > getPageRange(BlockKey const key) const
        {
            std::vector< std::pair<BlockKey, Block<page_size, align_t>* > > vec;

            BOOST_FOREACH(auto b, this->blocks->equal_range(key))
            {
                if(key.accessor_id == b.first.accessor_id)
                {
                    this->syncPage(key.page_id, b.first);
                    vec.push_back(b);
                }
            }

            return vec;
        }

        Block<page_size, align_t>* getBlock(BlockKey const key) const
        {
//    auto const p = this->blocks->find(key, BlockMap::hasher(), std::equal_to<BlockKey>());
//	if(p != this->blocks->end())

            // overloading find doesn't work..
            auto const p = this->blocks->equal_range(key);
            for(auto it = p.first; it != p.second; ++it)
            {
                if(key == it->first)
                {
                    this->syncPage(key.page_id, it->first);
                    return it->second;
                }
            }

            return NULL;
        }

        void markPageDirty(BlockKey const key)
        {
            Block<page_size, align_t>* const block = this->getBlock(key);
            this->markPageDirty(key, block);
        }

        void markPageDirty(BlockKey const key, Block<page_size, align_t>* const block) const
        {
            this->masters->insert(std::make_pair(key, block));
        }

    private:
        BlockMap* blocks;
        MasterMap* masters;

        void syncPage(unsigned int const page_id, BlockKey const req_block) const
        {
            auto const masterp = this->masters->find({page_id});
            if(masterp != this->masters->end())
            {
                BlockKey const masterkey = masterp->first;
                Block<page_size, align_t>* const master = masterp->second;

                if(! (masterkey.accessor_id == req_block.accessor_id))
                {
                    this->masters->erase(masterp);

                    if(this->blocks->count({page_id}) > 1)
                    {
                        auto const itp = this->blocks->equal_range({page_id});
                        align_t page[page_size];

                        ContextSync<page_size, align_t>* sync = master->getSync(this);
                        sync->read_page(page_id, page);

                        for(auto it = itp.first; it != itp.second; it++)
                        {
                            Block<page_size, align_t>* block = it->second;
                            if(block != master)
                            {
                                ContextSync<page_size, align_t>* sync = block->getSync(this);
                                sync->write_page_block(*it, page);
                                delete sync;
                            }
                        }

                        delete sync;
                    }
                }
            }
        }
};

} // namespace memory

} // namespace giecs

