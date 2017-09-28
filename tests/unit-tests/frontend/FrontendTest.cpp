#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "FrontendTest.h"


/**
 * \file FrontendTest.cpp
 * Provides the implementation for the FrontendTest class, which
 * tests the FrontEnd and related classes
 */


#include "boomerang/db/Prog.h"

#define HELLO_SPARC      "tests/inputs/sparc/hello"
#define HELLO_PENTIUM    "tests/inputs/pentium/hello"
#define HELLO_HPPA       "tests/inputs/hppa/hello"
#define STARTER_PALM     "tests/inputs/mc68328/Starter.prc"


void FrontendTest::test1()
{
}


QTEST_MAIN(FrontendTest)
