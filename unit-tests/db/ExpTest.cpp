/**
 * \file ExpTest.cpp
 * Provides the implementation for the ExpTest class, which
 * tests the Exp and derived classes
 */

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

#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Ternary.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/Visitor.h"

#include <map>


void ExpTest::initTestCase()
{
	m_99   = Const::get(99);
	m_rof2.reset(new Location(opRegOf, Const::get(2), nullptr));
}


void ExpTest::test99()
{
    QString actual;
	QTextStream ost(&actual);
	m_99->print(ost);
	QCOMPARE(actual, QString("99"));
}


void ExpTest::testFlt()
{
	QString actual;
	QTextStream ost(&actual);
    std::shared_ptr<Const> c = Const::get(3.14);
	c->print(ost);
	QCOMPARE(actual, QString("%1").arg(3.14));
}


void ExpTest::testRegOf2()
{
	QString actual;
    QTextStream ost(&actual);
	ost << m_rof2;
	QCOMPARE(actual, QString("r2"));
}


void ExpTest::testBinaries()
{
    QString actual;
	QTextStream ost(&actual);

	std::shared_ptr<Binary> b(new Binary(opPlus, m_99->clone(), m_rof2->clone()));
	b->print(ost);
	QCOMPARE(actual, QString("99 + r2"));

    actual = "";
	b.reset(new Binary(opMinus, m_99->clone(), m_rof2->clone()));
	b->print(ost);
	QCOMPARE(actual, QString("99 - r2"));

    actual = "";
	b.reset(new Binary(opMult, m_99->clone(), m_rof2->clone()));
	b->print(ost);
	QCOMPARE(actual, QString("99 * r2"));

    actual = "";
	b.reset(new Binary(opDiv, m_99->clone(), m_rof2->clone()));
	b->print(ost);
	QCOMPARE(actual, QString("99 / r2"));

    actual = "";
	b.reset(new Binary(opMults, m_99->clone(), m_rof2->clone()));
	b->print(ost);
	QCOMPARE(actual, QString("99 *! r2"));

    actual = "";
	b.reset(new Binary(opDivs, m_99->clone(), m_rof2->clone()));
	b->print(ost);
	QCOMPARE(actual, QString("99 /! r2"));

    actual = "";
	b.reset(new Binary(opMod, m_99->clone(), m_rof2->clone()));
	b->print(ost);
	QCOMPARE(actual, QString("99 % r2"));

    actual = "";
	b.reset(new Binary(opMods, m_99->clone(), m_rof2->clone()));
	b->print(ost);
	QCOMPARE(actual, QString("99 %! r2"));
}


void ExpTest::testUnaries()
{
    QString actual;
    QTextStream ost(&actual);

	SharedExp u = Unary::get(opNot, Terminal::get(opZF));
	u->print(ost);
	QCOMPARE(actual, QString("~%ZF"));

    actual = "";
	u = Unary::get(opLNot, Terminal::get(opCF));
	u->print(ost);
	QCOMPARE(actual, QString("L~%CF"));

    actual = "";
	u = Unary::get(opNeg, m_rof2->clone());
	u->print(ost);
	QCOMPARE(actual, QString("-r2"));
}


void ExpTest::testIsAfpTerm()
{
	SharedExp afp = Terminal::get(opAFP);
	SharedExp plus = Binary::get(opPlus, afp->clone(), Const::get(-99));
	SharedExp minus = Binary::get(opMinus, afp->clone(), m_99->clone());

	QVERIFY(afp->isAfpTerm());
	QVERIFY(plus->isAfpTerm());
	QVERIFY(minus->isAfpTerm());
	QVERIFY(!m_99->isAfpTerm());
	QVERIFY(!m_rof2->isAfpTerm());

    // Now with typed expressions
	SharedExp tafp = std::make_shared<TypedExp>(IntegerType::get(Address::getSourceBits()), afp->clone());
	// Unary tafp  (opTypedExp, afp.clone());
	SharedExp tplus  = Unary::get(opTypedExp, plus->clone());
	SharedExp tminus = Unary::get(opTypedExp, minus->clone());

	QVERIFY(tafp->isAfpTerm());
	QVERIFY(tplus->isAfpTerm());
	QVERIFY(tminus->isAfpTerm());
}


void ExpTest::testCompare1()
{
	QVERIFY(!(*m_99 == *m_rof2));
}


void ExpTest::testCompare2()
{
	Const nineNine(99);

	QVERIFY(*m_99 == nineNine);
}


void ExpTest::testCompare3()
{
	Const minus(-99);

	QVERIFY(!(*m_99 == minus));
}


void ExpTest::testCompare4()
{
	Location regOf2(opRegOf, Const::get(2), nullptr);

	QVERIFY(regOf2 == *m_rof2);
}


void ExpTest::testCompare5()
{
	Binary one(opMult, m_99->clone(), m_rof2->clone());
	Binary two(opMult, m_rof2->clone(), m_99->clone());

	QVERIFY(!(one == two));
}


void ExpTest::testCompare6()
{
	Binary one(opMult, m_99->clone(), m_rof2->clone());
	Binary two(opMult, m_99->clone(), m_rof2->clone());

	QVERIFY((one == two));
}


void ExpTest::testSearchReplace1()
{
	// Null test: should not replace. Also tests Ternary class
	bool change;

	SharedExp p = std::make_shared<Ternary>(opAt, m_rof2->clone(), Const::get(15), Const::get(8));
	p = p->searchReplace(*m_99, m_rof2, change);

    QString actual;
    QTextStream ost(&actual);

	std::string        expected("r2@15:8");
	p->print(ost);
    QCOMPARE(actual, QString("r2@15:8"));

    Ternary t2(*std::dynamic_pointer_cast<Ternary>(p));
	QVERIFY(*p == t2);
	p = p->searchReplaceAll(*m_99, m_rof2, change);
    QVERIFY(*p == t2);
}


void ExpTest::testSearchReplace2()
{
	// Whole expression replacements
	bool  change;
	SharedExp p1 = Const::get(55);
	SharedExp c2 = Const::get(1234);

	p1 = p1->searchReplace(*p1, c2, change);
	QVERIFY(*p1 == *c2);
	QVERIFY(p1 != c2); // Pointers should not be same

    p1 = p1->searchReplace(*c2, m_rof2, change);
	QVERIFY(*p1 == *m_rof2);
}


void ExpTest::testSearchReplace3()
{
	// Subexpression replacement
	bool  change;
	SharedExp two = Const::get(2);
	SharedExp three = Const::get(3);
	SharedExp p = m_rof2->clone();

	p = p->searchReplaceAll(*two, three, change);

    QString actual;
    QTextStream ost(&actual);
	p->print(ost);
	QCOMPARE(actual, QString("r3"));
}


void ExpTest::testSearchReplace4()
{
	// Subexpression replacement with different subexpression form
	bool  change;
	Const two(2);
	SharedExp p = m_rof2->clone();

	// Note recursion. OK to use the all function, since it does the search
	// first.
	p = p->searchReplaceAll(two, m_rof2, change);

    QString actual;
    QTextStream ost(&actual);
    p->print(ost);
    QCOMPARE(actual, QString("r[r2]"));
}


void ExpTest::testSearch1()
{
	SharedExp two = Const::get(2);
	SharedExp result;

	// Search inside r2 for const 2
	QVERIFY(m_rof2->search(*two, result));
	QVERIFY(*result == *two);

	// Test for false positives
	QVERIFY(!m_99->search(*two, result));
	QVERIFY(!m_rof2->search(*m_99, result));

	// Note: opDiv's enum has value 3
	SharedExp e = Binary::get(opMult, m_rof2->clone(), m_99->clone()); // r2 / 99
	Const  three(3);
	QVERIFY(!e->search(three, result));
}


void ExpTest::testSearch2()
{
	// Search using wildcards
	SharedExp e = Binary::get(opDivs, m_rof2->clone(), m_99->clone());      // r2 /! 99
	SharedExp result;
	SharedExp search = Location::get(opRegOf, Terminal::get(opWild), nullptr); // r[?]

	QVERIFY(e->search(*search, result));
	QVERIFY(*result == *m_rof2); // Should be r2

	QVERIFY(!e->search(*Const::get(3), result));
	QVERIFY(e->search(*m_99, result));
}


void ExpTest::testSearch3()
{
	// A more complex expression:
	// (r2 * 99) + (m[1000] * 4)
	SharedExp result;
	SharedExp e = Binary::get(opPlus,
                              Binary::get(opMult, m_rof2->clone(), m_99->clone()),
                              Binary::get(opMult, Location::memOf(Const::get(1000)), Const::get(4)));

	SharedExp mem1000 = Location::get(opMemOf, Const::get(1000), nullptr);
	SharedExp prod = Binary::get(opMult, m_rof2->clone(), m_99->clone());

	QVERIFY(e->search(*Const::get(4), result));
	QVERIFY(e->search(*mem1000, result));
	QVERIFY(e->search(*prod, result));
	QVERIFY(e->search(*m_99, result));
	QVERIFY(!e->search(*Const::get(3), result));
}


void ExpTest::testSearchAll()
{
	SharedExp search = Location::get(opRegOf, Terminal::get(opWild), nullptr); // r[?]

	// A more complex expression:
	// (r2 * 99) + (r8 * 4)
	std::list<SharedExp> result;
	SharedExp e = Binary::get(opPlus,
                              Binary::get(opMult, m_rof2->clone(), m_99->clone()),
                              Binary::get(opMult, Location::regOf(8), Const::get(4)));
	QVERIFY(e->searchAll(*search, result));
	QVERIFY(result.size() == 2);
	QVERIFY(*result.front() == *m_rof2);
	Location rof8(opRegOf, Const::get(8), nullptr);
	QVERIFY(*result.back() == rof8);
}


void ExpTest::testAccumulate()
{
	SharedExp rof2 = Location::get(opRegOf, Const::get(2), nullptr);
	SharedExp nineNine = Const::get(99);

	// Zero terms
	std::list<SharedExp> le;
	SharedExp res = Exp::accumulate(le);
	Const            zero(0);
	QVERIFY(*res == zero);

	// One term
	le.push_back(rof2);
	res = Exp::accumulate(le);
	QVERIFY(*res == *rof2);

	// Two terms
	SharedExp nn = nineNine->clone();
	le.push_back(nn);
	res = Exp::accumulate(le);
	Binary expected2(opPlus, rof2->clone(), nineNine->clone());
	QVERIFY(*res == expected2);

	// Three terms, one repeated
	le.push_back(nineNine);
	res = Exp::accumulate(le);
	Binary expected3(opPlus, rof2->clone(), Binary::get(opPlus, nineNine->clone(), nineNine->clone()));
	QVERIFY(*res == expected3);

	// Four terms, one repeated
	le.push_back(Terminal::get(opAFP));
	res = Exp::accumulate(le);
	Binary expected4(opPlus, rof2->clone(),
					 Binary::get(opPlus, nineNine->clone(), Binary::get(opPlus, nineNine->clone(), Terminal::get(opAFP))));
	QVERIFY(*res == expected4);
}


void ExpTest::testPartitionTerms()
{
	// afp + 108 + n - (afp + 92)
	Binary e(opMinus, Binary::get(opPlus, Binary::get(opPlus, Terminal::get(opAFP), Const::get(108)),
								 Unary::get(opVar, Const::get("n"))),
			 Binary::get(opPlus, Terminal::get(opAFP), Const::get(92)));

	std::list<SharedExp> positives, negatives;
	std::vector<int> integers;

    e.partitionTerms(positives, negatives, integers, false);
	SharedExp res = Exp::accumulate(positives);
	Binary expected1(opPlus, Terminal::get(opAFP), Unary::get(opVar, Const::get("n")));
	QVERIFY(*res == expected1);

	res = Exp::accumulate(negatives);
	Terminal expected2(opAFP);
	QVERIFY(*res == expected2);

	QCOMPARE(integers.size(), (size_t)2);
	QCOMPARE(integers.front(), 108);
	QCOMPARE(integers.back(), -92);
}


void ExpTest::testSimplifyArith()
{
	// afp + 108 + n - (afp + 92)
	SharedExp e = Binary::get(opMinus, Binary::get(opPlus, Binary::get(opPlus, Terminal::get(opAFP), Const::get(108)),
											Unary::get(opVar, Const::get("n"))),
						Binary::get(opPlus, Terminal::get(opAFP), Const::get(92)));
	e = e->simplifyArith();

	QString actual;
    QTextStream ost(&actual);
    e->print(ost);
    QCOMPARE(actual, QString("v[n] + 16"));

	// m[(r28 + -4) + 8]
	SharedExp mm = Location::memOf(Binary::get(opPlus, Binary::get(opPlus, Location::regOf(28), Const::get(-4)), Const::get(8)));
	mm = mm->simplifyArith();


   	actual = "";
	mm->print(ost);
	QCOMPARE(actual, QString("m[r28 + 4]"));

	// r24 + m[(r28 - 4) - 4]
	mm = Binary::get(
		opPlus, Location::regOf(24),
		Location::memOf(Binary::get(opMinus, Binary::get(opMinus, Location::regOf(28), Const::get(4)), Const::get(4))));
	mm = mm->simplifyArith();


	actual = "";
	mm->print(ost);
    QCOMPARE(actual, QString("r24 + m[r28 - 8]"));
}


void ExpTest::testSimplifyUnary()
{
	// Unaries with integer constant argument
	SharedExp u = Unary::get(opNeg, Const::get(55));

	u = u->simplify();
	Const minus55(-55);
	QVERIFY(*u == minus55);

	u = Unary::get(opNot, Const::get(0x55AA));
	u = u->simplify();
	Const exp(0xFFFFAA55);
	QVERIFY(*u == exp);

	u = Unary::get(opLNot, Const::get(55));
	u = u->simplify();
	Const zero(0);
	QVERIFY(*u == zero);

	u = Unary::get(opLNot, zero.clone());
	u = u->simplify();
	Const one(1);
	QVERIFY(*u == one);

	// Null test
	u = Unary::get(opNeg, Unary::get(opVar, Const::get("abc")));
	Unary abc(opNeg, Unary::get(opVar, Const::get("abc")));
	QVERIFY(*u == abc);
}


void ExpTest::testSimplifyBinary()
{
	// Add integer consts
	SharedExp b = Binary::get(opPlus, Const::get(2), Const::get(3));
	b = b->simplify();
	Const five(5);
	QVERIFY(*b == five);

	// Multiply integer consts
	b = Binary::get(opMult, Const::get(2), Const::get(3));
	b = b->simplify();
	Const six(6);
	QVERIFY(*b == six);

	// Shift left two integer constants
	b = Binary::get(opShiftL, Const::get(2), Const::get(3));
	b = b->simplify();
	Const sixteen(16);
	QVERIFY(*b == sixteen);

	// Shift right arithmetic two integer contants
	b = Binary::get(opShiftRA, Const::get(-144), Const::get(3));
	b = b->simplify();
	Const minus18(-18);
	QVERIFY(*b == minus18);

	// Bitwise XOR
	b = Binary::get(opBitXor, Const::get(0x55), Const::get(0xF));
	b = b->simplify();
	Const fiveA(0x5A);
	QVERIFY(*b == fiveA);

	// Xor with self
	b = Binary::get(opBitXor, m_rof2->clone(), m_rof2->clone());
	b = b->simplify();
	Const zero(0);
	QVERIFY(*b == zero);

	// Test commute
	// 77 * r2
	b = Binary::get(opMults, Const::get(77), m_rof2->clone());
	b = b->simplify();

	// r2 * 77
	Binary exp(opMults, m_rof2->clone(), Const::get(77));
	QVERIFY(*b == exp);

	// x*1
    std::shared_ptr<Const> subExp = std::dynamic_pointer_cast<Const>(b->getSubExp2());
	QVERIFY(subExp != nullptr);
    subExp->setInt(1);
	b = b->simplify();
	QVERIFY(*b == *m_rof2);

	// 0 | r2
	b = Binary::get(opBitOr, Const::get(0), m_rof2->clone());
	b = b->simplify();
	QVERIFY(*b == *m_rof2);

	// Left shift by const
	b = Binary::get(opShiftL, m_rof2->clone(), Const::get(0));
	b = b->simplify();
	QVERIFY(*b == *m_rof2);

	b = Binary::get(opShiftL, m_rof2->clone(), Const::get(2));
	b = b->simplify();
	Binary expb1(opMult, m_rof2->clone(), Const::get(4));
	QVERIFY(*b == expb1);

	// Add negative constant
	// r2 + -99
	b = Binary::get(opPlus, m_rof2->clone(), Const::get(-99));
	// r2 - 99
	SharedExp expb2 = Binary::get(opMinus, m_rof2->clone(), Const::get(99));

// As of June 2003, I've decided to go the old way. esp + -4 is just
// too ugly, and all the code has to cope with pluses and minuses anyway,
// just in case
	b = b->simplify();
	QVERIFY(*b == *expb2);

    QString actual;
    QTextStream ost(&actual);
	SharedExp e = Binary::get(opBitOr,
                              Binary::get(opMinus,
                                          Binary::get(opPlus, Const::get(0), Unary::get(opVar, Const::get("a"))),
                                          Const::get(0)),
                              Const::get(0));
	e->print(ost);
    QCOMPARE(actual, QString("((0 + v[a]) - 0) | 0"));

	// The above should simplify to just "v[a]"
	e = e->simplify();
	Unary a(opVar, Const::get("a"));
	actual = "";
    e->print(ost);
    QCOMPARE(actual, QString("v[a]"));

	// r27 := m[r29 + -4]
	std::shared_ptr<Assign> as(new Assign(Location::regOf(27), Location::memOf(Binary::get(opPlus, Location::regOf(29), Const::get(-4)))));
	as->simplify();
    actual = "";
	as->print(ost);
    QCOMPARE(actual, QString("   0 *v* r27 := m[r29 - 4]"));

	// (false and true) or (Tr24 = <int>)
	e = Binary::get(opOr, Binary::get(opAnd, Terminal::get(opFalse), Terminal::get(opTrue)),
				   Binary::get(opEquals, Unary::get(opTypeOf, Location::regOf(24)), TypeVal::get(IntegerType::get(32, 1))));
	e = e->simplify();

    actual = "";
	e->print(ost);
    QCOMPARE(actual, QString("T[r24] = <int>"));
}


void ExpTest::testSimplifyAddr()
{
	// a[m[1000]] - a[m[r2]{64}]@0:15
	SharedExp e = Binary::get(opMinus,
                              Unary::get(opAddrOf, Location::memOf(Const::get(1000))),
                              Ternary::get(opAt,
                                           Unary::get(opAddrOf,
                                                      Binary::get(opSize,
                                                                  Const::get(64),
                                                                  Location::memOf(Location::regOf(2)))),
                                           Const::get(0),
                                           Const::get(15)));

	e = e->simplifyAddr();

    QString actual;
    QTextStream ost(&actual);
    e->print(ost);
    QCOMPARE(actual, QString("1000 - (r2@0:15)"));

    actual = "";
	// Now test at top level
	e = Unary::get(opAddrOf, Location::memOf(Const::get(1000)));
	e = e->simplifyAddr();
	e->print(ost);
    QCOMPARE(actual, QString("1000"));
}


void ExpTest::testSimpConstr()
{
	// After
	//     (T[local1{16}] = <int>) or (Tlocal1{16} = <alpha2*>)
	// gets substituted to
	//     (<char*> = <int>) or (<char*> = <alpha2*>)
	// it should simplify to
	//    <char*> = <alpha2*>
	SharedExp e = Binary::get(opOr,
                              Binary::get(opEquals,
                                          TypeVal::get(PointerType::get(CharType::get())),
                                          TypeVal::get(IntegerType::get(Address::getSourceBits()))),
                              Binary::get(opEquals,
                                          TypeVal::get(PointerType::get(CharType::get())),
                                          TypeVal::get(PointerType::newPtrAlpha())));
	e = e->simplifyConstraint();
    QString actual;
    QTextStream ost(&actual);
    e->print(ost);
    QCOMPARE(actual, QString("<char *> = <alpha0 *>"));

	// Similarly,
	//     <char*> = <alpha0*>) and (T[134517848\1\] = <alpha0*>
	// becomes after alpha substitution
	//     (<char*> = <char*>) and (T[134517848\1\] = <char*>)
	// which should simplify to
	//     T[134517848\1\] = <char*>
	e = Binary::get(opAnd,
                    Binary::get(opEquals,
                                TypeVal::get(PointerType::get(CharType::get())),
						        TypeVal::get(PointerType::get(CharType::get()))),
                    Binary::get(opEquals,
                                Unary::get(opTypeOf, Const::get(0x123456)),
                                TypeVal::get(PointerType::get(CharType::get()))));
	e = e->simplifyConstraint();

    actual = "";
	e->print(ost);
    QCOMPARE(actual, QString("T[0x123456] = <char *>"));
}


void ExpTest::testLess()
{
	// Simple constants
	Const two(2), three(3), mThree(-3), twoPointTwo(2.2), threePointThree(3.3);
	Const mThreePointThree(-3.3);

	QVERIFY(two < three);
	QVERIFY(mThree < two);
	QVERIFY(twoPointTwo < threePointThree);
	QVERIFY(mThreePointThree < twoPointTwo);
	// Terminal
	Terminal afp(opAFP), agp(opAGP);

	if (opAFP < opAGP) {
		QVERIFY(afp < agp);
	}
	else {
		QVERIFY(agp < afp);
	}

	// Unary
	Unary negTwo(opNeg, Const::get(2)), negThree(opNeg, Const::get(3));
	// Note that the ordering is not arithmetic!
	QVERIFY(negTwo < negThree);
	// Binary
	Binary twoByThr(opMult, Const::get(2), Const::get(3));
	Binary twoByFou(opMult, Const::get(2), Const::get(4));
	Binary thrByThr(opMult, Const::get(3), Const::get(3));
	QVERIFY(twoByThr < twoByFou);
	QVERIFY(twoByThr < thrByThr);
	// Ternary
	Ternary twoAtThrToFou(opAt, Const::get(2), Const::get(3), Const::get(4));
	Ternary twoAtThrToFiv(opAt, Const::get(2), Const::get(3), Const::get(5));
	QVERIFY(twoAtThrToFou < twoAtThrToFiv);
	// TypedExp later
}


void ExpTest::testMapOfExp()
{
	std::map<SharedExp, int, lessExpStar> m;
	m[m_rof2] = 200;
	m[m_99]   = 99;
	SharedExp e = Binary::get(opPlus,
                              Const::get(0),
                              Binary::get(opMinus,
                                          Binary::get(opMult, Const::get(2), Const::get(3)),
                                          Binary::get(opMult, Const::get(4), Const::get(5))));
	m[e] = -100;
	SharedExp rof2 = Location::get(opRegOf, Const::get(2), nullptr);
	m[rof2] = 2; // Should overwrite

	QCOMPARE(m.size(), (size_t)3);
	int i = m[m_rof2];
	QCOMPARE(i, 2);
	i = m[rof2];
	QCOMPARE(i, 2);
	i = m[m_99];
	QCOMPARE(i, 99);
	i = m[e];
	QCOMPARE(i, -100);
}


void ExpTest::testList()
{
    QString actual;
    QTextStream ost(&actual);

	// Empty list
	SharedExp e =  Binary::get(opList, Terminal::get(opNil), Terminal::get(opNil));
	ost << e;
    QCOMPARE(actual, QString(""));

	// 1 element list
	e = Binary::get(opList, Location::get(opParam, Const::get("a"), nullptr), Terminal::get(opNil));
    actual = "";
	ost << e;
    QCOMPARE(actual, QString("a"));

	// 2 element list
	e = Binary::get(opList,
                    Location::get(opParam, Const::get("a"), nullptr),
					Binary::get(opList, Location::get(opParam, Const::get("b"), nullptr), Terminal::get(opNil)));
	actual = "";
    ost << e;
    QCOMPARE(actual, QString("a, b"));

	// 3 element list
	e = Binary::get(opList,
                    Location::get(opParam, Const::get("a"), nullptr),
                    Binary::get(opList,
                                Location::get(opParam, Const::get("b"), nullptr),
                                Binary::get(opList, Location::get(opParam, Const::get("c"), nullptr), Terminal::get(opNil))));
    actual = "";
    ost << e;
    QCOMPARE(actual, QString("a, b, c"));

	// 4 element list
	e = Binary::get(opList,
                    Location::get(opParam, Const::get("a"), nullptr),
					Binary::get(opList,
                                Location::get(opParam, Const::get("b"), nullptr),
                                Binary::get(opList,
                                            Location::get(opParam, Const::get("c"), nullptr),
										    Binary::get(opList,
                                                        Location::get(opParam, Const::get("d"), nullptr),
													    Terminal::get(opNil)))));
	actual = "";
    ost << e;
    QCOMPARE(actual, QString("a, b, c, d"));
}


void ExpTest::testParen()
{
	Assign a(Location::regOf(Location::get(opParam, Const::get("rd"), nullptr)),
                             Binary::get(opBitAnd, Location::regOf(Location::get(opParam, Const::get("rs1"), nullptr)),
                                         Binary::get(opMinus,
                                                     Binary::get(opMinus,
                                                                 Const::get(0),
                                                                 Location::get(opParam, Const::get("reg_or_imm"), nullptr)),
                                                     Const::get(1))));

    QString actual;
    QTextStream ost(&actual);
    a.print(ost);
    QCOMPARE(actual, QString("   0 *v* r[rd] := r[rs1] & ((0 - reg_or_imm) - 1)"));
}


void ExpTest::testFixSuccessor()
{
	// Trivial test (should not affect)
	SharedExp  b = Binary::get(opMinus,
                               m_99->clone(),
                               m_rof2->clone());

    QString actual;
    QTextStream ost(&actual);

    SharedExp e = b->fixSuccessor();
    e->print(ost);
    QCOMPARE(actual, QString("99 - r2"));

    actual = "";
	SharedExp u = Unary::get(opSuccessor, Location::regOf(2));
	e = u->fixSuccessor();
	e->print(ost);
    QCOMPARE(actual, QString("r3"));
}


void ExpTest::testKillFill()
{
	// r18 + sgnex(16,32,m[r16 + 16])
	SharedExp e = Binary::get(opPlus,
                              Location::regOf(18),
                              Ternary::get(opSgnEx, Const::get(16), Const::get(32),
                                           Location::memOf(Binary::get(opPlus, Location::regOf(16), Const::get(16)))));
	SharedExp res = e->killFill();

    QString actual;
    QTextStream ost(&actual);
	res->print(ost);
    QCOMPARE(actual, QString("r18 + m[r16 + 16]"));

	// Note: e2 has to be a pointer, not a local Ternary, because it
	// gets changed at the top level (and so would die in its destructor)
	SharedExp e2 = Ternary::get(opZfill, Const::get(16), Const::get(32),
                                Location::memOf(Binary::get(opPlus, Location::regOf(16), Const::get(16))));

    // Try again but at top level
    actual = "";
	res = e2->killFill();
	res->print(ost);
    QCOMPARE(actual, QString("m[r16 + 16]"));
}


void ExpTest::testAssociativity()
{
	// (r8 + m[m[r8 + 12] + -12]) + 12
	SharedExp e1 = Binary::get(opPlus,
              Binary::get(opPlus,
                          Location::regOf(8),
                          Location::memOf(Binary::get(opPlus,
                                                      Location::memOf(Binary::get(opPlus, Location::regOf(8), Const::get(12))),
                                                      Const::get(-12)))),
              Const::get(12));

	// (r8 + 12) + m[m[r8 + 12] + -12]
	SharedExp e2 = Binary::get(opPlus,
              Binary::get(opPlus, Location::regOf(8), Const::get(12)),
			  Location::memOf(Binary::get(opPlus,
                                          Location::memOf(Binary::get(opPlus, Location::regOf(8), Const::get(12))),
                                          Const::get(-12))));

	// Note: at one stage, simplifyArith was part of simplify().
	// Now call implifyArith() explicitly only where needed
	SharedExp p1 = e1->simplify()->simplifyArith();
	SharedExp p2 = e2->simplify()->simplifyArith();

    QString expected, actual;
    QTextStream ost(&expected);
    p1->print(ost);
    ost.setString(&actual);
    p2->print(ost);
    QCOMPARE(actual, expected);
}


void ExpTest::testSubscriptVar()
{
	// m[r28 - 4] := r28 + r29
	SharedExp left = Location::memOf(Binary::get(opMinus, Location::regOf(28), Const::get(4)));
	Assign *ae   = new Assign(left->clone(), Binary::get(opPlus, Location::regOf(28), Location::regOf(29)));

    // Subtest 1: should do nothing
	SharedExp r28  = Location::regOf(28);
	   Statement *def1 = new Assign(r28->clone(), r28->clone());

	def1->setNumber(12);
	def1->subscriptVar(left, def1); // Should do nothing

    QString actual;
    QTextStream ost(&actual);
    ost << ae;
    QCOMPARE(actual, QString("   0 *v* m[r28 - 4] := r28 + r29"));

	// m[r28 - 4]

	// Subtest 2: Ordinary substitution, on LHS and RHS
	actual = "";
    ae->subscriptVar(r28, def1);
    ost << ae;
    QCOMPARE(actual, QString("   0 *v* m[r28{12} - 4] := r28{12} + r29"));


	// Subtest 3: change to a different definition
	// 99: r28 := 0
	// Note: behaviour has changed. Now, we don't allow re-renaming, so it should stay the same
	actual = "";
    Statement *def3 = new Assign(Location::regOf(28), Const::get(0));
	def3->setNumber(99);
	ae->subscriptVar(r28, def3);
    ost << ae;
    QCOMPARE(actual, QString("   0 *v* m[r28{12} - 4] := r28{12} + r29"));
}


void ExpTest::testTypeOf()
{
	// T[r24{5}] = T[r25{9}]
	   Statement *s5 = new Assign;
	   Statement *s9 = new Assign;
	s5->setNumber(5);
	s9->setNumber(9);
	SharedExp e = Binary::get(opEquals,
                              Unary::get(opTypeOf, RefExp::get(Location::regOf(24), s5)),
                              Unary::get(opTypeOf, RefExp::get(Location::regOf(25), s9)));

    QString actual;
    QTextStream ost(&actual);
    ost << e;
    QCOMPARE(actual, QString("T[r24{5}] = T[r25{9}]"));

	// T[r24{5}] = <float>
    actual = "";
	SharedType t = FloatType::get(32);
	e = Binary::get(opEquals, Unary::get(opTypeOf, RefExp::get(Location::regOf(24), s5)), TypeVal::get(t));
    ost << e;
    QCOMPARE(actual, QString("T[r24{5}] = <float>"));
}


void ExpTest::testSetConscripts()
{
    QString actual;
    QTextStream ost(&actual);

	// m[1000] + 1000
	SharedExp e = Binary::get(opPlus, Location::memOf(Const::get(1000), nullptr), Const::get(1000));
    e->setConscripts(0, false);
	ost << e;
    QCOMPARE(actual, QString("m[1000\\1\\] + 1000\\2\\"));

	// Clear them
	actual = "";
    e->setConscripts(0, true);
	ost << e;
    QCOMPARE(actual, QString("m[1000] + 1000"));

	// m[r28 + 1000]
	actual = "";
    e = Location::memOf(Binary::get(opPlus, Location::regOf(28), Const::get(1000)));
	e->setConscripts(0, false);
	ost << e;
    QCOMPARE(actual, QString("m[r28 + 1000\\1\\]"));

	// Clear
	actual = "";
    e->setConscripts(0, true);
    ost << e;
    QCOMPARE(actual, QString("m[r28 + 1000]"));
}


void ExpTest::testAddUsedLocs()
{
	// Null case
	SharedExp e = Terminal::get(opNil);
	LocationSet l;

	e->addUsedLocs(l);
	QVERIFY(l.size() == 0);

	// Const: "foo"
	e = Const::get("foo");
	e->addUsedLocs(l);
	QVERIFY(l.size() == 0);

	// Simple terminal: %pc
	e = Terminal::get(opPC);
	e->addUsedLocs(l);

    QString actual;
    QTextStream ost(&actual);
    l.print(ost);
    QCOMPARE(actual, QString("%pc"));

	// Simple location: r28
	l.clear();
	actual = "";
    e = Location::regOf(28);
	e->addUsedLocs(l);
    l.print(ost);
    QCOMPARE(actual, QString("r28"));

	// Memory location: m[r28-4]
	l.clear();
    actual = "";
	e = Location::memOf(Binary::get(opMinus, Location::regOf(28), Const::get(4)));
	e->addUsedLocs(l);
    l.print(ost);
    QCOMPARE(actual, QString("r28,\tm[r28 - 4]"));

	// Unary: a[m[r28-4]]
	l.clear();
    actual = "";
	e = Unary::get(opAddrOf, e);
	e->addUsedLocs(l);
	l.print(ost);
	QCOMPARE(actual, QString("r28,\tm[r28 - 4]"));

	// Binary: r24 + r25
	l.clear();
    actual = "";
	e = Binary::get(opPlus, Location::regOf(24), Location::regOf(25));
	e->addUsedLocs(l);
	l.print(ost);
    QCOMPARE(actual, QString("r24,\tr25"));

	// Ternary: r24@r25:r26
	l.clear();
    actual = "";
	e = Ternary::get(opAt, Location::regOf(24), Location::regOf(25), Location::regOf(26));
	e->addUsedLocs(l);
	l.print(ost);
	QCOMPARE(actual, QString("r24,\tr25,\tr26"));

	// Simple RefExp: r28{2}
	l.clear();
    actual = "";
	Assign a(e, e);
	a.setNumber(2);
	e = RefExp::get(Location::regOf(28), &a);
	e->addUsedLocs(l);
	l.print(ost);
    QCOMPARE(actual, QString("r28{2}"));

	// RefExp: m[r28{2} - 4]{3}
	Assign t(e, e);
    actual = "";
	t.setNumber(3);
	e = RefExp::get(Location::memOf(Binary::get(opMinus, RefExp::get(Location::regOf(28), &a), Const::get(4))), &t);
	e->addUsedLocs(l);
	l.print(ost);
    QCOMPARE(actual, QString("r28{2},\tm[r28{2} - 4]{3}"));
}


void ExpTest::testSubscriptVars()
{
    QString actual;
    QTextStream ost(&actual);

	// Null case: %pc
	Assign s9(Terminal::get(opNil), Terminal::get(opNil));

	s9.setNumber(9);
	SharedExp search = Location::regOf(28);
	SharedExp e      = Terminal::get(opPC);
	e = e->expSubscriptVar(search, &s9);
	ost << e;
    QCOMPARE(actual, QString("%pc"));

	// Simple case: r28
	actual = "";
    e = search->clone();
	e = e->expSubscriptVar(search, &s9);
	ost << e;
    QCOMPARE(actual, QString("r28{9}"));


	// A temp
    actual = "";
	e = Location::tempOf(Const::get("tmp1"));
	e = e->expSubscriptVar(e->clone(), &s9);
	ost << e;
    QCOMPARE(actual, QString("tmp1{9}"));

	// m[r28] + r28
    actual = "";
	e = Binary::get(opPlus, Location::memOf(Location::regOf(28)), Location::regOf(28));
	e = e->expSubscriptVar(search, &s9);
	ost << e;
    QCOMPARE(actual, QString("m[r28{9}] + r28{9}"));

	// RefExp: r28{7} -> r28{9}
	// Again, changed behaviour: don't resubscript any location
	actual = "";
    Assign s7(Terminal::get(opNil), Terminal::get(opNil));
	s7.setNumber(7);
	e = RefExp::get(search->clone(), &s7);
	e = e->expSubscriptVar(search, &s9);
	ost << e;
    QCOMPARE(actual, QString("r28{7}"));

	// m[r28{7} + 4]{8}
	actual = "";
    Assign s8(Terminal::get(opNil), Terminal::get(opNil));
	s8.setNumber(8);
	e = RefExp::get(Location::memOf(Binary::get(opPlus, RefExp::get(Location::regOf(28), &s7), Const::get(4))), &s8);
	e = e->expSubscriptVar(search, &s9);
	ost << e;
    QCOMPARE(actual, QString("m[r28{7} + 4]{8}"));

	// r24{7} with r24{7} and 0: should not change: RefExps should not compare
	// at the top level, only with their base expression (here r24, not r24{7})
	actual = "";
    e = RefExp::get(Location::regOf(24), &s7);
	e = e->expSubscriptVar(e->clone(), nullptr);
	ost << e;
	QCOMPARE(actual, QString("r24{7}"));
}


void ExpTest::testVisitors()
{
	Assign s7(Terminal::get(opNil), Terminal::get(opNil));

	// m[SETTFLAGS(m[1000], r8)]{7}
	s7.setNumber(7);
	FlagsFinder ff;
	SharedExp e1 = RefExp::get(Location::memOf(Binary::get(opFlagCall,
                                                           Const::get("SETFFLAGS"),
                                                           Binary::get(opList,
                                                                       Location::memOf( // A bare memof
                                                                       Const::get(0x1000)),
                                                                       Binary::get(opList, Location::regOf(8), Terminal::get(opNil))))),
                               &s7);

	// m[0x2000]
	SharedExp e2 = Location::memOf(Const::get(0x2000));

	// r1+m[1000]{7}*4
	SharedExp e3 = Binary::get(opPlus,
                               Location::regOf(1),
                               Binary::get(opMult, RefExp::get(Location::memOf(Const::get(1000)), &s7), Const::get(4)));

    QVERIFY(e1->containsFlags());
    QVERIFY(!e2->containsFlags());
    QVERIFY(!e3->containsFlags());
}


QTEST_MAIN(ExpTest)
