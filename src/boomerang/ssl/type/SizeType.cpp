#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SizeType.h"

#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/util/log/Log.h"


SizeType::SizeType()
    : Type(TypeClass::Size)
    , m_size(0)
{
}


SizeType::SizeType(Type::Size sz)
    : Type(TypeClass::Size)
    , m_size(sz)
{
}


SizeType::~SizeType()
{
}


SharedType SizeType::clone() const
{
    return SizeType::get(m_size);
}


Type::Size SizeType::getSize() const
{
    return m_size;
}


bool SizeType::operator==(const Type &other) const
{
    return other.isSize() && (m_size == static_cast<const SizeType &>(other).m_size);
}


bool SizeType::operator<(const Type &other) const
{
    if (getId() != other.getId()) {
        return getId() < other.getId();
    }

    return m_size < static_cast<const SizeType &>(other).m_size;
}


std::shared_ptr<SizeType> SizeType::get(Type::Size sz)
{
    return std::make_shared<SizeType>(sz);
}


std::shared_ptr<SizeType> SizeType::get()
{
    return std::make_shared<SizeType>();
}


void SizeType::setSize(Size sz)
{
    m_size = sz;
}


bool SizeType::isComplete()
{
    return false;
}


QString SizeType::getCtype(bool /*final*/) const
{
    // Emit a comment and the size
    QString res;
    OStream ost(&res);

    ost << "__size" << m_size;
    return res;
}


SharedType SizeType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return const_cast<SizeType *>(this)->shared_from_this();
    }
    else if (other->resolvesToPointer() && other->getSize() == getSize()) {
        // e.g. size32 meet void *
        changed = true;
        return other->clone();
    }

    if (other->resolvesToSize()) {
        SharedType result = this->clone();

        if (other->as<SizeType>()->m_size != m_size) {
            LOG_VERBOSE("Size %1 meet with size %2!", m_size, other->as<SizeType>()->m_size);
        }

        const Size newSize = std::max(result->getSize(), other->as<SizeType>()->getSize());
        changed |= (newSize != result->getSize());
        result->setSize(newSize);
        return result;
    }

    changed = true;

    if (other->resolvesToInteger()) {
        if (other->getSize() == 0) {
            other->setSize(m_size);
            return other->clone();
        }

        if (other->getSize() != m_size) {
            LOG_VERBOSE("Size %1 meet with %2; allowing temporarily", m_size, other->getCtype());
        }

        return other->clone();
    }
    else if (other->resolvesToChar() && other->getSize() <= getSize()) {
        changed = true;
        return other->clone();
    }
    else if (other->resolvesToFloat() && getSize() == other->getSize()) {
        changed = true;
        return other->clone();
    }

    return createUnion(other, changed, useHighestPtr);
}


bool SizeType::isCompatible(const Type &other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    const Size otherSize = other.getSize();

    if (other.resolvesToFunc()) {
        return false;
    }

    // FIXME: why is there a test for size 0 here?
    // This is because some signatures leave us with 0-sized NamedType -> using GLEnum when it was
    // not defined.
    if (otherSize == m_size) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToArray()) {
        return isCompatibleWith(*static_cast<const ArrayType &>(other).getBaseType());
    }

    // return false;
    // For now, size32 and double will be considered compatible (helps test/x86/global2)
    return false;
}
