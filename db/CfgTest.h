#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

#include "cfg.h"

class CfgTest : public CppUnit::TestCase {
  protected:
    Cfg*  m_prog;

  public:
    CfgTest(std::string name) : CppUnit::TestCase (name)
    {}

    virtual void registerTests(CppUnit::TestSuite* suite);

    int countTestCases () const;

    void setUp ();
    void tearDown ();

    void testDominators ();
    void testSemiDominators ();
    void testPlacePhi ();
    void testPlacePhi2();
};

