
#pragma once

#include <functional>
#include <boost/type_index.hpp>
#include <boost/shared_ptr.hpp>

#include <giecs/memory/accessor.hpp>
#include <giecs/memory/reference.hpp>
#include <giecs/bits.hpp>

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

    protected:
        static size_t const block_size = 512; // 32bit/val -> 2 MiB.  // TODO: more dynamic
        static int const nalign = (bitsize<val_t>() > bitsize<align_t>()) ?
                                  (1+int(bitsize<val_t>()-1)/int(bitsize<align_t>())) :  // positive: number of align-blocks per value
                                  (-int(bitsize<align_t>())/int(bitsize<val_t>())); // negative: number of values per align-block

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
        inline int alignShift() const
        {
            return alignShift(this->reference);
        }

        inline index_t valueIndex(addr_t const addr) const
        {
            return index_t((valueOffset(this->reference.page_id*page_size+this->reference.offset - this->alignShift()) + int(addr)) % block_size);
        }
        inline unsigned int pageIndex(addr_t const addr) const
        {
            return (this->reference.page_id*page_size+this->reference.offset + alignOffset(int(addr))) / page_size;
        }
        inline unsigned int blockIndex(addr_t const addr) const
        {
            return blockOffset(valueOffset(this->reference.page_id*page_size+this->reference.offset - this->alignShift()) + int(addr));
        }

        using typename ContextSync<page_size, align_t>::BlockPtr;
        using typename ContextSync<page_size, align_t>::BlockRef;
        typedef boost::shared_ptr<TypeBlock<page_size, align_t, val_t> const> TypeBlockPtr;

        static inline Reference offsetReference(Reference const ref, int const offset)
        {
            unsigned int const a = ref.offset + offset;
            return Reference({ref.page_id + a/page_size, a % page_size});
        }

    public:
#define TYPEID boost::typeindex::type_id< Linear<page_size, align_t, addr_t, val_t, buf_t, index_t> >()
#define ID_F(flags) {TYPEID, size_t( flags )}
#define ID(ref) ID_F( alignShift(ref) )

        Linear(Context<page_size, align_t> const& context_, Reference const ref_= {})
            : Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::Accessor(context_, ID(ref_)), reference(ref_)
        {}

        template <typename addr2_t, typename val2_t, typename buf2_t, typename index2_t>
        Linear(Linear<page_size, align_t, addr2_t, val2_t, buf2_t, index2_t> const& l, int const offset=0)
            : Linear(l.context, offsetReference(l.reference,offset))
        {}

        template <typename addr2_t, typename val2_t, typename buf2_t, typename index2_t>
        Linear(Linear<page_size, align_t, addr2_t, val2_t, buf2_t, index2_t> const& l, Reference const ref)
            : Linear(l.context, ref)
        {}

#undef TYPEID
#undef ID_F
#undef ID

        index_t operate(addr_t addr, index_t const len, std::function<void (val_t&)> const operation, bool dirty) const
        {
            index_t l;
            if(operation)
            {
                addr_t const end = addr + len;
                while(addr < end)
                {
                    unsigned int page_id = this->pageIndex(addr);
                    unsigned int block_id = this->blockIndex(addr);
                    TypeBlockPtr block = this->getBlock(page_id, block_id, dirty);

                    for(index_t i = valueIndex(addr); (i != block_size) && (addr < end); ++i, ++l, ++addr)
                    {
                        val_t& v = (*block)[i];
                        operation(v);
                    }
                }
            }

            return l;
        }

        using Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::read;
        using Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::write;

        index_t read(addr_t addr, index_t const len, buf_t buf) const final
        {
            std::function<void (val_t&)> const operation = [&buf](val_t& v)
            {
                *buf = v;
                ++buf;
            };
            return this->operate(addr, len, operation, false);
        }

        index_t write(addr_t addr, index_t const len, buf_t buf) const final
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

            int offset = this->alignShift() + alignOffset(block_id * block_size) - page_id * page_size; // offset in page
            block->read(0, block_size, buf, offset);
        }

        void write_page_block(BlockRef const b, std::array<align_t, page_size> const& buf) const final
        {
            unsigned int const page_id = b.first.page_id;
            unsigned int const block_id = b.first.block_id;
            BlockPtr const block = b.second;

            int offset = this->alignShift() + alignOffset(block_id * block_size) - page_id * page_size; // offset in page
            block->write(0, block_size, buf, offset);
        }

    private:
        Reference const reference;

        // TODO: optimize?
        std::vector<BlockKey> blockKeys(unsigned int const block_id) const
        {
            std::vector<BlockKey> keys;
            unsigned int last_page_id = 0;
            for(int i = 0; i < this->alignOffset(block_size); ++i)
            {
                unsigned int const page_id = (this->alignShift() + this->alignOffset(block_id*block_size) + i) / page_size;
                if(page_id != last_page_id || i == 0)
                {
                    keys.push_back({page_id, block_id, this->accessor_id});
                }
                last_page_id = page_id;
            }
            return keys;
        }

        TypeBlockPtr getBlock(unsigned int const page_id, unsigned int const block_id, bool dirty) const
        {
            BlockKey const key = {page_id, block_id, this->accessor_id};
            TypeBlockPtr block = boost::static_pointer_cast<TypeBlock<page_size, align_t, val_t> const>(this->context.getBlock(key));
            if(block == NULL)
            {
                Reference const ref = this->reference;
                auto const createSync = [ref](Context<page_size, align_t> const& c)
                {
                    return new Linear<page_size, align_t, addr_t, val_t, buf_t, index_t>(c, ref);
                };
                block = TypeBlockPtr(new TypeBlock<page_size, align_t, val_t>(block_size, createSync));
                this->context.addBlock(block, this->blockKeys(block_id));
            }
            if(dirty)
                this->context.markPageDirty(key, block);

            return block;
        }
};

} // namespace accessors

} // namespace memory

} // namespace giecs

