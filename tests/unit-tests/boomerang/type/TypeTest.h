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


class TypeTest : public BoomerangTestWithPlugins
{
    Q_OBJECT

private slots:
    void testTypeLong();
    void testNotEqual();
    void testCompound();

    // Test the DataIntervalMap class
    void testDataInterval();
    void testDataIntervalOverlaps();
};
