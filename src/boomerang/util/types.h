/**
 * \file types.h
 * \brief Contains some often used basic type definitions
 */
#pragma once

#include <QObject>
#include <iosfwd>
#include <cstdint>

#include "boomerang/util/Util.h"

class QTextStream;

// Machine types
typedef uint8_t         Byte;  /*  8 bits */
typedef uint16_t        SWord; /* 16 bits */
typedef uint32_t        DWord; /* 32 bits */
typedef uint64_t        QWord; /* 64 bits */

/* pointer. size depends on platform */
class Address
{
public:
	typedef uintptr_t   value_type;

public:
	value_type     m_value;

	static Address g(value_type x)   // construct host/native oblivious address
	{
		      Address z;

		z.m_value = x;
		return z;
	}

	static Address n(value_type x)   // construct native address
	{
		      Address z;

		z.m_value = x;
		return z.native();
	}

	/// query if the ADDRESS is the source, if it's host address returns false
	bool           isSourceAddr() const { return sizeof(m_value) == 4 || (uint64_t(m_value) >> 32) == 0; }
	Address        native() const { return Address::g(m_value & 0xFFFFFFFF); }
	static Address host_ptr(const void *x)
	{
		Address z;

		z.m_value = value_type(x);
		return z;
	}

	bool           isZero() const { return m_value == 0; }
	bool operator==(const Address& other) const { return m_value == other.m_value; }
	bool operator!=(const Address& other) const { return m_value != other.m_value; }
	bool operator<(const Address& other) const { return m_value < other.m_value; }
	bool operator>(const Address& other) const { return m_value > other.m_value; }
	bool operator>=(const Address& other) const { return m_value >= other.m_value; }
	bool operator<=(const Address& other) const { return m_value <= other.m_value; }

	   Address operator+(const Address& other) const { return Address::g(m_value + other.m_value); }
	   Address operator++()
	{
		++m_value;
		return *this;
	}

	   Address operator++(int)
	{
		      Address res = *this;

		++m_value;
		return res;
	}

	   Address operator--()
	{
		--m_value;
		return *this;
	}

	   Address operator--(int)
	{
		      Address res = *this;

		--m_value;
		return res;
	}

	   Address operator+=(const Address& other)
	{
		m_value += other.m_value;
		return *this;
	}

	   Address operator+=(intptr_t other)
	{
		m_value += other;
		return *this;
	}

	   Address& operator=(intptr_t v)
	{
		m_value = v;
		return *this;
	}

	   Address operator+(intptr_t val) const { return Address::g(m_value + val); }
	   Address operator-(const Address& other) const { return Address::g(m_value - other.m_value); }
	   Address operator-=(intptr_t v)
	{
		m_value -= v;
		return *this;
	}

	   Address operator-(intptr_t other) const { return Address::g(m_value - other); }
	friend QTextStream& operator<<(QTextStream& os, const Address& mdv);

	QString        toString(bool zerofill = false) const
	{
		if (zerofill) {
			return "0x" + QString("%1").arg(m_value, 8, 16, QChar('0'));
		}

		return "0x" + QString::number(m_value, 16);
	}

	// operator intptr_t() const {return int(m_value);}
};

template<class T, class U>
bool IN_RANGE(const T& val, const U& range_start, const U& range_end)
{
	return((val >= range_start) && (val < range_end));
}


Q_DECLARE_METATYPE(Address)

#define STD_SIZE    32 // Standard size
// Note: there is a known name collision with NO_ADDRESS in WinSock.h
#ifdef NO_ADDRESS
#undef NO_ADDRESS
#endif
#define NO_ADDRESS    (Address::g(-1)) // For invalid ADDRESSes

typedef uint64_t QWord;                // 64 bits
