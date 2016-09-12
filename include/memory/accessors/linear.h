
#pragma once

#include <memory/accessor.h>

#include <functional>
#include <boost/type_index.hpp>

namespace giecs
{
namespace memory
{
namespace accessors
{

template <typename addr_t, typename val_t, typename buf_t=val_t*, typename index_t=size_t>
class Linear : public Accessor<addr_t, val_t, buf_t, index_t>
{
    public:
        Linear(Context const* const context_)
            : Accessor<addr_t, val_t, buf_t, index_t>::Accessor(context_)
        {
            this->offset = addr_t();
        }

        Linear(Context const* const context_, addr_t offset_)
            : Accessor<addr_t, val_t, buf_t, index_t>::Accessor(context_), offset(offset_)
        {}

        addr_t offset;

        index_t operate(addr_t const addr, index_t const len, std::function<void (val_t&)> const operation, bool dirty) const
        {
            index_t l;

            if(operation)
            {
                size_t block_size = this->context->page_size / sizeof(val_t);

                int page_id = (int)(offset+addr) / block_size;
                int last_page_id = page_id + ((int)len / block_size);

                int i = (int)(offset+addr) % block_size;
                int last_i = (i + (int)len) % block_size;

                for(; page_id <= last_page_id; ++page_id)
                {
                    BlockKey const key = {page_id, page_id, boost::typeindex::type_id< Linear<addr_t, val_t, buf_t, index_t> >()};
                    TypeBlock<val_t>* block = (TypeBlock<val_t>*)this->context->getBlock(key);
                    if(block == NULL)
                    {
                        static auto const createSync = [](Context const* const c)
                        {
                            return new Linear<addr_t, val_t, buf_t, index_t>(c);
                        };
                        block = new TypeBlock<val_t>(block_size, createSync);
                        this->context->addBlock(block, key);
                    }

                    int end = (page_id == last_page_id) ? last_i : block_size;
                    for(; i < end; ++i, ++l)
                        operation((*block)[i]);

                    i = 0;

                    if(dirty)
                        this->context->markPageDirty(page_id, block);
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

        void read_page(int const page_id, uint8_t* const buf) const
        {
            BlockKey const key = {page_id, page_id, boost::typeindex::type_id< Linear<addr_t, val_t, buf_t, index_t> >()};
            TypeBlock<uint8_t>* block = (TypeBlock<uint8_t>*)this->context->getBlock(key);
            if(block != NULL)
            {
                for(int i = 0; i < this->context->page_size; i++)
                {
                    buf[i] = (*block)[i];
                }
            }
        }

        void write_page(int const page_id, uint8_t const* const buf) const
        {
            BlockKey const key = {page_id, page_id, boost::typeindex::type_id< Linear<addr_t, val_t, buf_t, index_t> >()};
            TypeBlock<uint8_t>* block = (TypeBlock<uint8_t>*)this->context->getBlock(key);
            if(block != NULL)
            {
                for(int i = 0; i < this->context->page_size; i++)
                {
                    (*block)[i] = buf[i];
                }
            }
        }
};

}; // namespace accessors

}; // namespace memory

}; // namespace giecs

