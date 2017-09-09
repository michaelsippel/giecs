
#pragma once

#include <cstddef>

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

template <std::size_t page_size, typename align_t, typename addr_t, typename val_t>
class Stack : public Linear<page_size, align_t, addr_t, val_t, val_t*, std::size_t>
{
        template <std::size_t, typename, typename, typename> friend class Stack;

    public:
        Stack(Context<page_size, align_t> const& context_, Reference ref_= {0,0}, addr_t pos_=0)
            : Linear<page_size, align_t, addr_t, val_t, val_t*, std::size_t>::Linear(context_, ref_), pos(pos_)
        {}

        template <typename val2_t>
        Stack(Stack<page_size, align_t, addr_t, val2_t> const& s, std::ptrdiff_t offset=0, addr_t pos_=0)
            : Linear<page_size, align_t, addr_t, val_t, val_t*, std::size_t>::Linear(s, s.alignOffset(std::ptrdiff_t(s.pos))+offset), pos(pos_)
        {}

        template <typename val2_t=val_t>
        void move(std::ptrdiff_t off)
        {
#define SIGN ((off>0)-(off<0))
            std::ptrdiff_t vo = this->valueOffset(Stack<page_size, align_t, addr_t, val2_t>::alignOffset(off-SIGN))+SIGN;
            this->pos += addr_t(vo);
        }

        void push(std::size_t n, val_t const* v)
        {
            for(std::size_t i=n; i > 0; ++this->pos)
                this->write(this->pos, v[--i]);
        }

        void pop(std::size_t n, val_t* v)
        {
            for(std::size_t i=0; i < n; ++i)
                v[i] = this->read(--this->pos);
        }

        template <typename val2_t>
        void push(std::size_t n, val2_t const* v)
        {
            auto s = Stack<page_size, align_t, addr_t, val2_t>(*this);
            s.push(n, v);
            this->move<val2_t>(n);
        }

        template <typename val2_t>
        void pop(std::size_t n, val2_t* v)
        {
            auto s = Stack<page_size, align_t, addr_t, val2_t>(*this);
            s.pop(n, v);
            this->move<val2_t>(-n);
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

        addr_t pos;
};

template <std::size_t page_size, typename align_t, typename addr_t, typename val_t, typename val2_t=val_t>
void operator >> (val2_t v, Stack<page_size, align_t, addr_t, val_t>& s)
{
    s.template push<val2_t>(v);
}

template <std::size_t page_size, typename align_t, typename addr_t, typename val_t, typename val2_t=val_t>
void operator << (Stack<page_size, align_t, addr_t, val_t>& s, val2_t v)
{
    s.template push<val2_t>(v);
}

template <std::size_t page_size, typename align_t, typename addr_t, typename val_t, typename val2_t=val_t>
void operator >> (Stack<page_size, align_t, addr_t, val_t>& s, val2_t& v)
{
    v = s.template pop<val2_t>();
}

template <std::size_t page_size, typename align_t, typename addr_t, typename val_t, typename val2_t=val_t>
void operator << (val2_t& v, Stack<page_size, align_t, addr_t, val_t>& s)
{
    v = s.template pop<val2_t>();
}

} // namespace accessors

} // namespace memory

} // namespace giecs

