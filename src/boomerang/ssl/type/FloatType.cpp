#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "FloatType.h"

#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/SizeType.h"


FloatType::FloatType(int sz)
    : Type(TypeClass::Float)
    , size(sz)
{}


std::shared_ptr<FloatType> FloatType::get(int sz)
{
    return std::make_shared<FloatType>(sz);
}


FloatType::~FloatType()
{}


SharedType FloatType::clone() const
{
    return FloatType::get(size);
}


size_t FloatType::getSize() const
{
    return size;
}


bool FloatType::operator==(const Type &other) const
{
    if (!other.isFloat()) {
        return false;
    }
    else if (size == 0 || static_cast<const FloatType &>(other).size == 0) {
        return true;
    }

    return size == static_cast<const FloatType &>(other).size;
}


bool FloatType::operator<(const Type &other) const
{
    if (id != other.getId()) {
        return id < other.getId();
    }

    return size < static_cast<const FloatType &>(other).size;
}


QString FloatType::getCtype(bool /*final*/) const
{
    switch (size) {
    case 32: return "float";
    case 64: return "double";
    default: return "double";
    }
}


QString FloatType::getTempName() const
{
    switch (size) {
    case 32: return "tmpf";
    case 64: return "tmpd";
    case 80: return "tmpF";
    case 128: return "tmpD";
    }

    return "tmp";
}


SharedType FloatType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return const_cast<FloatType *>(this)->shared_from_this();
    }

    if (other->resolvesToFloat() || other->resolvesToSize()) {
        const size_t newSize = std::max(size, other->getSize());
        changed |= (newSize != size);
        return FloatType::get(newSize);
    }

    return createUnion(other, changed, useHighestPtr);
}


bool FloatType::isCompatible(const Type &other, bool /*all*/) const
{
    if (other.resolvesToVoid() || other.resolvesToFloat()) {
        return true;
    }
    else if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }
    else if (other.resolvesToArray()) {
        return isCompatibleWith(*static_cast<const ArrayType &>(other).getBaseType());
    }
    else if (other.resolvesToSize() && static_cast<const SizeType &>(other).getSize() == size) {
        return true;
    }

    return false;
}
