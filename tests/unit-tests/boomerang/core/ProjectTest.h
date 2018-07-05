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


/**
 * Test the Project class.
 */
class ProjectTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    /// Test the import binary function.
    void testLoadBinaryFile();

    // test loading/writing to/from a save file
    void testLoadSaveFile();
    void testWriteSaveFile();

    // test whether a binary is loaded after loading unloading
    void testIsBinaryLoaded();

    void testDecodeBinaryFile();
    void testDecompileBinaryFile();
    void testGenerateCode();
};
