/*==============================================================================
 * FILE:	   testAll.cc
 * OVERVIEW:   Command line test of all of Boomerang
 *============================================================================*/
/*
 * $Revision$
 * 15 Jul 02 - Mike: Created from testDbase
*/


#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"

#include "exp.h"

#include "ExpTest.h"
#include "ProgTest.h"
#include "ProcTest.h"
#include "StatementTest.h"
#include "RtlTest.h"
#include "DfaTest.h"
#include "ParserTest.h"
#include "TypeTest.h"
#include "FrontSparcTest.h"
#include "FrontPentTest.h"
#include "AnalysisTest.h"
#include "CTest.h"
#include "CfgTest.h"

#include "prog.h"

#include <sstream>
#include <iostream>

int main(int argc, char** argv)
{
//std::cerr << "Prog at " << std::hex << &prog << std::endl;
	CppUnit::TestSuite suite;

	ExpTest	 expt("Exp Test");
	ProgTest progt("Prog Test");
	ProcTest proct("Proc Test");
	RtlTest rtlt("Rtl Test");
	ParserTest parsert("SSL Parser Test");
	TypeTest typet("Type Test");
	FrontSparcTest fst("SPARC Frontend Test");
//	  FrontendTest fet("FrontendTest");
	FrontPentTest fpt("Pentium Frontend Test");
	AnalysisTest ant("Analysis Test");
	CTest ct("C Parser Test");
	StatementTest stt("Statement Test");
	CfgTest cfgt("Cfg Test");
	DfaTest dfat("Dfa Test");

	expt.registerTests(&suite);
	progt.registerTests(&suite);
	proct.registerTests(&suite);
	rtlt.registerTests(&suite);
	parsert.registerTests(&suite);
	typet.registerTests(&suite);
	fst.registerTests(&suite);
	fpt.registerTests(&suite);
	ant.registerTests(&suite);
	ct.registerTests(&suite);
	stt.registerTests(&suite);
	cfgt.registerTests(&suite);
	dfat.registerTests(&suite);

	CppUnit::TextTestResult res;

	suite.run( &res );
	std::cout << res << std::endl;

	return 0;
}
