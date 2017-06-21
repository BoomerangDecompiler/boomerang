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

#include <map>
#include <sstream>
#include "ProgTest.h"
#include "pentiumfrontend.h"
#include "core/BinaryFile.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ProgTest);

#define HELLO_PENTIUM    "test/pentium/hello"

/***************************************************************************/ /**
 * FUNCTION:        ProgTest::setUp
 * OVERVIEW:        Set up some expressions for use with all the tests
 * NOTE:            Called before any tests
 * PARAMETERS:        <none>
 *
 *============================================================================*/
void ProgTest::setUp()
{
	// prog.setName("default name");
}


/***************************************************************************/ /**
 * \brief Delete expressions created in setUp
 * \note Called after all tests
 *============================================================================*/
void ProgTest::tearDown()
{
}


/***************************************************************************/ /**
 * FUNCTION:        ProgTest::testName
 * OVERVIEW:        Test setting and reading name
 *============================================================================*/
void ProgTest::testName()
{
	BinaryFileFactory bff;
	BinaryFile        *pBF  = bff.Load(HELLO_PENTIUM); // Don't actually use it
	Prog              *prog = new Prog();
	FrontEnd          *pFE  = new PentiumFrontEnd(pBF, prog, &bff);

	// We need a Prog object with a pBF (for getEarlyParamExp())
	prog->setFrontEnd(pFE);
	std::string actual(prog->getName());
	std::string expected(HELLO_PENTIUM);
	CPPUNIT_ASSERT_EQUAL(expected, actual);
	std::string name("Happy prog");
	prog->setName(name.c_str());
	actual = prog->getName();
	CPPUNIT_ASSERT_EQUAL(name, actual);
	delete pFE;
}


// Pathetic: the second test we had (for readLibraryParams) is now obsolete;
// the front end does this now.
