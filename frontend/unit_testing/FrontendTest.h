#include <cppunit/extensions/HelperMacros.h>

class FrontendTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE( FrontendTest );
  CPPUNIT_TEST( test1 );
  CPPUNIT_TEST_SUITE_END();

  public:
    void setUp ();
    void tearDown ();

  protected:
    void test1 ();
};

