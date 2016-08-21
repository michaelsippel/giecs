
#pragma once

namespace giecs
{
namespace memory
{

class Accessor;

class Block
{
    public:
        Block(size_t l)
        {
            this->length = l;
            this->ptr = malloc(this->length);
        }

        Block(const Block& b)
        {
            this->length = b.length;
            this->ptr = b.ptr;

        }

        ~Block()
        {
            free(this->ptr);
        }

    protected:
        size_t length;
        void* ptr;

        Accessor (*createAccesor)(Context* context);
};

template <typename T>
class TypeBlock : public Block
{
    public:
        TypeBlock(size_t n)
            : Block(sizeof(T) * n)
        {
        }

        TypeBlock(const Block& b)
            : Block(b)
        {
        }

        int numElements(void) const
        {
            return (this->length / sizeof(T));
        }

        T& operator[] (const int& index) const
        {
            return ((T*)this->ptr)[index % this->numElements()];
        }
};

} // namespace memory

} // namespace giecs

