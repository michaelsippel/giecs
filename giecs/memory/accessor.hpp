
#pragma once

#include <cstddef>

#include <giecs/memory/context.hpp>
#include <giecs/memory/block.hpp>

namespace giecs
{
namespace memory
{

template <std::size_t page_size, typename align_t, typename addr_t, typename val_t, typename buf_t, typename index_t>
class Accessor : public ContextSync<page_size, align_t>
{
    public:
        Accessor(Context<page_size, align_t> const& context_, AccessorId const accessor_id_)
            : ContextSync<page_size, align_t>(context_, accessor_id_)
        {
        }

        virtual index_t read(addr_t addr, index_t const len, buf_t buf) const
        {
            return index_t();
        }
        virtual index_t write(addr_t addr, index_t const len, buf_t buf) const
        {
            return index_t();
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
            Accessor<page_size, align_t, addr_t, val_t, buf_t, index_t> const& parent;
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
};

} // namespace memory

} // namespace giecs

