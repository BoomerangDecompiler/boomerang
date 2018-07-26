#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Register.h"


#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"

#include <cassert>
#include <cstring>
#include <string>


Register::Register(const QString& name, uint16_t sizeInBits, bool isFloatReg)
    : m_name(name)
    , m_size(sizeInBits)
    , m_fltRegister(isFloatReg)
    , m_mappedIndex(-1)
    , m_mappedOffset(-1)
{
}


Register::Register(const Register& r)
    : m_size(r.m_size)
    , m_fltRegister(r.m_fltRegister)
    , m_mappedIndex(r.m_mappedIndex)
    , m_mappedOffset(r.m_mappedOffset)
{
    if (!r.m_name.isEmpty()) {
        m_name = r.m_name;
    }
}


Register& Register::operator=(const Register& r2)
{
    if (this == &r2) {
        return *this;
    }

    m_name        = r2.m_name;
    m_size        = r2.m_size;
    m_fltRegister = r2.m_fltRegister;
    m_mappedIndex  = r2.m_mappedIndex;
    m_mappedOffset = r2.m_mappedOffset;

    return *this;
}


bool Register::operator==(const Register& r2) const
{
    // compare on name
    assert(!m_name.isEmpty() && !r2.m_name.isEmpty());
    return m_name == r2.m_name;
}


bool Register::operator<(const Register& r2) const
{
    assert(!m_name.isEmpty() && !r2.m_name.isEmpty());
    // compare on name
    return m_name < r2.m_name;
}


void Register::setName(const QString& s)
{
    assert(!s.isEmpty());
    m_name = s;
}


const QString& Register::getName() const
{
    return m_name;
}


SharedType Register::getType() const
{
    if (m_fltRegister) {
        return FloatType::get(m_size);
    }
    else {
        return IntegerType::get(m_size);
    }
}


uint16_t Register::getSize() const
{
    return m_size;
}
