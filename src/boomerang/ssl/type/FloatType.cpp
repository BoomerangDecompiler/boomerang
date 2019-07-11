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


FloatType::FloatType(Size sz)
    : Type(TypeClass::Float)
    , m_size(sz)
{
}


std::shared_ptr<FloatType> FloatType::get(Size sz)
{
    return std::make_shared<FloatType>(sz);
}


FloatType::~FloatType()
{
}


SharedType FloatType::clone() const
{
    return FloatType::get(m_size);
}


Type::Size FloatType::getSize() const
{
    return m_size;
}


void FloatType::setSize(Type::Size sz)
{
    m_size = sz;
}


bool FloatType::operator==(const Type &other) const
{
    if (!other.isFloat()) {
        return false;
    }
    else if (m_size == 0 || static_cast<const FloatType &>(other).m_size == 0) {
        return true;
    }

    return m_size == static_cast<const FloatType &>(other).m_size;
}


bool FloatType::operator<(const Type &other) const
{
    if (m_id != other.getId()) {
        return m_id < other.getId();
    }

    return m_size < static_cast<const FloatType &>(other).m_size;
}


QString FloatType::getCtype(bool /*final*/) const
{
    switch (m_size) {
    case 32: return "float";
    case 64: return "double";
    case 80: return "long double";
    default: return QString("__float%1").arg(m_size);
    }
}


SharedType FloatType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return const_cast<FloatType *>(this)->shared_from_this();
    }
    else if (other->resolvesToFloat()) {
        const Size newSize = std::max(getSize(), other->getSize());
        if (newSize != getSize()) {
            changed = true;
            return FloatType::get(newSize);
        }
        else {
            return const_cast<FloatType *>(this)->shared_from_this();
        }
    }
    else if (other->resolvesToSize() && other->getSize() == getSize()) {
        return const_cast<FloatType *>(this)->shared_from_this();
    }

    return createUnion(other, changed, useHighestPtr);
}


bool FloatType::isCompatible(const Type &other, bool all) const
{
    if (other.resolvesToVoid()) {
        return true;
    }
    else if (other.resolvesToFloat()) {
        return getSize() == other.getSize();
    }
    else if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }
    else if (!all && other.resolvesToArray()) {
        return isCompatibleWith(*static_cast<const ArrayType &>(other).getBaseType());
    }
    else if (other.resolvesToSize() && static_cast<const SizeType &>(other).getSize() == m_size) {
        return true;
    }

    return false;
}
