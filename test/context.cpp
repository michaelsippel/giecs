#include <boost/test/unit_test.hpp>

#include <giecs/bits.h>
#include <giecs/memory/context.h>
#include <giecs/memory/accessor.h>
#include <giecs/memory/accessors/linear.h>

BOOST_AUTO_TEST_SUITE(context);

using namespace giecs;

BOOST_AUTO_TEST_CASE(accessor)
{
    typedef Bits<6> byte;
    typedef Bits<24> word;

    auto c1 = new memory::Context<4, word>();

    struct Index
    {
        int x,y;
        Index()
            :x(0),y(0)
        {}

        Index(int x)
            : Index(x, 0)
        {
        }

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

    auto acc = c1->createLinear<Index, byte, Array, Index>();

    Index len = Index(0,4);

    int a = 0;
    for(Index i = Index(); i < len; ++i)
    {
        ++a;
        acc[i] = a;
    }

    a = 0;
    for(Index i = Index(); i < len; ++i)
    {
        ++a;
        byte b = acc[i];
        BOOST_CHECK( b == a );
    }

    delete c1;
}
/*
BOOST_AUTO_TEST_CASE(synchronization)
{
    memory::Context* c = new memory::Context();

    auto acc1 = memory::accessors::Linear<int, uint8_t>(c);
    auto acc2 = memory::accessors::Linear<int, uint16_t>(c);

    static int const n = 4;

    for(int i = 0; i < n; i++)
    {
        acc2[i] = 0;
    }

    uint8_t v8 = 0;
    for(int i = 0; i < 2*n; i++)
    {
        ++v8;
        acc1[i] = v8;
    }

    v8 = 0;
    for(int i = 0; i < n; i++)
    {
        uint16_t v16 = 0;

        ++v8;
        BOOST_CHECK(acc1[i*2] == v8);
        v16 = v8;

        printf("%x ", v8);

        ++v8;
        BOOST_CHECK(acc1[i*2+1] == v8);
        v16 |= v8 << 8;

        uint16_t r16 = acc2[i];

        printf("%x -> %x == %x\n", v8, v16, r16);
        BOOST_CHECK(acc2[i] == v16);
    }

    delete c;
}
*/
BOOST_AUTO_TEST_SUITE_END();

