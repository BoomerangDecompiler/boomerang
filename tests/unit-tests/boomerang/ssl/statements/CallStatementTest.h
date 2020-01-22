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


class CallStatementTest : public BoomerangTestWithProject
{
    Q_OBJECT

private slots:
    void testClone();
    void testNumber(); // get/setNumber
    void testGetDefinitions();
    void testDefinesLoc();
    void testSearch();
    void testSearchAll();
    void testSearchAndReplace();
    void testSimplify();
    void testTypeForExp(); // get/setTypeForExp
    void testToString();

    void testArguments(); // get/setArguments
    void testSetSigArguments();
    void testUpdateArguments();
    void testArgumentExp(); // get/setArgumentExp
    void testNumArguments(); // get/setNumArguments
    void testRemoveArguments();
    void testArgumentType(); // get/setArgumentType
    void testEliminateDuplicateArgs();

    void testDestProc(); // get/setDestProc
    void testReturnAfterCall(); // get/setReturnAfterCall
    void testIsChildless();
    void testIsCallToMemOffset();

    void testAddDefine();
    void testRemoveDefine();
    void testSetDefines();
    void testFindDefFor();

    void testCalcResults();
    void testGetProven();
    void testLocaliseExp();
    void testLocaliseComp();
    void testBypassRef();
    void testDoEllipsisProcessing();
    void testTryConvertToDirect();

};
