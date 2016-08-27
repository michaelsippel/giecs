
#pragma once

#include <cstddef>

namespace giecs
{
namespace memory
{

class Context;
//class Accessor;

class Block
{
    public:
        Block(size_t l);
        Block(const Block& b);
        ~Block();

        void resize(size_t l);
        size_t getLength(void);

//        Accessor getAccessor(Context* context);

    protected:
        size_t length;
        void* ptr;

//        Accessor (*createAccesor)(Context* context);
};

template <typename T>
class TypeBlock : public Block
{
    public:
        TypeBlock(size_t n)
            : Block(sizeof(T) * n)
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

