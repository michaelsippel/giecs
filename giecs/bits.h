#pragma once

#include <array>
#include <cstdint>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

#include <giecs/memory/block.h>

namespace giecs
{

template <int N> struct bittype_tag
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
            this->value = *reinterpret_cast<size_t*>(&v);
        }

        template <typename T>
        operator T () const
        {
            size_t a = this->value & ((1 << N) - 1);
            T r = *reinterpret_cast<T*>(&a);
            return r;
        }

        operator Bits& () const
        {
            return *this;
        }

        template <typename T>
        bool operator == (T const& v) const
        {
            return (size_t(*this) == size_t(v));
        }

        Bits operator ~ () const
        {
            return Bits(~this->value);
        }

#define OPERATOR(op) \
		template <typename T> \
		Bits operator op (T const v) const \
		{ \
			return Bits(this->value op v); \
		} \
		Bits operator op (Bits const v) const \
		{ \
			return Bits(this->value op v.value); \
		} \

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
		} \

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
constexpr size_t bitsize(void)
{
    return sizeof(T) * 8;
}

#define DEF_BITSIZE(z, N, data) \
	template <> constexpr size_t bitsize< Bits<N> >(void) { return N; }

BOOST_PP_REPEAT(64, DEF_BITSIZE,)

#undef DEF_BITSIZE

template <typename T>
constexpr size_t bitsize(T)
{
    return bitsize<T>();
}

template <int N>
size_t hash_value(Bits<N> const& a)
{
    return size_t(a);
}

namespace memory
{
/*  E X A M P L E
 *  page_size = 2
 *  block_size = 2
 *                          offset = 1 (x align_t)
 *                          V
 *          |     buf[0]    |      buf[1]   |
 *   – – – –|– – – – – – – –|– – – – – – – –|– – – –   buf (align_t)
 *          |            P A G E            |
 *  –––––––––––––––––––––––––––––––––––––––––––––––––
 *  |       B L O C K       |       B L O C K       |
 *  |– – – – – –|– – – – – –|– – – – – –|– – – – – –|  block (val_t)
 *  |block[i-1] | block[i]  |  block[0] | block[1]  |
 *
 *  ^       ^ boff = 8 bits ^
 *  |       |_ _ _ _ _ _ _ _|
 *  |
 *  |           ^ ioff = 1  ^
 *  |           | i = 1     |
 *  |       ^   ^
 *  |       |_ _| bitoff = 2 bits
 *  |
 *  block_id = -1
 */

template <size_t page_size, typename align_t, typename val_t>
void read_block(TypeBlock<page_size, align_t, val_t> const& b, int i, int const end, std::array<align_t, page_size>& buf, int off, int bitoff)
{
    constexpr int N_align = bitsize<align_t>();
    constexpr int N_val = bitsize<val_t>();
    TypeBlock<page_size, Bits<N_align>, Bits<N_val>> const block(b);

    if(off < 0)
    {
        i -= (off * N_align + bitoff) / N_val;
        off = 0;
    }

    while(off < (int)page_size && i <= end)
    {
        Bits<N_align> a = Bits<N_align>();

        if(bitoff > 0 && i > 0)
            a = Bits<N_align>(block[i-1] >> (N_val - bitoff));

        while(bitoff < N_align && i < end)
        {
            if(bitoff < 0)
                a = Bits<N_align>(block[i++] >> (-bitoff));
            else
                a |= Bits<N_align>(block[i++]) << bitoff;
            bitoff += N_val;
        }
        bitoff -= N_align;
        buf[off++] |= a;
    }
}

template <size_t page_size, typename align_t, typename val_t>
void write_block(TypeBlock<page_size, align_t, val_t> const& b, int i, int const end, std::array<align_t, page_size> const& buf, int off, int bitoff, std::pair<int, int> range)
{
    constexpr int N_align = bitsize<align_t>();
    constexpr int N_val = bitsize<val_t>();
    TypeBlock<page_size, Bits<N_align>, Bits<N_val>> const block(b);

    if(off < 0)
    {
        i -= (off * N_align + bitoff) / N_val;
        off = 0;
    }

    int tbitoff = bitoff;
    while(off < (int)page_size && i <= end)
    {
        Bits<N_align> a = buf[off++];

        if(bitoff > 0 && i > 0 && tbitoff >= range.first && tbitoff < range.second)
            block[i-1] |= Bits<N_val>(a) << (N_val - bitoff);

        while(bitoff < N_align && i < end)
        {
            if(tbitoff >= range.first && tbitoff < range.second)
            {
                if(bitoff < 0)
                    block[i] |= Bits<N_val>(a) << (-bitoff);
                else
                    block[i] |= Bits<N_val>(a >> bitoff);
            }

            ++i;
            bitoff += N_val;
            tbitoff += N_val;
        }
        bitoff -= N_align;
    }
}

} // namespace memory

} // namespace giecs

