/*==============================================================================
 * FILE:       testUtil.cc
 * OVERVIEW:   Command line test of the utility functions (including class Type)
 *============================================================================*/
/*
 * $Revision$
 * 09 Apr 02 - Mike: Created
 */


#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"

#include "UtilTest.h"
#include <iostream>


int main(int argc, char** argv)
{
    CppUnit::TestSuite suite;

    UtilTest lt("UtilTest");

    lt.registerTests(&suite);

    CppUnit::TextTestResult res;

    suite.run( &res );
    std::cout << res << std::endl;

    return 0;
}

