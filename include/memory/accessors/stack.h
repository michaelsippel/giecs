
#pragma once

#include <memory/accessor.h>
#include <memory/accessors/linear.h>

#include <functional>
#include <boost/type_index.hpp>

namespace giecs
{
namespace memory
{
namespace accessors
{

template <typename addr_t, typename val_t>
class Stack : public Linear<addr_t, val_t>
{
        template <typename, typename> friend class Stack;

    public:
        Stack(Context const* const context_)
            : Linear<addr_t, val_t>::Linear(context_)
        {
            this->pos = 0;
        }

        Stack(Context const* const context_, addr_t offset_)
            : Linear<addr_t, val_t>::Linear(context_, offset_)
        {
            this->pos = 0;
        }

        template <typename val2_t>
        Stack(Stack<addr_t, val2_t> const& s)
            : Linear<addr_t, val_t>::Linear(s)
        {
            this->pos = 1 + (s.pos * bitsize<val2_t>() - 1) / bitsize<val_t>();
        }

        void move(int const off)
        {
            this->pos += off;
            printf("move %d\n", off);
        }

        void setFrame(void)
        {
            this->offset += this->pos;
            this->pos = 0;
        }

        void push(int const n, val_t* v)
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
            auto s = Stack<addr_t, val2_t>(*this);
            s.push(n, v);

            this->move(1 + (n*bitsize<val2_t>() - 1)/bitsize<val_t>());
        }

        template <typename val2_t>
        void pop(int const n, val2_t* v)
        {
            auto s = Stack<addr_t, val2_t>(*this);
            s.pop(n, v);

            this->move(-1 - (n*bitsize<val2_t>() - 1)/bitsize<val_t>());
        }

        template <typename val2_t=val_t>
        void push(val2_t v)
        {
            this->push(1, &v);
        }

        template <typename val2_t=val_t>
        val2_t pop(void)
        {
            val_t v;
            this->pop(1, &v);
            return v;
        }

    protected:
        int pos;
};

template <typename addr_t, typename val_t, typename val2_t=val_t>
void operator >> (val2_t v, Stack<addr_t, val_t>& s)
{
    s.push(v);
}

template <typename addr_t, typename val_t, typename val2_t=val_t>
void operator << (Stack<addr_t, val_t>& s, val2_t v)
{
    s.push(v);
}

template <typename addr_t, typename val_t, typename val2_t=val_t>
void operator >> (Stack<addr_t, val_t>& s, val2_t& v)
{
    v = s.pop();
}

template <typename addr_t, typename val_t, typename val2_t=val_t>
void operator << (val2_t& v, Stack<addr_t, val_t>& s)
{
    v = s.pop();
}

}; // namespace accessors

}; // namespace memory

}; // namespace giecs

