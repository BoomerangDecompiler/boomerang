#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ProgTest.h"


/**
 * \file ProgTest.cpp
 * Provides the implementation for the ProgTest class, which
 * tests the Prog object
 */

/*
 * $Revision$
 *
 * 18 Apr 02 - Mike: Created
 * 18 Jul 02 - Mike: Set up prog.pFE before calling readLibParams
 */


#include "boomerang/db/Prog.h"

#define HELLO_PENTIUM    (BOOMERANG_TEST_BASE "/tests/input/pentium/hello")


void ProgTest::testName()
{
    QString progName = HELLO_PENTIUM;
    Prog    *prog    = new Prog(progName);

    QCOMPARE(prog->getName(), progName);

    progName = "Happy Prog";
    prog->setName(progName);
    QCOMPARE(prog->getName(), progName);
}


QTEST_MAIN(ProgTest)
