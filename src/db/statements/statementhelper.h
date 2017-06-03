#pragma once

#include "db/exp.h"
#include "db/statements/statement.h"

// Common to BranchStatement and BoolAssign
// Return true if this is now a floating point Branch
bool condToRelational(SharedExp& pCond, BranchType jtCond);
