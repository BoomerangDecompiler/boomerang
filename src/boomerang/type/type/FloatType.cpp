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


#include "boomerang/type/type/ArrayType.h"
#include "boomerang/type/type/SizeType.h"


FloatType::FloatType(int sz)
    : Type(eFloat)
    , size(sz)
{
}


std::shared_ptr<FloatType> FloatType::get(int sz)
{
    return std::make_shared<FloatType>(sz);
}


FloatType::~FloatType()
{
}


SharedType FloatType::clone() const
{
    return FloatType::get(size);
}


size_t FloatType::getSize() const
{
    return size;
}


bool FloatType::operator==(const Type& other) const
{
    return other.isFloat() && (size == 0 || ((FloatType&)other).size == 0 || (size == ((FloatType&)other).size));
}


bool FloatType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return(size < ((FloatType&)other).size);
}


QString FloatType::getCtype(bool /*final*/) const
{
    switch (size)
    {
    case 32:
        return "float";

    case 64:
        return "double";

    default:
        return "double";
    }
}


QString FloatType::getTempName() const
{
    switch (size)
    {
    case 32:
        return "tmpf";

    case 64:
        return "tmpd";

    case 80:
        return "tmpF";

    case 128:
        return "tmpD";
    }

    return "tmp";
}


SharedType FloatType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return ((FloatType *)this)->shared_from_this();
    }

    if (other->resolvesToFloat() || other->resolvesToSize()) {
        const size_t newSize = std::max(size, other->getSize());
        ch |= (newSize != size);
        return FloatType::get(newSize);
    }

    return createUnion(other, ch, bHighestPtr);
}


bool FloatType::isCompatible(const Type& other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToFloat()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToArray()) {
        return isCompatibleWith(*((const ArrayType&)other).getBaseType());
    }

    if (other.resolvesToSize() && (((const SizeType&)other).getSize() == size)) {
        return true;
    }

    return false;
}
