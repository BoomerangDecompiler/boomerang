#include <cppunit/extensions/HelperMacros.h>

#include "prog.h"

class ProgTest : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( ProgTest );
    CPPUNIT_TEST( testName );
    CPPUNIT_TEST_SUITE_END();

protected:
    Prog*  m_prog;

public:
    void setUp ();
    void tearDown ();

protected:
    void testName ();
};

