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



class RegDBTest : public BoomerangTest
{
    Q_OBJECT

private slots:
    void testClear();
    void testIsRegNumDefined();
    void testIsRegDefined();
    void testGetRegByNum();
    void testGetRegByName();
    void testGetRegNumByName();
    void testGetRegNameByNum();
    void testGetRegSizeByNum();
    void testCreateReg();
    void testCreateRegRelation();
    void testProcessOverlappedRegs();
};
