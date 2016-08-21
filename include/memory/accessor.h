
#pragma once

#include <memory/context.h>
#include <memory/block.h>

namespace giecs
{
namespace memory
{

class Accessor
{
    public:
        Accessor(Context* context_)
            : context(context_)
        {
        }

    protected:
        Context* context;
};

namespace accessors
{

template <typename addr_t, typename val_t>
class Linear : public Accessor
{
    public:
        using Accessor::Accessor;

        static const int page_size = 0x1000;
        static const int page_mask = 0xfff;

        val_t& operator[] (const addr_t& addr) const
        {
            int blockid = (int)addr & ~page_mask;
            TypeBlock<val_t>* block = (TypeBlock<val_t>*)this->context->getBlock(blockid);
            if(block == NULL)
            {
                block = new TypeBlock<val_t>(page_size);
                this->context->addBlock(blockid, block);
            }
            int i = (int)addr & page_mask;
            return (*block)[i];
        }

        int read(addr_t pos, size_t len, val_t* buf)
        {
        }

        int write(addr_t pos, size_t len, val_t* buf)
        {
        }
};

} // namespace accessors

} // namespace memory

} // namespace giecs


