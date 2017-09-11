
#pragma once
#include <iostream>
#include <cstddef>
#include <boost/type_index.hpp>

#include <giecs/memory/accessor.hpp>
#include <giecs/memory/reference.hpp>
#include <giecs/bits.hpp>

namespace giecs
{
namespace memory
{
namespace accessors
{

template <std::size_t page_size, typename align_t, typename addr_t, typename val_t, typename buf_t=val_t*, typename index_t=size_t>
class Linear : public Accessor<page_size, align_t, val_t, Linear<page_size, align_t, addr_t, val_t, buf_t, index_t>>
{
        template <std::size_t, typename, typename, typename, typename, typename> friend class Linear;
        template <std::size_t, typename, typename, typename> friend class Accessor;

    protected:
        static std::size_t const block_size = 512; // 32bit/val -> 2 MiB.  // TODO: more dynamic
        static std::ptrdiff_t const nalign = (bitsize<val_t>() > bitsize<align_t>()) ?
                                             (1+(bitsize<val_t>()-1)/bitsize<align_t>()) :  // positive: number of align-blocks per value
                                             (-(bitsize<align_t>()/bitsize<val_t>())); // negative: number of values per align-block

        // align per value
        static inline std::ptrdiff_t alignOffset(int const nval)
        {
            return (nalign > 0) ? (nval * nalign) : (nval / (-nalign));
        }
        // value per align
        static inline std::ptrdiff_t valueOffset(int const na)
        {
            return (nalign > 0) ? (na / nalign) : (na * (-nalign));
        }
        // pages per value
        static inline std::ptrdiff_t pageOffset(int const nval)
        {
            return alignOffset(nval) / page_size;
        }
        // blocks per value
        static inline std::ptrdiff_t blockOffset(int const nval)
        {
            return nval / block_size;
        }
        // align offset in value
        static inline std::ptrdiff_t alignShift(Reference const ref)
        {
            // TODO: fix possible integer overflow
            return (nalign < 1) ? 0 : ((ref.page_id * page_size + ref.offset) % nalign);
        }
        inline std::ptrdiff_t alignShift() const
        {
            return alignShift(this->reference);
        }

        inline index_t valueIndex(addr_t const addr) const
        {
            return index_t((valueOffset(this->reference.page_id*page_size+this->reference.offset - this->alignShift()) + std::ptrdiff_t(addr)) % block_size);
        }
        inline std::size_t pageIndex(addr_t const addr) const
        {
            return (this->reference.page_id*page_size+this->reference.offset + alignOffset(addr)) / page_size;
        }
        inline std::size_t blockIndex(addr_t const addr) const
        {
            return blockOffset(valueOffset(this->reference.page_id*page_size+this->reference.offset - this->alignShift()) + std::ptrdiff_t(addr));
        }

        using typename ContextSync<page_size, align_t>::BlockPtr;
        using typename ContextSync<page_size, align_t>::BlockRef;

        static inline Reference offsetReference(Reference const ref, std::ptrdiff_t const offset)
        {
            std::size_t const a = ref.offset + offset;
            return Reference({ref.page_id + a/page_size, a % page_size});
        }

    public:
#define TYPEID boost::typeindex::type_id< Linear<page_size, align_t, addr_t, val_t, buf_t, index_t> >()
#define ID_F(flags) {TYPEID, std::uintmax_t( flags )}
#define ID(ref) ID_F( alignShift(ref) )

        Linear(Context<page_size, align_t> const& context_, Reference const ref_= {})
            : Accessor<page_size, align_t, val_t, Linear>::Accessor(context_, ID(ref_)), reference(ref_)
        {}

        template <typename addr2_t, typename val2_t, typename buf2_t, typename index2_t>
        Linear(Linear<page_size, align_t, addr2_t, val2_t, buf2_t, index2_t> const& l, std::ptrdiff_t const offset=0)
            : Linear(l.context, offsetReference(l.reference,offset))
        {}

        template <typename addr2_t, typename val2_t, typename buf2_t, typename index2_t>
        Linear(Linear<page_size, align_t, addr2_t, val2_t, buf2_t, index2_t> const& l, Reference const ref)
            : Linear(l.context, ref)
        {}

#undef TYPEID
#undef ID_F
#undef ID

        template <typename Functor>
        index_t operate(addr_t addr, index_t const len, Functor operation, bool dirty) const
        {
            index_t l;
            Reference const& r = this->reference;
            auto const createSync = [r](Context<page_size, align_t> const& c)
            {
                return new Linear<page_size, align_t, addr_t, val_t, buf_t, index_t>(c, r);
            };

            addr_t const end = addr + len;
            while(addr < end)
            {
                std::size_t page_id = this->pageIndex(addr);
                std::size_t block_id = this->blockIndex(addr);

                auto const block = this->getBlock(page_id, block_id, dirty, createSync, block_size);

                for(index_t i = valueIndex(addr); (i != index_t(block_size)) && (addr < end); ++i, ++l, ++addr)
                {
                    val_t& v = (*block)[i];
                    operation(v);
                }
            }

            return l;
        }

        index_t read(addr_t addr, index_t const len, buf_t buf) const
        {
            auto const operation = [&buf](val_t& v)
            {
                *buf = v;
                ++buf;
            };
            return this->operate(addr, len, operation, false);
        }

        index_t write(addr_t addr, index_t const len, buf_t buf) const
        {
            auto const operation = [&buf](val_t& v)
            {
                v = *buf;
                ++buf;
            };
            return this->operate(addr, len, operation, true);
        }

        val_t read(addr_t const addr) const
        {
            val_t val;
            index_t l = index_t();
            this->read(addr, ++l, buf_t(&val));
            return val;
        }

        val_t const& write(addr_t const addr, val_t const& val) const
        {
            index_t l = index_t();
            this->write(addr, ++l, buf_t(&val));
            return val;
        }

        struct Proxy
        {
            Linear const& parent;
            addr_t const addr;

            operator val_t ()
            {
                return this->parent.read(addr);
            }

            val_t const& operator= (val_t const& val)
            {
                return this->parent.write(addr, val);
            }
        };

        Proxy operator[] (addr_t const addr) const
        {
            return {*this, addr};
        }



        void read_page_block(BlockRef const b, std::array<align_t, page_size>& buf) const final
        {
            std::size_t const page_id = b.first.page_id;
            std::size_t const block_id = b.first.block_id;
            BlockPtr const block = b.second;

            std::ptrdiff_t offset = this->alignShift() + alignOffset(block_id * block_size) - page_id * page_size; // offset in page
            block->read(0, block_size, buf, offset);
        }

        void write_page_block(BlockRef const b, std::array<align_t, page_size> const& buf) const final
        {
            std::size_t const page_id = b.first.page_id;
            std::size_t const block_id = b.first.block_id;
            BlockPtr const block = b.second;

            std::ptrdiff_t offset = this->alignShift() + alignOffset(block_id * block_size) - page_id * page_size; // offset in page
            block->write(0, block_size, buf, offset);
        }

    private:
        Reference const reference;

        // TODO: optimize?
        std::vector<BlockKey> blockKeys(std::size_t const block_id) const
        {
            std::vector<BlockKey> keys;
            std::size_t last_page_id = 0;
            for(int i = 0; i < this->alignOffset(block_size); ++i)
            {
                std::size_t const page_id = (this->alignShift() + this->alignOffset(block_id*block_size) + i) / page_size;
                if(page_id != last_page_id || i == 0)
                    keys.push_back({page_id, block_id, this->accessor_id});
                last_page_id = page_id;
            }
            return keys;
        }
};

} // namespace accessors

} // namespace memory

} // namespace giecs

