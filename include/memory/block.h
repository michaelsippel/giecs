
#pragma once

#include <cstddef>
#include <functional>

namespace giecs
{
namespace memory
{

template <size_t page_size, typename align_t>
class Context;

template <size_t page_size, typename align_t>
class ContextSync
{
    public:
        ContextSync(const Context<page_size, align_t>* const context_)
            : context(context_)
        {
        }

        virtual void read_page(unsigned int const page_id, align_t* buf) const {};
        virtual void write_page(unsigned int const page_id, align_t const* buf) const {};

    protected:
        Context<page_size, align_t> const* const context;
};

template <size_t page_size, typename align_t>
class Block
{
    public:
        Block(size_t const l, std::function<ContextSync<page_size, align_t>* (Context<page_size, align_t> const* const)> const createSync_)
            : length(l), createSync(createSync_)
        {
            this->ptr = malloc(this->length);
        }

        Block(Block const& b)
        {
            this->length = b.length;
            this->ptr = b.ptr;
        }

        ~Block()
        {
            free(this->ptr);
        }

        ContextSync<page_size, align_t>* getSync(const Context<page_size, align_t>* const context) const
        {
            return this->createSync(context);
        }

    protected:
        size_t length;
        void* ptr;

        std::function<ContextSync<page_size, align_t>* (Context<page_size, align_t> const* const)> const createSync;
};

template <size_t page_size, typename align_t, typename T>
class TypeBlock : public Block<page_size, align_t>
{
    public:
        TypeBlock(size_t n, std::function<ContextSync<page_size, align_t>* (Context<page_size, align_t> const* const)> const createSync_)
            : Block<page_size, align_t>(sizeof(T) * n, createSync_)
        {
        }

        TypeBlock(Block<page_size, align_t> const& b)
            : Block<page_size, align_t>(b)
        {
        }

        int numElements(void) const
        {
            return (this->length / sizeof(T));
        }

        T& operator[] (int const& index) const
        {
            return ((T*)this->ptr)[index % this->numElements()];
        }
};

} // namespace memory

} // namespace giecs

