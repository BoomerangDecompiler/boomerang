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


/**
 * Test the Prog class.
 */
class ProgTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    /// Test setting and reading name
    void testName();

    void testCreateModule();
    void testGetOrInsertModule();
    void testGetRootModule();
    void testFindModule();
    void testIsModuleUsed();

    void testAddEntryPoint();
    void testGetOrCreateFunction();
    void testGetOrCreateLibraryProc();
    void testGetFunctionByAddr();
    void testGetFunctionByName();
    void testRemoveFunction();
    void testGetNumFunctions();

    void testIsWellFormed();
    void testIsWin32();
    void testGetRegName();
    void testGetRegSize();

    void testGetFrontEndId();
    void testGetMachine();
    void testGetDefaultSignature();

    void testGetStringConstant();
    void testGetFloatConstant();
    void testGetSymbolNameByAddr();
    void testGetSectionByAddr();
    void testGetLimitText();
    void testIsReadOnly();
    void testIsStringConstant();
    void testIsDynamicallyLinkedProcPointer();
    void testGetDynamicProcName();
    void testGetModuleForSymbol();

    void testRead(); // readNative[1, 2, 4, As]
    void testReadSymbolFile();

    void testAddDecodedRTL();
    void testAddReloc();

    void testDecodeEntryPoint();
    void testDecodeFragment();
    void testReDecode();
    void testFinishDecode();

    void testGetGlobalName();
    void testGetGlobalAddr();
    void testGetGlobal();
    void testNewGlobalName();
    void testGuessGlobalType();
    void testMakeArrayType();
    void testMarkGlobalUsed();
    void testGlobalType(); // getGlobalType/setGlobalType
};
