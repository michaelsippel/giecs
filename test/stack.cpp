#include <boost/test/unit_test.hpp>

#include <boost/range/adaptor/reversed.hpp>
#include <array>

#include <giecs/bits.h>
#include <giecs/types.h>
#include <giecs/memory/context.h>
#include <giecs/memory/accessors/stack.h>
#include <giecs/core.h>

BOOST_AUTO_TEST_SUITE(stack);

using namespace giecs;

typedef Bits<8> byte;
typedef Bits<32> word;
typedef Int<32> iword;
typedef memory::accessors::Stack<8, byte, iword, word> Stack;

BOOST_AUTO_TEST_CASE(pushpop)
{
    auto c1 = memory::Context<8, byte>();
    Stack s1 = Stack(c1);

    std::array<word, 5> data1{{123, 984, 2, 46911, 567}};
    std::array<word, 3> data2{{99, 43, 23}};
    std::array<word, 7> data3{{2,3,4,5,6,7,8}};

    Stack s3 = Stack(s1);
    for(word a : data3)
    {
        s3.push(a);
        s1.move(1);
    }

    for(word a : data1)
    {
        s1.push(a);
        s3.move(1);
    }

    Stack s2 = Stack(s1);

    for(word a : boost::adaptors::reverse(data1))
        BOOST_CHECK( a == s1.pop() );
    for(word a : boost::adaptors::reverse(data1))
        BOOST_CHECK( a == s2.pop() );
    for(word a : boost::adaptors::reverse(data1))
        BOOST_CHECK( a == s3.pop() );

    for(word a : data2)
    {
        s2.push(a);
        s1.move(1);
        s3.move(1);
    }

    for(word a : boost::adaptors::reverse(data2))
        BOOST_CHECK( a == s1.pop() );
    for(word a : boost::adaptors::reverse(data2))
        BOOST_CHECK( a == s2.pop() );
    for(word a : boost::adaptors::reverse(data2))
        BOOST_CHECK( a == s3.pop() );

    for(word a : boost::adaptors::reverse(data3))
        BOOST_CHECK( a == s1.pop() );
    for(word a : boost::adaptors::reverse(data3))
        BOOST_CHECK( a == s2.pop() );
    for(word a : boost::adaptors::reverse(data3))
        BOOST_CHECK( a == s3.pop() );
}

BOOST_AUTO_TEST_SUITE_END();

