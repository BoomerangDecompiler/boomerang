#include <cppunit/extensions/HelperMacros.h>

class RtlTest : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( RtlTest );
    CPPUNIT_TEST( testAppend );
    CPPUNIT_TEST( testClone );
    CPPUNIT_TEST( testVisitor );
    CPPUNIT_TEST( testIsCompare );
    CPPUNIT_TEST( testSetConscripts );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp ();
    void tearDown ();

protected:
    void testAppend ();
    void testClone ();
    void testVisitor();
    void testIsCompare();
    void testSetConscripts();
};

