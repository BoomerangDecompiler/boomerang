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


/// Before Type Analysis, refs like r28{0} have a nullptr Statement pointer.
/// After this, they will point to an implicit assignment for the location.
/// Thus, during and after type analysis, you can find the type of any
/// location by following the reference to the definition
/// \note you need something recursive to make sure that child subexpressions
/// are processed before parents
/// Example: m[r28{0} - 12]{0} could end up adding an implicit assignment
/// for r28{-} with a null reference, when other pieces of code add r28{0}
class ImplicitPlacementPass final : public IPass
{
public:
    ImplicitPlacementPass();

public:
    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;

private:
    bool makeSymbolsImplicit(UserProc *proc);
};
