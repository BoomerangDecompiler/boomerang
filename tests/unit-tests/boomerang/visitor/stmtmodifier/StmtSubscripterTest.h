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


#include "TestUtils.h"


class StmtSubscripterTest : public BoomerangTest
{
    Q_OBJECT

private:
    /// Subscript \p e by \p e{def} in \p stmt
    void subscriptVarForStmt(const SharedStmt &stmt, SharedExp e, const SharedStmt &def);

private slots:
    void testSubscriptVars();
    void testSubscriptVar();
};
