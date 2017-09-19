
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
class Deque : public Linear<page_size, align_t, addr_t, val_t, val_t*, std::size_t>
{
        template <std::size_t, typename, typename, typename> friend class Deque;

    public:
        using value_type = val_t;
        using size_type = std::size_t;
        using reference = value_type&;
        using const_reference = value_type const&;

        Deque(Context<page_size, align_t> const& context_, Reference ref_= {0,0}, addr_t back_ptr_=0, addr_t front_ptr_=0)
            : Linear<page_size, align_t, addr_t, val_t, val_t*, std::size_t>::Linear(context_, ref_), back_ptr(back_ptr_), front_ptr(front_ptr_)
        {}

        template <typename val2_t>
        Deque(Deque<page_size, align_t, addr_t, val2_t> const& s, std::ptrdiff_t offset=0, addr_t back_ptr_=0, addr_t front_ptr_=0)
            : Linear<page_size, align_t, addr_t, val_t, val_t*, std::size_t>::Linear(s, s.alignOffset(std::ptrdiff_t(s.pos))+offset), back_ptr(back_ptr_), front_ptr(front_ptr_)
        {}

        std::size_t size(void) const
        {
            return std::size_t(this->back_ptr - this->front_ptr);
        }

        bool empty(void) const
        {
            return (this->size() == 0);
        }

        val_t const& front(void) const
        {
            return (*this)[this->front_ptr];
        }

        val_t& front(void)
        {
            return (*this)[this->front_ptr];
        }

        val_t const& back(void) const
        {
            return (*this)[this->back_ptr-1];
        }

        val_t& back(void)
        {
            return (*this)[this->back_ptr-1];
        }

        void push_front(std::size_t n, val_t const* v)
        {
            for(std::size_t i=0; i < n; ++i)
                this->write(--this->front_ptr, v[i]);
        }

        void push_back(std::size_t n, val_t const* v)
        {
            for(std::size_t i=n; i > 0; ++this->back_ptr)
                this->write(this->back_ptr, v[--i]);
        }

        void pop_front(std::size_t n, val_t* v)
        {
            for(std::size_t i=n; i > 0; ++this->front_ptr)
                v[--i] = this->read(this->front_ptr);
        }

        void pop_back(std::size_t n, val_t* v)
        {
            for(std::size_t i=0; i < n; ++i)
                v[i] = this->read(--this->back_ptr);
        }

        template <typename val2_t>
        void push_front(std::size_t n, val2_t const* val)
        {
            auto s = Deque<page_size, align_t, addr_t, val2_t>(*this);
            s.push_front(n, val);
            this->front_ptr += this->diff<val2_t>(-n);
        }

        template <typename val2_t>
        void push_back(std::size_t n, val2_t const* val)
        {
            auto s = Deque<page_size, align_t, addr_t, val2_t>(*this);
            s.push_front(n, val);
            this->back_ptr += this->diff<val2_t>(n);
        }

        template <typename val2_t>
        void pop_front(std::size_t n, val2_t* val)
        {
            auto s = Deque<page_size, align_t, addr_t, val2_t>(*this);
            s.pop_front(n, val);
            this->front_ptr += this->diff<val2_t>(n);
        }

        template <typename val2_t>
        void pop_back(std::size_t n, val2_t* val)
        {
            auto s = Deque<page_size, align_t, addr_t, val2_t>(*this);
            s.pop_front(n, val);
            this->back_ptr += this->diff<val2_t>(-n);
        }

        template <typename val2_t=val_t>
        void push_front(val2_t const& val)
        {
            this->push_front(1, &val);
        }

        template <typename val2_t=val_t>
        void push_back(val2_t const& val)
        {
            this->push_back(1, &val);
        }

        template <typename val2_t=val_t>
        val2_t pop_front(void)
        {
            val2_t val;
            this->pop_back(1, &val);
            return val;
        }

        template <typename val2_t=val_t>
        val2_t pop_back(void)
        {
            val2_t val;
            this->pop_back(1, &val);
            return val;
        }

    private:
        addr_t back_ptr;
        addr_t front_ptr;

        template <typename val2_t=val_t>
        addr_t diff(std::ptrdiff_t off)
        {
#define SIGN ((off>0)-(off<0))
            std::ptrdiff_t vo = this->valueOffset(Linear<page_size, align_t, addr_t, val2_t>::alignOffset(off-SIGN))+SIGN;
            return addr_t(vo);
        }
};

} // namespace accessors

} // namespace memory

} // namespace giecs

