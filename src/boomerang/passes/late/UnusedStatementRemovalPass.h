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
#include "boomerang/db/proc/Proc.h"


/// Remove unused statements
class UnusedStatementRemovalPass : public IPass
{
public:
    UnusedStatementRemovalPass();

public:
    bool execute(UserProc *proc) override;

private:
    /// Count references to the things that are under SSA control.
    /// For each SSA subscripting, increment a counter for that definition
    void updateRefCounts(UserProc *proc, Function::RefCounter& refCounts);

    void remUnusedStmtEtc(UserProc *proc, Function::RefCounter& refCounts);

    /// Remove statements of the form x := x
    bool removeNullStatements(UserProc *proc);
};
