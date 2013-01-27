#include <cppunit/extensions/HelperMacros.h>

#include "exp.h"

class ExpTest : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( ExpTest );
    CPPUNIT_TEST( test99 );
    CPPUNIT_TEST( testFlt );
    CPPUNIT_TEST( testRegOf2 );
    CPPUNIT_TEST( testBinaries );
    CPPUNIT_TEST( testUnaries );
    CPPUNIT_TEST( testIsAfpTerm );
    CPPUNIT_TEST( testCompare1 );
    CPPUNIT_TEST( testCompare2 );
    CPPUNIT_TEST( testCompare3 );
    CPPUNIT_TEST( testCompare4 );
    CPPUNIT_TEST( testCompare5 );
    CPPUNIT_TEST( testCompare6 );
    CPPUNIT_TEST( testSearchReplace1 );
    CPPUNIT_TEST( testSearchReplace2 );
    CPPUNIT_TEST( testSearchReplace3 );
    CPPUNIT_TEST( testSearchReplace4 );
    CPPUNIT_TEST( testSearch1 );
    CPPUNIT_TEST( testSearch2 );
    CPPUNIT_TEST( testSearch3 );
    CPPUNIT_TEST( testSearchAll );
    CPPUNIT_TEST( testPartitionTerms );
    CPPUNIT_TEST( testAccumulate );
    CPPUNIT_TEST( testSimplifyArith );
    CPPUNIT_TEST( testSimplifyUnary );
    CPPUNIT_TEST( testSimplifyBinary );
    CPPUNIT_TEST( testSimplifyAddr );
    CPPUNIT_TEST( testSimpConstr );
    CPPUNIT_TEST( testLess );
    CPPUNIT_TEST( testMapOfExp );
    CPPUNIT_TEST( testList );
    CPPUNIT_TEST( testParen );
    CPPUNIT_TEST( testFixSuccessor );
    CPPUNIT_TEST( testKillFill );
    CPPUNIT_TEST( testAssociativity );
    CPPUNIT_TEST( testSubscriptVar );
    CPPUNIT_TEST( testTypeOf );
    CPPUNIT_TEST( testSetConscripts );
    CPPUNIT_TEST( testAddUsedLocs );
    CPPUNIT_TEST( testSubscriptVars );
    CPPUNIT_TEST( testVisitors );
    CPPUNIT_TEST_SUITE_END();

protected:
    Const*		m_99;
    Location*	m_rof2;

public:
    void setUp ();
    void tearDown ();

protected:
    void test99 ();
    void testFlt ();
    void testRegOf2 ();

    void testBinaries ();
    void testUnaries ();

    void testIsAfpTerm();

    void testCompare1();
    void testCompare2();
    void testCompare3();
    void testCompare4();
    void testCompare5();
    void testCompare6();

    void testSearchReplace1();
    void testSearchReplace2();
    void testSearchReplace3();
    void testSearchReplace4();

    void testSearch1();
    void testSearch2();
    void testSearch3();
    void testSearchAll();

    void testPartitionTerms();
    void testAccumulate();
    void testSimplifyArith();
    void testSimplifyUnary();
    void testSimplifyBinary();
    void testSimplifyAddr();
    void testSimpConstr();

    void testLess();
    void testMapOfExp();

    void testList();
    void testParen();
    void testFixSuccessor();
    void testKillFill();
    void testAssociativity();

    void testSubscriptVar();
    void testTypeOf();
    void testSetConscripts();
    void testAddUsedLocs();
    void testSubscriptVars();
    void testVisitors();
};

