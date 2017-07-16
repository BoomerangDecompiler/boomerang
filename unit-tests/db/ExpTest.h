#include <cppunit/extensions/HelperMacros.h>

#include "db/exp.h"

class ExpTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(ExpTest);
	CPPUNIT_TEST(test99);
	CPPUNIT_TEST(testFlt);
	CPPUNIT_TEST(testRegOf2);
	CPPUNIT_TEST(testBinaries);
	CPPUNIT_TEST(testUnaries);
	CPPUNIT_TEST(testIsAfpTerm);
	CPPUNIT_TEST(testCompare1);
	CPPUNIT_TEST(testCompare2);
	CPPUNIT_TEST(testCompare3);
	CPPUNIT_TEST(testCompare4);
	CPPUNIT_TEST(testCompare5);
	CPPUNIT_TEST(testCompare6);
	CPPUNIT_TEST(testSearchReplace1);
	CPPUNIT_TEST(testSearchReplace2);
	CPPUNIT_TEST(testSearchReplace3);
	CPPUNIT_TEST(testSearchReplace4);
	CPPUNIT_TEST(testSearch1);
	CPPUNIT_TEST(testSearch2);
	CPPUNIT_TEST(testSearch3);
	CPPUNIT_TEST(testSearchAll);
	CPPUNIT_TEST(testPartitionTerms);
	CPPUNIT_TEST(testAccumulate);
	CPPUNIT_TEST(testSimplifyArith);
	CPPUNIT_TEST(testSimplifyUnary);
	CPPUNIT_TEST(testSimplifyBinary);
	CPPUNIT_TEST(testSimplifyAddr);
	CPPUNIT_TEST(testSimpConstr);
	CPPUNIT_TEST(testLess);
	CPPUNIT_TEST(testMapOfExp);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST(testParen);
	CPPUNIT_TEST(testFixSuccessor);
	CPPUNIT_TEST(testKillFill);
	CPPUNIT_TEST(testAssociativity);
	CPPUNIT_TEST(testSubscriptVar);
	CPPUNIT_TEST(testTypeOf);
	CPPUNIT_TEST(testSetConscripts);
	CPPUNIT_TEST(testAddUsedLocs);
	CPPUNIT_TEST(testSubscriptVars);
	CPPUNIT_TEST(testVisitors);
	CPPUNIT_TEST_SUITE_END();

protected:
	Const *m_99;
	Location *m_rof2;

public:
    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::setUp
    * OVERVIEW:        Set up some expressions for use with all the tests
    * NOTE:            Called before any tests
    * PARAMETERS:        <none>
    *
    *============================================================================*/
	void setUp();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::tearDown
    * OVERVIEW:        Delete expressions created in setUp
    * NOTE:            Called after all tests
    * PARAMETERS:        <none>
    *
    *============================================================================*/
	void tearDown();

protected:
    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::test99
    * OVERVIEW:        Test integer constant
    *============================================================================*/
	void test99();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testFlt
    * OVERVIEW:        Test float constant
    *============================================================================*/
	void testFlt();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testRegOf2
    * OVERVIEW:        Tests r[2], which is used in many tests. Also tests opRegOf,
    *                    and ostream::operator&(Exp*)
    * NOTE:            r[2] prints as r2, as of June 2003
    *============================================================================*/
	void testRegOf2();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testBinaries
    * OVERVIEW:        Test opPlus, opMinus, etc
    *============================================================================*/
	void testBinaries();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testUnaries
    * OVERVIEW:        Test LNot, unary minus, etc
    *============================================================================*/
	void testUnaries();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testIsAfpTerm
    * OVERVIEW:        Test [ a[m[ ] %afp [+|- const]
    *============================================================================*/
	void testIsAfpTerm();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testCompare1-6
    * OVERVIEW:        Test the operator== function
    *============================================================================*/
	void testCompare1();
	void testCompare2();
	void testCompare3();
	void testCompare4();
	void testCompare5();
	void testCompare6();


    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testSearchReplace1-4
    * OVERVIEW:        Test the searchReplace function
    *============================================================================*/
	void testSearchReplace1();
	void testSearchReplace2();
	void testSearchReplace3();
	void testSearchReplace4();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testSearch1-4
    * OVERVIEW:        Test the search function, including wildcards
    *============================================================================*/
	void testSearch1();
	void testSearch2();
	void testSearch3();
	void testSearchAll();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testPartitionTerms
    * OVERVIEW:        Test the partitionTerms function
    *============================================================================*/
	void testPartitionTerms();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testAccumulate
    * OVERVIEW:        Test the Accumulate function
    *============================================================================*/
	void testAccumulate();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testSimplifyArith
    * OVERVIEW:        Test the simplifyArith function
    *============================================================================*/
	void testSimplifyArith();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testSimplifyUnary
    * OVERVIEW:        Test the simplifyArith function
    *============================================================================*/
	void testSimplifyUnary();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testSimplifyBinary
    * OVERVIEW:        Test the simplifyArith function
    *============================================================================*/
	void testSimplifyBinary();

	void testSimplifyAddr();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testSimpConstr
    * OVERVIEW:        Test the simplifyConstraint functions
    *============================================================================*/
	void testSimpConstr();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testLess
    * OVERVIEW:        Various tests of the operator< function
    *============================================================================*/
	void testLess();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testMapOfExp
    * OVERVIEW:        Test maps of Exp*s; exercises some comparison operators
    *============================================================================*/
	void testMapOfExp();

    /***************************************************************************/ /**
    * FUNCTION:        Exp::testList
    * OVERVIEW:        Test the opList creating and printing
    *============================================================================*/
	void testList();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testParens
    * OVERVIEW:        Test the printing of parentheses in complex expressions
    *============================================================================*/
	void testParen();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testFixSuccessor
    * OVERVIEW:        Test succ(r[k]) == r[k+1]
    *============================================================================*/
	void testFixSuccessor();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testKillFill
    * OVERVIEW:        Test removal of zero fill, sign extend, truncates
    *============================================================================*/
	void testKillFill();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testAssociativity
    * OVERVIEW:        Test that a+K+b is the same as a+b+K when each is simplified
    *============================================================================*/
	void testAssociativity();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testSubscriptVar
    * OVERVIEW:        Test Assign::subscriptVar and thereby Exp::expSubscriptVar
    *============================================================================*/
	void testSubscriptVar();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testTypeOf
    * OVERVIEW:        Test opTypeOf and TypeVal (type values)
    *============================================================================*/
	void testTypeOf();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testSetConscript
    * OVERVIEW:        Test setting and printing of constant "subscripts"
    *============================================================================*/
	void testSetConscripts();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testAddUsedLocs
    * OVERVIEW:        Test finding the locations used by an expression
    *============================================================================*/
	void testAddUsedLocs();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testSubscriptVars
    * OVERVIEW:        Test the subscripting of variables (locations)
    *============================================================================*/
	void testSubscriptVars();

    /***************************************************************************/ /**
    * FUNCTION:        ExpTest::testVisitors
    * OVERVIEW:        Test the FlagsFinder and BareMemofFinder visitors
    *============================================================================*/
	void testVisitors();
};
