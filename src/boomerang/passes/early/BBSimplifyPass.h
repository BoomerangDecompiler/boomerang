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


/// Simplifies basic blocks of a function.
class BBSimplifyPass final : public IPass
{
public:
    BBSimplifyPass();

public:
    /// \copydoc IPass::isProcLocal
    bool isProcLocal() const override { return true; }

    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;
};
