#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SizeStripper.h"


#include "boomerang/db/exp/Binary.h"


SharedExp SizeStripper::preVisit(const std::shared_ptr<Binary>& b, bool& recur)
{
    recur = true; // Visit the binary's children

    if (b->isSizeCast()) {
        // Could be a size cast of a size cast
        return b->getSubExp2()->stripSizes();
    }

    return b;
}
