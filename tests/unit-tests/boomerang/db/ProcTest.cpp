#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ProcTest.h"


#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/Proc.h"
#include "boomerang/core/Project.h"
#include "boomerang/frontend/pentium/pentiumfrontend.h"
#include "boomerang/core/Boomerang.h"

#include <map>


#define HELLO_PENTIUM    "test/pentium/hello"


void ProcTest::testName()
{
    IProject& project = *Boomerang::get()->getOrCreateProject();
    project.loadBinaryFile(HELLO_PENTIUM);
    Prog *prog = project.getProg();
    QVERIFY(prog != nullptr);

    IFrontEnd *fe = new PentiumFrontEnd(project.getBestLoader(HELLO_PENTIUM), prog);
    QVERIFY(fe != nullptr);
    prog->setFrontEnd(fe);

    fe->readLibraryCatalog();              // Since we are not decoding

    Function *f       = prog->createFunction(Address(0x00020000));
    QString  procName = "default name";
    f->setName(procName);
    QCOMPARE(f->getName(), procName);

    f = prog->findFunction("printf");
    QVERIFY(f != nullptr);
    QVERIFY(f->isLib());
    QCOMPARE(f->getName(), QString("printf"));
}


QTEST_MAIN(ProcTest)
