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
 * \file register.cc
 * Desc: Register class descriptions.  Holds detailed information about
 *         a single register.
 ******************************************************************************/

/* $Revision$
 *
 * 28 Apr 02 - Mike: Mods for boomerang
 */


#if defined(_MSC_VER) && _MSC_VER <= 1200
// For MSVC 5 or 6: warning about debug into truncated to 255 chars
#pragma warning(disable:4786)
#endif
#include <cassert>
#include <cstring>
#include "register.h"
#include "type.h"
#include "util.h"

#if defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(disable:4996)        // Warnings about e.g. _strdup deprecated in VS 2005
#endif
#ifndef nullptr                        // Don't always include stdio.h
#define nullptr 0
#endif

/***************************************************************************//**
 * FUNCTION:      Register::Register
 * \brief      Constructor.
 * PARAMETERS:      <none>
 * \returns           N/A
 ******************************************************************************/
Register::Register() : name(nullptr), address(nullptr), mappedIndex(-1),
    mappedOffset(-1), flt(false)
{}

/***************************************************************************//**
 * FUNCTION:      Register::Register
 * \brief      Copy constructor.
 * PARAMETERS:      Reference to another Register object to construct from
 * \returns           N/A
 ******************************************************************************/
Register::Register(const Register& r) : name(nullptr), size(r.size),
    address(r.address),    mappedIndex(r.mappedIndex),
    mappedOffset(r.mappedOffset), flt(r.flt)
{
    if (r.name != nullptr)
        name = strdup(r.name);
}

/***************************************************************************//**
 * FUNCTION:      Register::operator=
 * \brief      Copy operator
 * PARAMETERS:      Reference to another Register object (to be copied)
 * \returns           This object
 ******************************************************************************/
Register Register::operator=(const Register& r2) {
    // copy operator

    //if (name != nullptr)
    //free(name);
    name = r2.name;
    size = r2.size;
    flt     = r2.flt;
    address = r2.address;

    mappedIndex = r2.mappedIndex;
    mappedOffset = r2.mappedOffset;

    return(*this);
}

/***************************************************************************//**
 * FUNCTION:      Register::operator==
 * \brief      Equality operator
 * PARAMETERS:      Reference to another Register object
 * \returns           True if the same
 ******************************************************************************/
bool Register::operator==(const Register& r2) const {
    // compare on name
    assert(name != nullptr && r2.name != nullptr);
    if (strcmp(name, r2.name) != 0)
        return false;
    return true;
}

/***************************************************************************//**
 * FUNCTION:      Register::operator<
 * \brief      Comparison operator (to establish an ordering)
 * PARAMETERS:      Reference to another Register object
 * \returns           true if this name is less than the given Register's name
 ******************************************************************************/
bool Register::operator<(const Register& r2) const {
    assert(name != nullptr && r2.name != nullptr);

    // compare on name
    if (strcmp(name, r2.name) < 0)
        return true;
    return false;
}

/***************************************************************************//**
 * FUNCTION:      Register::s_name
 * \brief      Set the name for this register
 * PARAMETERS:      s: name to set it to
 * \returns           <nothing>
 ******************************************************************************/
void Register::s_name(const char *s) {
    assert(s != nullptr);

    //if (name != nullptr)
    //free(name);
    name = strdup(s);
}

/***************************************************************************//**
 * FUNCTION:      Register::g_name
 * \brief      Get the name for this register
 * PARAMETERS:      <none>
 * \returns           The name as a character string
 ******************************************************************************/
const char *Register::g_name() const {
    static char outname[100];

    strncpy(outname, name, 100);
    outname[99] = '\0';
    return(outname);
}

/***************************************************************************//**
 * FUNCTION:      Register::g_type
 * \brief      Get the type for this register
 * PARAMETERS:      <none>
 * \returns           The type as a pointer to a Type object
 ******************************************************************************/
Type* Register::g_type() const {
    if (flt)
        return new FloatType(size);
    return new IntegerType(size);
}
