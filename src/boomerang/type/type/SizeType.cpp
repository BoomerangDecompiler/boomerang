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


SharedType SizeType::mergeWith(SharedType other) const
{
    SharedType ret = other->clone();

    ret->setSize(size);
    return ret;
}


std::shared_ptr< SizeType > SizeType::get(unsigned int sz)
{
    return std::make_shared<SizeType>(sz);
}


std::shared_ptr< SizeType > SizeType::get()
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
