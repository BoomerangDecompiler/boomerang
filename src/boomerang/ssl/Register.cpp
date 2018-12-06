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

#include "boomerang/ssl/type/BooleanType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/VoidType.h"

#include <cassert>
#include <cstring>
#include <string>


Register::Register(RegType type, const QString &name, uint16_t sizeInBits)
    : m_name(name)
    , m_size(sizeInBits)
    , m_regType(type)
    , m_mappedIndex(-1)
    , m_mappedOffset(-1)
{
}


Register::Register(const Register &r)
    : m_size(r.m_size)
    , m_regType(r.m_regType)
    , m_mappedIndex(r.m_mappedIndex)
    , m_mappedOffset(r.m_mappedOffset)
{
    if (!r.m_name.isEmpty()) {
        m_name = r.m_name;
    }
}


Register &Register::operator=(const Register &r2)
{
    if (this == &r2) {
        return *this;
    }

    m_name         = r2.m_name;
    m_size         = r2.m_size;
    m_regType      = r2.m_regType;
    m_mappedIndex  = r2.m_mappedIndex;
    m_mappedOffset = r2.m_mappedOffset;

    return *this;
}


bool Register::operator==(const Register &r2) const
{
    // compare on name
    assert(!m_name.isEmpty() && !r2.m_name.isEmpty());
    return m_name == r2.m_name;
}


bool Register::operator<(const Register &r2) const
{
    // compare on name
    assert(!m_name.isEmpty() && !r2.m_name.isEmpty());
    return m_name < r2.m_name;
}


const QString &Register::getName() const
{
    return m_name;
}


SharedType Register::getType() const
{
    switch (m_regType) {
    case RegType::Flags:
        if (m_size == 1) {
            return BooleanType::get();
        }
        [[fallthrough]];
    case RegType::Int: return IntegerType::get(m_size, Sign::Unknown);
    case RegType::Float: return FloatType::get(m_size);
    case RegType::Invalid: return VoidType::get();
    }

    return VoidType::get();
}


uint16_t Register::getSize() const
{
    return m_size;
}
