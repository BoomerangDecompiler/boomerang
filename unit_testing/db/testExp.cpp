/***************************************************************************/ /**
 * \file       testExp.cpp
 * OVERVIEW:   Command line test of the Exp class
 *============================================================================*/

/*
 * $Revision$
 * 15 Jul 02 - Mike: Created from testDbase
 * 29 Jul 03 - Mike: Created from testAll
 */

#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"

#include "ExpTest.h"

#include <sstream>
#include <iostream>

int main(int argc, char **argv)
{
	CppUnit::TestSuite suite;

	ExpTest expt("ExpTest");

	expt.registerTests(&suite);

	CppUnit::TextTestResult res;

	suite.run(&res);
	std::cout << res << std::endl;

	return 0;
}
