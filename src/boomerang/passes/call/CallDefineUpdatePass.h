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


class CallStatement;


/// Set the defines to the set of locations modified by the callee,
/// or if no callee, to all variables live at this call
class CallDefineUpdatePass final : public IPass
{
public:
    CallDefineUpdatePass();

public:
    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;

private:
    bool updateCallDefines(UserProc *proc, CallStatement *callStmt);
};
