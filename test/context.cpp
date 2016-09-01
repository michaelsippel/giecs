#include <boost/test/unit_test.hpp>

#include <bits.h>
#include <memory/context.h>
#include <memory/accessor.h>

using namespace giecs;

BOOST_AUTO_TEST_SUITE(context);

BOOST_AUTO_TEST_CASE(accessor)
{
    memory::Context* c1 = new memory::Context();

    typedef Bits<6> byte;
    typedef Bits<24> word;

    struct Index
    {
        int x,y;
        Index()
            :x(0),y(0)
        {}

        Index(int x_, int y_)
            : x(x_), y(y_)
        {
            this->y += this->x / 4;
            this->x %= 4;
        }

        Index operator+(Index const& i) const
        {
            return Index(this->x+i.x, this->y+i.y);
        }

        bool operator<(Index const& i) const
        {
            return ((int)*this  < (int)i);
        }

        Index& operator++()
        {
            *this = Index(this->x+1, this->y);
            return *this;
        }

        operator int() const
        {
            return (this->x + this->y*4);
        }
    };

    struct Array
    {
        byte* ptr;

        Array(void const* ptr_)
            :ptr((byte*)ptr_)
        {}

        byte& operator[] (Index const& i) const
        {
            return this->ptr[(int)i];
        }

        byte& operator* () const
        {
            return *this->ptr;
        }

        Array& operator++()
        {
            this->ptr++;
            return *this;
        }
    };

    auto acc = memory::accessors::Linear<Index, byte, Array, Index>(c1);

    Array buf = Array(malloc(0x1000));
    Index len = Index(0,4);

    int a = 0;
    for(Index i = Index(); i < len; ++i)
    {
        ++a;
        buf[i] = a;
    }

    Index l = acc.write(Index(), len, buf);

    a = 0;
    for(Index i = Index(); i < len; ++i)
    {
        ++a;
        byte b = acc[i];
        BOOST_CHECK( b == a );
    }

    delete c1;
}

BOOST_AUTO_TEST_SUITE_END();

