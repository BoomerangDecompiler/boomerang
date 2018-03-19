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


#include "boomerang/db/Prog.h"
#include "boomerang/core/Boomerang.h"
#include "boomerang/core/Settings.h"


#define HELLO_PENTIUM    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/hello"))


void ProgTest::testName()
{
    QString progName = HELLO_PENTIUM;
    Prog    *prog    = new Prog(progName, nullptr);

    QCOMPARE(prog->getName(), progName);

    progName = "Happy Prog";
    prog->setName(progName);
    QCOMPARE(prog->getName(), progName);
    delete prog;
}


QTEST_MAIN(ProgTest)
