#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

#include "exp.h"

class ExpTest : public CppUnit::TestCase {
  protected:
    Const*  m_99;
    Unary*  m_rof2;

  public:
    ExpTest(std::string name) : CppUnit::TestCase (name)
    {}

    virtual void registerTests(CppUnit::TestSuite* suite);

    int countTestCases () const;

    void setUp ();
    void tearDown ();

    void test99 ();
    void testRegOf2 ();

    void testPlus ();
    void testMinus ();
    void testMult ();
    void testDiv ();
    void testMults ();
    void testDivs ();
    void testMod ();
    void testMods ();
};

