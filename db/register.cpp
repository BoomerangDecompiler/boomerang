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

/***************************************************************************//**
 * \file register.cpp
 * Desc: Register class descriptions.  Holds detailed information about
 *         a single register.
 ******************************************************************************/

#include <cassert>
#include <cstring>
#include "register.h"
#include "type.h"
#include "util.h"

Register::Register() : address(nullptr), mappedIndex(-1),
    mappedOffset(-1), flt(false)
{}

/***************************************************************************//**
 * \brief      Copy constructor.
 * PARAMETERS:      Reference to another Register object to construct from
 ******************************************************************************/
Register::Register(const Register& r) : size(r.size),
    address(r.address),    mappedIndex(r.mappedIndex),
    mappedOffset(r.mappedOffset), flt(r.flt)
{
    if (!r.name.empty())
        name = r.name;
}

/***************************************************************************//**
 * \brief      Copy operator
 * \param      Reference to another Register object (to be copied)
 * \returns           This object
 ******************************************************************************/
Register &Register::operator=(const Register& r2) {
    // copy operator
    if(this==&r2)
        return *this;
    name    = r2.name;
    size    = r2.size;
    flt     = r2.flt;
    address = r2.address;

    mappedIndex = r2.mappedIndex;
    mappedOffset = r2.mappedOffset;

    return(*this);
}

/***************************************************************************//**
 * \brief   Equality operator
 * \param   Reference to another Register object
 * \returns True if the same
 ******************************************************************************/
bool Register::operator==(const Register& r2) const {
    // compare on name
    assert(!name.empty() && !r2.name.empty());
    return strcmp(name.c_str(), r2.name.c_str()) == 0;
}

/***************************************************************************//**
 * \brief   Comparison operator (to establish an ordering)
 * \param   Reference to another Register object
 * \returns true if this name is less than the given Register's name
 ******************************************************************************/
bool Register::operator<(const Register& r2) const {
    assert( !name.empty() && !r2.name.empty());

    // compare on name
    if (strcmp(name.c_str(), r2.name.c_str()) < 0)
        return true;
    return false;
}

/***************************************************************************//**
 * \brief      Set the name for this register
 * \param      s: name to set it to
 *
 ******************************************************************************/
void Register::s_name(const char *s) {
    assert(s != nullptr);
    name = s;
}

/***************************************************************************//**
 * \brief      Get the name for this register
 * \returns           The name as a character string
 ******************************************************************************/
std::string Register::g_name() const {
    return name;
}

/***************************************************************************//**
 * \brief   Get the type for this register
 * \returns The type as a pointer to a Type object
 ******************************************************************************/
Type* Register::g_type() const {
    if (flt)
        return FloatType::get(size);
    return IntegerType::get(size);
}
