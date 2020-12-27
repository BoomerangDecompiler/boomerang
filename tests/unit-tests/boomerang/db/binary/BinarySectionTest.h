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


class BinarySectionTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    void testIsAddressBss();
    void testAnyDefinedValues();
    void testResize();
    void testAddDefinedArea();

    void testAttributes();
};
