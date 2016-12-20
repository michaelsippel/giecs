
#pragma once

#include <array>
#include <cstddef>
#include <functional>

#include <boost/type_index.hpp>
#include <boost/functional/hash.hpp>

namespace giecs
{
namespace memory
{

template <size_t page_size, typename align_t>
class Context;

template <size_t page_size, typename align_t>
class Block;

struct AccessorId
{
    boost::typeindex::type_index type;
    size_t flags;

    bool operator==(AccessorId const& i) const
    {
        return (this->type == i.type &&
                this->flags == i.flags);
    }
};

struct BlockKey
{
    unsigned int page_id;
    unsigned int block_id;
    AccessorId accessor_id;

    std::pair<int, int> page_range;

    bool operator==(BlockKey const& b) const
    {
        return (this->page_id == b.page_id &&
                this->block_id == b.block_id &&
                this->accessor_id == b.accessor_id);
    }
};

inline size_t hash_value(BlockKey const& block)
{
    boost::hash<unsigned int> hasher;
    return hasher(block.page_id);
}

template <size_t page_size, typename align_t>
class ContextSync
{
        friend class Context<page_size, align_t>;

    public:
        ContextSync(Context<page_size, align_t> const& context_, AccessorId const accessor_id_)
            : context(context_), accessor_id(accessor_id_)
        {
        }

        virtual ~ContextSync() {}

        typedef std::pair< BlockKey const, Block<page_size, align_t>* const > BlockRef;

        virtual void read_page_block(BlockRef const b, std::array<align_t, page_size>& buf) const {}
        virtual void write_page_block(BlockRef const b, std::array<align_t, page_size> const& buf, std::pair<int,int> range) const {}

        inline void read_page(unsigned int const page_id, std::array<align_t, page_size>& buf) const
        {
            for(BlockRef b : this->context.getPageRange({page_id, 0, this->accessor_id}))
                this->read_page_block(b, buf);
        }

        inline void write_page(unsigned int const page_id, std::array<align_t, page_size> const& buf, std::pair<int, int> range) const
        {
            for(BlockRef b : this->context.getPageRange({page_id, 0, this->accessor_id}))
                this->write_page_block(b, buf, range);
        }

    protected:
        Context<page_size, align_t> const& context;
        AccessorId accessor_id;
};

template <size_t page_size, typename align_t>
class Block
{
    public:
        Block(size_t const l, std::function<ContextSync<page_size, align_t>* (Context<page_size, align_t> const&)> createSync_)
            : length(l), createSync(createSync_)
        {
            this->ptr = malloc(this->length);
        }

        Block(Block const& b)
        {
            this->length = b.length;
            this->ptr = b.ptr;
        }

        virtual ~Block()
        {
            free(this->ptr);
        }

        virtual void read(int i, size_t const end, std::array<align_t, page_size>& buf, int off, int bitoff) const {}
        virtual void write(int i, size_t const end, std::array<align_t, page_size> const& buf, int off, int bitoff, std::pair<int, int> range) const {}

        ContextSync<page_size, align_t>* getSync(Context<page_size, align_t> const& context) const
        {
            return this->createSync(context);
        }

    protected:
        size_t length;
        void* ptr;

        std::function<ContextSync<page_size, align_t>* (Context<page_size, align_t> const&)> createSync;
};

template <size_t page_size, typename align_t, typename val_t>
class TypeBlock : public Block<page_size, align_t>
{
    public:
        TypeBlock(size_t n, std::function<ContextSync<page_size, align_t>* (Context<page_size, align_t> const&)> createSync_)
            : Block<page_size, align_t>(sizeof(val_t) * n, createSync_)
        {
        }

        TypeBlock(Block<page_size, align_t> const& b)
            : Block<page_size, align_t>(b)
        {
        }

        void read(int i, size_t const end, std::array<align_t, page_size>& buf, int off, int bitoff) const
        {
            read_block(*this, i, end, buf, off, bitoff);
        }

        void write(int i, size_t const end, std::array<align_t, page_size> const& buf, int off, int bitoff, std::pair<int, int> range) const
        {
            write_block(*this, i, end, buf, off, bitoff, range);
        }

        inline int numElements(void) const
        {
            return (this->length / sizeof(val_t));
        }

        val_t& operator[] (int const& index) const
        {
            return ((val_t*)this->ptr)[index % this->numElements()];
        }
};

} // namespace memory

} // namespace giecs

