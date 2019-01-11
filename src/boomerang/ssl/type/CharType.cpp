#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CharType.h"

#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/SizeType.h"


CharType::CharType()
    : Type(TypeClass::Char)
{
}


CharType::~CharType()
{
}


SharedType CharType::clone() const
{
    return CharType::get();
}


Type::Size CharType::getSize() const
{
    return 8;
}


bool CharType::operator==(const Type &other) const
{
    return other.isChar();
}


bool CharType::operator<(const Type &other) const
{
    return m_id < other.getId();
}


QString CharType::getCtype(bool /*final*/) const
{
    return "char";
}


SharedType CharType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid() || other->resolvesToChar()) {
        return const_cast<CharType *>(this)->shared_from_this();
    }

    // Also allow char to merge with integer
    if (other->resolvesToInteger()) {
        changed = true;
        return other->clone();
    }

    if (other->resolvesToSize() && getSize() <= (other->as<SizeType>()->getSize())) {
        return const_cast<CharType *>(this)->shared_from_this();
    }

    return createUnion(other, changed, useHighestPtr);
}


bool CharType::isCompatible(const Type &other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToChar()) {
        return true;
    }

    if (other.resolvesToInteger()) {
        return true;
    }

    if (other.resolvesToSize() && static_cast<const SizeType &>(other).getSize() == 8) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToArray()) {
        return isCompatibleWith(*static_cast<const ArrayType &>(other).getBaseType());
    }

    return false;
}
