/*==============================================================================
 * FILE:       ExpTest.cc
 * OVERVIEW:   Provides the implementation for the ExpTest class, which
 *              tests the Exp and derived classes
 *============================================================================*/
/*
 * $Revision$
 *
 * 05 Apr 02 - Mike: Fixed problems caused by lack of clone() calls
 * 09 Apr 02 - Mike: Compare, searchReplace
 * 14 Apr 02 - Mike: search and replace functions take Exp*, was Exp&
 * 27 Apr 02 - Mike: Added testDecideType
 * 09 Dec 02 - Mike: Added test for fixSuccessor
 * 13 Dec 02 - Mike: Added test for killFill()
 */

#include "ExpTest.h"
#include <map>
#include <sstream>      // Gcc >= 3.0 needed

/*==============================================================================
 * FUNCTION:        ExpTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<ExpTest> ("testExp", \
    &ExpTest::name, *this))

void ExpTest::registerTests(CppUnit::TestSuite* suite) {
MYTEST(testFixSuccessor);
    MYTEST(test99);
    MYTEST(testFlt);
    MYTEST(testRegOf2);
    MYTEST(testBinaries);
    MYTEST(testUnaries);
    MYTEST(testIsAfpTerm);
    MYTEST(testCompare1);
    MYTEST(testCompare2);
    MYTEST(testCompare3);
    MYTEST(testCompare4);
    MYTEST(testCompare5);
    MYTEST(testCompare6);
    MYTEST(testSearchReplace1);
    MYTEST(testSearchReplace2);
    MYTEST(testSearchReplace3);
    MYTEST(testSearchReplace4);
    MYTEST(testSearch1);
    MYTEST(testSearch2);
    MYTEST(testSearch3);
    MYTEST(testSearchAll);
    MYTEST(testAccumulate);
    MYTEST(testPartitionTerms);
    MYTEST(testSimplifyArith);
    MYTEST(testSimplifyUnary);
    MYTEST(testSimplifyBinary);
    MYTEST(testSimplifyAddr);
    MYTEST(testSimpConstr);

    MYTEST(testBecome);
    MYTEST(testLess);
    MYTEST(testMapOfExp);
    MYTEST(testList);
    MYTEST(testParen);
	MYTEST(testFixSuccessor);
	MYTEST(testKillFill);
	MYTEST(testAssociativity);
    MYTEST(testSubscriptVar);
    MYTEST(testTypeOf);
    MYTEST(testSetConscripts);
}

int ExpTest::countTestCases () const
{ return 2; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        ExpTest::setUp
 * OVERVIEW:        Set up some expressions for use with all the tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void ExpTest::setUp () {
    m_99 = new Const(99);
    m_rof2 = new Location(opRegOf, new Const(2), NULL);
}

/*==============================================================================
 * FUNCTION:        ExpTest::tearDown
 * OVERVIEW:        Delete expressions created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void ExpTest::tearDown () {
    delete m_99;
    delete m_rof2;
}

/*==============================================================================
 * FUNCTION:        ExpTest::test99
 * OVERVIEW:        Test integer constant
 *============================================================================*/
void ExpTest::test99 () {
    std::ostringstream ost;
    m_99->print(ost);
    CPPUNIT_ASSERT (std::string("99") == std::string(ost.str()));
}

/*============================================================================== * FUNCTION:        ExpTest::testFlt
 * OVERVIEW:        Test float constant
 *============================================================================*/void ExpTest::testFlt () {
    std::ostringstream ost;
    Const *c = new Const(3.14);
    c->print(ost);
    CPPUNIT_ASSERT_EQUAL (std::string("3.14"), std::string(ost.str()));
    delete c;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testRegOf2
 * OVERVIEW:        Tests r[2], which is used in many tests. Also tests opRegOf,
 *                  and ostream::operator&(Exp*)
 * NOTE:            r[2] prints as r2, as of June 2003
 *============================================================================*/
void ExpTest::testRegOf2 () {
    std::ostringstream ost;
    ost << m_rof2;
    CPPUNIT_ASSERT_EQUAL (std::string("r2"), std::string(ost.str()));
}

/*==============================================================================
 * FUNCTION:        ExpTest::testBinaries
 * OVERVIEW:        Test opPlus, opMinus, etc
 *============================================================================*/
void ExpTest::testBinaries () {
    std::ostringstream ost1;
    Binary* b = new Binary(opPlus, m_99->clone(), m_rof2->clone());
    b->print(ost1);
    CPPUNIT_ASSERT_EQUAL (std::string("99 + r2"), std::string(ost1.str()));
    delete b;

    std::ostringstream ost2;
    b = new Binary(opMinus, m_99->clone(), m_rof2->clone());
    b->print(ost2);
    CPPUNIT_ASSERT_EQUAL (std::string("99 - r2"), std::string(ost2.str()));
    delete b;

    std::ostringstream ost3;
    b = new Binary(opMult, m_99->clone(), m_rof2->clone());
    b->print(ost3);
    CPPUNIT_ASSERT_EQUAL (std::string("99 * r2"), std::string(ost3.str()));
    delete b;

    std::ostringstream ost4;
    b = new Binary(opDiv, m_99->clone(), m_rof2->clone());
    b->print(ost4);
    CPPUNIT_ASSERT_EQUAL (std::string("99 / r2"), std::string(ost4.str()));
    delete b;

    std::ostringstream ost5;
    b = new Binary(opMults, m_99->clone(), m_rof2->clone());
    b->print(ost5);
    CPPUNIT_ASSERT_EQUAL (std::string("99 *! r2"), std::string(ost5.str()));
    delete b;

    std::ostringstream ost6;
    b = new Binary(opDivs, m_99->clone(), m_rof2->clone());
    b->print(ost6);
    CPPUNIT_ASSERT_EQUAL (std::string("99 /! r2"), std::string(ost6.str()));
    delete b;

    std::ostringstream ost7;
    b = new Binary(opMod, m_99->clone(), m_rof2->clone());
    b->print(ost7);
    CPPUNIT_ASSERT_EQUAL (std::string("99 % r2"), std::string(ost7.str()));
    delete b;

    std::ostringstream ost8;
    b = new Binary(opMods, m_99->clone(), m_rof2->clone());
    b->print(ost8);
    CPPUNIT_ASSERT_EQUAL (std::string("99 %! r2"), std::string(ost8.str()));
    delete b;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testUnaries
 * OVERVIEW:        Test LNot, unary minus, etc
 *============================================================================*/
void ExpTest::testUnaries () {
    std::ostringstream ost1;
    Unary* u = new Unary(opNot, new Terminal(opZF));
    u->print(ost1);
    CPPUNIT_ASSERT_EQUAL (std::string("~%ZF"), std::string(ost1.str()));
    delete u;

    std::ostringstream ost2;
    u = new Unary(opLNot, new Terminal(opCF));
    u->print(ost2);
    CPPUNIT_ASSERT_EQUAL (std::string("L~%CF"), std::string(ost2.str()));
    delete u;

    std::ostringstream ost3;
    u = new Unary(opNeg, m_rof2->clone());
    u->print(ost3);
    CPPUNIT_ASSERT_EQUAL (std::string("-r2"), std::string(ost3.str()));
    delete u;

}
/*==============================================================================
 * FUNCTION:        ExpTest::testIsAfpTerm
 * OVERVIEW:        Test [ a[m[ ] %afp [+|- const]
 *============================================================================*/
void ExpTest::testIsAfpTerm () {
    Terminal afp(opAFP);
    Binary plus(opPlus,   afp.clone(), new Const(-99));
    Binary minus(opMinus, afp.clone(), m_99->clone());
    CPPUNIT_ASSERT(   afp.  isAfpTerm());
    CPPUNIT_ASSERT(   plus. isAfpTerm());
    CPPUNIT_ASSERT(   minus.isAfpTerm());
    CPPUNIT_ASSERT(!m_99  ->isAfpTerm());
    CPPUNIT_ASSERT(!m_rof2->isAfpTerm());
    // Now with typed expressions
    TypedExp tafp(new IntegerType(), afp.clone());
    //Unary tafp  (opTypedExp, afp.clone());
    Unary tplus (opTypedExp, plus.clone());
    Unary tminus(opTypedExp, minus.clone());
    CPPUNIT_ASSERT(tafp.  isAfpTerm());
    CPPUNIT_ASSERT(tplus. isAfpTerm());
    CPPUNIT_ASSERT(tminus.isAfpTerm());
}

/*==============================================================================
 * FUNCTION:        ExpTest::testCompare1-6
 * OVERVIEW:        Test the operator== function
 *============================================================================*/
void ExpTest::testCompare1 () {
    CPPUNIT_ASSERT(! (*m_99 == *m_rof2));
}
void ExpTest::testCompare2 () {
    Const nineNine(99);
    CPPUNIT_ASSERT(*m_99 == nineNine);
}
void ExpTest::testCompare3 () {
    Const minus(-99);
    CPPUNIT_ASSERT(! (*m_99 == minus));
}
void ExpTest::testCompare4 () {
    Location regOf2(opRegOf, new Const(2), NULL);
    CPPUNIT_ASSERT(regOf2 == *m_rof2);
}
void ExpTest::testCompare5 () {
    Binary one(opMult, m_99->clone(), m_rof2->clone());
    Binary two(opMult, m_rof2->clone(), m_99->clone());
    CPPUNIT_ASSERT(! (one == two) );
}
void ExpTest::testCompare6 () {
    Binary one(opMult, m_99->clone(), m_rof2->clone());
    Binary two(opMult, m_99->clone(), m_rof2->clone());
    CPPUNIT_ASSERT(  (one == two) );
}

/*==============================================================================
 * FUNCTION:        ExpTest::testSearchReplace1-4
 * OVERVIEW:        Test the searchReplace function
 *============================================================================*/
void ExpTest::testSearchReplace1() {
    // Null test: should not replace. Also tests Ternary class
    Exp* p; bool change;
    p = new Ternary(opAt, m_rof2->clone(), new Const(15), new Const(8));
    p = p->searchReplace(m_99, m_rof2, change);
    std::string expected("r2@15:8");
    std::ostringstream ost;
    p->print(ost);
    std::string actual(ost.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
    Ternary t2(*(Ternary*)p);
    CPPUNIT_ASSERT (*p == t2);
    p = p->searchReplaceAll(m_99, m_rof2, change);
    CPPUNIT_ASSERT (*p == t2);
    delete p;
}

void ExpTest::testSearchReplace2() {
    // Whole expression replacements
    bool change;
    Exp* p1 = new Const (55);
    Const p2(*(Const*)p1);
    Const c2(1234);
    p1 = p1->searchReplace(&p2, &c2, change);
    CPPUNIT_ASSERT(*p1 == c2);
    CPPUNIT_ASSERT(p1 != &c2);       // Pointers should not be same
    p1 = p1->searchReplace(&c2, m_rof2, change);
    CPPUNIT_ASSERT(*p1 == *m_rof2);
    delete p1;
}

void ExpTest::testSearchReplace3() {
    // Subexpression replacement
    bool change;
    Const two(2);
    Const three(3);
    Exp* p = m_rof2->clone();
    p = p->searchReplaceAll(&two, &three, change);
    std::string expected("r3");
    std::ostringstream ost;
    p->print(ost);
    std::string actual(ost.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
    delete p;
}
void ExpTest::testSearchReplace4() {
    // Subexpression replacement with different subexpression form
    bool change;
    Const two(2);
    Exp* p = m_rof2->clone();
    // Note recursion. OK to use the all function, since it does the search
    // first.
    p = p->searchReplaceAll(&two, m_rof2, change);
    std::string expected("r[r2]");
    std::ostringstream ost;
    p->print(ost);
    std::string actual(ost.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
    delete p;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testSearch1-4
 * OVERVIEW:        Test the search function, including wildcards
 *============================================================================*/
void ExpTest::testSearch1() {
    Const two(2);
    Exp* result;
    // Search inside r2 for const 2
    CPPUNIT_ASSERT(m_rof2->search(&two, result));
    CPPUNIT_ASSERT(*result == two);

    // Test for false positives
    CPPUNIT_ASSERT(! m_99->search(&two, result));
    CPPUNIT_ASSERT(! m_rof2->search(m_99, result));

    // Note: opDiv's enum has value 3
    Binary e(opMult, m_rof2->clone(), m_99->clone());   // r2 / 99
    Const three(3);
    CPPUNIT_ASSERT(! e.search(&three, result));
}

void ExpTest::testSearch2() {
    // Search using wildcards
    Binary e(opDivs, m_rof2->clone(), m_99->clone());   // r2 /! 99
    Exp* result;
    Location search(opRegOf, new Terminal(opWild), NULL);    // r[?]
    CPPUNIT_ASSERT(e.search(&search, result));
    CPPUNIT_ASSERT(*result == *m_rof2);             // Should be r2

    Const three(3);
    CPPUNIT_ASSERT(! e.search(&three, result));
    CPPUNIT_ASSERT(e.search(m_99, result));
}

void ExpTest::testSearch3() {
    // A more complex expression:
    // (r2 * 99) + (m[1000] * 4)
    Exp* result;
    Binary e(opPlus, new Binary(opMult, m_rof2->clone(), m_99->clone()),
        new Binary(opMult,
            Location::memOf(new Const(1000)),
            new Const(4)));
    Const four(4);
    Location mem1000(opMemOf, new Const(1000), NULL);
    Binary prod(opMult, m_rof2->clone(), m_99->clone());
    CPPUNIT_ASSERT(e.search(&four,   result));
    CPPUNIT_ASSERT(e.search(&mem1000,result));
    CPPUNIT_ASSERT(e.search(&prod,   result));
    CPPUNIT_ASSERT(e.search(m_99,  result));
    Const three(3);
    CPPUNIT_ASSERT(! e.search(&three, result));
}

void ExpTest::testSearchAll() {
    // A more complex expression:
    // (r2 * 99) + (r8 * 4)
    Location search(opRegOf, new Terminal(opWild), NULL);    // r[?]
    std::list<Exp*> result;
    Binary e(opPlus, new Binary(opMult, m_rof2->clone(), m_99->clone()),
        new Binary(opMult,
            Location::regOf(8),
            new Const(4)));
    CPPUNIT_ASSERT(e.searchAll(&search, result));
    CPPUNIT_ASSERT(result.size() == 2);
    CPPUNIT_ASSERT(*result.front() == *m_rof2);
    Location rof8(opRegOf, new Const(8), NULL);
    CPPUNIT_ASSERT(*result.back() == rof8);
}
/*==============================================================================
 * FUNCTION:        ExpTest::testAccumulate
 * OVERVIEW:        Test the Accumulate function
 *============================================================================*/
void ExpTest::testAccumulate () {
    Location rof2(opRegOf, new Const(2), NULL);
    Const nineNine(99);
    // Zero terms
    std::list<Exp*> le;
    Exp* res = Exp::Accumulate(le);
    Const zero(0);
    CPPUNIT_ASSERT(*res == zero);
    delete res;

    // One term
    le.push_back(&rof2);
    res = Exp::Accumulate(le);
    CPPUNIT_ASSERT(*res == rof2);
    delete res;

    // Two terms
    Exp* nn = nineNine.clone();
    le.push_back(nn);
    res = Exp::Accumulate(le);
    Binary expected2(opPlus, rof2.clone(), nineNine.clone());
    CPPUNIT_ASSERT(*res == expected2);
    delete res;

    // Three terms, one repeated
    le.push_back(&nineNine);
    res = Exp::Accumulate(le);
    Binary expected3(opPlus, rof2.clone(),
        new Binary(opPlus, nineNine.clone(), nineNine.clone()));
    CPPUNIT_ASSERT(*res == expected3);
    delete res;

    // Four terms, one repeated
    Terminal afp(opAFP);
    le.push_back(&afp);
    res = Exp::Accumulate(le);
    Binary expected4(opPlus, rof2.clone(),
        new Binary(opPlus, nineNine.clone(), 
            new Binary(opPlus, nineNine.clone(), new Terminal(opAFP))));
    CPPUNIT_ASSERT(*res == expected4);
    delete res;
    delete nn;
}  

/*==============================================================================
 * FUNCTION:        ExpTest::testPartitionTerms
 * OVERVIEW:        Test the partitionTerms function
 *============================================================================*/
void ExpTest::testPartitionTerms() {
    std::ostringstream ost;
    // afp + 108 + n - (afp + 92)
    Binary e(opMinus,
        new Binary(opPlus,
            new Binary(opPlus, new Terminal(opAFP), new Const(108)),
            new Unary(opVar, new Const("n"))),
        new Binary(opPlus, new Terminal(opAFP), new Const(92))
    );
    std::list<Exp*> positives, negatives;
    std::vector<int> integers;
    e.partitionTerms(positives, negatives, integers, false);
    Exp* res = Exp::Accumulate(positives);
    Binary expected1(opPlus, new Terminal(opAFP),
        new Unary(opVar, new Const("n")));
    CPPUNIT_ASSERT(*res == expected1);
    delete res;

    res = Exp::Accumulate(negatives);
    Terminal expected2(opAFP);
    CPPUNIT_ASSERT(*res == expected2);
    int size = integers.size();
    CPPUNIT_ASSERT_EQUAL(2, size);
    CPPUNIT_ASSERT_EQUAL(108, integers.front());
    CPPUNIT_ASSERT_EQUAL(-92, integers.back());
    delete res;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testSimplifyArith
 * OVERVIEW:        Test the simplifyArith function
 *============================================================================*/
void ExpTest::testSimplifyArith() {
    std::ostringstream ost;
    // afp + 108 + n - (afp + 92)
    Exp* e = new Binary(opMinus,
        new Binary(opPlus,
            new Binary(opPlus, new Terminal(opAFP), new Const(108)),
            new Unary(opVar, new Const("n"))),
        new Binary(opPlus, new Terminal(opAFP), new Const(92))
    );
    e = e->simplifyArith();
    e->print(ost);
    std::string expected ("v[n] + 16");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(ost.str()));
    delete e;

    // m[(r28 + -4) + 8]
    Exp* mm = Location::memOf(
        new Binary(opPlus,
            new Binary(opPlus,
                Location::regOf(28),
                new Const(-4)),
            new Const(8)));
    mm = mm->simplifyArith();
    std::ostringstream ost2;
    mm->print(ost2);
    expected = "m[r28 + 4]";
    CPPUNIT_ASSERT_EQUAL(expected, std::string(ost2.str()));
    delete mm;
   
    // r24 + m[(r28 - 4) - 4] 
    mm = new Binary(opPlus,
        Location::regOf(24),
        Location::memOf(
            new Binary(opMinus,
                new Binary(opMinus,
                    Location::regOf(28),
                    new Const(4)),
                new Const(4))));
    mm = mm->simplifyArith();
    std::ostringstream ost3;
    mm->print(ost3);
    expected = "r24 + m[r28 - 8]";
    CPPUNIT_ASSERT_EQUAL(expected, std::string(ost3.str()));
    delete mm;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testSimplifyUnary
 * OVERVIEW:        Test the simplifyArith function
 *============================================================================*/
void ExpTest::testSimplifyUnary() {
    // Unaries with integer constant argument
    Exp* u = new Unary(opNeg, new Const (55));
    u = u->simplify();
    Const minus55(-55);
    CPPUNIT_ASSERT(*u == minus55);
    delete u;

    u = new Unary(opNot, new Const(0x55AA));
    u = u->simplify();
    Const exp((int)0xFFFFAA55);
    CPPUNIT_ASSERT(*u == exp);
    delete u;

    u = new Unary(opLNot, new Const(55));
    u = u->simplify();
    Const zero(0);
    CPPUNIT_ASSERT(*u == zero);
    delete u;

    u = new Unary(opLNot, zero.clone());
    u = u->simplify();
    Const one(1);
    CPPUNIT_ASSERT(*u == one);
    delete u;

    // Null test
    u = new Unary(opNeg, new Unary(opVar, new Const("abc")));
    Unary abc(opNeg, new Unary(opVar, new Const("abc")));
    CPPUNIT_ASSERT(*u == abc);
    delete u;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testSimplifyBinary
 * OVERVIEW:        Test the simplifyArith function
 *============================================================================*/
void ExpTest::testSimplifyBinary() {
    // Add integer consts
    Exp* b = new Binary(opPlus, new Const(2), new Const(3));
    b = b->simplify();
    Const five(5);
    CPPUNIT_ASSERT(*b == five);
    delete b;

    // Multiply integer consts
    b = new Binary(opMult, new Const(2), new Const(3));
    b = b->simplify();
    Const six(6);
    CPPUNIT_ASSERT(*b == six);
    delete b;

    // Shift left two integer constants
    b = new Binary(opShiftL, new Const(2), new Const(3));
    b = b->simplify();
    Const sixteen(16);
    CPPUNIT_ASSERT(*b == sixteen);
    delete b;

    // Shift right arithmetic two integer contants
    b = new Binary(opShiftRA, new Const(-144), new Const(3));
    b = b->simplify();
    Const minus18(-18);
    CPPUNIT_ASSERT(*b == minus18);
    delete b;

    // Bitwise XOR
    b = new Binary(opBitXor, new Const(0x55), new Const(0xF));
    b = b->simplify();
    Const fiveA(0x5A);
    CPPUNIT_ASSERT(*b == fiveA);
    delete b;

    // Xor with self
    b = new Binary(opBitXor, m_rof2->clone(), m_rof2->clone());
    b = b->simplify();
    Const zero(0);
    CPPUNIT_ASSERT(*b == zero);
    delete b;

    // Test commute
    // 77 * r2
    b = new Binary(opMults, new Const(77), m_rof2->clone());
    b = b->simplify();
    // r2 * 77
    Binary exp(opMults, m_rof2->clone(), new Const(77));
    CPPUNIT_ASSERT(*b == exp);

    // x*1
    ((Const*)b->getSubExp2())->setInt(1);
    b = b->simplify();
    CPPUNIT_ASSERT(*b == *m_rof2);
    delete b;

    // 0 | r2
    b = new Binary(opBitOr, new Const(0), m_rof2->clone());
    b = b->simplify();
    CPPUNIT_ASSERT(*b == *m_rof2);
    delete b;

    // Left shift by const
    b = new Binary(opShiftL, m_rof2->clone(), new Const(0));
    b = b->simplify();
    CPPUNIT_ASSERT(*b == *m_rof2);
    delete b;

    b = new Binary(opShiftL, m_rof2->clone(), new Const(2));
    b = b->simplify();
    Binary expb1(opMult, m_rof2->clone(), new Const(4));
    CPPUNIT_ASSERT(*b == expb1);
    delete b;

    // Add negative constant
    // r2 + -99
    b = new Binary(opPlus, m_rof2->clone(), new Const(-99));
    // r2 - 99
    Exp* expb2 = new Binary(opMinus, m_rof2->clone(), new Const(99));
    // As of June 2003, I've decided to go the old way. esp + -4 is just
    // too ugly, and all the code has to cope with pluses and minuses anyway,
    // just in case
#define OLD_WAY 1
#if OLD_WAY
    b = b->simplify();
#else
    expb2 = expb2->simplify();
#endif
    CPPUNIT_ASSERT(*b == *expb2);
    delete b; delete expb2;

    std::string expected("((0 + v[a]) - 0) | 0");
    std::ostringstream ost;
    Exp* e =
        new Binary(opBitOr,
            new Binary(opMinus,
                new Binary(opPlus,
                    new Const(0),
                    new Unary(opVar,
                        new Const("a")
                    )
                ),
                new Const(0)
            ),
            new Const(0)
        );
    e->print(ost);
    CPPUNIT_ASSERT_EQUAL(expected, std::string(ost.str()));
    // The above should simplify to just "v[a]"
    e = e->simplify();
    Unary a(opVar, new Const("a"));
    expected = "v[a]";
    std::ostringstream ost2;
    e->print(ost2);
    CPPUNIT_ASSERT_EQUAL(expected, ost2.str());
    delete e;

    // r27 := m[r29 + -4]
    Assign* as = new Assign(
        Location::regOf(27),
        Location::memOf(
            new Binary(opPlus,
                Location::regOf(29),
                new Const(-4))));
    as->simplify();
    expected = "   0 *32* r27 := m[r29 - 4]";
    std::ostringstream ost3;
    as->print(ost3);
    CPPUNIT_ASSERT_EQUAL(expected, ost3.str());
    delete as;

    // (false and true) or (Tr24 = <int>)
    e = new Binary(opOr,
        new Binary(opAnd,
            new Terminal(opFalse),
            new Terminal(opTrue)),
        new Binary(opEquals,
            new Unary(opTypeOf, Location::regOf(24)),
            new TypeVal(new IntegerType())));
    e = e->simplify();
    expected = "Tr24 = <int>";
    std::ostringstream ost4;
    e->print(ost4);
    CPPUNIT_ASSERT_EQUAL(expected, ost4.str());
    delete e;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testSimplifyBinary
 * OVERVIEW:        Test the simplifyArith function
 *============================================================================*/
void ExpTest::testSimplifyAddr() {
    // a[m[1000]] - a[m[r2]{64}]@0:15
    Exp* e = new Binary(opMinus,
        new Unary(opAddrOf,
            Location::memOf(
                new Const(1000))),
        new Ternary(opAt,
            new Unary(opAddrOf,
                new Binary(opSize,
                    new Const(64),
                    Location::memOf(
                        Location::regOf(2)
                            ))),
            new Const(0),
            new Const(15)));
    e = e->simplifyAddr();
    std::ostringstream ost;
    e->print(ost);
    std::string actual(ost.str());
    std::string expected("1000 - (r2@0:15)");
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    // Now test at top level
    delete e;
    e = new Unary(opAddrOf,
        Location::memOf(
            new Const(1000)));
    expected = "1000";
    e = e->simplifyAddr();
    std::ostringstream ost2;
    e->print(ost2);
    actual = ost2.str();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
    delete e;

}

/*==============================================================================
 * FUNCTION:        ExpTest::testSimpConstr
 * OVERVIEW:        Test the simplifyConstraint functions
 *============================================================================*/
void ExpTest::testSimpConstr() {
    // (Tlocal1{16} = <int>) or (Tlocal1{16} = <alpha2*>)
    // gets substituted into
    // (<char*> = <int>) or (<char*> = <alpha2*>)
    Exp* e = new Binary(opOr,
        new Binary(opEquals,
            new TypeVal(new PointerType(new CharType())),
            new TypeVal(new IntegerType())),
        new Binary(opEquals,
            new TypeVal(new PointerType(new CharType())),
            new TypeVal(PointerType::newPtrAlpha())));
    e = e->simplifyConstraint();
    std::string expected("<char*> = <alpha0*>");
    std::ostringstream ost;
    e->print(ost);
    std::string actual = ost.str();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
    delete e;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testBecome
 * OVERVIEW:        Test the becomeSubExp2 function
 *============================================================================*/
void ExpTest::testBecome() {
    Binary* e;
    Exp* f;
    std::string expected("(2 * 3) - (4 * 5)");
    std::ostringstream ost1;
    e = new Binary(opPlus, new Const(0),
        new Binary(opMinus,
            new Binary(opMult, new Const(2), new Const(3)),
            new Binary(opMult, new Const(4), new Const(5))));
    e = (Binary*)e->becomeSubExp2();
    e->print(ost1);
    CPPUNIT_ASSERT_EQUAL(expected, std::string(ost1.str()));
    // e still used (and effectively deleted) below
 
    std::ostringstream ost2;
    f = new Binary(opPlus, e, new Const(0));
    f = ((Unary*)f)->becomeSubExp1();
    f->print(ost2);
    CPPUNIT_ASSERT_EQUAL(expected, std::string(ost2.str()));
    // f still used (and effectively deleted) below
 
    std::ostringstream ost3;
    Binary* b = new Binary(opPlus, f, new Const(0));
    b = (Binary*) b->becomeSubExp1();
    b->print(ost3);
    CPPUNIT_ASSERT_EQUAL(expected, std::string(ost3.str()));
    delete b;       // Also deletes e and f, effectively
}

/*==============================================================================
 * FUNCTION:        ExpTest::testLess
 * OVERVIEW:        Various tests of the operator< function
 *============================================================================*/
void ExpTest::testLess() {
    // Simple constants
    Const two(2), three(3), mThree(-3), twoPointTwo(2.2), threePointThree(3.3);
    Const mThreePointThree(-3.3);
    CPPUNIT_ASSERT(two < three);
    CPPUNIT_ASSERT(mThree < two);
    CPPUNIT_ASSERT(twoPointTwo < threePointThree);
    CPPUNIT_ASSERT(mThreePointThree < twoPointTwo);
    // Terminal
    Terminal afp(opAFP), agp(opAGP);
    if (opAFP < opAGP)
        CPPUNIT_ASSERT(afp < agp);
    else
        CPPUNIT_ASSERT(agp < afp);
    // Unary
    Unary negTwo(opNeg, new Const(2)), negThree(opNeg, new Const(3));
    // Note that the ordering is not arithmetic!
    CPPUNIT_ASSERT(negTwo < negThree);
    // Binary
    Binary twoByThr(opMult, new Const(2), new Const(3));
    Binary twoByFou(opMult, new Const(2), new Const(4));
    Binary thrByThr(opMult, new Const(3), new Const(3));
    CPPUNIT_ASSERT(twoByThr < twoByFou);
    CPPUNIT_ASSERT(twoByThr < thrByThr);
    // Ternary
    Ternary twoAtThrToFou(opAt, new Const(2), new Const(3), new Const(4));
    Ternary twoAtThrToFiv(opAt, new Const(2), new Const(3), new Const(5));
    CPPUNIT_ASSERT(twoAtThrToFou < twoAtThrToFiv);
    // TypedExp later
}
/*==============================================================================
 * FUNCTION:        ExpTest::testMapOfExp
 * OVERVIEW:        Test maps of Exp*s; exercises some comparison operators
 *============================================================================*/
void ExpTest::testMapOfExp() {
    std::map<Exp*, int, lessExpStar> m;
    m[m_rof2] = 200;
    m[m_99] = 99;
    Exp* e = new Binary(opPlus, new Const(0),
        new Binary(opMinus,
            new Binary(opMult, new Const(2), new Const(3)),
            new Binary(opMult, new Const(4), new Const(5))));
    m[e] = -100;
    Location rof2(opRegOf, new Const(2), NULL);
    m[&rof2] = 2;            // Should overwrite

    int i = m.size();
    CPPUNIT_ASSERT_EQUAL(3, i); 
    i = m[m_rof2];
    CPPUNIT_ASSERT_EQUAL(2, i);
    i = m[&rof2];
    CPPUNIT_ASSERT_EQUAL(2, i);
    i = m[m_99];
    CPPUNIT_ASSERT_EQUAL(99, i);
    i = m[e];
    CPPUNIT_ASSERT_EQUAL(-100, i); 
    // When the map goes out of scope, the expressions pointed to still exist
    delete e;
}

/*==============================================================================
 * FUNCTION:        Exp::testList
 * OVERVIEW:        Test the opList creating and printing
 *============================================================================*/
void ExpTest::testList () {
    std::ostringstream o0, o1, o2, o3, o4;
    Exp *l0, *l1, *l2, *l3, *l4;
    // Empty list
    l0 = new Binary(opList, new Terminal(opNil), new Terminal(opNil));
    o0 << l0;
    std::string expected0("");
    std::string actual0(o0.str());
    CPPUNIT_ASSERT_EQUAL(expected0, actual0);
    delete l0;

    // 1 element list
    l1 = new Binary(opList,
        new Unary(opParam, new Const("a")),
        new Terminal(opNil));
    o1 << l1;
    std::string expected1("a");
    std::string actual1(o1.str());
    CPPUNIT_ASSERT_EQUAL(expected1, actual1);
    delete l1;

    // 2 element list
    l2 = new Binary(opList,
        new Unary(opParam, new Const("a")),
        new Binary(opList,
            new Unary(opParam, new Const("b")),
            new Terminal(opNil)));
    o2 << l2;
    std::string expected2("a, b");
    std::string actual2(o2.str());
    CPPUNIT_ASSERT_EQUAL(expected2, actual2);
    delete l2;

    // 3 element list
    l3 = new Binary(opList,
        new Unary(opParam, new Const("a")),
        new Binary(opList,
            new Unary(opParam, new Const("b")),
            new Binary(opList,
                new Unary(opParam, new Const("c")),
                new Terminal(opNil))));
    o3 << l3;
    std::string expected3("a, b, c");
    std::string actual3(o3.str());
    CPPUNIT_ASSERT_EQUAL(expected3, actual3);
    delete l3;

    // 4 element list
    l4 = new Binary(opList,
        new Unary(opParam, new Const("a")),
        new Binary(opList,
            new Unary(opParam, new Const("b")),
            new Binary(opList,
                new Unary(opParam, new Const("c")),
                new Binary(opList,
                    new Unary(opParam, new Const("d")),
                    new Terminal(opNil)))));
    o4 << l4;
    std::string expected4("a, b, c, d");
    std::string actual4(o4.str());
    CPPUNIT_ASSERT_EQUAL(expected4, actual4);
    delete l4;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testParens
 * OVERVIEW:        Test the printing of parentheses in complex expressions
 *============================================================================*/
void ExpTest::testParen () {
    Assign a(32,
        Location::regOf(
            new Unary(opParam, new Const("rd"))),
        new Binary(opBitAnd,
            Location::regOf(
                new Unary(opParam, new Const("rs1"))),
            new Binary(opMinus,
                new Binary(opMinus,
                    new Const(0),
                    new Unary(opParam, new Const("reg_or_imm"))),
                new Const(1))));
    std::string expected("   0 *32* r[rd] := r[rs1] & ((0 - reg_or_imm) - 1)");
    std::ostringstream o;
    a.print(o);
    // a.createDotFile("andn.dot");
    std::string actual(o.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

/*==============================================================================
 * FUNCTION:        ExpTest::testFixSuccessor
 * OVERVIEW:        Test succ(r[k]) == r[k+1]
 *============================================================================*/
void ExpTest::testFixSuccessor() {
	// Trivial test (should not affect)
	Binary* b = new Binary(opMinus,
		m_99->clone(),
		m_rof2->clone());
	std::ostringstream o1;
	Exp* e = b->fixSuccessor();
	e->print(o1);
	std::string expected("99 - r2");
	std::string actual(o1.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
	delete e;
	
	Unary* u = new Unary(opSuccessor,
		Location::regOf(2));
	std::ostringstream o2;
	e = u->fixSuccessor();
	e->print(o2);
	expected = "r3";
	actual = o2.str();
    CPPUNIT_ASSERT_EQUAL(expected, actual);
	delete e;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testKillFill
 * OVERVIEW:        Test removal of zero fill, sign extend, truncates
 *============================================================================*/
void ExpTest::testKillFill() {
	// r18 + sgnex(16,32,m[r16 + 16])
	Binary e(opPlus,
		Location::regOf(18),
		new Ternary(opSgnEx, new Const(16), new Const(32),
			Location::memOf(
				new Binary(opPlus,
					Location::regOf(16),
					new Const(16)))));
	Exp* res = e.killFill();
	std::string expected("r18 + m[r16 + 16]");
	std::ostringstream ost1;
	res->print(ost1);
	std::string actual(ost1.str());
	CPPUNIT_ASSERT_EQUAL(expected, actual);

	// Note: e2 has to be a pointer, not a local Ternary, because it
	// gets changed at the top level (and so would die in its destructor)
	Ternary* e2 = new Ternary(opZfill, new Const(16), new Const(32),
            Location::memOf(
                new Binary(opPlus,
                    Location::regOf(16),
                    new Const(16))));
	// Try again but at top level
	res = e2->killFill();
	expected = "m[r16 + 16]";
	std::ostringstream ost2;
	res->print(ost2);
	actual = ost2.str();
	CPPUNIT_ASSERT_EQUAL(expected, actual);
	delete res;

}

/*==============================================================================
 * FUNCTION:        ExpTest::testAssociativity
 * OVERVIEW:        Test that a+K+b is the same as a+b+K when each is simplified
 *============================================================================*/
void ExpTest::testAssociativity() {
    
    // (r8 + m[m[r8 + 12] + -12]) + 12
    Binary e1(opPlus,
        new Binary(opPlus,
            Location::regOf(8),
            Location::memOf(
                new Binary(opPlus,
                    Location::memOf(
                        new Binary(opPlus,
                            Location::regOf(8),
                            new Const(12))),
                    new Const(-12)))),
        new Const(12));
    // (r8 + 12) + m[m[r8 + 12] + -12]
    Binary e2(opPlus,
        new Binary(opPlus,
            Location::regOf(8),
            new Const(12)),
        Location::memOf(
            new Binary(opPlus,
                Location::memOf(
                    new Binary(opPlus,
                        Location::regOf(8),
                        new Const(12))),
                new Const(-12))));
    // Note: at one stage, simplifyArith was part of simplify().
    // Now call implifyArith() explicitly only where needed
    Exp* p1 = e1.simplify()->simplifyArith();
    Exp* p2 = e2.simplify()->simplifyArith();
    std::ostringstream os1, os2;
    p1->print(os1); p2->print(os2);
    std::string expected(os1.str());
    std::string actual  (os2.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);
}

/*==============================================================================
 * FUNCTION:        ExpTest::testSubscriptVar
 * OVERVIEW:        Test Assign::subscriptVar and thereby
 *                    Exp::expSubscriptVar
 *============================================================================*/
void ExpTest::testSubscriptVar() {
    // m[r28 - 4] := r28 + r29
    Exp* left = Location::memOf(
            new Binary(opMinus,
                Location::regOf(28),
                new Const(4)));
    Assign* ae = new Assign(
        left->clone(),
        new Binary(opPlus,
            Location::regOf(28),
            Location::regOf(29)));

    Statement* s = dynamic_cast<Statement*>(ae);
    // Subtest 1: should do nothing
    Exp* r28 = Location::regOf(28);
    Statement* def1 = dynamic_cast<Statement*>(new Assign(r28->clone(),
      r28->clone()));
    def1->setNumber(12);
    def1->subscriptVar(left, def1);           // Should do nothing
    std::string expected1;
    expected1 = "   0 *32* m[r28 - 4] := r28 + r29";
    std::ostringstream actual1;
    actual1 << s;
    CPPUNIT_ASSERT_EQUAL(expected1, actual1.str());
    // m[r28 - 4]

    // Subtest 2: Ordinary substitution, on LHS and RHS
    s->subscriptVar(r28, def1);
    std::string expected2("   0 *32* m[r28{12} - 4] := r28{12} + r29");
    std::ostringstream actual2;
    actual2 << s;
    CPPUNIT_ASSERT_EQUAL(expected2, actual2.str());

    // Subtest 3: change to a different definition
    Statement* def3 = dynamic_cast<Statement*>(new Assign(Location::regOf(29),
        new Const(0)));
    def3->setNumber(99);
    s->subscriptVar(r28, def3);
    std::string expected3("   0 *32* m[r28{99} - 4] := r28{99} + r29");
    std::ostringstream actual3;
    actual3 << s;
    CPPUNIT_ASSERT_EQUAL(expected3, actual3.str());

    delete ae; delete def1; delete def3; delete r28; delete left;
}

/*==============================================================================
 * FUNCTION:        ExpTest::testTypeOf
 * OVERVIEW:        Test opTypeOf and TypeVal (type values)
 *============================================================================*/
void ExpTest::testTypeOf() {
    // Tr24{5} = Tr25{9}
    std::string expected1("Tr24{5} = Tr25{9}");
    Statement* s5 = new Assign;
    Statement* s9 = new Assign;
    s5->setNumber(5);
    s9->setNumber(9);
    Exp* e = new Binary(opEquals,
        new Unary(opTypeOf,
            new RefExp(Location::regOf(24), s5)),
        new Unary(opTypeOf,
            new RefExp(Location::regOf(25), s9)));
    std::ostringstream actual1;
    actual1 << e;
    CPPUNIT_ASSERT_EQUAL(expected1, actual1.str());

    // Tr24{5} = <float>
    std::string expected2("Tr24{5} = <float>");
    delete e;
    Type* t = new FloatType(32);
    e = new Binary(opEquals,
        new Unary(opTypeOf,
            new RefExp(Location::regOf(24), s5)),
        new TypeVal(t));
    std::ostringstream actual2;
    actual2 << e;
    CPPUNIT_ASSERT_EQUAL(expected2, actual2.str());
}
/*==============================================================================
 * FUNCTION:        ExpTest::testSetConscript
 * OVERVIEW:        Test setting and printing of constant "subscripts"
 *============================================================================*/
void ExpTest::testSetConscripts() {
    // m[1000] + 1000
    Exp* e = new Binary(opPlus,
        Location::memOf(
            new Const(1000), NULL),
        new Const(1000));
    e->setConscripts(0);
    std::string expected("m[1000\\1\\] + 1000\\2\\");
    std::ostringstream actual;
    actual << e;
    CPPUNIT_ASSERT_EQUAL(expected, actual.str());
}
