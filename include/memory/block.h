
#pragma once

#include <cstddef>
#include <functional>

namespace giecs
{
namespace memory
{

class Context;
class ContextSync
{
    public:
        ContextSync(const Context* const context_);

        virtual void read_page(int const page_id, uint8_t* const buf) const {};
        virtual void write_page(int const page_id, uint8_t const* const buf) const {};

    protected:
        Context const* const context;
};

class Block
{
    public:
        Block(size_t const l, std::function<ContextSync* (Context const* const)> const createSync_);
        Block(Block const& b);
        ~Block();

        ContextSync* getSync(const Context* const context) const;

    protected:
        size_t length;
        void* ptr;

        std::function<ContextSync* (Context const* const)> const createSync;
};

template <typename T>
class TypeBlock : public Block
{
    public:
        TypeBlock(size_t n, std::function<ContextSync* (Context const* const)> const createSync_)
            : Block(sizeof(T) * n, createSync_)
        {
        }

        TypeBlock(Block const& b)
            : Block(b)
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

