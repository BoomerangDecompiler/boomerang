/*==============================================================================
 * FILE:       testDbase.cc
 * OVERVIEW:   Command line test of the Exp and related classes.
 *============================================================================*/
/*
 * $Revision$
 *    Apr 02 - Mike: Created
 * 03 Apr 02 - Mike: Modified to use CppUnit 1.6.2.
 * 18 Apr 02 - Mike: Test prog as well
 * 13 May 02 - Mike: Added RtlTest and ParserTest
 */


#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"

#include "exp.h"

#include "ExpTest.h"
#include "ProgTest.h"
#include "ProcTest.h"
#include "RtlTest.h"
#include "ParserTest.h"

#include <sstream>
#include <iostream>

int main(int argc, char** argv)
{
    CppUnit::TestSuite suite;

    ExpTest  expt("ExpTest");
    ProgTest progt("ProgTest");
    ProcTest proct("ProcTest");
    RtlTest rtlt("RtlTest");
    ParserTest parsert("ParserTest");

    expt.registerTests(&suite);
    progt.registerTests(&suite);
    proct.registerTests(&suite);
    rtlt.registerTests(&suite);
    parsert.registerTests(&suite);

    CppUnit::TextTestResult res;

    suite.run( &res );
    std::cout << res << std::endl;

    return 0;
}

// It's not worth linking to libutil.so for just this function
/*==============================================================================
 * FUNCTION:      str
 * OVERVIEW:      Return the null terminated string for an std::ostringstream object
 * PARAMETERS:    os: the stringstream object
 * RETURNS:       The null terminated char*
 *============================================================================*/
char* str(std::ostringstream& os)
{
    const char* p = os.str().c_str();
	return (char*)p;
}
