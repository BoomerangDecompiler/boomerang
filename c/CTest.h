#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include "util.h"
#include "type.h"

#include "ansi-c-parser.h"

class CTest : public CppUnit::TestCase {

  public:
    CTest(std::string name) : CppUnit::TestCase (name)
    {}

    virtual void registerTests(CppUnit::TestSuite* suite);

    int countTestCases () const;

    void setUp ();
    void tearDown ();

    void testSignature();
};

