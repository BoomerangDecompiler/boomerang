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


ArrayType::ArrayType(SharedType p, unsigned _length)
    : Type(eArray)
    , BaseType(p)
    , m_length(_length)
{
}


ArrayType::ArrayType(SharedType p)
    : Type(eArray)
    , BaseType(p)
    , m_length(NO_BOUND)
{
}


ArrayType::~ArrayType()
{
}


bool ArrayType::isUnbounded() const
{
    return m_length == NO_BOUND;
}


size_t ArrayType::convertLength(SharedType b) const
{
    // MVE: not sure if this is always the right thing to do
    if (m_length != NO_BOUND) {
        size_t baseSize = BaseType->getSize() / 8; // Old base size (one element) in bytes

        if (baseSize == 0) {
            baseSize = 1;   // Count void as size 1
        }

        baseSize *= m_length; // Old base size (length elements) in bytes
        size_t newSize = b->getSize() / 8;

        if (newSize == 0) {
            newSize = 1;
        }

        return baseSize / newSize; // Preserve same byte size for array
    }

    return NO_BOUND;
}


void ArrayType::setBaseType(SharedType b)
{
    // MVE: not sure if this is always the right thing to do
    if (m_length != NO_BOUND) {
        size_t baseSize = BaseType->getSize() / 8; // Old base size (one element) in bytes

        if (baseSize == 0) {
            baseSize = 1;   // Count void as size 1
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


bool ArrayType::operator==(const Type& other) const
{
    return other.isArray() && *BaseType == *((ArrayType&)other).BaseType && ((ArrayType&)other).m_length == m_length;
}


bool ArrayType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return(*BaseType < *((ArrayType&)other).BaseType);
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
