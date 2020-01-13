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
 * Test for basic data-flow related code
 */
class DataFlowTest : public BoomerangTestWithPlugins
{
    Q_OBJECT

private slots:
    /// Test calculating (semi-)dominators and the Dominance Frontier
    void testCalculateDominators1();
    void testCalculateDominators2();
    void testCalculateDominatorsSelfLoop();
    void testCalculateDominatorsComplex();

    /// Test the placing of phi functions
    void testPlacePhi();

    /// Test a case where a phi function is not needed
    void testPlacePhi2();

    /// Test the renaming of variables
    void testRenameVars();
    void testRenameVarsSelfLoop();
};
