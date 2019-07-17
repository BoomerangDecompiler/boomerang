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


class LocationSet;
class UseCollector;


class StatementPropagationPass final : public IPass
{
public:
    StatementPropagationPass();

public:
    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;

private:
    /// Propagate into xxx of m[xxx] in the UseCollector (locations live at the entry of \p proc)
    void propagateToCollector(UseCollector *collector);
};
