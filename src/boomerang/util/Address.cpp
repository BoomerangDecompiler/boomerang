#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Address.h"


#include "boomerang/util/Util.h"
#include "boomerang/util/Log.h"

#include <cassert>

const Address Address::ZERO         = Address(0);
const Address Address::INVALID      = Address((Address::value_type)-1);
Byte          Address::m_sourceBits = 32U;

const HostAddress HostAddress::ZERO    = HostAddress(nullptr);
const HostAddress HostAddress::INVALID = HostAddress((HostAddress::value_type)-1);


Address::Address()
    : m_value(0)
{
}


Address::Address(value_type _value)
    : m_value(_value)
{
    if ((m_value != (value_type) - 1) && ((_value & ~getSourceMask()) != 0)) {
        LOG_WARN("Address initialized with invalid value %1",
                 QString("0x%1").arg(m_value, 2 * sizeof(value_type), 16, QChar('0')));
    }
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


HostAddress::HostAddress(const void *ptr)
    : m_value((value_type)ptr)
{
}


HostAddress::HostAddress(value_type _value)
    : m_value(_value)
{
}


HostAddress::HostAddress(Address srcAddr, ptrdiff_t hostDiff)
{
    m_value = srcAddr.value() + hostDiff;
}


QString HostAddress::toString() const
{
    return QString("0x%1").arg(m_value, 2 * sizeof(value_type), 16, QChar('0'));
}


QTextStream& operator<<(QTextStream& os, const Address& addr)
{
    return os << addr.toString();
}


QTextStream& operator<<(QTextStream& os, const HostAddress& addr)
{
    return os << addr.toString();
}
