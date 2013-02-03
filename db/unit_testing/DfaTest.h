#include <cppunit/extensions/HelperMacros.h>

class DfaTest : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( DfaTest );
    CPPUNIT_TEST( testMeetInt );
    CPPUNIT_TEST( testMeetSize );
    CPPUNIT_TEST( testMeetPointer );
    CPPUNIT_TEST( testMeetUnion );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp ();
    void tearDown ();

protected:
    void testMeetInt();
    void testMeetSize();
    void testMeetPointer();
    void testMeetUnion();
};

