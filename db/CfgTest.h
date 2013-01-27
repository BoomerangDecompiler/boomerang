#include <cppunit/extensions/HelperMacros.h>

#include "cfg.h"

class CfgTest : public CPPUNIT_NS::TestFixture
  {
    CPPUNIT_TEST_SUITE( CfgTest );
    CPPUNIT_TEST( testDominators );
    CPPUNIT_TEST( testSemiDominators );
    CPPUNIT_TEST( testPlacePhi );
    CPPUNIT_TEST( testPlacePhi2 );
    CPPUNIT_TEST( testRenameVars );
    CPPUNIT_TEST_SUITE_END();
  protected:
    Cfg*  m_prog;

  public:

    void setUp ();
    void tearDown ();

  protected:
    void testDominators ();
    void testSemiDominators ();
    void testPlacePhi ();
    void testPlacePhi2();
    void testRenameVars();
};

