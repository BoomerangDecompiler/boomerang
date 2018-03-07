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


#include "boomerang/passes/Pass.h"


/// Remove all liveness info in UseCollectors in calls
/**
 * Search for expressions without explicit definitions (i.e. WILDCARD{0}),
 * which represent parameters (use before definition).
 * These are called final parameters, because they are determined
 * from implicit references, not from the use collector at the start of the proc,
 * which include some caused by recursive calls
 */
class FinalParameterSearchPass : public IPass
{
public:
    FinalParameterSearchPass();

public:
    bool execute(UserProc *proc) override;
};
