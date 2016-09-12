
#pragma once

#include <cstddef>
#include <memory/context.h>
#include <memory/block.h>

namespace giecs
{
namespace memory
{

template <typename addr_t, typename val_t, typename buf_t, typename index_t>
class Accessor : public ContextSync
{
    public:
        Accessor(const Context* const context_)
            : ContextSync(context_)
        {
        }

        virtual index_t read(addr_t const addr, index_t const len, buf_t buf) const {};
        virtual index_t write(addr_t const addr, index_t const len, buf_t buf) const {};

        virtual void read_page(int const page_id, uint8_t* const buf) const {};
        virtual void write_page(int const page_id, uint8_t const* const buf) const {};

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
};

} // namespace memory

} // namespace giecs

