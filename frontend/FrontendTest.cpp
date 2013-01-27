/*==============================================================================
 * FILE:	   FrontendTest.cc
 * OVERVIEW:   Provides the implementation for the FrontendTest class, which
 *				tests the FrontEnd and related classes
 *============================================================================*/
/*
 * $Revision: 1.5 $
 *
 * 05 Apr 02 - Mike: Created
 */

#include "FrontendTest.h"
#include "prog.h"

CPPUNIT_TEST_SUITE_REGISTRATION( FrontendTest );

#define HELLO_SPARC		"test/sparc/hello"
#define HELLO_PENTIUM	"test/pentium/hello"
#define HELLO_HPPA		"test/hppa/hello"
#define STARTER_PALM	"test/mc68328/Starter.prc"

/*==============================================================================
 * FUNCTION:		FrontendTest::setUp
 * OVERVIEW:		Set up anything needed before all tests
 * NOTE:			Called before any tests
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
void FrontendTest::setUp () {
}

/*==============================================================================
 * FUNCTION:		FrontendTest::tearDown
 * OVERVIEW:		Delete objects created in setUp
 * NOTE:			Called after all tests
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
void FrontendTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:		FrontendTest::test1
 * OVERVIEW:		Test loading the sparc hello world program
 *============================================================================*/
void FrontendTest::test1 () {
}

