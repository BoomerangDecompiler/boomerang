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


class ElfBinaryLoaderTest : public BoomerangTestWithPlugins
{
    Q_OBJECT

private slots:
    /// test the loader using a "Hello World" program
    /// compiled with clang-4.0.0 (without debug info)
    void testElfLoadClang();

    /// Test loading the Pentium (Solaris) hello world program
    void testPentiumLoad();
    void testPentiumLoad_data();
};
