#include <boost/test/unit_test.hpp>

#include <giecs/bits.hpp>
#include <giecs/types.hpp>
#include <giecs/memory/context.hpp>
#include <giecs/memory/accessors/stack.hpp>
#include <giecs/core.hpp>

BOOST_AUTO_TEST_SUITE(core);

using namespace giecs;

typedef Bits<8> byte;
typedef Bits<32> word;
typedef Int<32> iword;
typedef memory::accessors::Stack<8, byte, iword, word> Stack;

void addi(Stack& stack)
{
    iword a = stack.pop<word>();
    iword b = stack.pop<word>();
    iword c = a + b;
    stack.push<word>(c);
}

BOOST_AUTO_TEST_CASE(eval)
{
    auto c1 = memory::Context<8, byte>();
    auto core = Core<8, byte, iword>();
    core.addOperation(1, addi);

    Stack stack = c1.createStack<iword, word>();

    // function to add 10 to parameter
    stack[333] = 2;   // length = 2 words
    stack[334] = 1;   // addr of addi
    stack[335] = 10;  // 10

    for(iword a = 0; a < 10; ++a)
    {
        stack << word(a);
        stack << word(333);
        core.eval(stack);

        word b = stack.pop();
        BOOST_CHECK( (a+10) == b );
    }
}

BOOST_AUTO_TEST_SUITE_END();

