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


class Win32BinaryLoaderTest : public BoomerangTestWithPlugins
{
    Q_OBJECT

private slots:
    /// Test loading Windows programs
    void testWinLoad();
};
