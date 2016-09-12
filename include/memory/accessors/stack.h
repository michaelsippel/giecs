
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
        /*
        		template<typename addr2_t, typename val2_t>
        		Stack(Stack<addr2_t, val2_t> const s)
        		{
        			// TODO
        		}
        */

        void move(int const off)
        {
            this->pos += off;
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

        void push(val_t v)
        {
            this->push(1, &v);
        }

        val_t pop(void)
        {
            val_t v;
            this->pop(1, &v);
            return v;
        }

    private:
        int pos;
};

template <typename addr_t, typename val_t>
void operator >> (val_t v, Stack<addr_t, val_t>& s)
{
    s.push(v);
}

template <typename addr_t, typename val_t>
void operator << (Stack<addr_t, val_t>& s, val_t v)
{
    s.push(v);
}

template <typename addr_t, typename val_t>
void operator >> (Stack<addr_t, val_t>& s, val_t& v)
{
    v = s.pop();
}

template <typename addr_t, typename val_t>
void operator << (val_t& v, Stack<addr_t, val_t>& s)
{
    v = s.pop();
}

}; // namespace accessors

}; // namespace memory

}; // namespace giecs

