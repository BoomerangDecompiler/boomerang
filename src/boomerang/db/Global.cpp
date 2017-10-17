#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Global.h"


Global::Global(SharedType type, Address addr, const QString& name, Prog *prog)
    : m_type(type)
    , m_addr(addr)
    , m_name(name)
    , m_program(prog)
{
}


Global::Global()
    : m_type(nullptr)
    , m_addr(Address::ZERO)
{
}


bool Global::containsAddress(Address addr) const
{
    return Util::inRange(addr, m_addr, m_addr + getType()->getSizeInBytes());
}
