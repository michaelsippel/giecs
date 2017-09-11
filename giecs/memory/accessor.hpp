
#pragma once

#include <cstddef>
#include <memory>

#include <giecs/memory/context.hpp>
#include <giecs/memory/block.hpp>

namespace giecs
{
namespace memory
{

template <std::size_t page_size, typename align_t, typename addr_t, typename val_t, typename buf_t, typename index_t, typename Derived>
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
            Accessor const& parent;
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

    protected:
        template <typename CreateSyncFunctor>
        std::shared_ptr<TypeBlock<page_size, align_t, val_t, CreateSyncFunctor> const> getBlock(std::size_t const page_id, std::size_t const block_id, bool dirty, CreateSyncFunctor createSync, std::size_t block_size) const
        {
            BlockKey const key = {page_id, block_id, this->accessor_id};
            std::shared_ptr<TypeBlock<page_size, align_t, val_t, CreateSyncFunctor> const> block = std::static_pointer_cast<TypeBlock<page_size, align_t, val_t, CreateSyncFunctor> const>(this->context.getBlock(key));
            if(block == NULL)
            {
                block = std::make_shared<TypeBlock<page_size, align_t, val_t, CreateSyncFunctor> const>(block_size, createSync);
                this->context.addBlock(block, reinterpret_cast<Derived const*>(this)->blockKeys(block_id));
            }
            if(dirty)
                this->context.markPageDirty(key, block);

            return block;
        }
};

} // namespace memory

} // namespace giecs

