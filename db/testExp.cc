// Test program for class Exp
// 03 Apr 02 - Mike: Modified to use CppUnit 1.6.2.

#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"

#include "exp.h"

#include "ExpTest.h"

#include <iostream>


int main(int argc, char** argv)
{
    CppUnit::TestSuite suite;

    ExpTest et("ExpTest");

    et.registerTests(&suite);

    CppUnit::TextTestResult res;

    suite.run( &res );
    std::cout << res << std::endl;

    return 0;
}

