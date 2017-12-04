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


#include "boomerang/util/Log.h"
#include "boomerang/type/type/ArrayType.h"


SizeType::SizeType()
    : Type(eSize)
    , size(0)
{
}


SizeType::SizeType(unsigned sz)
    : Type(eSize)
    , size(sz)
{
}


SizeType::~SizeType()
{
}


SharedType SizeType::clone() const
{
    return SizeType::get(size);
}


size_t SizeType::getSize() const
{
    return size;
}


bool SizeType::operator==(const Type& other) const
{
    return other.isSize() && (size == ((SizeType&)other).size);
}


bool SizeType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return(size < ((SizeType&)other).size);
}


std::shared_ptr<SizeType> SizeType::get(unsigned int sz)
{
    return std::make_shared<SizeType>(sz);
}


std::shared_ptr<SizeType> SizeType::get()
{
    return std::make_shared<SizeType>();
}


void SizeType::setSize(size_t sz)
{
    size = sz;
}


bool SizeType::isSize() const
{
    return true;
}


bool SizeType::isComplete()
{
    return false;
}


QString SizeType::getCtype(bool /*final*/) const
{
    // Emit a comment and the size
    QString     res;
    QTextStream ost(&res);

    ost << "__size" << size;
    return res;
}


SharedType SizeType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return ((SizeType *)this)->shared_from_this();
    }

    if (other->resolvesToSize()) {
        SharedType result = this->clone();

        if (other->as<SizeType>()->size != size) {
            LOG_VERBOSE("Size %1 meet with size %2!", size, other->as<SizeType>()->size);
        }

        result->setSize(std::max(result->getSize(), other->as<SizeType>()->getSize()));

        return result;
    }

    ch = true;

    if (other->resolvesToInteger()) {
        if (other->getSize() == 0) {
            other->setSize(size);
            return other->clone();
        }

        if (other->getSize() != size) {
            LOG_WARN("Size %1 meet with %2; allowing temporarily",
                     size, other->getCtype());
        }

        return other->clone();
    }

    return createUnion(other, ch, bHighestPtr);
}


bool SizeType::isCompatible(const Type& other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    size_t otherSize = other.getSize();

    if (other.resolvesToFunc()) {
        return false;
    }

    // FIXME: why is there a test for size 0 here?
    // This is because some signatures leave us with 0-sized NamedType -> using GLEnum when it was not defined.
    if (otherSize == size) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToArray()) {
        return isCompatibleWith(*((const ArrayType&)other).getBaseType());
    }

    // return false;
    // For now, size32 and double will be considered compatible (helps test/pentium/global2)
    return false;
}

