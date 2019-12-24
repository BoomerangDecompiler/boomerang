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


class UserProcTest : public BoomerangTestWithPlugins
{
    Q_OBJECT

private slots:
    void testIsNoReturn();
    void testRemoveStatement();
    void testInsertAssignAfter();
    void testInsertStatementAfter();
    void testReplacePhiByAssign();

    void testAddParameterToSignature();
    void testInsertParameter();
    void testParamType();
    void testLookupParam();
    void testCanBeParam();

    void testRetStmt();
    void testCanBeReturn();

    void testCreateLocal();
    void testAddLocal();
    void testEnsureExpIsMappedToLocal();
    void testGetSymbolExp();
    void testFindLocal();
    void testLocalType();
    void testIsLocalOrParamPattern();

    void testExpFromSymbol();
    void testMapSymbolTo();
    void testLookupSym();
    void testLookupSymFromRef();
    void testLookupSymFromRefAny();

    void testMarkAsNonChildless();
    void testAddCallee();
    void testPreservesExp();
    void testPreservesExpWithOffset();
    void testPromoteSignature();
    void testFindFirstSymbol();
    void testSearchAndReplace();
    void testAllPhisHaveDefs();
};
