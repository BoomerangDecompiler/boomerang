#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementSet.h"


bool lessAssign::operator()(const std::shared_ptr<Assign> &as1, const std::shared_ptr<Assign> &as2) const
{
    return *as1->getLeft() < *as2->getLeft();
}
