#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

#include "rtl.h"

class RtlTest : public CppUnit::TestCase {
  protected:

  public:
    RtlTest(std::string name) : CppUnit::TestCase (name)
    {}

    virtual void registerTests(CppUnit::TestSuite* suite);

    int countTestCases () const;

    void setUp ();
    void tearDown ();

    void testAppend ();
    void testClone ();
    void testVisitor();
    void testIsCompare();
};

