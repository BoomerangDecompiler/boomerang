#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "VoidType.h"

#include "boomerang/ssl/type/UnionType.h"


VoidType::VoidType()
    : Type(TypeClass::Void)
{
}


VoidType::~VoidType()
{
}


SharedType VoidType::clone() const
{
    return VoidType::get();
}


Type::Size VoidType::getSize() const
{
    return 0;
}


bool VoidType::operator==(const Type &other) const
{
    return other.isVoid();
}


bool VoidType::operator<(const Type &other) const
{
    return m_id < other.getId();
}


QString VoidType::getCtype(bool /*final*/) const
{
    return "void";
}


SharedType VoidType::meetWith(SharedType other, bool &changed, bool) const
{
    if (other->resolvesToUnion()) {
        changed = true;
        return other->as<UnionType>()->simplify(changed)->clone();
    }
    else {
        // void meet x = x
        changed |= !other->resolvesToVoid();
        return other->clone();
    }
}


bool VoidType::isCompatible(const Type & /*other*/, bool /*all*/) const
{
    return true; // Void is compatible with any type
}
