#include <cppunit/extensions/HelperMacros.h>

class CTest : public CPPUNIT_NS::TestFixture
  {
    CPPUNIT_TEST_SUITE( CTest );
    CPPUNIT_TEST( testSignature );
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp ();
    void tearDown ();

  protected:
    void testSignature();
};

