#pragma once

#include <cstdint>

#include <QString>
#include <QTextStream>

#include "boomerang/util/types.h"

/// Standard pointer size of source machine, in bits
#define STD_SIZE    32

/// return a bit mask with exactly @p bitCount of the lowest bits set to 1.
/// (example: 16 -> 0xFFFF)


/// Pointer / address value type for the source machine.
class Address
{
public:
	typedef uintptr_t   value_type;

public:
	static const Address ZERO;
	static const Address INVALID;

	explicit Address();
	explicit Address(value_type value);

	Address(const Address&) = default;
	Address& operator=(const Address&) = default;

	/// Set the bit count of the source machine.
	static void setSourceBits(Byte bitCount);
	static value_type getSourceMask();

	static Address g(value_type x);   // construct host/native oblivious address

	static Address host_ptr(const void *x)
	{
		return Address((value_type)x);
	}

	Address        native() const { return Address::g(m_value & 0xFFFFFFFF); }
	value_type     value() const { return m_value; }

	bool           isZero() const { return m_value == 0; }
	bool operator==(const Address& other) const { return m_value == other.value(); }
	bool operator!=(const Address& other) const { return m_value != other.value(); }
	bool operator<(const Address& other) const { return m_value < other.value(); }
	bool operator>(const Address& other) const { return m_value > other.value(); }
	bool operator>=(const Address& other) const { return m_value >= other.value(); }
	bool operator<=(const Address& other) const { return m_value <= other.value(); }

	Address operator+(const Address& other) const { return Address::g(m_value + other.value()); }
	Address operator-(const Address& other) const { return Address::g(m_value - other.value()); }

	Address operator++() { ++m_value; return *this; }
	Address operator--() { --m_value; return *this; }
	Address operator++(int)	{ return Address(m_value++); }
	Address operator--(int) { return Address(m_value--); }

	Address operator+=(const Address& other) { m_value += other.value(); return *this; }

	Address operator+=(intptr_t other) { m_value += other; return *this; }
	Address operator-=(intptr_t v) { m_value -= v; return *this; }

	Address operator+(intptr_t val) const { return Address::g(m_value + val); }
	Address operator-(intptr_t other) const { return Address::g(m_value - other); }

	QString toString() const;

private:
	static Byte m_sourceBits;
	value_type  m_value;
};

Q_DECLARE_METATYPE(Address)

QTextStream& operator<<(QTextStream& os, const Address& addr);
