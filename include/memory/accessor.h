
#pragma once

#include <cstddef>
#include <functional>

#include <memory/context.h>
#include <memory/block.h>

namespace giecs
{
namespace memory
{

template <typename addr_t, typename val_t>
class Accessor
{
    public:
        Accessor(Context* context_)
            : context(context_)
        {
        }

        virtual val_t read(addr_t const addr) const = 0;
        virtual val_t const& write(addr_t const addr, val_t const& val) const = 0;

        class Proxy
        {
            public:
                Accessor<addr_t, val_t> const* const parent;
                addr_t const addr;

                Proxy(Accessor<addr_t, val_t> const* const parent_, addr_t const addr_)
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

template <typename addr_t, typename val_t, int page_bit=12>
class Linear : public Accessor<addr_t, val_t>
{
    public:
        using Accessor<addr_t, val_t>::Accessor;

        static int const page_size = (1 << page_bit);
        static int const page_mask = page_size - 1;

        addr_t offset = 0;

        size_t operate(addr_t const addr, size_t const len, std::function<void (val_t&)> const operation) const
        {
            if(! operation)
                return -1;

            int page_id = ((int)addr + (int)offset) & ~page_mask;
            int last_page_id = page_id + (len & ~page_mask);

            int i = (int)addr & page_mask;
            int last_i = ((int) addr + len) & page_mask;

            for(; page_id <= last_page_id; page_id++)
            {
                TypeBlock<val_t>* block = (TypeBlock<val_t>*)this->context->getBlock({page_id, page_id});
                if(block == NULL)
                {
                    block = new TypeBlock<val_t>(page_size);
                    this->context->addBlock(block, {page_id, page_id});
                }

                int end = (page_id == last_page_id) ? last_i : page_size;
                for(; i < end;  i++)
                    operation((*block)[i]);

                i = 0;
            }

            return i;
        }

        size_t read(addr_t const addr, size_t const len, val_t* const buf) const
        {
            int i = 0;
            std::function<void (val_t&)> const operation = [&i, buf](val_t& v)
            {
                buf[i++] = v;
            };
            return this->operate(addr, len, operation);
        }

        size_t write(addr_t const addr, size_t const len, val_t const* buf) const
        {
            int i = 0;
            std::function<void (val_t&)> const operation = [&i, buf](val_t& v)
            {
                v = buf[i++];
            };
            return this->operate(addr, len, operation);
        }

        val_t read(addr_t const addr) const
        {
            val_t val;
            this->read(addr, 1, &val);
            return val;
        }

        val_t const& write(addr_t const addr, val_t const& val) const
        {
            this->write(addr, 1, &val);
            return val;
        }
};

} // namespace accessors

} // namespace memory

} // namespace giecs

