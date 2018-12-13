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


Register::Register(RegID id, const QString &name)
    : m_id(id)
    , m_name(name)
{
}


Register::Register(const Register &r)
    : m_id(r.m_id)
    , m_name(r.m_name)
{
}


Register &Register::operator=(const Register &other)
{
    if (this != &other) {
        m_id   = other.m_id;
        m_name = other.m_name;
    }

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
    switch (m_id.getRegType()) {
    case RegType::Flags:
        if (m_id.getSize() == 1) {
            return BooleanType::get();
        }
        [[fallthrough]];
    case RegType::Int: return IntegerType::get(m_id.getSize(), Sign::Unknown);
    case RegType::Float: return FloatType::get(m_id.getSize());
    case RegType::Invalid: return VoidType::get();
    }

    return VoidType::get();
}


uint16_t Register::getSize() const
{
    return m_id.getSize();
}
