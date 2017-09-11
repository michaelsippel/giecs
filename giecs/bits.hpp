#pragma once

#pragma GCC diagnostic ignored "-Wshift-count-overflow"
#pragma GCC diagnostic ignored "-Woverflow"

#include <array>
#include <cstdint>
#include <cstddef>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

#include <giecs/memory/block.hpp>

namespace giecs
{

template <std::size_t N>
struct bittype_tag
{
    typedef uintmax_t type;
};

#define GEN_BITTYPE_TAG(z, n, data) \
	template <> struct bittype_tag< BOOST_PP_TUPLE_ELEM(2, 0, data) + n > {typedef BOOST_PP_TUPLE_ELEM(2, 1, data) type;};

BOOST_PP_REPEAT(8, GEN_BITTYPE_TAG, (0x01, uint8_t))
BOOST_PP_REPEAT(8, GEN_BITTYPE_TAG, (0x09, uint16_t))
BOOST_PP_REPEAT(16, GEN_BITTYPE_TAG, (0x11, uint32_t))
BOOST_PP_REPEAT(32, GEN_BITTYPE_TAG, (0x21, uint64_t))

#undef GEN_BITTYPE_TAG

template <std::size_t N>
class Bits
{
    public:
        Bits()
        {
            this->value = 0;
        }

        template <std::size_t N2>
        Bits(Bits<N2> const& b)
        {
            this->value = b.getValue();
        }

        template <typename T>
        Bits(T v = 0)
        {
            this->value = reinterpret_cast<typename bittype_tag<N>::type&>(v);
        }

        inline typename bittype_tag<N>::type getValue(void) const
        {
            return (this->value & std::intmax_t((1 << N) - 1));
        }

        template <typename T>
        operator T () const
        {
            uintmax_t a = this->getValue();
            T r = reinterpret_cast<T&>(a);
            return r;
        }

        operator Bits& () const
        {
            return *this;
        }

        template <typename T>
        bool operator == (T const& v) const
        {
            return (std::uintmax_t(this->getValue()) == std::uintmax_t(v));
        }

        Bits operator ~ () const
        {
            return Bits(~this->getValue());
        }

#define OPERATOR(op) \
        template <typename T> \
        Bits operator op (T const v) const \
        { \
            return Bits(this->getValue() op v); \
        } \
        Bits operator op (Bits const v) const \
        { \
            return Bits(this->getValue() op v.getValue()); \
        }

#define OPERATOR_EQ(op) \
        template <typename T> \
        Bits& operator op (T const v) \
        { \
            this->value op v; \
            return *this; \
        } \
        Bits& operator op (Bits const v) \
        { \
            this->value op v.value; \
            return *this; \
        }

        OPERATOR(&)
        OPERATOR(|)
        OPERATOR(^)
        OPERATOR(<<)
        OPERATOR(>>)

        OPERATOR_EQ(=)
        OPERATOR_EQ(&=)
        OPERATOR_EQ(|=)
        OPERATOR_EQ(^=)
        OPERATOR_EQ(<<=)
        OPERATOR_EQ(>>=)

#undef OPERATOR
#undef OPERATOR_EQ

    protected:
        typename bittype_tag<N>::type value;
};

template <typename T>
constexpr std::size_t bitsize(void)
{
    return sizeof(T) * 8;
}

#define DEF_BITSIZE(z, N, data) \
  template <> constexpr std::size_t bitsize< Bits<N> >(void) { return N; }

BOOST_PP_REPEAT(64, DEF_BITSIZE,)

#undef DEF_BITSIZE

template <typename T>
constexpr std::size_t bitsize(T)
{
    return bitsize<T>();
}

template <std::size_t N>
std::size_t hash_value(Bits<N> const& a)
{
    return size_t(a);
}

namespace memory
{

template <std::size_t page_size, typename align_t, typename val_t, typename cs>
void read_block(TypeBlock<page_size, align_t, val_t, cs> const& b, std::ptrdiff_t i, std::ptrdiff_t const end, std::array<align_t, page_size>& buf, std::ptrdiff_t off)
{
    constexpr std::size_t N_align = bitsize<align_t>();
    constexpr std::size_t N_val = bitsize<val_t>();
    TypeBlock<page_size, align_t, Bits<N_val>, cs> const& block = reinterpret_cast<TypeBlock<page_size, align_t, Bits<N_val>, cs> const&>(b);

    int bitoff = 0;
    if(off < 0)
    {
        int z = off * N_align;
        i -= z / N_val;
        bitoff += z % N_val;
        off = 0;
    }

    while(off < std::ptrdiff_t(page_size) && i <= end)
    {
        Bits<N_align> a = Bits<N_align>();

        if(bitoff > 0 && i > 0)
            a = Bits<N_align>(block[i-1] >> (N_val - bitoff));

        while(bitoff < int(N_align) && i < end)
        {
            if(bitoff < 0)
                a = Bits<N_align>(block[i] >> (-bitoff));
            else
                a |= Bits<N_align>(block[i]) << bitoff;

            i++;
            bitoff += N_val;
        }
        bitoff -= N_align;
        buf[off++] |= align_t(a);
    }
}

template <std::size_t page_size, typename align_t, typename val_t, typename cs>
void write_block(TypeBlock<page_size, align_t, val_t, cs> const& b, std::ptrdiff_t i, std::ptrdiff_t const end, std::array<align_t, page_size> const& buf, std::ptrdiff_t off)
{
    constexpr std::size_t N_align = bitsize<align_t>();
    constexpr std::size_t N_val = bitsize<val_t>();
    TypeBlock<page_size, align_t, Bits<N_val>, cs> const& block = reinterpret_cast<TypeBlock<page_size, align_t, Bits<N_val>, cs> const&>(b);

    int bitoff = 0;
    int tbitoff = off * N_align + bitoff;

    if(off < 0)
    {
        i -= tbitoff / N_val;
        bitoff += tbitoff % N_val;
        off = 0;
        tbitoff = -bitoff;
    }

    Bits<N_val> mask = -1;
    if(N_val > N_align)
        mask = (1 << N_align)-1;

    while(off < std::ptrdiff_t(page_size) && i <= end)
    {
        Bits<N_align> a(buf[off++]);
        if(bitoff > 0 && i > 0)// && tbitoff >= range.first && tbitoff < range.second)
        {
            block[i-1] &= ~(mask << (N_val-bitoff) );
            block[i-1] |= Bits<N_val>(a) << (N_val-bitoff);
        }

        while(bitoff < int(N_align) && i < end)
        {
            // if(tbitoff >= range.first && tbitoff < range.second)
            {
                if(bitoff < 0)
                {
                    block[i] &= ~(mask >> (-bitoff));
                    block[i] |= Bits<N_val>(a >> (-bitoff));
                }
                else
                {
                    block[i] &= ~( mask << bitoff );
                    block[i] |= Bits<N_val>(a) << bitoff;
                }
            }

            ++i;
            bitoff += N_val;
        }
        bitoff -= N_align;
        tbitoff += N_align;
    }
}

} // namespace memory

} // namespace giecs

