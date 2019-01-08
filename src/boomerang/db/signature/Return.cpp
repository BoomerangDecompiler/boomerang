#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Return.h"

#include "boomerang/ssl/exp/Exp.h"

Return::Return(SharedType _type, SharedExp _exp)
    : m_type(_type)
    , m_exp(_exp)
{
}


bool Return::operator==(const Return &other) const
{
    if (*m_type != *other.m_type) {
        return false;
    }

    if (*m_exp != *other.m_exp) {
        return false;
    }

    return true;
}


bool Return::operator<(const Return &other) const
{
    if (*m_type != *other.m_type) {
        return *m_type < *other.m_type;
    }
    else if (*m_exp != *other.m_exp) {
        return *m_exp < *other.m_exp;
    }

    return false; // equal
}


std::shared_ptr<Return> Return::clone() const
{
    return std::make_shared<Return>(m_type->clone(), SharedExp(m_exp->clone()));
}
