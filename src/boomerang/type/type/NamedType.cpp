#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "IntegerType.h"


#include "boomerang/util/Log.h"


NamedType::NamedType(const QString& _name)
    : Type(eNamed)
    , name(_name)
{
}


NamedType::~NamedType()
{
}


SharedType NamedType::clone() const
{
    return NamedType::get(name);
}


size_t NamedType::getSize() const
{
    SharedType ty = resolvesTo();

    if (ty) {
        return ty->getSize();
    }

    LOG_WARN("Unknown size for named type %1", name);
    return 0; // don't know
}


bool NamedType::operator==(const Type& other) const
{
    return other.isNamed() && name == static_cast<const NamedType &>(other).name;
}


SharedType NamedType::resolvesTo() const
{
    SharedType ty = getNamedType(name);

    if (ty && ty->isNamed()) {
        return std::static_pointer_cast<NamedType>(ty)->resolvesTo();
    }

    return ty;
}


bool NamedType::operator<(const Type& other) const
{
    if (id != other.getId()) {
        return id < other.getId();
    }

    return name < static_cast<const NamedType &>(other).name;
}


QString NamedType::getCtype(bool /*final*/) const
{
    return name;
}


SharedType NamedType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    SharedType rt = resolvesTo();

    if (rt) {
        SharedType ret = rt->meetWith(other, ch, bHighestPtr);

        if (ret == rt) { // Retain the named type, much better than some compound type
            return const_cast<NamedType *>(this)->shared_from_this();
        }

        return ret;              // Otherwise, whatever the result is
    }

    if (other->resolvesToVoid()) {
        return const_cast<NamedType *>(this)->shared_from_this();
    }

    if (*this == *other) {
        return const_cast<NamedType *>(this)->shared_from_this();
    }

    return createUnion(other, ch, bHighestPtr);
}


bool NamedType::isCompatible(const Type& other, bool /*all*/) const
{
    if (other.isNamed() && (name == static_cast<const NamedType &>(other).getName())) {
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
