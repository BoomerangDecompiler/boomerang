#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BooleanType.h"

#include "boomerang/ssl/type/SizeType.h"


BooleanType::BooleanType()
    : Type(TypeClass::Boolean)
{
}


BooleanType::~BooleanType()
{
}


SharedType BooleanType::clone() const
{
    return std::make_shared<BooleanType>();
}


size_t BooleanType::getSize() const
{
    return 1;
}


bool BooleanType::operator==(const Type &other) const
{
    return other.isBoolean();
}


bool BooleanType::operator<(const Type &other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return true;
}


QString BooleanType::getCtype(bool /*final*/) const
{
    return "bool";
}


SharedType BooleanType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid() || other->resolvesToBoolean()) {
        return const_cast<BooleanType *>(this)->shared_from_this();
    }

    return createUnion(other, changed, useHighestPtr);
}


bool BooleanType::isCompatible(const Type &other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToBoolean()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToSize() && static_cast<const SizeType &>(other).getSize() == 1) {
        return true;
    }

    return false;
}
