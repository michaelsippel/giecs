
#pragma once

#include <functional>
#include <boost/type_index.hpp>

#include <giecs/memory/accessor.h>
#include <giecs/bits.h>

namespace giecs
{
namespace memory
{
namespace accessors
{

template <size_t page_size, typename align_t, typename addr_t, typename val_t, typename buf_t=val_t*, typename index_t=size_t>
class Linear : public Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>
{
        template <size_t, typename, typename, typename, typename, typename> friend class Linear;

    private:
        // one block is one page
        static size_t const block_size = (page_size * bitsize<align_t>()) / bitsize<val_t>();

    public:
#define TYPEID boost::typeindex::type_id< Linear<page_size, align_t, addr_t, val_t, buf_t, index_t> >()
#define ID(flags) {TYPEID, size_t( flags )}

        Linear(Context<page_size, align_t> const& context_)
            : Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::Accessor(context_, ID(0))
        {
            this->offset = index_t();
        }

        Linear(Context<page_size, align_t> const& context_, index_t offset_)
            : Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::Accessor(context_, ID(offset_)), offset(offset_)
        {}

        template <typename addr2_t, typename val2_t, typename buf2_t, typename index2_t>
        Linear(Linear<page_size, align_t, addr2_t, val2_t, buf2_t, index2_t> const& l)
            : Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::Accessor(l.context, ID(l.offset)), offset(l.offset)
        {}

        template <typename addr2_t, typename val2_t, typename buf2_t, typename index2_t>
        Linear(Linear<page_size, align_t, addr2_t, val2_t, buf2_t, index2_t> const& l, int off)
            : Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::Accessor(l.context, ID(l.offset+off)), offset(l.offset+off)
        {
        }

#undef TYPEID
#undef ID

        int getOffset(void)
        {
            return offset;
        }

        index_t operate(addr_t const addr, index_t const len, std::function<void (val_t&)> const operation, bool dirty) const
        {
            index_t l;

            if(operation)
            {
                unsigned int page_id = (this->offset + (uint(addr) * bitsize<val_t>()) / bitsize<align_t>()) / page_size;
                unsigned int block_id = uint(addr) / block_size;
                unsigned int last_block_id = (uint(addr)+len) / block_size;

                index_t i = index_t(uint(addr) % block_size);
                index_t last_i = index_t(uint(addr+len) % block_size);

                if(last_i == index_t())
                {
                    last_i = block_size;
                    --last_block_id;
                }

                for(; block_id <= last_block_id; ++block_id, ++page_id) // that's not correct is 1 block != 1 page
                {
                    TypeBlock<page_size, align_t, val_t>* block = this->getBlock(page_id, block_id, dirty);

                    index_t end = (block_id == last_block_id) ? last_i : index_t(block_size);
                    for(; i < end; ++i, ++l)
                        operation((*block)[i]);

                    i = 0;
                }
            }

            return l;
        }

        index_t read(addr_t const addr, index_t const len, buf_t buf) const final
        {
            std::function<void (val_t&)> const operation = [&buf](val_t& v)
            {
                *buf = v;
                ++buf;
            };
            return this->operate(addr, len, operation, false);
        }

        index_t write(addr_t const addr, index_t const len, buf_t buf) const final
        {
            std::function<void (val_t&)> const operation = [&buf](val_t& v)
            {
                v = *buf;
                ++buf;
            };
            return this->operate(addr, len, operation, true);
        }

        using typename ContextSync<page_size, align_t>::BlockRef;

        void read_page_block(BlockRef const b, std::array<align_t, page_size>& buf) const final
        {
            unsigned int const page_id = b.first.page_id;
            unsigned int const block_id = b.first.block_id;
            Block<page_size, align_t> const* const block = b.second;

            // hm.. counting bits is stupid
            int bitoff = block_id * block_size * bitsize<val_t>() - (page_id * page_size - this->offset) * bitsize<align_t>();
            int off = bitoff / bitsize<align_t>();
            bitoff %= bitsize<align_t>();

            block->read(0, block_size, buf, off, bitoff);
        }

        void write_page_block(BlockRef const b, std::array<align_t, page_size> const& buf, std::pair<int,int> range) const final
        {
            unsigned int const page_id = b.first.page_id;
            unsigned int const block_id = b.first.block_id;
            Block<page_size, align_t> const* const block = b.second;

            // same here
            int bitoff = block_id * block_size * bitsize<val_t>() - (page_id * page_size - this->offset) * bitsize<align_t>();
            int off = bitoff / bitsize<align_t>();
            bitoff %= bitsize<align_t>();

            block->write(0, block_size, buf, off, bitoff, range);
        }

    private:
        // offset in size of align_t
        int offset; // TODO: makes copy/move cheap? only page_id's and sync offsets have to be recalculated

        TypeBlock<page_size, align_t, val_t>* getBlock(unsigned int page_id, unsigned int block_id, bool dirty) const
        {
            int bitoff = block_id * block_size * bitsize<val_t>() + (this->offset -page_id * page_size) * bitsize<align_t>();

            std::pair<int, int> range(bitoff, bitoff+block_size*bitsize<align_t>());
            BlockKey const key = {page_id, block_id, this->accessor_id, range};
            TypeBlock<page_size, align_t, val_t>* block = (TypeBlock<page_size, align_t, val_t>*)this->context.getBlock(key);
            if(block == NULL)
            {
                index_t const off = this->offset;
                auto const createSync = [off](Context<page_size, align_t> const& c)
                {
                    return new Linear<page_size, align_t, addr_t, val_t, buf_t, index_t>(c, off);
                };
                block = new TypeBlock<page_size, align_t, val_t>(block_size, createSync);
                this->context.addBlock(block, key);
            }
            if(dirty)
                this->context.markPageDirty(key, block);

            return block;
        }
};

}; // namespace accessors

}; // namespace memory

}; // namespace giecs

