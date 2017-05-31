/***************************************************************************/ /**
 * \file       testFront.cc
 * OVERVIEW:   Command line test of the Frontend and related classes.
 *============================================================================*/

/*
 * $Revision$
 * 08 Apr 02 - Mike: Created
 * 23 May 02 - Mike: Added pentium tests
 */

#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"

#include "FrontSparcTest.h"
#include "FrontPentTest.h"
//#include "FrontendTest.h"
#include "include/prog.h"
#include <iostream>

int main(int argc, char **argv)
{
	CppUnit::TestSuite suite;

	//    FrontSparcTest fst("FrontSparcTest");
	//      FrontendTest fet("FrontendTest");
	//    FrontPentTest fpt("FrontPentTest");
	FrontPentTest fSt("FrontPentTest");

	//    fst.registerTests(&suite);
	//    fpt.registerTests(&suite);
	fSt.registerTests(&suite);

	CppUnit::TextTestResult res;

	prog.readLibParams(); // Read library signatures (once!)
	suite.run(&res);
	std::cout << res << std::endl;

	return 0;
}
