#include <boost/test/unit_test.hpp>

#include <boost/range/adaptor/reversed.hpp>
#include <array>

#include <giecs/bits.hpp>
#include <giecs/types.hpp>
#include <giecs/memory/context.hpp>
#include <giecs/memory/accessors/stack.hpp>
#include <giecs/memory/accessors/queue.hpp>

BOOST_AUTO_TEST_SUITE(deque);

using namespace giecs;

typedef Bits<8> byte;
typedef Bits<32> word;
typedef memory::accessors::Stack<8, byte, word> Stack;

BOOST_AUTO_TEST_CASE(queue)
{
    auto c1 = memory::Context<8, byte>();
    auto d1 = memory::accessors::Deque<8, byte, std::size_t, word>(c1);

    memory::accessors::Queue<8, byte, word> queue(d1);

    queue.push(123);
    queue.push(54);
    queue.push(98);

    BOOST_CHECK( queue.front() == 123 );
    BOOST_CHECK( queue.back() == 98 );
}

BOOST_AUTO_TEST_CASE(stack)
{
    auto c1 = memory::Context<8, byte>();
    auto d1 = memory::accessors::Deque<8, byte, std::size_t, word>(c1);

    std::array<word, 5> data1{{123, 984, 2, 46911, 567}};
    std::array<word, 3> data2{{99, 43, 23}};
    std::array<word, 7> data3{{2,3,4,5,6,7,8}};

    Stack s1 = Stack(d1);
    Stack s3 = Stack(s1);
    for(word a : data3)
    {
        s1.push(0);
        s3.push(a);
    }

    for(word a : data1)
    {
        s3.push(0);
        s1.push(a);
    }

    Stack s2 = Stack(s1);

    for(word a : boost::adaptors::reverse(data1))
    {
        BOOST_CHECK( a == s1.top() );
        s1.pop();
    }
    for(word a : boost::adaptors::reverse(data1))
    {
        BOOST_CHECK( a == s2.top() );
        s2.pop();
    }
    for(word a : boost::adaptors::reverse(data1))
    {
        BOOST_CHECK( a == s3.top() );
        s3.pop();
    }

    for(word a : data2)
    {
        s1.push(0);
        s3.push(0);
        s2.push(a);
    }

    for(word a : boost::adaptors::reverse(data2))
    {
        BOOST_CHECK( a == s1.top() );
        s1.pop();
    }
    for(word a : boost::adaptors::reverse(data2))
    {
        BOOST_CHECK( a == s2.top() );
        s2.pop();
    }
    for(word a : boost::adaptors::reverse(data2))
    {
        BOOST_CHECK( a == s3.top() );
        s3.pop();
    }
    for(word a : boost::adaptors::reverse(data3))
    {
        BOOST_CHECK( a == s1.top() );
        s1.pop();
    }
    for(word a : boost::adaptors::reverse(data3))
    {
        BOOST_CHECK( a == s2.top() );
        s2.pop();
    }
    for(word a : boost::adaptors::reverse(data3))
    {
        BOOST_CHECK( a == s3.top() );
        s3.pop();
    }
}

BOOST_AUTO_TEST_SUITE_END();

