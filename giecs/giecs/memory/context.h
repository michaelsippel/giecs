
#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

#include <giecs/memory/block.h>

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
            return accessors::Linear<page_size, align_t, addr_t, val_t, buf_t, index_t>(*this);
        }

        template <typename addr_t, typename val_t>
        inline accessors::Stack<page_size, align_t, addr_t, val_t> createStack() const
        {
            return accessors::Stack<page_size, align_t, addr_t, val_t>(*this);
        }

        void addBlock(Block<page_size, align_t>* const block, BlockKey const key) const
        {
            this->syncPage(key);
            auto const val = std::make_pair(key, block);
            this->blocks->insert(val);
            this->writePageFirst(val); // initialize, even if no master block is avaiable
        }

        void addBlock(Block<page_size, align_t>* const block, std::vector<BlockKey> const& keys) const
        {
            for(auto const k : keys)
                this->addBlock(block, k);
        }

        std::vector< std::pair< BlockKey const, Block<page_size, align_t>* const> > getPageRange(BlockKey const key) const
        {
            std::vector< std::pair<BlockKey const, Block<page_size, align_t>* const> > vec;

            BOOST_FOREACH(auto b, this->blocks->equal_range(key))
            {
                if(key.accessor_id == b.first.accessor_id)
                {
//                    this->syncPage(b.first);
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
            BOOST_FOREACH(auto b, this->blocks->equal_range(key))
            {
                if(key == b.first)
                {
                    this->syncPage(b.first);
                    return b.second;
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

        // TODO: optimize
        void writePageFirst(std::pair< BlockKey const, Block<page_size, align_t>* const > const ref) const
        {
            unsigned int const page_id = ref.first.page_id;

            if(this->blocks->count({page_id}) > 1)
            {
                std::array<align_t, page_size> page;

                // take all different accessors.. dumb
                BOOST_FOREACH(auto it, this->blocks->equal_range({page_id}))
                {
                    if(!(it.first.accessor_id == ref.first.accessor_id))
                    {
                        ContextSync<page_size, align_t>* const msync = it.second->getSync(*this);
                        msync->read_page(page_id, page);
                        delete msync;
                    }
                }

                ContextSync<page_size, align_t>* const sync = ref.second->getSync(*this);
                sync->write_page(page_id, page, std::make_pair(0, page_size*bitsize<align_t>()));
                delete sync;
            }
        }

        void syncPage(BlockKey const req_block) const
        {
            unsigned int const page_id = req_block.page_id;
            auto const masterp = this->masters->find({page_id});
            if(masterp != this->masters->end())
            {
                this->masters->erase(masterp);
                BlockKey const masterkey = masterp->first;
                if(! (masterkey.accessor_id == req_block.accessor_id))
                {

                    unsigned int const page_id = masterkey.page_id;
                    if(this->blocks->count({page_id}) > 1)
                    {
                        ContextSync<page_size, align_t>* const sync = masterp->second->getSync(*this);

                        auto const itp = this->blocks->equal_range({page_id});
                        std::array<align_t, page_size> page;
                        sync->read_page(page_id, page);

						BOOST_FOREACH(auto it, itp)
                        {
                            if(! (it.first.accessor_id == masterp->first.accessor_id))
                            {
                                ContextSync<page_size, align_t>* const sync = it.second->getSync(*this);
                                sync->write_page_block(it, page, masterkey.page_range);
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

