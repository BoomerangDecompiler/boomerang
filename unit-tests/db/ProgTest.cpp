/***************************************************************************/ /**
 * \file       ProgTest.cc
 * OVERVIEW:   Provides the implementation for the ProgTest class, which
 *                tests the Exp and derived classes
 *============================================================================*/

/*
 * $Revision$
 *
 * 18 Apr 02 - Mike: Created
 * 18 Jul 02 - Mike: Set up prog.pFE before calling readLibParams
 */

#include "ProgTest.h"

#include "boomerang/db/prog.h"

#define HELLO_PENTIUM    (BOOMERANG_TEST_BASE "/tests/input/pentium/hello")


void ProgTest::testName()
{
    Prog *prog = new Prog(HELLO_PENTIUM);

    QString progName = HELLO_PENTIUM;
    QCOMPARE(prog->getName(), progName);

    progName = "Happy Prog";
	prog->setName(progName);
	QCOMPARE(prog->getName(), progName);
}

QTEST_MAIN(ProgTest)
