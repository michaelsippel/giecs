
#pragma once

#include <giecs/memory/accessor.h>
#include <giecs/memory/accessors/linear.h>

#include <functional>
#include <boost/type_index.hpp>

namespace giecs
{
namespace memory
{
namespace accessors
{

template <size_t page_size, typename align_t, typename addr_t, typename val_t>
class Stack : public Linear<page_size, align_t, addr_t, val_t, val_t*, int>
{
        template <size_t, typename, typename, typename> friend class Stack;

    public:
        Stack(Context<page_size, align_t> const& context_)
            : Linear<page_size, align_t, addr_t, val_t, val_t*, int>::Linear(context_)
        {
            this->pos = 0;
        }

        Stack(Context<page_size, align_t> const& context_, addr_t offset_)
            : Linear<page_size, align_t, addr_t, val_t, val_t*, int>::Linear(context_, offset_)
        {
            this->pos = 0;
        }

        template <typename val2_t>
        Stack(Stack<page_size, align_t, addr_t, val2_t> const& s)
            : Linear<page_size, align_t, addr_t, val_t, val_t*, int>::Linear(s, 1+(s.pos*bitsize<val2_t>()-1)/bitsize<align_t>())
        {
            this->pos = 0;
        }

        void move(int const off)
        {
            this->pos += off;
        }

        void pushn(int const n, val_t* v)
        {
            this->write(this->pos, n, v);
            this->move(n);
        }

        void pop(int const n, val_t* v)
        {
            this->move(-n);
            this->read(this->pos, n, v);
        }

        template <typename val2_t>
        void push(int const n, val2_t* v)
        {
            auto s = Stack<page_size, align_t, addr_t, val2_t>(*this);
            s.push(n, v);

            size_t bitoff = (s.getOffset() - this->getOffset())*bitsize<align_t>() - this->pos*bitsize<val_t>() + n*bitsize<val2_t>();
            this->move(1 + (bitoff-1)/bitsize<val_t>());
        }

        template <typename val2_t>
        void pop(int const n, val2_t* v)
        {
            auto s = Stack<page_size, align_t, addr_t, val2_t>(*this);
            s.pop(n, v);

            size_t bitoff = (s.getOffset() - this->getOffset())*bitsize<align_t>() - this->pos*bitsize<val_t>() + n*bitsize<val2_t>();
            this->move(-1 - (bitoff-1)/bitsize<val_t>());
        }

        template <typename val2_t=val_t>
        void push(val2_t v)
        {
            this->push(1, &v);
        }

        template <typename val2_t=val_t>
        val2_t pop(void)
        {
            val2_t v;
            this->pop(1, &v);
            return v;
        }

    protected:
        int pos;
};

template <size_t page_size, typename align_t, typename addr_t, typename val_t, typename val2_t=val_t>
void operator >> (val2_t v, Stack<page_size, align_t, addr_t, val_t>& s)
{
    s.template push<val2_t>(v);
}

template <size_t page_size, typename align_t, typename addr_t, typename val_t, typename val2_t=val_t>
void operator << (Stack<page_size, align_t, addr_t, val_t>& s, val2_t v)
{
    s.template push<val2_t>(v);
}

template <size_t page_size, typename align_t, typename addr_t, typename val_t, typename val2_t=val_t>
void operator >> (Stack<page_size, align_t, addr_t, val_t>& s, val2_t& v)
{
    v = s.template pop<val2_t>();
}

template <size_t page_size, typename align_t, typename addr_t, typename val_t, typename val2_t=val_t>
void operator << (val2_t& v, Stack<page_size, align_t, addr_t, val_t>& s)
{
    v = s.template pop<val2_t>();
}

}; // namespace accessors

}; // namespace memory

}; // namespace giecs

