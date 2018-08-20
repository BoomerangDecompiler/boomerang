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


ArrayType::ArrayType(SharedType baseType, unsigned length)
    : Type(TypeClass::Array)
    , BaseType(baseType)
    , m_length(length)
{}


ArrayType::ArrayType()
    : Type(TypeClass::Array)
    , BaseType(nullptr)
    , m_length(0)

{}


bool ArrayType::isUnbounded() const
{
    return m_length == ARRAY_UNBOUNDED;
}


size_t ArrayType::convertLength(SharedType b) const
{
    // MVE: not sure if this is always the right thing to do
    if (m_length != ARRAY_UNBOUNDED) {
        size_t baseSize = BaseType->getSize() / 8; // Old base size (one element) in bytes

        if (baseSize == 0) {
            baseSize = 1; // Count void as size 1
        }

        baseSize *= m_length; // Old base size (length elements) in bytes
        size_t newSize = b->getSize() / 8;

        if (newSize == 0) {
            newSize = 1;
        }

        return baseSize / newSize; // Preserve same byte size for array
    }

    return ARRAY_UNBOUNDED;
}


void ArrayType::setBaseType(SharedType b)
{
    // MVE: not sure if this is always the right thing to do
    if (m_length != ARRAY_UNBOUNDED) {
        size_t baseSize = BaseType->getSize() / 8; // Old base size (one element) in bytes

        if (baseSize == 0) {
            baseSize = 1; // Count void as size 1
        }

        baseSize *= m_length; // Old base size (length elements) in bytes
        size_t newElementSize = b->getSize() / 8;

        if (newElementSize == 0) {
            newElementSize = 1;
        }

        m_length = baseSize / newElementSize; // Preserve same byte size for array
    }

    BaseType = b;
}


SharedType ArrayType::clone() const
{
    return ArrayType::get(BaseType->clone(), m_length);
}


size_t ArrayType::getSize() const
{
    return BaseType->getSize() * getLength();
}


bool ArrayType::operator==(const Type &other) const
{
    return other.isArray() && *BaseType == *static_cast<const ArrayType &>(other).BaseType &&
           static_cast<const ArrayType &>(other).m_length == m_length;
}


bool ArrayType::operator<(const Type &other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return (*BaseType < *static_cast<const ArrayType &>(other).BaseType);
}


QString ArrayType::getCtype(bool final) const
{
    QString s = BaseType->getCtype(final);

    if (isUnbounded()) {
        return s + "[]";
    }

    return s + "[" + QString::number(m_length) + "]";
}


void ArrayType::fixBaseType(SharedType b)
{
    if (BaseType == nullptr) {
        BaseType = b;
    }
    else {
        assert(BaseType->isArray());
        BaseType->as<ArrayType>()->fixBaseType(b);
    }
}


SharedType ArrayType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return const_cast<ArrayType *>(this)->shared_from_this();
    }

    if (other->resolvesToArray()) {
        auto otherArr      = other->as<ArrayType>();
        SharedType newBase = BaseType->clone()->meetWith(otherArr->BaseType, changed,
                                                         useHighestPtr);
        size_t newLength   = m_length;

        if (*newBase != *BaseType) {
            changed   = true;
            newLength = convertLength(newBase);
        }

        newLength = std::min(newLength, other->as<ArrayType>()->getLength());
        return ArrayType::get(newBase, newLength);
    }

    if (*BaseType == *other) {
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
        size_t bitsize  = BaseType->getSize();
        size_t new_size = other->getSize();

        if (BaseType->isComplete() && !other->isComplete()) {
            // complete types win
            return std::const_pointer_cast<Type>(this->shared_from_this());
        }

        if (bitsize == new_size) {
            // same size, prefer Int/Float over SizeType
            if (!BaseType->isSize() && other->isSize()) {
                return std::const_pointer_cast<Type>(this->shared_from_this());
            }
        }

        auto bt = BaseType->clone();
        bool base_changed;
        auto res = bt->meetWith(other, base_changed);

        if (res == bt) {
            return std::const_pointer_cast<Type>(this->shared_from_this());
        }

        size_t new_length = m_length;

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

    if (other.resolvesToArray() && BaseType->isCompatibleWith(*other.as<ArrayType>()->BaseType)) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (!all && BaseType->isCompatibleWith(other)) {
        return true; // An array of x is compatible with x
    }

    return false;
}
