#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include "util.h"
#include "type.h"


class TypeTest : public CppUnit::TestCase {
  protected:

  public:
    TypeTest(std::string name) : CppUnit::TestCase (name)
    {}

    virtual void registerTests(CppUnit::TestSuite* suite);

    int countTestCases () const;

    void setUp ();
    void tearDown ();

    void testTypeLong ();
    void testNotEqual ();
    void testCompound();
};

