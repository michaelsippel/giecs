
#pragma once

#include <functional>
#include <boost/type_index.hpp>
#include <boost/shared_ptr.hpp>

#include <giecs/memory/accessor.h>
#include <giecs/memory/reference.hpp>
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
        static size_t const block_size = 512; // 32bit/val -> 2 MiB.  // TODO: more dynamic
        static int const nalign = (bitsize<val_t>() > bitsize<align_t>()) ?
                                  (1+int(bitsize<val_t>()-1)/int(bitsize<align_t>())) :  // positive: number of align-blocks per value
                                  (-1-int(bitsize<align_t>()-1)/int(bitsize<val_t>())); // negative: number of values per align-block

        // align per value
        static inline int alignOffset(int const nval)
        {
            return (nalign > 0) ? (nval * nalign) : (nval / (-nalign));
        }
        // value per align
        static inline int valueOffset(int const na)
        {
            return (nalign > 0) ? (na / nalign) : (na * (-nalign));
        }
        // pages per value
        static inline int pageOffset(int const nval)
        {
            return alignOffset(nval) / page_size;
        }
        // blocks per value
        static inline int blockOffset(int const nval)
        {
            return nval / block_size;
        }
        // align offset in value
        static inline int alignShift(Reference const ref)
        {
            // TODO: fix possible integer overflow
            return (nalign < 1) ? 0 : ((ref.page_id * page_size + ref.offset) % nalign);
        }
        inline unsigned int alignShift() const
        {
            return alignShift(this->reference);
        }

        inline index_t valueIndex(addr_t const addr) const
        {
            return index_t((valueOffset(this->reference.page_id*page_size+this->reference.offset) + (unsigned int)addr) % block_size);
        }
        inline unsigned int pageIndex(addr_t const addr) const
        {
            return this->reference.page_id + pageOffset((unsigned int) addr);
        }
        inline unsigned int blockIndex(addr_t const addr) const
        {
            return blockOffset(valueOffset(this->reference.page_id*page_size+this->reference.offset) + (unsigned int)addr);
        }

        using typename ContextSync<page_size, align_t>::BlockPtr;
        using typename ContextSync<page_size, align_t>::BlockRef;
        typedef boost::shared_ptr<TypeBlock<page_size, align_t, val_t> const> TypeBlockPtr;

    public:
#define TYPEID boost::typeindex::type_id< Linear<page_size, align_t, addr_t, val_t, buf_t, index_t> >()
#define ID_F(flags) {TYPEID, size_t( flags )}
#define ID(ref) ID_F( this->alignShift(ref) )


        Linear(Context<page_size, align_t> const& context_)
            : Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::Accessor(context_, ID(0))
        {}

        Linear(Context<page_size, align_t> const& context_, Reference ref_)
            : Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::Accessor(context_, ID(ref_)), reference(ref_)
        {
            std::cout << "create linear at " << reference.page_id << " " << reference.offset << "nalign:" << nalign << ": blockid " << blockIndex(addr_t()) << " , index " << valueIndex(addr_t()) << std::endl;
        }

        template <typename addr2_t, typename val2_t, typename buf2_t, typename index2_t>
        Linear(Linear<page_size, align_t, addr2_t, val2_t, buf2_t, index2_t> const& l)
            : Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::Accessor(l.context, ID(l.reference)), reference(l.reference)
        {}
        /*
              template <typename addr2_t, typename val2_t, typename buf2_t, typename index2_t>
              Linear(Linear<page_size, align_t, addr2_t, val2_t, buf2_t, index2_t> const& l, int off)
              : Accessor<>((l.offset+off), offset(l.offset+off))
              {
              }
        */
#undef TYPEID
#undef ID

        size_t operate(addr_t const oaddr, index_t const len, std::function<void (val_t&)> const operation, bool dirty) const
        {
            size_t l;
            addr_t addr = oaddr;
            if(operation)
            {
                addr_t const end = addr + len;
                unsigned int block_id, page_id;
                unsigned int const last_block = this->blockIndex(end);
                do
                {
                    page_id = this->pageIndex(addr);
                    block_id = this->blockIndex(addr);
                    TypeBlockPtr block = this->getBlock(page_id, block_id, dirty);

                    index_t i = valueIndex(addr);
                    for(; (i != block_size) && (addr < end); ++i, ++l, ++addr)
                    {
                        val_t& v = (*block)[i];
                        operation(v);
                    }

                }
                while(block_id != last_block);
            }

            return l;
        }

        using Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::read;
        using Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::write;

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

        void read_page_block(BlockRef const b, std::array<align_t, page_size>& buf) const final
        {
            unsigned int const page_id = b.first.page_id;
            unsigned int const block_id = b.first.block_id;
            BlockPtr const block = b.second;

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
            BlockPtr const block = b.second;

            // same here
            int bitoff = block_id * block_size * bitsize<val_t>() - (page_id * page_size - this->offset) * bitsize<align_t>();
            int off = bitoff / bitsize<align_t>();
            bitoff %= bitsize<align_t>();

            block->write(0, block_size, buf, off, bitoff, range);
        }

    private:
        // offset in size of align_t
        int offset; // TODO: makes copy/move cheap? only page_id's and sync offsets have to be recalculated
        Reference reference;

        TypeBlockPtr getBlock(unsigned int page_id, unsigned int block_id, bool dirty) const
        {
            int bitoff = block_id * block_size * bitsize<val_t>() + (this->offset - page_id * page_size) * bitsize<align_t>();

            std::pair<int, int> range(bitoff, bitoff+block_size*bitsize<val_t>());
            BlockKey const key = {page_id, block_id, this->accessor_id, range};
            TypeBlockPtr block = boost::static_pointer_cast<TypeBlock<page_size, align_t, val_t> const>(this->context.getBlock(key));
            if(block == NULL)
            {
                Reference const ref = this->reference;
                auto const createSync = [ref](Context<page_size, align_t> const& c)
                {
                    return new Linear<page_size, align_t, addr_t, val_t, buf_t, index_t>(c, ref);
                };
                block = TypeBlockPtr(new TypeBlock<page_size, align_t, val_t>(block_size, createSync));
                this->context.addBlock(block, key);
            }
            if(dirty)
                this->context.markPageDirty(key, block);

            return block;
        }
};

} // namespace accessors

} // namespace memory

} // namespace giecs

