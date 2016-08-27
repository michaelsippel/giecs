#pragma once

#include <cstdint>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

namespace giecs
{

template <int N> struct bittype_tag
{
    typedef uintmax_t type;
};

#define GEN_BITTYPE_TAG(z, n, data) \
	template <> struct bittype_tag< BOOST_PP_TUPLE_ELEM(2, 0, data) + n > {typedef BOOST_PP_TUPLE_ELEM(2, 1, data) type;};

BOOST_PP_REPEAT(8, GEN_BITTYPE_TAG, (0x01, uint8_t));
BOOST_PP_REPEAT(8, GEN_BITTYPE_TAG, (0x09, uint16_t));
BOOST_PP_REPEAT(16, GEN_BITTYPE_TAG, (0x11, uint32_t));
BOOST_PP_REPEAT(32, GEN_BITTYPE_TAG, (0x21, uint64_t));

template <int N>
class Bits
{
    public:
        Bits()
        {
            this->value = 0;
        }

        Bits(Bits<N> const& b)
        {
            this->value = b.value;
        }

        template <typename T>
        Bits(T v = 0)
        {
            this->value = v;
        }

        template <typename T>
        Bits& operator=(T v)
        {
            this->value = v;
            return *this;
        }

        template <typename T>
        operator T () const
        {
            return this->value & ((1 << N) - 1);
        }

    private:
        typename bittype_tag<N>::type value;
};

} // namespace giecs

