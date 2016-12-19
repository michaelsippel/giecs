#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <cstdint>

#include <giecs/bits.h>

using namespace giecs;

BOOST_AUTO_TEST_SUITE(bits);

BOOST_AUTO_TEST_CASE(length)
{
    BOOST_CHECK( sizeof(uint8_t) == sizeof(Bits<8>) );
    BOOST_CHECK( sizeof(uint16_t) == sizeof(Bits<16>) );
    BOOST_CHECK( sizeof(uint32_t) == sizeof(Bits<32>) );
}

BOOST_AUTO_TEST_CASE(overflow)
{
    Bits<14> a;
    const int max = (1 << 14) - 1;

    a = max - 1;
    BOOST_CHECK(a == max - 1);

    a = max;
    BOOST_CHECK(a == max);

    a = max + 1;
    BOOST_CHECK(a == 0);
}

BOOST_AUTO_TEST_SUITE_END();

