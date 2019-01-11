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


class ArrayTypeTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    void testConstruct();
    void testEqual();
    void testLess();
    void testIsCompatibleWith();
    void testGetSize();
    void testGetCtype();
    void testIsUnbounded();
};
