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


Type::Size BooleanType::getSize() const
{
    return 1;
}


bool BooleanType::operator==(const Type &other) const
{
    return other.isBoolean();
}


bool BooleanType::operator<(const Type &other) const
{
    return getId() < other.getId();
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
    if (other.resolvesToVoid() || other.resolvesToBoolean()) {
        return true;
    }
    else if (other.resolvesToSize() && static_cast<const SizeType &>(other).getSize() == 1) {
        return true;
    }
    else if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    return false;
}
