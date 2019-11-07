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
    : iclass(std::move(other.iclass))
    , reDecode(std::move(other.reDecode))
    , numBytes(std::move(other.numBytes))
    , rtl(std::move(other.rtl))
{
}


DecodeResult::~DecodeResult()
{
}


DecodeResult &DecodeResult::operator=(DecodeResult &&other)
{
    iclass   = std::move(other.iclass);
    reDecode = std::move(other.reDecode);
    numBytes = std::move(other.numBytes);
    rtl      = std::move(other.rtl);

    return *this;
}


void DecodeResult::reset()
{
    numBytes = 0;
    iclass   = IClass::NCT;
    rtl      = nullptr;
    reDecode = false;
}
