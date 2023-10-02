/*
integer.h

Copyright (c) 2013 - 2017 Jason Lee @ calccrypto at gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <cmath> //For fft sin, cos, M_PI, and floor
#include <cstdint>
#include <deque>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#ifndef __INTEGER__
#define __INTEGER__

#ifndef INTEGER_DIGIT_T
#define INTEGER_DIGIT_T        uint8_t
#endif

#ifndef INTEGER_DOUBLE_DIGIT_T
#define INTEGER_DOUBLE_DIGIT_T uint64_t
#endif

// INTEGET_DIGIT_T and INTEGER_DOUBLE_DIGIT_T
// should be unsigned integers
static_assert(std::is_unsigned <INTEGER_DIGIT_T>::value &&
              std::is_unsigned <INTEGER_DOUBLE_DIGIT_T>::value
        , "Internal types must be unsigned integers");

// INTEGER_DOUBLE_DIGIT_T should be at least 2 times the size of INTEGER_DIGIT_T
static_assert((2 * sizeof(INTEGER_DIGIT_T)) <= sizeof(INTEGER_DOUBLE_DIGIT_T)
        , "INTEGER_DOUBLE_DIGIT_T should be at least twice the size of INTEGER_DIGIT_T");

class integer{
public:
    typedef std::deque <INTEGER_DIGIT_T> REP;                                                 // internal representation of values
    typedef REP::size_type               REP_SIZE_T;                                          // size type of internal representation

private:
    static constexpr INTEGER_DIGIT_T NEG1     = std::numeric_limits <INTEGER_DIGIT_T>::max(); // value with all bits ON - will only work for unsigned integer types
    static constexpr std::size_t     OCTETS   = sizeof(INTEGER_DIGIT_T);                      // number of octets per INTEGER_DIGIT_T
    static constexpr std::size_t     BITS     = OCTETS << 3;                                  // number of bits per INTEGER_DIGIT_T; hardcode this if INTEGER_DIGIT_T is not standard int type
    static constexpr INTEGER_DIGIT_T HIGH_BIT = 1 << (BITS - 1);                              // highest bit of INTEGER_DIGIT_T (uint8_t -> 128)

public:
    typedef bool Sign;
    static constexpr Sign POSITIVE = false;                                                   // includes 0
    static constexpr Sign NEGATIVE = true;

private:
    bool _sign;     // sign of value
    REP _value;     // absolute value of *this

    template <typename Z>
    integer & setFromZ(Z val){
        static_assert( std::is_integral  <Z>::value &&
                       !std::is_const     <Z>::value &&
                       !std::is_reference <Z>::value
                , "Input to integer::setFromZ should be passed by value");
        _value.clear();
        _sign = POSITIVE;

        // make positive
        if (std::is_signed <Z>::value && (val < 0)){
            _sign = NEGATIVE;
            val = -val; // treat as positive even if top bit is still set
        }

        // keep this here just in case value is sign extended
        for(std::size_t d = std::max(sizeof(Z) / OCTETS, (std::size_t) 1); d > 0; d--){
            _value.push_front(val & NEG1);
            val >>= BITS;
        }

        return trim();
    }

    // remove 0 digits from top of deque to save memory
    integer & trim();

public:
    // Constructors
    integer();
    integer(const integer & rhs);
    integer(integer && rhs);
    integer(const REP & rhs, const Sign & sign = POSITIVE);

    // Special boolean constructor
    integer(const bool & b);

    // Constructors for integral input
    template <typename Z>
    integer(const Z & val){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        setFromZ(val);
    }

    // Special Constructor for Strings
    // bases 2-16 and 256 are allowed
    //      Written by Corbin http://codereview.stackexchange.com/a/13452
    //      Modified by me
    integer(const std::string & val, const integer & base);

    // Use this to construct integers with other types that have pointers/iterators to their beginning and end
    // all inputs are treated as positive values
    template <typename Iterator> integer(Iterator start, const Iterator & end, const integer & base) : integer()
    {
        if (base < 2){
            throw std::runtime_error("Error: Cannot convert from base " + base.str(10));
        }

        for(; start != end; start++){
            *this = (*this * base) | *start;
        }
    }

public:
    //  RHS input args only

    // Assignment Operator
    integer & operator=(const integer & rhs);
    integer & operator=(integer && rhs);
    template <typename Z>
    integer & operator=(const Z & rhs){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        setFromZ(rhs);
        return *this;
    }

    // Typecast Operators
    operator bool()     const;
    operator uint8_t()  const;
    operator uint16_t() const;
    operator uint32_t() const;
    operator uint64_t() const;
    operator int8_t()   const;
    operator int16_t()  const;
    operator int32_t()  const;
    operator int64_t()  const;

    // Bitwise Operators
    integer operator&(const integer & rhs) const;
    template <typename Z>
    integer operator&(const Z & rhs)       const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this & integer(rhs);
    }

    integer & operator&=(const integer & rhs);
    template <typename Z>
    integer & operator&=(const Z & rhs){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this &= integer(rhs);
    }

    integer operator|(const integer & rhs) const;
    template <typename Z>
    integer operator|(const Z & rhs)       const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this | integer(rhs);
    }

    integer & operator|=(const integer & rhs);
    template <typename Z>
    integer & operator|=(const Z & rhs){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this |= integer(rhs);
    }

    integer operator^(const integer & rhs) const;
    template <typename Z>
    integer operator^(const Z & rhs)       const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this ^ integer(rhs);
    }

    integer & operator^=(const integer & rhs);
    template <typename Z>
    integer & operator^=(const Z & rhs){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this ^= integer(rhs);
    }

    integer operator~() const;

    // Bitshift Operators
    // left bitshift. sign is maintained
    integer operator<<(const integer & shift) const;
    template <typename Z>
    integer operator<<(const Z & rhs)         const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this << integer(rhs);
    }

    integer & operator<<=(const integer & shift);
    template <typename Z>
    integer & operator<<=(const Z & rhs){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this <<= integer(rhs);
    }

    // right bitshift. sign is maintained
    integer operator>>(const integer & shift) const;
    template <typename Z>
    integer operator>>(const Z & rhs)         const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this >> integer(rhs);
    }

    integer & operator>>=(const integer & shift);
    template <typename Z>
    integer & operator>>=(const Z & rhs){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this >>= integer(rhs);
    }

    // Logical Operators
    bool operator!();

    // Comparison Operators
    bool operator==(const integer & rhs) const;
    template <typename Z>
    integer operator==(const Z & rhs)    const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return (*this == integer(rhs));
    }

    bool operator!=(const integer & rhs) const;
    template <typename Z>
    integer operator!=(const Z & rhs)    const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return (*this != integer(rhs));
    }

private:
    // operator> not considering signs
    bool gt(const integer & lhs, const integer & rhs) const;

public:
    bool operator>(const integer & rhs) const;
    template <typename Z>
    integer operator>(const Z & rhs)    const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return (*this > integer(rhs));
    }

    bool operator>=(const integer & rhs) const;
    template <typename Z>
    integer operator>=(const Z & rhs)    const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return (*this >= integer(rhs));
    }

private:
    // operator< not considering signs
    bool lt(const integer & lhs, const integer & rhs) const;

public:
    bool operator<(const integer & rhs) const;
    template <typename Z>
    integer operator<(const Z & rhs)    const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return (*this < integer(rhs));
    }

    bool operator<=(const integer & rhs) const;
    template <typename Z>
    integer operator<=(const Z & rhs)    const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return (*this <= integer(rhs));
    }

private:
    // Arithmetic Operators
    integer add(const integer & lhs, const integer & rhs) const;

public:
    integer operator+(const integer & rhs) const;
    template <typename Z>
    integer operator+(const Z & rhs)       const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this + integer(rhs);
    }

    integer & operator+=(const integer & rhs);
    template <typename Z>
    integer & operator+=(const Z & rhs){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this += integer(rhs);
    }

private:
    // Subtraction as done by hand
    // lhs must be larger than rhs
    integer long_sub(const integer & lhs, const integer & rhs) const;

    // // Two's Complement Subtraction
    // integer two_comp_sub(const integer & lhs, const integer & rhs) const;

    // subtraction not considering signs
    // lhs must be larger than rhs
    integer sub(const integer & lhs, const integer & rhs) const;

public:
    integer operator-(const integer & rhs) const;
    template <typename Z>
    integer operator-(const Z & rhs)       const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this - integer(rhs);
    }

    integer & operator-=(const integer & rhs);
    template <typename Z>
    integer & operator-=(const Z & rhs){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this -= integer(rhs);
    }

private:
    // // Peasant Multiplication
    // integer peasant(const integer & lhs, const integer & rhs) const;

    // // Recurseive Peasant Algorithm
    // integer recursive_peasant(const integer & lhs, const integer & rhs) const;

    // // Recursive Multiplication
    // integer recursive_mult(const integer & lhs, const integer & rhs) const;

    // // Karatsuba Algorithm O(n-log2(3) = n - 1.585)
    // // The Peasant Multiplication function is needed if Karatsuba is used.
    // // Thanks to kjo @ stackoverflow for fixing up my original Karatsuba Algorithm implementation
    // // which I then converted to C++ and made a few changes.
    // // http://stackoverflow.com/questions/7058838/karatsuba-algorithm-too-much-recursion
    // integer karatsuba(const integer & lhs, const integer & rhs, integer bm = 0x1000000U) const;

    // // // Toom-Cook multiplication
    // // // as described at http://en.wikipedia.org/wiki/Toom%E2%80%93Cook_multiplications
    // // // The peasant function is needed if karatsuba is used.
    // // // This implementation is a bit weird. In the pointwise Multiplcation step, using
    // // // operator* and long_mult works, but everything else fails.
    // // // It's also kind of slow.
    // // integer toom_cook_3(integer m, integer n, integer bm = 0x1000000U);

    // // Long multiplication
    // integer long_mult(const integer & lhs, const integer & rhs) const;

    //Private FFT helper function
    int fft(std::deque<double>& data, bool dir = true) const;

    // FFT-based multiplication
    //Based on the convolution theorem which states that the Fourier
    //transform of a convolution is the pointwise product of their
    //Fourier transforms.
    integer fft_mult(const integer& lhs, const integer& rhs) const;

public:
    integer operator*(const integer & rhs) const;
    template <typename Z>
    integer operator*(const Z & rhs)       const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this * integer(rhs);
    }

    integer & operator*=(const integer & rhs);
    template <typename Z>
    integer & operator*=(const Z & rhs){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this *= integer(rhs);
    }

private:
    // // Naive Division: keep subtracting until lhs == 0
    // std::pair <integer, integer> naive_divmod(const integer & lhs, const integer & rhs) const;

    // // Long Division returning both quotient and remainder
    // std::pair <integer, integer> long_divmod(const integer & lhs, const integer & rhs) const;

    // // Recursive Division that returns both the quotient and remainder
    // // Recursion took up way too much memory
    // std::pair <integer, integer> recursive_divmod(const integer & lhs, const integer & rhs) const;

    // Non-Recursive version of above algorithm
    std::pair <integer, integer> non_recursive_divmod(const integer & lhs, const integer & rhs) const;

    // division and modulus ignoring signs
    std::pair <integer, integer> dm(const integer & lhs, const integer & rhs) const;

public:
    // division and modulus with signs
    std::pair <integer, integer> divmod(const integer & lhs, const integer & rhs) const;

    integer operator/(const integer & rhs) const;
    template <typename Z>
    integer operator/(const Z & rhs)       const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this / integer(rhs);
    }

    integer & operator/=(const integer & rhs);
    template <typename Z>
    integer & operator/=(const Z & rhs){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this /= integer(rhs);
    }

    integer operator%(const integer & rhs) const;
    template <typename Z>
    integer operator%(const Z & rhs)       const {
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this % integer(rhs);
    }

    integer & operator%=(const integer & rhs);
    template <typename Z>
    integer & operator%=(const Z & rhs){
        static_assert(std::is_integral <Z>::value
                , "Input type must be integral");
        return *this %= integer(rhs);
    }

    // Increment Operator
    integer & operator++();
    integer operator++(int);

    // Decrement Operator
    integer & operator--();
    integer operator--(int);

    // Nothing done since promotion doesn't work here
    integer operator+() const;

    // Flip Sign
    integer operator-() const;

    // get private values
    Sign sign() const;

    // get minimum number of bits needed to hold this value
    integer bits() const;

    // get minimum number of bytes needed to hold this value
    REP_SIZE_T bytes() const;

    // get number of digits of internal representation
    REP_SIZE_T digits() const;

    // get internal data
    REP data() const;

    // Miscellaneous Functions
    integer & negate();

    // Two's compliment - specify number of bits to make output make sense
    integer twos_complement(const REP_SIZE_T & b) const;

    // fills an integer with 1s
    integer & fill(const REP_SIZE_T & b);

    // get bit, where 0 is the lsb and bits() - 1 is the msb
    bool operator[](const REP_SIZE_T & b) const;

    // Output _value as a string in bases 2 to 16, and 256
    std::string str(const integer & base = 10, const std::string::size_type & length = 1) const;
};

// Give integer type traits
namespace std {  // This is probably not a good idea
    template <> struct is_arithmetic <integer> : std::true_type {};
    template <> struct is_integral   <integer> : std::true_type {};
    template <> struct is_signed     <integer> : std::true_type {};
};

// operators where lhs is not of type integer

// Bitwise Operators
template <typename Z>
integer operator&(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return integer(lhs) & rhs;
}

template <typename Z>
Z & operator&=(Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value &&
                  !std::is_const <Z>::value
            , "Input type must be integral");
    return lhs = static_cast <Z> (integer(lhs) & rhs);
}

template <typename Z>
integer operator|(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return integer(lhs) | rhs;
}

template <typename Z>
Z & operator|=(Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value &&
                  !std::is_const <Z>::value
            , "Input type must be integral");
    return lhs = static_cast <Z> (integer(lhs) | rhs);
}

template <typename Z>
integer operator^(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return integer(lhs) ^ rhs;
}

template <typename Z>
Z & operator^=(Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value &&
                  !std::is_const <Z>::value
            , "Input type must be integral");
    return lhs = static_cast <Z> (integer(lhs) ^ rhs);
}

// Bitshift operators
integer operator<<(const bool     & lhs, const integer & rhs);
integer operator<<(const uint8_t  & lhs, const integer & rhs);
integer operator<<(const uint16_t & lhs, const integer & rhs);
integer operator<<(const uint32_t & lhs, const integer & rhs);
integer operator<<(const uint64_t & lhs, const integer & rhs);
integer operator<<(const int8_t   & lhs, const integer & rhs);
integer operator<<(const int16_t  & lhs, const integer & rhs);
integer operator<<(const int32_t  & lhs, const integer & rhs);
integer operator<<(const int64_t  & lhs, const integer & rhs);

template <typename Z>
Z & operator<<=(Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value &&
                  !std::is_const <Z>::value
            , "Input type must be integral");
    return lhs = static_cast <Z> (integer(lhs) << rhs);
}

integer operator>>(const bool     & lhs, const integer & rhs);
integer operator>>(const uint8_t  & lhs, const integer & rhs);
integer operator>>(const uint16_t & lhs, const integer & rhs);
integer operator>>(const uint32_t & lhs, const integer & rhs);
integer operator>>(const uint64_t & lhs, const integer & rhs);
integer operator>>(const int8_t   & lhs, const integer & rhs);
integer operator>>(const int16_t  & lhs, const integer & rhs);
integer operator>>(const int32_t  & lhs, const integer & rhs);
integer operator>>(const int64_t  & lhs, const integer & rhs);

template <typename Z>
Z & operator>>=(Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value &&
                  !std::is_const <Z>::value
            , "Input type must be integral");
    return lhs = static_cast <Z> (integer(lhs) >> rhs);
}

// Comparison Operators
template <typename Z>
bool operator==(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return (integer(lhs) == rhs);
}

template <typename Z>
bool operator!=(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return (integer(lhs) != rhs);
}

template <typename Z>
bool operator>(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return (rhs < lhs);
}

template <typename Z>
bool operator>=(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return (rhs <= lhs);
}

template <typename Z>
bool operator<(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return (rhs > lhs);
}

template <typename Z>
bool operator<=(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return (rhs >= lhs);
}

// Arithmetic Operators
template <typename Z>
integer operator+(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return integer(lhs) + rhs;
}

template <typename Z>
Z & operator+=(Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value &&
                  !std::is_const <Z>::value
            , "Input type must be integral");
    return lhs = static_cast <Z> (integer(lhs) + rhs);
}

template <typename Z>
integer operator-(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return integer(lhs) - rhs;
}

template <typename Z>
Z & operator-=(Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value &&
                  !std::is_const <Z>::value
            , "Input type must be integral");
    return lhs = static_cast <Z> (integer(lhs) - rhs);
}

template <typename Z>
integer operator*(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return integer(lhs) * rhs;
}

template <typename Z>
Z & operator*=(Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value &&
                  !std::is_const <Z>::value
            , "Input type must be integral");
    return lhs = static_cast <Z> (integer(lhs) * rhs);
}

template <typename Z>
integer operator/(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return integer(lhs) / rhs;
}

template <typename Z>
Z & operator/=(Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value &&
                  !std::is_const <Z>::value
            , "Input type must be integral");
    return lhs = static_cast <Z> (integer(lhs) / rhs);
}

template <typename Z>
integer operator%(const Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value
            , "Input type must be integral");
    return integer(lhs) % rhs;
}

template <typename Z>
Z & operator%=(Z & lhs, const integer & rhs){
    static_assert(std::is_integral <Z>::value &&
                  !std::is_const <Z>::value
            , "Input type must be integral");
    return lhs = static_cast <Z> (integer(lhs) % rhs);
}

// IO Operators
std::ostream & operator<<(std::ostream & stream, const integer & rhs);
std::istream & operator>>(std::istream & stream, integer & rhs);

// Miscellaneous functions
std::string makebin  (const integer & value, const unsigned int & size = 1);
std::string makehex  (const integer & value, const unsigned int & size = 1);
std::string makeascii(const integer & value, const unsigned int & size = 1);

integer abs(const integer & value);

// floor(log_b(x))
template <typename Z>
integer log(integer value, Z base){
    static_assert(std::is_integral <Z>::value
            , "Base type should be a non-negative integer");

    if ((base < 1) || (value <= 0)){
        throw std::domain_error("Error: Domain error");
    }

    integer count = 0;
    while (value){
        value /= base;
        count++;
    }
    return count;
}

template <typename Z>
integer pow(integer value, Z exp){
    static_assert(std::is_integral <Z>::value
            , "Exponent type should be integral");

    if (exp < 0){
        return 0;
    }

    Z one = 1;
    integer result = 1;
    while (exp){
        if (exp & one){
            result *= value;
        }
        exp >>= one;
        value *= value;
    }

    return result;
}

template <typename Z_e, typename Z_m>
integer pow(integer base, Z_e exponent, const Z_m modulus){
    static_assert(std::is_integral <Z_e>::value &&
                  std::is_integral <Z_m>::value
            , "Exponent type should be integral");
    // check modulus first
    if (!modulus){
        throw std::domain_error("Error: modulus by 0");
    }

    if (exponent < 0){
        return 0;
    }

    const Z_e one = 1;
    integer exp = exponent;
    const integer mod = modulus;

    integer result = one;
    while (exp){
        if (exp & one){
            result = (result * base) % mod;
        }
        exp >>= one;
        base = (base * base) % mod;
    }

    return result;
}

#endif // INTEGER_H