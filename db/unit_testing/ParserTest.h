#include <cppunit/extensions/HelperMacros.h>

class ParserTest : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( ParserTest );
    CPPUNIT_TEST( testRead );
    CPPUNIT_TEST( testExp );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp ();
    void tearDown ();

protected:
    void testRead ();
    void testExp ();
};

