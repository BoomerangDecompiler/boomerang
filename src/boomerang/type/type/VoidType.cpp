#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "VoidType.h"


VoidType::VoidType()
    : Type(eVoid)
{
}


VoidType::~VoidType()
{
}


SharedType VoidType::clone() const
{
    return VoidType::get();
}


size_t VoidType::getSize() const
{
    return 0;
}


bool VoidType::operator==(const Type& other) const
{
    return other.isVoid();
}


bool VoidType::operator<(const Type& other) const
{
    return id < other.getId();
}


SharedExp VoidType::match(SharedType pattern)
{
    return Type::match(pattern);
}


QString VoidType::getCtype(bool /*final*/) const
{
    return "void";
}
