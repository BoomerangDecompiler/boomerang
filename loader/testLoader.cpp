/*==============================================================================
 * FILE:       testLoader.cc
 * OVERVIEW:   Command line test of the BinaryFile and related classes.
 *============================================================================*/
/*
 * $Revision$
 *    Apr 02 - Mike: Created
 * 03 Apr 02 - Mike: Modified to use CppUnit 1.6.2.
 */


#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"

#include <iostream>
#include "LoaderTest.h"

int main(int argc, char** argv)
{
    CppUnit::TestSuite suite;

    LoaderTest lt("ExpTest");

    lt.registerTests(&suite);

    CppUnit::TextTestResult res;

    suite.run( &res );
    std::cout << res << std::endl;

    return 0;
}

