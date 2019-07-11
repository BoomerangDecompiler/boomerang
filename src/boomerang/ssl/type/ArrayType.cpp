#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ArrayType.h"


ArrayType::ArrayType(SharedType baseType, uint64 length)
    : Type(TypeClass::Array)
    , m_baseType(baseType)
    , m_length(length)
{
}


std::shared_ptr<ArrayType> ArrayType::get(SharedType p, uint64 length)
{
    return std::make_shared<ArrayType>(p, length);
}


bool ArrayType::isCompatibleWith(const Type &other, bool all) const
{
    return isCompatible(other, all);
}


bool ArrayType::isUnbounded() const
{
    return m_length == ARRAY_UNBOUNDED;
}


uint64 ArrayType::convertLength(SharedType b) const
{
    // MVE: not sure if this is always the right thing to do
    if (m_length != ARRAY_UNBOUNDED) {
        // Old base size (one element) in bytes. Count void as size 1
        const Size oldSize     = std::max((Size)1, m_baseType->getSize() / 8) * m_length;
        const Size newBaseSize = std::max((Size)1, b->getSize() / 8);
        return oldSize / newBaseSize; // Preserve same byte size for array
    }

    return ARRAY_UNBOUNDED;
}


void ArrayType::setBaseType(SharedType b)
{
    // MVE: not sure if this is always the right thing to do
    if (m_baseType && m_length != ARRAY_UNBOUNDED) {
        Size baseSize = m_baseType->getSize() / 8; // Old base size (one element) in bytes

        if (baseSize == 0) {
            baseSize = 1; // Count void as size 1
        }

        baseSize *= m_length; // Old base size (length elements) in bytes
        Size newElementSize = b->getSize() / 8;

        if (newElementSize == 0) {
            newElementSize = 1;
        }

        m_length = baseSize / newElementSize; // Preserve same byte size for array
    }

    m_baseType = b;
}


SharedType ArrayType::clone() const
{
    return ArrayType::get(m_baseType->clone(), m_length);
}


Type::Size ArrayType::getSize() const
{
    return m_baseType->getSize() * getLength();
}


bool ArrayType::operator==(const Type &other) const
{
    if (!other.isArray()) {
        return false;
    }

    const ArrayType &otherArr = static_cast<const ArrayType &>(other);
    return m_length == otherArr.getLength() && *m_baseType == *otherArr.getBaseType();
}


bool ArrayType::operator<(const Type &other) const
{
    if (m_id != other.getId()) {
        return m_id < other.getId();
    }

    const ArrayType &otherArr = static_cast<const ArrayType &>(other);
    return *m_baseType < *otherArr.getBaseType() || m_length < otherArr.getLength();
}


QString ArrayType::getCtype(bool final) const
{
    const QString baseType = m_baseType->getCtype(final);

    if (isUnbounded()) {
        return baseType + "[]";
    }
    else {
        return baseType + "[" + QString::number(m_length, 10) + "]";
    }
}


SharedType ArrayType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return const_cast<ArrayType *>(this)->shared_from_this();
    }

    if (other->resolvesToArray()) {
        auto otherArr      = other->as<ArrayType>();
        SharedType newBase = m_baseType->clone()->meetWith(otherArr->m_baseType, changed,
                                                           useHighestPtr);
        uint64 newLength   = m_length;

        if (*newBase != *m_baseType) {
            changed   = true;
            newLength = convertLength(newBase);
        }

        if (other->as<ArrayType>()->getLength() < newLength) {
            newLength = other->as<ArrayType>()->getLength();
            changed   = true;
        }

        return ArrayType::get(newBase, newLength);
    }

    if (*m_baseType == *other) {
        return const_cast<ArrayType *>(this)->shared_from_this();
    }

    /*
     * checks if 'other' is compatible with the ArrayType, if it is
     * checks if other's 'completeness' is less then current BaseType, if it is, unchanged type is
     * returned. checks if sizes of BaseType and other match, if they do, checks if other is less
     * complete ( SizeType vs NonSize type ), if that happens unchanged type is returned then it
     * clones the BaseType and tries to 'meetWith' with other, if this results in unchanged type,
     * unchanged type is returned otherwise a new ArrayType is returned, with it's size recalculated
     * based on new BaseType
     */
    if (isCompatible(*other, false)) { // compatible with all ?
        Type::Size bitsize  = m_baseType->getSize();
        Type::Size new_size = other->getSize();

        if (m_baseType->isComplete() && !other->isComplete()) {
            // complete types win
            return std::const_pointer_cast<Type>(this->shared_from_this());
        }

        if (bitsize == new_size) {
            // same size, prefer Int/Float over SizeType
            if (!m_baseType->isSize() && other->isSize()) {
                return std::const_pointer_cast<Type>(this->shared_from_this());
            }
        }

        auto bt = m_baseType->clone();
        bool base_changed;
        auto res = bt->meetWith(other, base_changed);

        if (res == bt) {
            return std::const_pointer_cast<Type>(this->shared_from_this());
        }

        uint64 new_length = m_length;

        if (m_length != ARRAY_UNBOUNDED) {
            new_length = (m_length * bitsize) / new_size;
        }

        return ArrayType::get(res, new_length);
    }

    // Needs work?
    return createUnion(other, changed, useHighestPtr);
}


bool ArrayType::isCompatible(const Type &other, bool all) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToArray() &&
        m_baseType->isCompatibleWith(*other.as<ArrayType>()->m_baseType)) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (!all && m_baseType->isCompatibleWith(other)) {
        return true; // An array of x is compatible with x
    }

    return false;
}
