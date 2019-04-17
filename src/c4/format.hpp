#ifndef _C4_FORMAT_HPP_
#define _C4_FORMAT_HPP_

/** @file format.hpp provides type-safe facilities for formatting arguments
 * to string buffers */

#include "c4/to_chars.hpp"

#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable: 4800) // forcing value to bool 'true' or 'false' (performance warning)
#   pragma warning(disable: 4996) // snprintf/scanf: this function or variable may be unsafe
#endif

namespace c4 {

/** @defgroup formatting_functions
 * Convert a sequence of values to/from a string.
 */

/** a generic class for providing custom formatting information for a type
 * @ingroup formatting_functions */
template<class T>
struct fmt_wrapper;

/** mark a variable to be written in its custom format wrapper
 * @ingroup formatting_functions */
template<class T, class... Args>
inline fmt_wrapper<T> fmt(T v, Args && ...args)
{
    return fmt_wrapper<T>(v, std::forward<Args>(args)...);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// formatting integral types


namespace detail {
template<class T>
struct int_formatter
{
    T val;
    T radix;
    int_formatter(T val_, uint8_t radix_=10) : val(val_), radix(radix_) {}
};
} // namespace detail

template<> struct fmt_wrapper<  int8_t> : public detail::int_formatter<  int8_t> { using detail::int_formatter<  int8_t>::int_formatter; };
template<> struct fmt_wrapper< int16_t> : public detail::int_formatter< int16_t> { using detail::int_formatter< int16_t>::int_formatter; };
template<> struct fmt_wrapper< int32_t> : public detail::int_formatter< int32_t> { using detail::int_formatter< int32_t>::int_formatter; };
template<> struct fmt_wrapper< int64_t> : public detail::int_formatter< int64_t> { using detail::int_formatter< int64_t>::int_formatter; };

template<> struct fmt_wrapper< uint8_t> : public detail::int_formatter< uint8_t> { using detail::int_formatter< uint8_t>::int_formatter; };
template<> struct fmt_wrapper<uint16_t> : public detail::int_formatter<uint16_t> { using detail::int_formatter<uint16_t>::int_formatter; };
template<> struct fmt_wrapper<uint32_t> : public detail::int_formatter<uint32_t> { using detail::int_formatter<uint32_t>::int_formatter; };
template<> struct fmt_wrapper<uint64_t> : public detail::int_formatter<uint64_t> { using detail::int_formatter<uint64_t>::int_formatter; };


inline size_t to_chars(substr buf, fmt_wrapper<  int8_t> fmt) { return itoa(buf, fmt.val, fmt.radix); } //!< @ingroup formatting_functions
inline size_t to_chars(substr buf, fmt_wrapper< uint8_t> fmt) { return utoa(buf, fmt.val, fmt.radix); } //!< @ingroup formatting_functions
inline size_t to_chars(substr buf, fmt_wrapper< int16_t> fmt) { return itoa(buf, fmt.val, fmt.radix); } //!< @ingroup formatting_functions
inline size_t to_chars(substr buf, fmt_wrapper<uint16_t> fmt) { return utoa(buf, fmt.val, fmt.radix); } //!< @ingroup formatting_functions
inline size_t to_chars(substr buf, fmt_wrapper< int32_t> fmt) { return itoa(buf, fmt.val, fmt.radix); } //!< @ingroup formatting_functions
inline size_t to_chars(substr buf, fmt_wrapper<uint32_t> fmt) { return utoa(buf, fmt.val, fmt.radix); } //!< @ingroup formatting_functions
inline size_t to_chars(substr buf, fmt_wrapper< int64_t> fmt) { return itoa(buf, fmt.val, fmt.radix); } //!< @ingroup formatting_functions
inline size_t to_chars(substr buf, fmt_wrapper<uint64_t> fmt) { return utoa(buf, fmt.val, fmt.radix); } //!< @ingroup formatting_functions


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// formatting real types

namespace detail {
template<class T>
struct float_formatter
{
    T val;
    int precision;
    RealFormat_e fmt;
    float_formatter(T v, int prec=-1, RealFormat_e f=FTOA_FLOAT) : val(v), precision(prec), fmt(f)  {}
};
} // namespace detail


template<> struct fmt_wrapper< float> : public detail::float_formatter< float> { using detail::float_formatter< float>::float_formatter; };
template<> struct fmt_wrapper<double> : public detail::float_formatter<double> { using detail::float_formatter<double>::float_formatter; };

inline size_t to_chars(substr buf, fmt_wrapper<   float> fmt) { return ftoa(buf, fmt.val, fmt.precision, fmt.fmt); } //!< @ingroup formatting_functions
inline size_t to_chars(substr buf, fmt_wrapper<  double> fmt) { return dtoa(buf, fmt.val, fmt.precision, fmt.fmt); } //!< @ingroup formatting_functions


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// formatting real types

/** format the argument as a hex value
 * @ingroup formatting_functions */
template<class T>
inline fmt_wrapper<T> fmthex(T v)
{
    static_assert(std::is_integral<T>::value, "must be an integral type");
    return fmt_wrapper<T>(v, T(16));
}

/** format the argument as an octal value
 * @ingroup formatting_functions */
template<class T>
inline fmt_wrapper<T> fmtoct(T v)
{
    static_assert(std::is_integral<T>::value, "must be an integral type");
    return fmt_wrapper<T>(v, T(8));
}

/** format the argument as a binary 0-1 value
 * @ingroup formatting_functions */
template<class T>
inline fmt_wrapper<T> fmtbin(T v)
{
    static_assert(std::is_integral<T>::value, "must be an integral type");
    return fmt_wrapper<T>(v, T(2));
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// writing binary data

/** @todo add custom alignment
 * @ingroup formatting_functions */
template<class T>
struct binary_wrapper
{
    T& val;
    binary_wrapper(T& v) : val(v) {}
};

/** mark a variable to be written in binary format
 * @ingroup formatting_functions  */
template<class T>
inline binary_wrapper<T> bin(T &v)
{
    return binary_wrapper<T>(v);
}

/** write a variable in binary format
 * @ingroup formatting_functions */
template<class T>
inline size_t to_chars(substr buf, binary_wrapper<T> b)
{
    if(sizeof(T) <= buf.len) memcpy(buf.str, &b.val, sizeof(T));
    return sizeof(T);
}

/** read a variable in binary format
 * @ingroup formatting_functions */
template<class T>
inline size_t from_chars(csubstr buf, binary_wrapper<T> *b)
{
    if(sizeof(T) > buf.len) return csubstr::npos;
    memcpy(&b->val, buf.str, sizeof(T));
    return sizeof(T);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// writing base64 data
// https://en.m.wikipedia.org/wiki/Base64

/** @todo add custom alignment
 * @ingroup formatting_functions */
template<typename CharOrConstChar>
struct base64_wrapper_
{
    CharOrConstChar* data;
    size_t len;
    base64_wrapper_(CharOrConstChar *data_, size_t sz) : data(data_), len(sz) {}
    static_assert(std::is_same<CharOrConstChar,char>::value || std::is_same<CharOrConstChar,const char>::value, "invalid template parameter");
};

/** mark a variable to be written in base64 format
 * @ingroup formatting_functions  */
using const_base64_wrapper = base64_wrapper_<const char>;

/** mark a variable to be read in base64 format
 * @ingroup formatting_functions  */
using base64_wrapper = base64_wrapper_<char>;

namespace detail {
struct base64
{
    constexpr static const uint32_t sextet_mask = uint32_t(1 << 6) - 1;
    constexpr static const char index_table[64] = {
        /* 0*/ 'A', /* 1*/ 'B', /* 2*/ 'C', /* 3*/ 'D', /* 4*/ 'E', /* 5*/ 'F', /* 6*/ 'G', /* 7*/ 'H',
        /* 8*/ 'I', /* 9*/ 'J', /*10*/ 'K', /*11*/ 'L', /*12*/ 'M', /*13*/ 'N', /*14*/ 'O', /*15*/ 'P',
        /*16*/ 'Q', /*17*/ 'R', /*18*/ 'S', /*19*/ 'T', /*20*/ 'U', /*21*/ 'V', /*22*/ 'W', /*23*/ 'X',
        /*24*/ 'Y', /*25*/ 'Z', /*26*/ 'a', /*27*/ 'b', /*28*/ 'c', /*29*/ 'd', /*30*/ 'e', /*31*/ 'f',
        /*32*/ 'g', /*33*/ 'h', /*34*/ 'i', /*35*/ 'j', /*36*/ 'k', /*37*/ 'l', /*38*/ 'm', /*39*/ 'n',
        /*40*/ 'o', /*41*/ 'p', /*42*/ 'q', /*43*/ 'r', /*44*/ 's', /*45*/ 't', /*46*/ 'u', /*47*/ 'v',
        /*48*/ 'w', /*49*/ 'x', /*50*/ 'y', /*51*/ 'z', /*52*/ '0', /*53*/ '1', /*54*/ '2', /*55*/ '3',
        /*56*/ '4', /*57*/ '5', /*58*/ '6', /*59*/ '7', /*60*/ '8', /*61*/ '9', /*62*/ '+', /*63*/ '/'
    };
    static const char table_idx(char c)
    {
        if(c >= 64/*A*/ && c <= 90/*Z*/)
        {
            return c - 'A';
        }
        else if(c >= 97/*a*/ && c <= 122/*z*/)
        {
            return 26 + (c - 'a');
        }
        else if(c >= 48/*0*/ && c <= 57/*9*/)
        {
            return 52 + (c - '0');
        }
        else if(c == '+')
        {
            return 62;
        }
        else if(c == '/')
        {
            return 63;
        }
        C4_NEVER_REACH();
        return 0;
    }

#define c4append_(c) { if(pos < buf.len) { buf[pos] = (c); } ++pos; }
#define c4append_idx_(char_idx) { C4_ASSERT((char_idx) < sizeof(index_table)); c4append_(index_table[(char_idx)]); }

    static size_t encode(substr buf, const_base64_wrapper data)
    {
        size_t rem, pos = 0;
        const char *C4_RESTRICT d = data.data;
        for(rem = data.len; rem >= 3; rem -= 3, d += 3)
        {
            const char vb[4] = {d[2], d[1], d[0], 0};
            uint32_t val = *reinterpret_cast<const uint32_t*>(vb);
            c4append_idx_((val >> (3 * 6)) & sextet_mask);
            c4append_idx_((val >> (2 * 6)) & sextet_mask);
            c4append_idx_((val >> (1 * 6)) & sextet_mask);
            c4append_idx_((val           ) & sextet_mask);
        }
        C4_ASSERT(rem < 3);
        if(rem == 2)
        {
            const char vb[4] = {0, d[1], d[0], 0};
            uint32_t val = *reinterpret_cast<const uint32_t*>(vb);
            c4append_idx_((val >> (3 * 6)) & sextet_mask);
            c4append_idx_((val >> (2 * 6)) & sextet_mask);
            c4append_idx_((val >> (1 * 6)) & sextet_mask);
            c4append_('=');
        }
        else if(rem == 1)
        {
            const char vb[4] = {0, 0, d[0], 0};
            uint32_t val = *reinterpret_cast<const uint32_t*>(vb);
            c4append_idx_((val >> (3 * 6)) & sextet_mask);
            c4append_idx_((val >> (2 * 6)) & sextet_mask);
            c4append_('=');
            c4append_('=');
        }
        return pos;
    }

#undef c4append_
#undef c4append_idx_
#define c4append_(c) { if(wpos < buf.len) { buf.data[wpos] = (c); } ++wpos; }

    static size_t decode(csubstr encoded, base64_wrapper buf)
    {
        C4_CHECK(encoded.len % 4 == 0);
        size_t wpos = 0;
        const char *C4_RESTRICT d = encoded.str;
        // process every quartet of input bytes --> triplet of output bytes
        for(size_t rpos = 0; rpos < encoded.len; rpos += 4, d += 4)
        {
            if(d[2] == '=' || d[3] == '=') // skip the last quartet if it is padded
            {
                C4_ASSERT(d + 4 == encoded.str + encoded.len);
                break;
            }
            uint32_t val = 0;
            val |= table_idx(d[3]);
            val |= table_idx(d[2]) << (1 * 6);
            val |= table_idx(d[1]) << (2 * 6);
            val |= table_idx(d[0]) << (3 * 6);
            c4append_((val >> (2 * 8)) & 0xff);
            c4append_((val >> (1 * 8)) & 0xff);
            c4append_((val           ) & 0xff);
        }
        // deal with the last quartet when it is padded
        if(d[2] == '=') // 2 padding chars
        {
            C4_ASSERT(d + 4 == encoded.str + encoded.len);
            C4_ASSERT(d[3] == '=');
            uint32_t val = 0;
            val |= table_idx(d[1]) << (2 * 6);
            val |= table_idx(d[0]) << (3 * 6);
            c4append_((val >> (2 * 8)) & 0xff);
        }
        else if(d[3] == '=') // 1 padding char
        {
            C4_ASSERT(d + 4 == encoded.str + encoded.len);
            uint32_t val = 0;
            val |= table_idx(d[2]) << (1 * 6);
            val |= table_idx(d[1]) << (2 * 6);
            val |= table_idx(d[0]) << (3 * 6);
            c4append_((val >> (2 * 8)) & 0xff);
            c4append_((val >> (1 * 8)) & 0xff);
        }
        return wpos;
    }

#undef c4append_

};
} // namespace detail

/** mark a variable to be written in base64 format
 * @ingroup formatting_functions */
template<class T>
inline const_base64_wrapper base64(T const& v, size_t num=1)
{
    return const_base64_wrapper(reinterpret_cast<const char*>(&v), sizeof(T) * num);
}
/** mark a variable to be written in base64 format
 * @ingroup formatting_functions */
template<class T, size_t N>
inline const_base64_wrapper base64(T const (&arr)[N])
{
    return const_base64_wrapper(reinterpret_cast<const char*>(arr), sizeof(T) * N);
}
/** mark a variable to be written in base64 format
 * @ingroup formatting_functions */
template<size_t N>
inline const_base64_wrapper base64(const char (&arr)[N])
{
    return const_base64_wrapper(arr, N);
}
/** mark a variable to be written in base64 format
 * @ingroup formatting_functions */
inline const_base64_wrapper base64(csubstr s)
{
    return const_base64_wrapper(s.str, s.len);
}
/** mark a variable to be written in base64 format
 * @ingroup formatting_functions */
inline const_base64_wrapper base64(const char* s)
{
    return const_base64_wrapper(s, strlen(s));
}


/** mark a variable to be read in base64 format
 * @ingroup formatting_functions */
inline base64_wrapper base64(substr s)
{
    return base64_wrapper(s.str, s.len);
}
/** mark a variable to be read in base64 format
 * @ingroup formatting_functions */
template<class T>
inline base64_wrapper base64(T &v, size_t num=1)
{
    return base64_wrapper(reinterpret_cast<char*>(&v), sizeof(T) * num);
}


/** write a variable in base64 format
 * @ingroup formatting_functions */
inline size_t to_chars(substr buf, const_base64_wrapper b)
{
    return detail::base64::encode(buf, b);
}

/** read a variable in base64 format
 * @ingroup formatting_functions */
inline size_t from_chars(csubstr buf, base64_wrapper *b)
{
    return detail::base64::decode(buf, *b);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** terminates the variadic recursion
 * @ingroup formatting_functions */
inline size_t cat(substr /*buf*/)
{
    return 0;
}

/** serialize the arguments, concatenating them to the given fixed-size buffer.
 * The buffer size is strictly respected: no writes will occur beyond its end.
 * @return the number of characters needed to write all the arguments into the buffer.
 * @see catrs() if instead of a fixed-size buffer, a resizeable container is desired
 * @see uncat() for the inverse function
 * @see catsep() if a separator between each argument is to be used
 * @see format() if a format string is desired
 * @ingroup formatting_functions */
template<class Arg, class... Args>
size_t cat(substr buf, Arg const& C4_RESTRICT a, Args const& C4_RESTRICT ...more)
{
    size_t num = to_chars(buf, a);
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num += cat(buf, more...);
    return num;
}

/** like cat but return a substr instead of a size */
template<class... Args>
substr cat_sub(substr buf, Args && ...args)
{
    size_t sz = cat(buf, std::forward<Args>(args)...);
    return {buf.str, sz <= buf.len ? sz : buf.len};
}

//-----------------------------------------------------------------------------

/** terminates the variadic recursion
 * @ingroup formatting_functions */
inline size_t uncat(csubstr /*buf*/)
{
    return 0;
}

/** deserialize the arguments from the given buffer.
 *
 * @return the number of characters read from the buffer, or csubstr::npos
 *   if a conversion was not successful.
 * @see cat(). uncat() is the inverse of cat().
 * @ingroup formatting_functions */
template<class Arg, class... Args>
size_t uncat(csubstr buf, Arg & C4_RESTRICT a, Args & C4_RESTRICT ...more)
{
    size_t out = from_chars_first(buf, &a);
    if(C4_UNLIKELY(out == csubstr::npos)) return csubstr::npos;
    buf  = buf.len >= out ? buf.sub(out) : substr{};
    size_t num = uncat(buf, more...);
    if(C4_UNLIKELY(num == csubstr::npos)) return csubstr::npos;
    return out + num;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace detail {

template<class Sep>
inline size_t catsep_more(substr /*buf*/, Sep const& C4_RESTRICT /*sep*/)
{
    return 0;
}

template<class Sep, class Arg, class... Args>
size_t catsep_more(substr buf, Sep const& C4_RESTRICT sep, Arg const& C4_RESTRICT a, Args const& C4_RESTRICT ...more)
{
    size_t ret = to_chars(buf, sep), num = ret;
    buf  = buf.len >= ret ? buf.sub(ret) : substr{};
    ret  = to_chars(buf, a);
    num += ret;
    buf  = buf.len >= ret ? buf.sub(ret) : substr{};
    ret  = catsep_more(buf, sep, more...);
    num += ret;
    return num;
}

template<class Sep>
inline size_t uncatsep_more(csubstr /*buf*/, Sep & /*sep*/)
{
    return 0;
}

template<class Sep, class Arg, class... Args>
size_t uncatsep_more(csubstr buf, Sep & C4_RESTRICT sep, Arg & C4_RESTRICT a, Args & C4_RESTRICT ...more)
{
    size_t ret = from_chars_first(buf, &sep), num = ret;
    if(C4_UNLIKELY(ret == csubstr::npos)) return csubstr::npos;
    buf  = buf.len >= ret ? buf.sub(ret) : substr{};
    ret  = from_chars_first(buf, &a);
    if(C4_UNLIKELY(ret == csubstr::npos)) return csubstr::npos;
    num += ret;
    buf  = buf.len >= ret ? buf.sub(ret) : substr{};
    ret  = uncatsep_more(buf, sep, more...);
    if(C4_UNLIKELY(ret == csubstr::npos)) return csubstr::npos;
    num += ret;
    return num;
}

} // namespace detail


/** serialize the arguments, concatenating them to the given fixed-size
 * buffer, using a separator between each argument.
 * The buffer size is strictly respected: no writes will occur beyond its end.
 * @return the number of characters needed to write all the arguments into the buffer.
 * @see catseprs() if instead of a fixed-size buffer, a resizeable container is desired
 * @see uncatsep() for the inverse function
 * @see cat() if no separator is needed
 * @see format() if a format string is desired
 * @ingroup formatting_functions */
template<class Sep, class Arg, class... Args>
size_t catsep(substr buf, Sep const& C4_RESTRICT sep, Arg const& C4_RESTRICT a, Args const& C4_RESTRICT ...more)
{
    size_t num = to_chars(buf, a);
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num += detail::catsep_more(buf, sep, more...);
    return num;
}

/** like catsep but return a substr instead of a size
 * @see catsep(). uncatsep() is the inverse of catsep().
 * @ingroup formatting_functions */
template<class... Args>
substr catsep_sub(substr buf, Args && ...args)
{
    size_t sz = catsep(buf, std::forward<Args>(args)...);
    return {buf.str, sz <= buf.len ? sz : buf.len};
}

/** deserialize the arguments from the given buffer, using a separator.
 *
 * @return the number of characters read from the buffer, or csubstr::npos
 *   if a conversion was not successful
 * @see catsep(). uncatsep() is the inverse of catsep().
 * @ingroup formatting_functions */
template<class Sep, class Arg, class... Args>
size_t uncatsep(csubstr buf, Sep & C4_RESTRICT sep, Arg & C4_RESTRICT a, Args & C4_RESTRICT ...more)
{
    size_t ret = from_chars_first(buf, &a), num = ret;
    if(C4_UNLIKELY(ret == csubstr::npos)) return csubstr::npos;
    buf  = buf.len >= ret ? buf.sub(ret) : substr{};
    ret  = detail::uncatsep_more(buf, sep, more...);
    if(C4_UNLIKELY(ret == csubstr::npos)) return csubstr::npos;
    num += ret;
    return num;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** terminates the variadic recursion
 * @ingroup formatting_functions */
inline size_t format(substr buf, csubstr fmt)
{
    return to_chars(buf, fmt);
}

/** using a format string, serialize the arguments into the given
 * fixed-size buffer.
 * The buffer size is strictly respected: no writes will occur beyond its end.
 * In the format string, each argument is marked with a compact
 * curly-bracket pair: {}. Arguments beyond the last curly bracket pair
 * are silently ignored. For example:
 * @code{.cpp}
 * c4::format(buf, "the {} drank {} {}", "partier", 5, "beers"); // the partier drank 5 beers
 * c4::format(buf, "the {} drank {} {}", "programmer", 6, "coffees"); // the programmer drank 6 coffees
 * @endcode
 * @return the number of characters needed to write into the buffer.
 * @see formatrs() if instead of a fixed-size buffer, a resizeable container is desired
 * @see unformat() for the inverse function
 * @see cat() if no format or separator is needed
 * @see catsep() if no format is needed, but a separator must be used
 * @ingroup formatting_functions */
template<class Arg, class... Args>
size_t format(substr buf, csubstr fmt, Arg const& C4_RESTRICT a, Args const& C4_RESTRICT ...more)
{
    auto pos = fmt.find("{}");
    if(pos != csubstr::npos)
    {
        size_t num = to_chars(buf, fmt.sub(0, pos));
        size_t out = num;
        buf  = buf.len >= num ? buf.sub(num) : substr{};
        num  = to_chars(buf, a);
        out += num;
        buf  = buf.len >= num ? buf.sub(num) : substr{};
        num  = format(buf, fmt.sub(pos + 2), more...);
        out += num;
        return out;
    }
    else
    {
        return format(buf, fmt);
    }
}

/** like format but return a substr instead of a size
 * @see catsep(). uncatsep() is the inverse of catsep().
 * @ingroup formatting_functions */
template<class... Args>
substr format_sub(substr buf, Args && ...args)
{
    size_t sz = c4::format(buf, std::forward<Args>(args)...);
    return {buf.str, sz <= buf.len ? sz : buf.len};
}


//-----------------------------------------------------------------------------

/** terminates the variadic recursion
 * @ingroup formatting_functions */
inline size_t unformat(csubstr buf, csubstr fmt)
{
    return 0;
}

/** using a format string, deserialize the arguments from the given
 * buffer.
 * @return the number of characters read from the buffer, or npos if a conversion failed.
 * @see format() this is the inverse function to format().
 * @ingroup formatting_functions */
template<class Arg, class... Args>
size_t unformat(csubstr buf, csubstr fmt, Arg & C4_RESTRICT a, Args & C4_RESTRICT ...more)
{
    auto pos = fmt.find("{}");
    if(pos != csubstr::npos)
    {
        size_t num = pos;
        size_t out = num;
        buf  = buf.len >= num ? buf.sub(num) : substr{};
        num  = from_chars_first(buf, &a);
        if(C4_UNLIKELY(num == csubstr::npos)) return csubstr::npos;
        out += num;
        buf  = buf.len >= num ? buf.sub(num) : substr{};
        num  = unformat(buf, fmt.sub(pos + 2), more...);
        if(C4_UNLIKELY(num == csubstr::npos)) return csubstr::npos;
        out += num;
        return out;
    }
    else
    {
        return unformat(buf, fmt);
    }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** C-style printing into a buffer
 * @ingroup formatting_functions */
size_t sprintf(substr buf, const char * fmt, ...);
//size_t sscanf(csubstr buf, const char *fmt, ...);


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/** a tag type
 * @see catrs
 * @ingroup formatting_functions
 * */
struct append_t {};

/** a tag variable
 * @see catrs
 * @ingroup formatting_functions
 * */
constexpr const append_t append = {};

//-----------------------------------------------------------------------------

/** like cat(), but receives a container, and resizes it as needed to contain
 * the result. The container is overwritten. To append to it, use the append
 * overload.
 *
 * @see cat()
 * @ingroup formatting_functions */
template<class CharOwningContainer, class... Args>
inline void catrs(CharOwningContainer * C4_RESTRICT cont, Args const& C4_RESTRICT ...args)
{
    substr buf = to_substr(*cont);
    size_t ret = cat(buf, args...);
    cont->resize(ret);
    if(ret > buf.len)
    {
        buf = to_substr(*cont);
        ret = cat(buf, args...);
        if(ret != buf.len)
        {
            cont->resize(ret);
        }
    }
}

/** like cat(), but receives a container, and appends to it instead of
 * overwriting it. The container is resized as needed to contain the result.
 *
 * @return the region newly appended to the original container
 * @see cat()
 * @see catrs()
 * @ingroup formatting_functions */
template<class CharOwningContainer, class... Args>
inline csubstr catrs(append_t, CharOwningContainer * C4_RESTRICT cont, Args const& C4_RESTRICT ...args)
{
    const size_t pos = cont->size();
    substr buf = to_substr(*cont).sub(pos);
    size_t ret = cat(buf, args...);
    cont->resize(pos + ret);
    if(ret > buf.len)
    {
        buf = to_substr(*cont).sub(pos);
        ret = cat(buf, args...);
        if(ret != buf.len)
        {
            cont->resize(pos + ret);
        }
    }
    return to_csubstr(*cont).range(pos, cont->size());
}

//-----------------------------------------------------------------------------

/** like catsep(), but receives a container, and resizes it as needed to contain the result.
 * The container is overwritten. To append to the container use the append overload.
 * @see catsep()
 * @ingroup formatting_functions */
template<class CharOwningContainer, class Sep, class... Args>
inline void catseprs(CharOwningContainer * C4_RESTRICT cont, Sep const& C4_RESTRICT sep, Args const& C4_RESTRICT ...args)
{
    substr buf = to_substr(*cont);
    size_t ret = catsep(buf, sep, args...);
    cont->resize(ret);
    if(ret > buf.len)
    {
        buf = to_substr(*cont);
        ret = catsep(buf, sep, args...);
        if(ret != buf.len)
        {
            cont->resize(ret);
        }
    }
}

/** like catsep(), but receives a container, and appends the arguments, resizing the
 * container as needed to contain the result. The buffer is appended to.
 * @ingroup formatting_functions */
template<class CharOwningContainer, class Sep, class... Args>
inline csubstr catseprs(append_t, CharOwningContainer * C4_RESTRICT cont, Sep const& C4_RESTRICT sep, Args const& C4_RESTRICT ...args)
{
    const size_t pos = cont->size();
    substr buf = to_substr(*cont).sub(pos);
    size_t ret = catsep(buf, sep, args...);
    cont->resize(pos + ret);
    if(ret > buf.len)
    {
        buf = to_substr(*cont).sub(pos);
        ret = catsep(buf, sep, args...);
        if(ret != buf.len)
        {
            cont->resize(pos + ret);
        }
    }
    return to_csubstr(*cont).range(pos, cont->size());
}

//-----------------------------------------------------------------------------

/** like format(), but receives a container, and resizes it as needed
 * to contain the result.  The container is overwritten. To append to
 * the container use the append overload.
 * @see format()
 * @ingroup formatting_functions */
template<class CharOwningContainer, class... Args>
inline void formatrs(CharOwningContainer * C4_RESTRICT cont, csubstr fmt, Args const&  C4_RESTRICT ...args)
{
    substr buf = to_substr(*cont);
    size_t ret = format(buf, fmt, args...);
    cont->resize(ret);
    if(ret > buf.len)
    {
        buf = to_substr(*cont);
        ret = format(buf, fmt, args...);
        if(ret != buf.len)
        {
            cont->resize(ret);
        }
    }
}

/** like format(), but receives a container, and appends the
 * arguments, resizing the container as needed to contain the
 * result. The buffer is appended to.
 * @return the region newly appended to the original container
 * @ingroup formatting_functions */
template<class CharOwningContainer, class... Args>
inline csubstr formatrs(append_t, CharOwningContainer * C4_RESTRICT cont, csubstr fmt, Args const& C4_RESTRICT ...args)
{
    const size_t pos = cont->size();
    substr buf = to_substr(*cont).sub(pos);
    size_t ret = format(buf, fmt, args...);
    cont->resize(pos + ret);
    if(ret > buf.len)
    {
        buf = to_substr(*cont).sub(pos);
        ret = format(buf, fmt, args...);
        if(ret != buf.len)
        {
            cont->resize(pos + ret);
        }
    }
    return to_csubstr(*cont).range(pos, cont->size());
}

} // namespace c4

#ifdef _MSC_VER
#   pragma warning(pop)
#endif

#endif /* _C4_FORMAT_HPP_ */