#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CharType.h"


CharType::CharType()
    : Type(eChar)
{
}


CharType::~CharType()
{
}


SharedType CharType::clone() const
{
    return CharType::get();
}


size_t CharType::getSize() const
{
    return 8;
}


bool CharType::operator==(const Type& other) const
{
    return other.isChar();
}


bool CharType::operator<(const Type& other) const
{
    return id < other.getId();
}


SharedExp CharType::match(SharedType pattern)
{
    return Type::match(pattern);
}


QString CharType::getCtype(bool /*final*/) const
{
    return "char";
}
