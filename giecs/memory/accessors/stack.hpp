
#pragma once

#include <giecs/memory/reference.hpp>
#include <giecs/memory/accessor.hpp>
#include <giecs/memory/accessors/linear.hpp>

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
        Stack(Context<page_size, align_t> const& context_, Reference const ref_= {0,0}, int const pos_=0)
            : Linear<page_size, align_t, addr_t, val_t, val_t*, int>::Linear(context_, ref_), pos(pos_)
        {}

        template <typename val2_t>
        Stack(Stack<page_size, align_t, addr_t, val2_t> const& s, int const offset=0, int const pos_=0)
            : Linear<page_size, align_t, addr_t, val_t, val_t*, int>::Linear(s, s.alignOffset(s.pos)+offset), pos(pos_)
        {}

        void move(int const off)
        {
            this->pos += off;
        }

        void push(int const n, val_t const* const v)
        {
            for(int i=n-1; i >= 0; --i)
                this->write(this->pos++, v[i]);
        }

        void pop(int const n, val_t* const v)
        {
            for(int i=0; i < n; ++i)
                v[i] = this->read(--this->pos);
        }

        template <typename val2_t>
        void push(int const n, val2_t const* const v)
        {
            auto s = Stack<page_size, align_t, addr_t, val2_t>(*this);
            s.push(n, v);
            this->move(1+this->valueOffset(s.alignOffset(n)-1));
        }

        template <typename val2_t>
        void pop(int const n, val2_t* v)
        {
            auto s = Stack<page_size, align_t, addr_t, val2_t>(*this);
            s.pop(n, v);
            this->move(-1-this->valueOffset(s.alignOffset(n)-1));
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

} // namespace accessors

} // namespace memory

} // namespace giecs

