#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include "BinaryFile.h"


class LoaderTest : public CppUnit::TestCase {
  protected:

  public:
    LoaderTest(std::string name) : CppUnit::TestCase (name)
    {}

    virtual void registerTests(CppUnit::TestSuite* suite);

    int countTestCases () const;

    void setUp ();
    void tearDown ();

    void testSparcLoad ();
    void testPentiumLoad ();
    void testHppaLoad ();
    void testPalmLoad ();
};

