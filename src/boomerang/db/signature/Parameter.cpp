#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Parameter.h"

#include "boomerang/ssl/exp/Exp.h"


Parameter::Parameter(SharedType type, const QString &name, SharedExp exp, const QString &boundMax)
    : m_type(type)
    , m_name(name)
    , m_exp(exp)
    , m_boundMax(boundMax)
{
}


bool Parameter::operator==(const Parameter &other) const
{
    if (*m_type != *other.m_type) {
        return false;
    }

    // Do we really care about a parameter's name?
    if (m_name != other.m_name) {
        return false;
    }

    if (*m_exp != *other.m_exp) {
        return false;
    }

    return true;
}


bool Parameter::operator<(const Parameter &other) const
{
    if (*m_type != *other.m_type) {
        return *m_type < *other.m_type;
    }

    // Do we really care about a parameter's name?
    if (m_name != other.m_name) {
        return m_name < other.m_name;
    }

    if (*m_exp != *other.m_exp) {
        return *m_exp < *other.m_exp;
    }

    return true;
}


std::shared_ptr<Parameter> Parameter::clone() const
{
    return std::make_shared<Parameter>(m_type->clone(), m_name, m_exp->clone(), m_boundMax);
}


void Parameter::setBoundMax(const QString &name)
{
    m_boundMax = name;
}
