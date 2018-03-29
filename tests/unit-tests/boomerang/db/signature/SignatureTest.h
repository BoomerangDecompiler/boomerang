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


#include <QtTest/QTest>


class SignatureTest : public QObject
{
    Q_OBJECT

private slots:
    void testAddReturn();
    void testGetReturnExp();
    void testGetReturnType();
    void testGetNumReturns();
    void testFindReturn();

    void testAddParameter();
    void testRemoveParameter();
    void testSetNumParams();

    void testGetParamName();
    void testGetParamExp();
    void testGetParamType();
    void testGetParamBoundMax();
    void testSetParamType();
    void testFindParam();
    void testRenameParam();

    void testGetArgumentExp();
    void testEllipsis();
    void testIsNoReturn();
    void testIsPromoted();
    void testPromote();
    void testGetStackRegister();

    void testIsStackLocal();
    void testIsAddrOfStackLocal();
    void testIsLocalOffsetNegative();
    void testIsLocalOffsetPositive();
    void testIsOpCompatStackLocal();

    void testGetProven();
    void testIsPreserved();
    void testGetLibraryDefines();
    void testGetABIDefines();

    void testSetPreferredReturn();
    void testSetPreferredName();
    void testAddPreferredParameter();
    void testGetPreferredReturn();
    void testGetPreferredName();
    void testGetNumPreferredParams();
    void testGetPreferredParam();

    void testCompare();
};
