#include "Address.h"

#include "boomerang/util/Util.h"

#include <cassert>
#include <QDebug>


const Address Address::ZERO    = Address(0);
const Address Address::INVALID = Address(-1);
Byte Address::m_sourceBits = 32U;

const HostAddress HostAddress::ZERO = HostAddress(nullptr);
const HostAddress HostAddress::INVALID = HostAddress(-1);


Address::Address()
	: m_value(0)
{}

Address::Address(value_type value)
	: m_value(value)
{
// 	if (!(m_value == (value_type)-1 || (value & ~getSourceMask()) == 0)) {
// 		qWarning() << "Address initialized with invalid value " <<
// 			QString("0x%1").arg(m_value, 2*sizeof(value_type), 16, QChar('0'));
// 	}
}

void Address::setSourceBits(Byte bitCount)
{
	m_sourceBits = bitCount;
}


QString Address::toString() const
{
	return QString("0x%1").arg(m_value, m_sourceBits / 4, 16, QChar('0'));
}


Address::value_type Address::getSourceMask()
{
    return Util::getLowerBitMask(m_sourceBits);
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


QTextStream& operator<<(QTextStream& os, const Address& addr)
{
	return os << addr.toString();
}


QTextStream& operator<<(QTextStream& os, const HostAddress& addr)
{
	return os << addr.toString();
}
