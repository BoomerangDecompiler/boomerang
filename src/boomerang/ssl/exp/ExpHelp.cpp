#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpHelp.h"


#include "boomerang/ssl/exp/Exp.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/util/log/Log.h"


// A helper class for comparing Exp*'s sensibly
bool lessExpStar::operator()(const SharedConstExp& x, const SharedConstExp& y) const
{
    return (*x < *y);   // Compare the actual Exps
}



void printChild(const SharedExp& e, int ind)
{
    if (e == nullptr) {
        LOG_MSG("%1<nullptr>", QString(ind + 4, ' '));
        return;
    }

    e->printx(ind + 4);
}
