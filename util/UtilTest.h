#include <cppunit/extensions/HelperMacros.h>


class UtilTest : public CPPUNIT_NS::TestFixture
  {
    CPPUNIT_TEST_SUITE( UtilTest );
    CPPUNIT_TEST( test_hasExt );
    CPPUNIT_TEST( test_changeExt );
    CPPUNIT_TEST( test_searchAndReplace);
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp ();
    void tearDown ();

  protected:
    void test_hasExt ();
    void test_changeExt ();
    void test_searchAndReplace ();
};

