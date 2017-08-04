/*
 * Copyright (C) 1999-2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file register.cpp
 * \brief Register class descriptions
 *
 * Holds detailed information about a single register.
 ******************************************************************************/
#include "boomerang/db/Register.h"

#include "boomerang/type/Type.h"

#include <cassert>
#include <cstring>
#include <string>


Register::Register()
    : m_fltRegister(false)
    , m_address(nullptr)
    , m_mappedIndex(-1)
    , m_mappedOffset(-1)
{
}


Register::Register(const Register& r)
    : m_size(r.m_size)
    , m_fltRegister(r.m_fltRegister)
    , m_address(r.m_address)
    , m_mappedIndex(r.m_mappedIndex)
    , m_mappedOffset(r.m_mappedOffset)
{
    if (!r.m_name.isEmpty()) {
        m_name = r.m_name;
    }
}


Register& Register::operator=(const Register& r2)
{
    // copy operator
    if (this == &r2) {
        return *this;
    }

    m_name        = r2.m_name;
    m_size        = r2.m_size;
    m_fltRegister = r2.m_fltRegister;
    m_address     = r2.m_address;

    m_mappedIndex  = r2.m_mappedIndex;
    m_mappedOffset = r2.m_mappedOffset;

    return(*this);
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
    return(m_name < r2.m_name);
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

    return IntegerType::get(m_size);
}
