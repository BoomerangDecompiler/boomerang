/***************************************************************************/ /**
 * \file       testDbase.cc
 * OVERVIEW:   Command line test of the Exp and related classes.
 *============================================================================*/

/*
 * $Revision$
 *      Apr 02 - Mike: Created
 * 03 Apr 02 - Mike: Modified to use CppUnit 1.6.2.
 * 18 Apr 02 - Mike: Test prog as well
 * 13 May 02 - Mike: Added RtlTest and ParserTest
 * 04 Jul 02 - Mike: Move UtilTest here as TypeTest
 */

#include "boomerang/cppunit/TextTestResult.h"
#include "boomerang/cppunit/TestSuite.h"

#include "boomerang/db/exp.h"

#include "boomerang/ExpTest.h"
#include "boomerang/ProgTest.h"
#include "boomerang/ProcTest.h"
#include "boomerang/RtlTest.h"
#include "boomerang/ParserTest.h"
#include "boomerang/TypeTest.h"

#include <sstream>
#include <iostream>

int main(int argc, char **argv)
{
	CppUnit::TestSuite suite;

	ExpTest    expt("ExpTest");
	ProgTest   progt("ProgTest");
	ProcTest   proct("ProcTest");
	RtlTest    rtlt("RtlTest");
	ParserTest parsert("ParserTest");
	TypeTest   typet("TypeTest");

	expt.registerTests(&suite);
	progt.registerTests(&suite);
	proct.registerTests(&suite);
	rtlt.registerTests(&suite);
	parsert.registerTests(&suite);
	typet.registerTests(&suite);

	CppUnit::TextTestResult res;

	suite.run(&res);
	std::cout << res << std::endl;

	return 0;
}
