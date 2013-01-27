#include <cppunit/extensions/HelperMacros.h>


class TypeTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(TypeTest);
    CPPUNIT_TEST(testTypeLong);
    CPPUNIT_TEST(testNotEqual);
    CPPUNIT_TEST(testCompound);
    CPPUNIT_TEST(testDataInterval);
    CPPUNIT_TEST(testDataIntervalOverlaps);
    CPPUNIT_TEST_SUITE_END();

  public:

    void setUp ();
    void tearDown ();

protected:
    void testTypeLong ();
    void testNotEqual ();
    void testCompound();

    void testDataInterval();
    void testDataIntervalOverlaps();
};

