/***************************************************************************/ /**
 * \file       ProcTest.cc
 * OVERVIEW:   Provides the implementation for the ProcTest class, which
 *                tests the Proc class
 *============================================================================*/

/*
 * $Revision$
 *
 * 23 Apr 02 - Mike: Created
 * 10 Mar 03 - Mike: Mods to not use Prog::pBF (no longer public)
 */

#include "ProcTest.h"
#include "BinaryFile.h"
#include "BinaryFileStub.h"
#include "pentiumfrontend.h"

#include <sstream>
#include <map>
#include "prog.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ProcTest);

#define HELLO_PENTIUM    "test/pentium/hello"

/***************************************************************************/ /**
 * FUNCTION:        ProcTest::setUp
 * OVERVIEW:        Set up some expressions for use with all the tests
 * NOTE:            Called before any tests
 * PARAMETERS:        <none>
 *
 *============================================================================*/
void ProcTest::setUp()
{
}


/***************************************************************************/ /**
 * FUNCTION:        ProcTest::tearDown
 * OVERVIEW:        Delete expressions created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:        <none>
 *
 *============================================================================*/
void ProcTest::tearDown()
{
	delete m_proc;
}


/***************************************************************************/ /**
 * FUNCTION:        ProcTest::testName
 * OVERVIEW:        Test setting and reading name, constructor, native address
 *============================================================================*/
void ProcTest::testName()
{
	Prog       *prog = new Prog();
	BinaryFile *pBF  = new BinaryFileStub();

	CPPUNIT_ASSERT(pBF != 0);
	std::string       nm("default name");
	BinaryFileFactory bff;
	pBF = bff.Load(HELLO_PENTIUM);
	FrontEnd *pFE = new PentiumFrontEnd(pBF, prog, &bff);
	CPPUNIT_ASSERT(pFE != 0);
	prog->setFrontEnd(pFE);
	CPPUNIT_ASSERT(prog);
	pFE->readLibraryCatalog();              // Since we are not decoding
	m_proc = new UserProc(prog, nm, 20000); // Will print in decimal if error
	std::string actual(m_proc->getName());
	CPPUNIT_ASSERT_EQUAL(std::string("default name"), actual);

	std::string name("printf");
	LibProc     lp(prog, name, 30000);
	actual = lp.getName();
	CPPUNIT_ASSERT_EQUAL(name, actual);

	ADDRESS a        = lp.getNativeAddress();
	ADDRESS expected = 30000;
	CPPUNIT_ASSERT_EQUAL(expected, a);
	a        = m_proc->getNativeAddress();
	expected = 20000;
	CPPUNIT_ASSERT_EQUAL(expected, a);

	delete prog;
	delete m_proc;
	// delete pFE;        // No! Deleting the prog deletes the pFE already (which deletes the BinaryFileFactory)
}
