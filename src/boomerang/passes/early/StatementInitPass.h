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


/// Initializes statements before decompilation.
class StatementInitPass final : public IPass
{
public:
    StatementInitPass();

public:
    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;
};
