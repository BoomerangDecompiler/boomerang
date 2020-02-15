#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/ssl/statements/Statement.h"

// Common to BranchStatement and BoolAssign
// Return true if this is now a floating point branch
bool BOOMERANG_API condToRelational(SharedExp &condExp, BranchType jtCond);
