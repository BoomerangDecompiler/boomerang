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
class Statement;


/// Removes unused local variables.
/// \note call the below after translating from SSA form
/// FIXME: this can be done before transforming out of SSA form now, surely...
class UnusedLocalRemovalPass final : public IPass
{
public:
    UnusedLocalRemovalPass();

public:
    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;

private:
    /// Special version of Statement::addUsedLocs for finding used locations.
    /// \return true if defineAll was found
    bool addUsedLocalsForStmt(Statement *stmt, LocationSet &locals);
};
