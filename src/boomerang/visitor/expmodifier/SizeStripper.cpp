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


#include "boomerang/ssl/exp/Binary.h"


SharedExp SizeStripper::preModify(const std::shared_ptr<Binary>& exp, bool& visitChildren)
{
    visitChildren = true;

    if (exp->isSizeCast()) {
        // Could be a size cast of a size cast
        return exp->getSubExp2()->stripSizes();
    }

    return exp;
}
