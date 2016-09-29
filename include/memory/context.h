
#pragma once

#include <cstddef>

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/type_index.hpp>

#include <memory/block.h>

namespace giecs
{
namespace memory
{

struct BlockKey
{
    unsigned int page_id;
    unsigned int block_id;
    boost::typeindex::type_index accessor_type;

    bool operator==(BlockKey const& b) const
    {
        return (this->page_id == b.page_id &&
                this->block_id == b.block_id &&
                this->accessor_type == b.accessor_type);
    }
};

size_t hash_value(const BlockKey& block);

template <size_t page_size, typename align_t>
class Context
{
        /*	namespace accessors
        	{
        		template <>
        		using Linear<>
        	};
        */
    public:
        struct CheckOverlapBlocks
        {
            bool operator() (const BlockKey& key1, const BlockKey& key2) const
            {
                return (key1.page_id == key2.page_id);
            }
        };

        typedef boost::unordered_multimap<BlockKey, Block<page_size, align_t>*, boost::hash<BlockKey>, CheckOverlapBlocks> BlockMap;
        typedef boost::unordered_map<int, Block<page_size, align_t>*> MasterMap;

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

        void addBlock(Block<page_size, align_t>* const block, BlockKey const key) const
        {
            this->blocks->insert(std::pair<BlockKey, Block<page_size, align_t>*>(key, block));
            this->syncPage(key.page_id, block);
        }

        void addBlock(Block<page_size, align_t>* const block, std::vector<BlockKey> const& keys) const
        {
            for(auto const k : keys)
                this->addBlock(block, k);
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
                    this->syncPage(key.page_id, it->second);
                    return it->second;
                }
            }

            return NULL;
        }

        void markPageDirty(unsigned int const page_id, Block<page_size, align_t>* const block) const
        {
            this->masters->insert(std::pair<int, Block<page_size, align_t>*>(page_id, block));
        }

    private:
        BlockMap* blocks;
        MasterMap* masters;

        void syncPage(unsigned int const page_id, Block<page_size, align_t>* const req_block) const
        {
            auto const masterp = this->masters->find(page_id);
            if(masterp != this->masters->end())
            {
                Block<page_size, align_t>* const master = masterp->second;
                if(master != req_block)
                {
                    this->masters->erase(masterp);

                    if(this->blocks->count({page_id}) > 1)
                    {
                        auto const itp = this->blocks->equal_range({page_id});
                        align_t* page = (align_t*) malloc(page_size * sizeof(align_t));

                        ContextSync<page_size, align_t> sync = master->getSync(this);
                        sync.read_page(page_id, page);

                        for(auto it = itp.first; it != itp.second; it++)
                        {
                            Block<page_size, align_t>* block = it->second;
                            if(block != master)
                            {
                                ContextSync<page_size, align_t> sync = block->getSync(this);
                                sync.write_page(page_id, page);
                            }
                        }

                        free(page);
                    }
                }
            }
        }
};

} // namespace memory

} // namespace giecs

