/*==============================================================================
 * FILE:	   ProcTest.h
 * OVERVIEW:   Provides the interface for the ProcTest class, which
 *				tests the Proc class
 *============================================================================*/
/*
 * $Revision: 1.5 $
 *
 * 23 Apr 02 - Mike: Created
 */

#include <cppunit/extensions/HelperMacros.h>

class Proc;
class ProcTest : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( ProcTest );
    CPPUNIT_TEST( testName );
    CPPUNIT_TEST_SUITE_END();

protected:
    Proc*  m_proc;

public:
    void setUp ();
    void tearDown ();

protected:
    void testName ();
};

