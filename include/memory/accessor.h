
#pragma once

#include <cstddef>
#include <functional>

#include <memory/context.h>
#include <memory/block.h>

namespace giecs
{
namespace memory
{

template <typename addr_t, typename val_t, typename buf_t, typename index_t>
class Accessor
{
    public:
        Accessor(Context* context_)
            : context(context_)
        {
        }

        virtual index_t read(addr_t const addr, index_t const len, buf_t buf) const {};
        virtual index_t write(addr_t const addr, index_t const len, buf_t buf) const {};

        val_t read(addr_t const addr) const
        {
            val_t val;
            index_t l = index_t();
            ++l;
            this->read(addr, l, buf_t(&val));
            return val;
        }

        val_t const& write(addr_t const addr, val_t const& val) const
        {
            index_t l = index_t();
            ++l;
            this->write(addr, l, buf_t(&val));
            return val;
        }

        class Proxy
        {
            public:
                Accessor<addr_t, val_t, buf_t, index_t> const* const parent;
                addr_t const addr;

                Proxy(Accessor<addr_t, val_t, buf_t, index_t> const* const parent_, addr_t const addr_)
                    : parent(parent_), addr(addr_)
                {}

                operator val_t () const
                {
                    return this->parent->read(addr);
                }

                val_t const& operator= (val_t const& val) const
                {
                    return this->parent->write(addr, val);
                }
        };

        Proxy operator[] (addr_t const addr) const
        {
            return Proxy(this, addr);
        }

    protected:
        Context* const context;
};

namespace accessors
{

template <typename addr_t, typename val_t, typename buf_t=val_t*, typename index_t=size_t>
class Linear : public Accessor<addr_t, val_t, buf_t, index_t>
{
    public:
        Linear(Context* context_)
            : Accessor<addr_t, val_t, buf_t, index_t>::Accessor(context_)
        {
            this->offset = addr_t();
        }

        Linear(Context* context_, addr_t offset_)
            : Accessor<addr_t, val_t, buf_t, index_t>::Accessor(context_), offset(offset_)
        {}

        addr_t offset;

        index_t operate(addr_t const addr, index_t const len, std::function<void (val_t&)> const operation) const
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
                    TypeBlock<val_t>* block = (TypeBlock<val_t>*)this->context->getBlock({page_id, page_id});
                    if(block == NULL)
                    {
                        block = new TypeBlock<val_t>(block_size);
                        this->context->addBlock(block, {page_id, page_id});
                    }

                    int end = (page_id == last_page_id) ? last_i : block_size;
                    for(; i < end; ++i, ++l)
                        operation((*block)[i]);

                    i = 0;
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
            return this->operate(addr, len, operation);
        }

        index_t write(addr_t const addr, index_t const len, buf_t buf) const
        {
            std::function<void (val_t&)> const operation = [&buf](val_t& v)
            {
                v = *buf;
                ++buf;
            };
            return this->operate(addr, len, operation);
        }
};

} // namespace accessors

} // namespace memory

} // namespace giecs

