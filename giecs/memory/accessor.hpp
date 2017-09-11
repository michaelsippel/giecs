
#pragma once

#include <cstddef>
#include <memory>

#include <giecs/memory/context.hpp>
#include <giecs/memory/block.hpp>

namespace giecs
{
namespace memory
{

template <std::size_t page_size, typename align_t, typename val_t, typename Derived>
class Accessor : public ContextSync<page_size, align_t>
{
    public:
        Accessor(Context<page_size, align_t> const& context_, AccessorId const accessor_id_)
            : ContextSync<page_size, align_t>(context_, accessor_id_)
        {}

        virtual ~Accessor()
        {}

    protected:
        template <typename CreateSyncFunctor>
        std::shared_ptr<TypeBlock<page_size, align_t, val_t, CreateSyncFunctor> const> getBlock(std::size_t const page_id, std::size_t const block_id, bool dirty, CreateSyncFunctor createSync, std::size_t block_size) const
        {
            BlockKey const key = {page_id, block_id, this->accessor_id};
            std::shared_ptr<TypeBlock<page_size, align_t, val_t, CreateSyncFunctor> const> block = std::static_pointer_cast<TypeBlock<page_size, align_t, val_t, CreateSyncFunctor> const>(this->context.getBlock(key));
            if(block == nullptr)
            {
                block = std::make_shared<TypeBlock<page_size, align_t, val_t, CreateSyncFunctor> const>(block_size, createSync);
                this->context.addBlock(block, static_cast<Derived const*>(this)->blockKeys(block_id));
            }
            if(dirty)
                this->context.markPageDirty(key, block);

            return block;
        }
};

} // namespace memory

} // namespace giecs

