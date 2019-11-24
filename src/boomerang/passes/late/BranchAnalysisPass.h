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

class IRFragment;


/// Simplifies branch condions.
/// Example:
///   if (cond1) {
///     if (cond2) {
///       ...
///     }
///   }
/// ->
///   if (cond1 && cond2) {
///     ...
///   }
class BranchAnalysisPass final : public IPass
{
public:
    BranchAnalysisPass();

public:
    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;

private:
    /// Look for short circuit branching
    /// \returns true if any change
    bool doBranchAnalysis(UserProc *proc);

    /// Fix any ugly branch statements (from propagating too much)
    void fixUglyBranches(UserProc *proc);

    /// \returns true if the fragment only contains a branch statement
    bool isOnlyBranch(IRFragment *frag) const;
};
