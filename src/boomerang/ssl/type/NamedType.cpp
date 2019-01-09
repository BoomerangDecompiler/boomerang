#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "NamedType.h"

#include "boomerang/util/log/Log.h"


NamedType::NamedType(const QString &_name)
    : Type(TypeClass::Named)
    , m_name(_name)
{
}


NamedType::~NamedType()
{
}


SharedType NamedType::clone() const
{
    return NamedType::get(m_name);
}


Type::Size NamedType::getSize() const
{
    SharedType ty = resolvesTo();

    if (ty) {
        return ty->getSize();
    }

    LOG_WARN("Unknown size for named type '%1'", m_name);
    return 0; // don't know
}


bool NamedType::operator==(const Type &other) const
{
    return other.isNamed() && m_name == static_cast<const NamedType &>(other).m_name;
}


bool NamedType::operator<(const Type &other) const
{
    if (m_id != other.getId()) {
        return m_id < other.getId();
    }

    return m_name < static_cast<const NamedType &>(other).m_name;
}


SharedType NamedType::resolvesTo() const
{
    SharedType ty = getNamedType(m_name);

    if (ty && ty->isNamed()) {
        return ty->as<NamedType>()->resolvesTo();
    }

    return ty;
}


QString NamedType::getCtype(bool /*final*/) const
{
    return m_name;
}


SharedType NamedType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    SharedType rt = resolvesTo();

    if (rt) {
        SharedType ret = rt->meetWith(other, changed, useHighestPtr);

        if (ret == rt) { // Retain the named type, much better than some compound type
            return const_cast<NamedType *>(this)->shared_from_this();
        }

        return ret; // Otherwise, whatever the result is
    }

    if (other->resolvesToVoid()) {
        return const_cast<NamedType *>(this)->shared_from_this();
    }

    if (*this == *other) {
        return const_cast<NamedType *>(this)->shared_from_this();
    }

    return createUnion(other, changed, useHighestPtr);
}


bool NamedType::isCompatible(const Type &other, bool /*all*/) const
{
    if (other.isNamed() && (m_name == static_cast<const NamedType &>(other).getName())) {
        return true;
    }

    SharedType resTo = resolvesTo();

    if (resTo) {
        return resolvesTo()->isCompatibleWith(other);
    }

    if (other.resolvesToVoid()) {
        return true;
    }

    return *this == other;
}
