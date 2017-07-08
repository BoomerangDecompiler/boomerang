#include "Address.h"

#include "boomerang/util/Util.h"

#include <cassert>


const Address Address::ZERO    = Address(0);
const Address Address::INVALID = Address(-1);
Byte Address::m_sourceBits = 32U;


Address Address::g(value_type x)
{
    return Address(x);
}


Address Address::n(value_type x)
{
    assert((x & ~Address::getSourceMask()) == 0);
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


