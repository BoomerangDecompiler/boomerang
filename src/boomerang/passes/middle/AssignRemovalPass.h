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

#include <memory>

using SharedExp = std::shared_ptr<class Exp>;


class AssignRemovalPass : public IPass
{
public:
    AssignRemovalPass();

public:
    bool execute(UserProc *proc) override;

private:
    /// Preservations only for the stack pointer
    bool removeSpAssigns(UserProc *proc);

    bool removeMatchingAssigns(UserProc *proc, SharedExp exp);
};
