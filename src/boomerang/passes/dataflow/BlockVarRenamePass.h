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
#include "boomerang/ssl/exp/ExpHelp.h"

#include <deque>
#include <map>


class Statement;


/// Rewrites Statements in BasicBlocks into SSA form.
class BlockVarRenamePass final : public IPass
{
public:
    BlockVarRenamePass();

public:
    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;

private:
    bool renameBlockVars(UserProc *proc, int n,
                         std::map<SharedExp, std::deque<Statement *>, lessExpStar> &stacks);

    /// For all expressions in \p stmt, replace \p var with var{varDef}
    void subscriptVar(Statement *stmt, SharedExp var, Statement *varDef);
};
