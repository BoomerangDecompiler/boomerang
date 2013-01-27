#include <cppunit/extensions/HelperMacros.h>

class FrontPentTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE( FrontPentTest );
  CPPUNIT_TEST( test1 );
  CPPUNIT_TEST( test2 );
  CPPUNIT_TEST( test3 );
  CPPUNIT_TEST( testBranch );
  CPPUNIT_TEST( testFindMain );
  CPPUNIT_TEST_SUITE_END();

  public:
    void setUp ();
    void tearDown ();

  protected:
    void test1 ();
    void test2 ();
    void test3 ();
    void testBranch();
    void testFindMain();
};

