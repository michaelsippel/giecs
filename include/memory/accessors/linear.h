
#pragma once

#include <functional>
#include <boost/type_index.hpp>

#include <memory/accessor.h>
#include <bits.h>

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
        static size_t const block_size = (page_size * bitsize<align_t>()) / bitsize<val_t>();

    public:
        Linear(Context<page_size, align_t> const* const context_)
            : Accessor<page_size, align_t,addr_t, val_t, buf_t, index_t>::Accessor(context_)
        {
            this->offset = addr_t();
        }

        Linear(Context<page_size, align_t> const* const context_, addr_t offset_)
            : Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::Accessor(context_), offset(offset_)
        {}

        template <typename addr2_t, typename val2_t, typename buf2_t, typename index2_t>
        Linear(Linear<page_size, align_t, addr2_t, val2_t, buf2_t, index2_t> const& l)
            : Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t>::Accessor(l.context)
        {
            this->offset = 1 + ((addr_t) l.offset * bitsize<val2_t>() - 1) / bitsize<val_t>();
        }

        addr_t offset;

        index_t operate(addr_t const addr, index_t const len, std::function<void (val_t&)> const operation, bool dirty) const
        {
            index_t l;

            if(operation)
            {
                unsigned int block_id = (int)(offset+addr) / block_size;
                unsigned int last_block_id = block_id + ((int)len / block_size);

                unsigned int i = (int)(offset+addr) % block_size;
                unsigned int last_i = (i + (int)len) % block_size;

                for(; block_id <= last_block_id; ++block_id)
                {
                    BlockKey const key = {block_id, block_id, boost::typeindex::type_id< Linear<page_size, align_t, addr_t, val_t, buf_t, index_t> >()};
                    TypeBlock<page_size, align_t, val_t>* block = (TypeBlock<page_size, align_t, val_t>*)this->context->getBlock(key);
                    if(block == NULL)
                    {
                        static auto const createSync = [](Context<page_size, align_t> const* const c)
                        {
                            return Linear<page_size, align_t, addr_t, val_t, buf_t, index_t>(c);
                        };
                        block = new TypeBlock<page_size, align_t, val_t>(block_size, createSync);
                        this->context->addBlock(block, key);
                    }

                    unsigned int end = (block_id == last_block_id) ? last_i : block_size;
                    for(; i < end; ++i, ++l)
                        operation((*block)[i]);

                    i = 0;

                    if(dirty)
                        this->context->markPageDirty(block_id, block);
                }
            }

            return l;
        }

        index_t read(addr_t const addr, index_t const len, buf_t buf) const
        {
            std::function<void (val_t&)> const operation = [&buf](val_t& v)
            {
                *buf = v;
                ++buf;
            };
            return this->operate(addr, len, operation, false);
        }

        index_t write(addr_t const addr, index_t const len, buf_t buf) const
        {
            std::function<void (val_t&)> const operation = [&buf](val_t& v)
            {
                v = *buf;
                ++buf;
            };
            return this->operate(addr, len, operation, true);
        }

        void read_page(unsigned int const page_id, align_t* buf) const
        {
            BlockKey const key = {page_id, page_id, boost::typeindex::type_id< Linear<page_size, align_t, addr_t, val_t, buf_t, index_t> >()};
            TypeBlock<page_size, align_t, val_t>* block = (TypeBlock<page_size, align_t, val_t>*)this->context->getBlock(key);
            if(block != NULL)
            {
                int bitoff = 0; // position in align_t
                *buf = align_t();
                for(int i = 0; i < block_size; i++)
                {
                    *buf |= (align_t) ((unsigned int)(*block)[i] << bitoff);
                    bitoff += bitsize<val_t>();

                    while(bitoff >= bitsize<align_t>())
                    {
                        *(++buf) = align_t();
                        bitoff -= bitsize<align_t>();

                        unsigned int rest = bitsize<val_t>() - bitoff;
                        if(rest > 0)
                            *buf |= (align_t) ((unsigned int)(*block)[i] >> rest) & ((1 << rest)-1);
                    }
                }
            }
        }

        void write_page(unsigned int const page_id, align_t const* buf) const
        {
            BlockKey const key = {page_id, page_id, boost::typeindex::type_id< Linear<page_size, align_t, addr_t, val_t, buf_t, index_t> >()};
            TypeBlock<page_size, align_t, val_t>* block = (TypeBlock<page_size, align_t, val_t>*)this->context->getBlock(key);
            if(block == NULL)
            {
                static auto const createSync = [](Context<page_size, align_t> const* const c)
                {
                    return Linear<page_size, align_t, addr_t, val_t, buf_t, index_t>(c);
                };
                block = new TypeBlock<page_size, align_t, val_t>(block_size, createSync);
                this->context->addBlock(block, key);
            }

            int bitoff = 0; // position in align_t
            for(int i = 0; i < block_size; i++)
            {
                unsigned int s = (((unsigned int)*buf) >> bitoff);
                bitoff += bitsize<val_t>();
                (*block)[i] = s & ((1 << bitoff)-1);

                while(bitoff >= bitsize<align_t>())
                {
                    ++buf;
                    bitoff -= bitsize<align_t>();

                    unsigned int rest = bitsize<val_t>() - bitoff;
                    if(rest > 0)
                        (*block)[i] |= (((unsigned int)*buf) << rest) & ((1 << bitsize<val_t>())-1);
                }
            }
        }
};

}; // namespace accessors

}; // namespace memory

}; // namespace giecs

