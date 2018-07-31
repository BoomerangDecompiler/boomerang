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


void DecodeResult::reset()
{
    numBytes     = 0;
    type         = NCT;
    valid        = true;
    rtl          = nullptr;
    reDecode     = false;
    forceOutEdge = Address::ZERO;
}
