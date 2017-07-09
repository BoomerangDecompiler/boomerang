#include "Address.h"

#include "boomerang/util/Util.h"

#include <cassert>


const Address Address::ZERO    = Address(0);
const Address Address::INVALID = Address(-1);
Byte Address::m_sourceBits = 32U;


Address::Address()
	: m_value(0)
{}

Address::Address(value_type value)
	: m_value(value)
{
//	assert(m_value == (value_type)-1 || (value & ~getSourceMask()) == 0);
}

Address Address::g(value_type x)
{
    return Address(x);
}

void Address::setSourceBits(Byte bitCount)
{
	m_sourceBits = bitCount;
}


QString Address::toString() const
{
	return QString("0x%1").arg(m_value, m_sourceBits / 4, 16, QChar('0'));
}

QTextStream& operator<<(QTextStream& os, const Address& addr)
{
	return os << addr.toString();
}

Address::value_type Address::getSourceMask()
{
    return getLowerBitMask(m_sourceBits);
}

HostAddress::HostAddress(const void* ptr)
	: m_value((value_type)ptr)
{}

HostAddress::HostAddress(value_type value)
	: m_value(value)
{}

QString HostAddress::toString() const
{
	return QString("0x%1").arg(m_value, 2*sizeof(value_type), 16, QChar('0'));
}

