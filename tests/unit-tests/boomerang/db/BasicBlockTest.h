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
 * Tests for low-level BasicBlocks
 */
class BasicBlockTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    void testType(); // get/setType
    void testAddress(); // getLow/HiAddr
    void testIsComplete();

    void testCompleteBB();
};
