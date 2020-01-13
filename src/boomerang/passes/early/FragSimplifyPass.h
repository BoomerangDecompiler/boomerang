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


/// Simplifies IR fragments of a function.
class FragSimplifyPass final : public IPass
{
public:
    FragSimplifyPass();

public:
    /// \copydoc IPass::isProcLocal
    bool isProcLocal() const override { return true; }

    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;

private:
    /// Simplify phis and replace them by Assigns, if possible
    bool simplifyPhis(UserProc *proc);
};
