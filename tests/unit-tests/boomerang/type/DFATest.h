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
 * Tests the Data Flow based type analysis code
 */
class DfaTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    /// Test meeting IntegerTypes with various other types
    void testMeet();
    void testMeet_data();
};
