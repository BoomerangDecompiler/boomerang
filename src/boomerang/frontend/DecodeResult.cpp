#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DecodeResult.h"

#include "boomerang/ssl/RTL.h"


DecodeResult::DecodeResult()
{
    reset();
}


DecodeResult::DecodeResult(DecodeResult &&other)
    : rtl(std::move(other.rtl))
    , reLift(std::move(other.reLift))
{
}


DecodeResult::~DecodeResult()
{
}


DecodeResult &DecodeResult::operator=(DecodeResult &&other)
{
    rtl    = std::move(other.rtl);
    reLift = std::move(other.reLift);

    return *this;
}


void DecodeResult::reset()
{
    rtl    = nullptr;
    reLift = false;
}
