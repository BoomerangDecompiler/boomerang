/**
 * \file ProcTest.cpp
 * Provides the implementation for the ProcTest class, which
 * tests the Proc class
 */

/*
 * $Revision$
 *
 * 23 Apr 02 - Mike: Created
 * 10 Mar 03 - Mike: Mods to not use Prog::pBF (no longer public)
 */

#include "ProcTest.h"
#include "boomerang/core/BinaryFileFactory.h"

#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/Proc.h"

#include "boomerang-frontend/pentium/pentiumfrontend.h"

#include <map>


#define HELLO_PENTIUM    "test/pentium/hello"


void ProcTest::testName()
{
	Prog       *prog = new Prog("testProcName");
	QVERIFY(prog != nullptr);

    std::string       nm("default name");
	BinaryFileFactory bff;
	IFileLoader* pBF = bff.loadFile(HELLO_PENTIUM);
	IFrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
	QVERIFY(pFE != nullptr);
	prog->setFrontEnd(pFE);

	pFE->readLibraryCatalog();              // Since we are not decoding

    Function* f = prog->createProc(Address(0x00020000));
    QString procName = "default name";
    f->setName(procName);
    QCOMPARE(f->getName(), procName);

    f = prog->findProc("printf");
    QVERIFY(f != nullptr);
    QVERIFY(f->isLib());
    QCOMPARE(f->getName(), QString("printf"));

	delete prog;
}

QTEST_MAIN(ProcTest)
