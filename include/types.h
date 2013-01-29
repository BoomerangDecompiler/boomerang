/*
 * types.h: some often used basic type definitions
 * $Revision$
 */
#ifndef __TYPES_H__
#define __TYPES_H__
#include <iostream>
#include <stdint.h>

// Machine types
typedef unsigned char       Byte;        /* 8 bits */
typedef unsigned short      SWord;       /* 16 bits */
typedef unsigned int        DWord;       /* 32 bits */
struct ADDRESS { /* pointer. size depends on platform */
//    ADDRESS() {}
//    ADDRESS(uint32_t v) : m_value(v) {}
    typedef intptr_t value_type;
    value_type m_value;
    static ADDRESS g(value_type x) { // construct host/native oblivious address
        ADDRESS z;
        z.m_value =x;
        return z;
    }
    ADDRESS native() { return ADDRESS::g(m_value&0xFFFFFFFF); }
    static ADDRESS host_ptr(void *x) {
        ADDRESS z;
        z.m_value =value_type(x);
        return z;
    }
        bool isZero() const {return m_value==0;}
        bool operator==(const ADDRESS &other) const { return m_value==other.m_value; }
        bool operator!=(const ADDRESS &other) const { return m_value!=other.m_value; }
    bool operator<(const ADDRESS &other) const  { return m_value < other.m_value; }
    bool operator>(const ADDRESS &other) const  { return m_value > other.m_value; }
    bool operator>=(const ADDRESS &other) const { return m_value >= other.m_value;}
        bool operator<=(const ADDRESS &other) const { return m_value <= other.m_value;}

    ADDRESS operator+(const ADDRESS &other) const {
        return ADDRESS::g(m_value + other.m_value);
    }
    ADDRESS operator++(int){
        ADDRESS res = *this;
        m_value++;
        return res;
    }
    ADDRESS operator+=(const ADDRESS &other) {
        m_value += other.m_value;
        return *this;
    }
    ADDRESS operator+=(int other) {
        m_value += other;
        return *this;
    }
    ADDRESS &operator=(intptr_t v) {
        m_value = v;
        return *this;
    }
    ADDRESS operator+(intptr_t val) const {
        return ADDRESS::g(m_value + val);
    }
    ADDRESS operator-(const ADDRESS &other) const {
        return ADDRESS::g(m_value - other.m_value);
    }
    ADDRESS operator-=(int v) {
        m_value -= v;
        return *this;
    }
    ADDRESS operator-(int other) const {
        return ADDRESS::g(m_value - other);
    }
        friend std::ostream& operator<< (std::ostream& stream, const ADDRESS& addr);
    //operator intptr_t() const {return int(m_value);}
};

#define STD_SIZE    32                    // Standard size
// Note: there is a known name collision with NO_ADDRESS in WinSock.h
#ifdef NO_ADDRESS
#undef NO_ADDRESS
#endif
#define NO_ADDRESS (ADDRESS::g(-1))		// For invalid ADDRESSes

#ifndef _MSC_VER
typedef long unsigned long QWord;        // 64 bits
#else
typedef unsigned __int64   QWord;
#endif

#if defined(_MSC_VER)
#pragma warning(disable:4390)
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1200
// For MSVC 5 or 6: warning about debug into truncated to 255 chars
#pragma warning(disable:4786)
#endif

#endif    // #ifndef __TYPES_H__
