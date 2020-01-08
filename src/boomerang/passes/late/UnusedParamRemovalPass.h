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


#include "boomerang/db/proc/UserProc.h"
#include "boomerang/passes/Pass.h"
#include "boomerang/ssl/exp/ExpHelp.h"

#include <set>


/// Removes unused function parameters from a function.
class UnusedParamRemovalPass final : public IPass
{
public:
    UnusedParamRemovalPass();

public:
    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;

private:
    /**
     * Check for a gainful use of bparam{0} in this proc.
     * Return with true when the first such use is found.
     * Ignore uses in return statements of recursive functions,
     * and phi statements that define them.
     * Procs in \p visited are already visited.
     *
     * \returns true if location \p e is used gainfully in this procedure.
     */
    bool checkForGainfulUse(UserProc *proc, SharedExp e, ProcSet &Visited);
};
