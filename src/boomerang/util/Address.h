#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/util/OStream.h"
#include "boomerang/util/Types.h"

#include <QString>


/// Standard pointer size of source machine, in bits
#define STD_SIZE 32


/// Pointer / address value type for the source machine.
class BOOMERANG_API Address
{
public:
    typedef uintptr_t value_type;

public:
    static const Address ZERO;
    static const Address INVALID;

    explicit Address();
    explicit Address(value_type value);

    Address(const Address &) = default;
    Address(Address &&)      = default;
    Address &operator=(const Address &) = default;
    Address &operator=(Address &&) = default;

public:
    /// Set the bit count of the source machine.
    static void setSourceBits(Byte bitCount = STD_SIZE);

    static Byte getSourceBits() { return m_sourceBits; }
    static value_type getSourceMask();

    Address native() const { return Address(m_value & 0xFFFFFFFF); }
    value_type value() const { return m_value; }

    bool isZero() const { return m_value == 0; }
    bool operator==(const Address &other) const { return m_value == other.value(); }
    bool operator!=(const Address &other) const { return m_value != other.value(); }
    bool operator<(const Address &other) const { return m_value < other.value(); }
    bool operator>(const Address &other) const { return m_value > other.value(); }
    bool operator>=(const Address &other) const { return m_value >= other.value(); }
    bool operator<=(const Address &other) const { return m_value <= other.value(); }

    Address operator+(const Address &other) const { return Address(m_value + other.value()); }
    Address operator-(const Address &other) const { return Address(m_value - other.value()); }

    Address operator++()
    {
        ++m_value;
        m_value &= getSourceMask();
        return *this;
    }

    Address operator--()
    {
        --m_value;
        m_value &= getSourceMask();
        return *this;
    }

    Address operator++(int)
    {
        Address addr(*this);
        m_value = (m_value + 1) & getSourceMask();
        return addr;
    }

    Address operator--(int)
    {
        Address addr(*this);
        m_value = (m_value - 1) & getSourceMask();
        return addr;
    }

    Address operator+=(const Address &other)
    {
        m_value += other.value();
        m_value &= getSourceMask();
        return *this;
    }

    Address operator+=(value_type offset)
    {
        m_value += offset;
        m_value &= getSourceMask();
        return *this;
    }

    Address operator-=(value_type offset)
    {
        m_value -= offset;
        m_value &= getSourceMask();
        return *this;
    }

    Address operator+(value_type offset) const
    {
        return Address((m_value + offset) & getSourceMask());
    }

    Address operator-(value_type offset) const
    {
        return Address((m_value - offset) & getSourceMask());
    }

    QString toString() const;

private:
    static Byte m_sourceBits; ///< number of bits in a source address (typically 32 or 64 bits)
    value_type m_value;       ///< Value of this address
};

/// Like \ref Address, but only for addresses of the host machine
class BOOMERANG_API HostAddress
{
public:
    typedef uintptr_t value_type;

public:
    static const HostAddress ZERO;
    static const HostAddress INVALID;

    HostAddress() { m_value = 0; }
    explicit HostAddress(value_type value);
    explicit HostAddress(const void *ptr);
    /// Initializes this HostAddress to \p srcAddr + \p hostDiff
    explicit HostAddress(Address srcAddr, ptrdiff_t hostDiff);

    HostAddress(const HostAddress &other) = default;
    HostAddress(HostAddress &&other)      = default;
    HostAddress &operator=(const HostAddress &other) = default;
    HostAddress &operator=(HostAddress &&other) = default;

public:
    inline value_type value() const { return m_value; }
    inline bool isZero() const { return m_value == 0; }

    inline bool operator==(const HostAddress &other) { return m_value == other.m_value; }
    inline bool operator!=(const HostAddress &other) { return m_value != other.m_value; }
    inline bool operator<(const HostAddress &other) { return m_value < other.m_value; }
    inline bool operator>(const HostAddress &other) { return m_value > other.m_value; }
    inline bool operator<=(const HostAddress &other) { return m_value <= other.m_value; }
    inline bool operator>=(const HostAddress &other) { return m_value >= other.m_value; }

    HostAddress operator+=(const Address &other)
    {
        m_value += static_cast<value_type>(other.value());
        return *this;
    }

    HostAddress operator-=(const Address &other)
    {
        m_value -= static_cast<value_type>(other.value());
        return *this;
    }

    HostAddress operator+=(value_type offset)
    {
        m_value += offset;
        return *this;
    }

    HostAddress operator-=(value_type offset)
    {
        m_value -= offset;
        return *this;
    }

    HostAddress operator+(const Address &other) { return HostAddress(*this) += other; }
    HostAddress operator-(const Address &other) { return HostAddress(*this) -= other; }

    HostAddress operator+(value_type offset) { return HostAddress(*this) += offset; }
    HostAddress operator-(value_type offset) { return HostAddress(*this) -= offset; }

    HostAddress operator+(const HostAddress &other) { return HostAddress(m_value + other.m_value); }
    HostAddress operator-(const HostAddress &other) { return HostAddress(m_value - other.m_value); }

    QString toString() const;

    operator const void *() const { return reinterpret_cast<const void *>(m_value); }

private:
    value_type m_value;
};

BOOMERANG_API OStream &operator<<(OStream &os, const Address &addr);
BOOMERANG_API OStream &operator<<(OStream &os, const HostAddress &addr);
