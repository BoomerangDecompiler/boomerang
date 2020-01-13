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
#include "boomerang/ssl/statements/Statement.h"

#include <map>
#include <stack>


/// Rewrites Statements in BasicBlocks into SSA form.
class BlockVarRenamePass final : public IPass
{
public:
    BlockVarRenamePass();

public:
    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;

private:
    bool renameBlockVars(UserProc *proc, std::size_t n);

    /// For all expressions in \p stmt, replace \p var with var{varDef}
    void subscriptVar(const SharedStmt &stmt, SharedExp var, const SharedStmt &varDef);

    bool subscriptUsedLocations(SharedStmt stmt);

    /// push definitions in this statement onto the stacks
    void pushDefinitions(SharedStmt stmt, bool assumeABI);

    /// pop definitions in this statement from the stacks
    void popDefinitions(SharedStmt stmt, bool assumeABI);

private:
    /// stores the last definition of a variable
    std::map<SharedExp, std::stack<SharedStmt>, lessExpStar> stacks;
};
