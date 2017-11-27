#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BooleanType.h"


BooleanType::BooleanType()
    : Type(eBoolean)
{
}


BooleanType::~BooleanType()
{
}


SharedType BooleanType::clone() const
{
    return std::make_shared<BooleanType>();
}


size_t BooleanType::getSize() const
{
    return 1;
}


bool BooleanType::operator==(const Type& other) const
{
    return other.isBoolean();
}


bool BooleanType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
    }

    return true;
}


QString BooleanType::getCtype(bool /*final*/) const
{
    return "bool";
}
