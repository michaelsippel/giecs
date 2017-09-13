
#pragma once

#include <array>
#include <cstddef>
#include <memory>

#include <boost/type_index.hpp>

namespace giecs
{
namespace memory
{

template <std::size_t page_size, typename align_t>
class Context;

template <std::size_t page_size, typename align_t>
class Block;

struct AccessorId
{
    boost::typeindex::type_index type;
    std::uintmax_t flags;

    bool operator==(AccessorId const& i) const
    {
        return (this->type == i.type &&
                this->flags == i.flags);
    }
};

struct BlockKey
{
    std::size_t page_id;
    std::size_t block_id;
    AccessorId accessor_id;

    bool operator==(BlockKey const& b) const
    {
        return (this->page_id == b.page_id &&
                this->block_id == b.block_id &&
                this->accessor_id == b.accessor_id);
    }
};

struct BlockKeyHash
{
    std::size_t operator() (BlockKey const& block) const
    {
        return block.page_id;
    }
};

template <std::size_t page_size, typename align_t>
class ContextSync
{
        friend class Context<page_size, align_t>;

    public:
        ContextSync(Context<page_size, align_t> const& context_, AccessorId accessor_id_)
            : context(context_), accessor_id(accessor_id_)
        {
        }

        virtual ~ContextSync() {}

        typedef std::shared_ptr<Block<page_size, align_t> const> BlockPtr;
        typedef std::pair< BlockKey const, BlockPtr const > BlockRef;

        virtual void read_page_block(BlockRef const b, std::array<align_t, page_size>& buf) const {}
        virtual void write_page_block(BlockRef const b, std::array<align_t, page_size> const& buf) const {}

        inline void read_page(std::size_t const page_id, std::array<align_t, page_size>& buf) const
        {
            for(BlockRef b : this->context.getPageRange({page_id, 0, this->accessor_id}))
                this->read_page_block(b, buf);
        }

        inline void write_page(std::size_t const page_id, std::array<align_t, page_size> const& buf) const
        {
            for(BlockRef b : this->context.getPageRange({page_id, 0, this->accessor_id}))
                this->write_page_block(b, buf);
        }

    protected:
        Context<page_size, align_t> const& context;
        AccessorId accessor_id;
};

template <std::size_t page_size, typename align_t>
class Block
{
    public:
        Block(std::size_t const l)
            : length(l)
        {
            this->ptr = calloc(this->length, 1);
        }

        virtual ~Block()
        {
            free(this->ptr);
        }

        virtual void read(std::ptrdiff_t i, std::size_t const end, std::array<align_t, page_size>& buf, std::ptrdiff_t off) const {}
        virtual void write(std::ptrdiff_t i, std::size_t const end, std::array<align_t, page_size> const& buf, std::ptrdiff_t off) const {}

        virtual ContextSync<page_size, align_t>* createSync(Context<page_size, align_t> const& context) const
        {
            return nullptr;
        }

    protected:
        std::size_t length;
        void* ptr;
};

template <std::size_t page_size, typename align_t, typename val_t, typename CreateSyncFunctor>
class TypeBlock : public Block<page_size, align_t>
{
    public:
        TypeBlock(std::size_t n, CreateSyncFunctor createSync_)
            : Block<page_size, align_t>(sizeof(val_t) * n), lcreateSync(createSync_)
        {
        }

        ContextSync<page_size, align_t>* createSync(Context<page_size, align_t> const& context) const final
        {
            return this->lcreateSync(context);
        }

        void read(std::ptrdiff_t i, std::size_t const end, std::array<align_t, page_size>& buf, std::ptrdiff_t off) const final
        {
            read_block(*this, i, end, buf, off);
        }

        void write(std::ptrdiff_t i, std::size_t const end, std::array<align_t, page_size> const& buf, std::ptrdiff_t off) const final
        {
            write_block(*this, i, end, buf, off);
        }

        inline int numElements(void) const
        {
            return (this->length / sizeof(val_t));
        }

        val_t& operator[] (std::size_t const& index) const
        {
            return ((val_t*)this->ptr)[index % this->numElements()];
        }
    private:
        CreateSyncFunctor lcreateSync;
};

} // namespace memory

} // namespace giecs

