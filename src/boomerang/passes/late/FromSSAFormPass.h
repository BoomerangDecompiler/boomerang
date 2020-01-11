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
#include "boomerang/ssl/statements/Statement.h"


class ConnectionGraph;


/// Transforms the statements a proc out of SSA form
class FromSSAFormPass final : public IPass
{
public:
    FromSSAFormPass();

public:
    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;

private:
    /// Add a mapping for the destinations of phi functions that have one
    /// argument that is a parameter.
    ///
    /// The idea here is to give a name to those SSA variables that have one
    /// and only one parameter amongst the phi arguments.
    /// For example, in test/source/param1, there is
    ///   18 *v* m[r28{-} + 8] := phi{- 7} with m[r28{-} + 8]{0}
    /// mapped to param1; insert a mapping for m[r28{-} + 8]{18} to param1.
    /// This will avoid a copy, and will use the name of the parameter only
    /// when it is acually used as a parameter.
    void nameParameterPhis(UserProc *proc);

    void mapParameters(UserProc *proc);

    void removeSubscriptsFromSymbols(UserProc *proc);

    void removeSubscriptsFromParameters(UserProc *proc);

    /// Find the locations united by Phi-functions
    void findPhiUnites(UserProc *proc, ConnectionGraph &pu);

    void insertCastsForStmt(const SharedStmt &stmt);

    /// map registers and temporaries to local variables
    void mapRegistersToLocals(const SharedStmt &stmt);

    void removePhis(UserProc *proc);
};
