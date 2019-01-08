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


/// Replace simple global constant references. This is useful for obj-c.
///
/// Statement level transform :
/// PREDICATE:
///   (statement IS_A Assign) AND
///   (statement.rhs IS_A MemOf) AND
///   (statement.rhs.sub(1) IS_A IntConst)
/// ACTION:
///   $tmp_addr = assgn.rhs.sub(1);
///   $tmp_val = prog->readNative($tmp_addr,statement.type.bitwidth/8);
///   statement.rhs.replace_with(Const($tmp_val))
class GlobalConstReplacePass final : public IPass
{
public:
    GlobalConstReplacePass();

public:
    /// \copydoc IPass::isProcLocal
    bool isProcLocal() const override { return true; }

    /// \copydoc IPass::execute
    bool execute(UserProc *proc) override;
};
