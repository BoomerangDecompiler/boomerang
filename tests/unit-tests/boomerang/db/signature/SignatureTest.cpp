#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SignatureTest.h"


#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/db/statements/Assign.h"


void SignatureTest::testAddReturn()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testGetReturnExp()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testGetReturnType()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testGetNumReturns()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testFindReturn()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testAddParameter()
{
    Signature sig("test");

    sig.addParameter(Location::regOf(25));
    QCOMPARE(sig.getNumParams(), 1);
    QVERIFY(*sig.getParamType(0) == *VoidType::get());

    sig.addParameter(Location::regOf(25), IntegerType::get(32, 1));
    QCOMPARE(sig.getNumParams(), 2);
    QVERIFY(*sig.getParamType(1) == *IntegerType::get(32, 1));
}


void SignatureTest::testRemoveParameter()
{
    Signature sig("test");

    // verify it does not crash
    sig.removeParameter(nullptr);
    QCOMPARE(sig.getNumParams(), 0);

    sig.removeParameter(0);
    QCOMPARE(sig.getNumParams(), 0);

    sig.addParameter(Location::regOf(25));
    sig.removeParameter(0);
    QCOMPARE(sig.getNumParams(), 0);

    sig.addParameter(Location::regOf(25), IntegerType::get(32, 1));
    sig.addParameter(Location::regOf(26));
    sig.removeParameter(Location::regOf(25));
    QCOMPARE(sig.getNumParams(), 1);
    QVERIFY(*sig.getParamExp(0) == *Location::regOf(26));
}


void SignatureTest::testSetNumParams()
{
    Signature sig("test");

    sig.setNumParams(0);
    QCOMPARE(sig.getNumParams(), 0);

    sig.addParameter("foo", Location::regOf(25));
    sig.addParameter("bar", Location::regOf(24));

    sig.setNumParams(1);
    QCOMPARE(sig.getNumParams(), 1);
}


void SignatureTest::testGetParamName()
{
    Signature sig("test");

    sig.addParameter("testParam", Location::regOf(25), VoidType::get());
    QCOMPARE(sig.getParamName(0), QString("testParam"));
}


void SignatureTest::testGetParamExp()
{
    Signature sig("test");

    sig.addParameter(Location::regOf(25));
    QVERIFY(*sig.getParamExp(0) == *Location::regOf(25));
}


void SignatureTest::testGetParamType()
{
    Signature sig("test");

    QVERIFY(sig.getParamType(0) == nullptr);

    sig.addParameter(Location::regOf(25), IntegerType::get(32, 1));
    QVERIFY(*sig.getParamType(0) == *IntegerType::get(32, 1));
}


void SignatureTest::testGetParamBoundMax()
{
    Signature sig("test");
    QCOMPARE(sig.getParamBoundMax(0), QString());

    sig.addParameter(Location::regOf(25), IntegerType::get(32, 1));
    QCOMPARE(sig.getParamBoundMax(0), QString());

    sig.addParameter("testParam", Location::regOf(26), IntegerType::get(32, 1), "r25");
    QCOMPARE(sig.getParamBoundMax(1), QString("r25"));
}


void SignatureTest::testSetParamType()
{
    Signature sig("test");

    sig.addParameter("testParam", Location::regOf(25), IntegerType::get(32, 1));
    sig.setParamType(0, VoidType::get());
    QVERIFY(*sig.getParamType(0) == *VoidType::get());

    sig.setParamType("testParam", IntegerType::get(32, 0));
    QVERIFY(*sig.getParamType(0) == *IntegerType::get(32, 0));
}


void SignatureTest::testFindParam()
{
    Signature sig("test");
    QCOMPARE(sig.findParam(Location::regOf(24)), -1);
    QCOMPARE(sig.findParam("testParam"), -1);

    sig.addParameter("testParam", Location::regOf(25), IntegerType::get(32, 1));
    QCOMPARE(sig.findParam(Location::regOf(25)), 0);
    QCOMPARE(sig.findParam(Location::regOf(24)), -1);
    QCOMPARE(sig.findParam("testParam"), 0);
    QCOMPARE(sig.findParam("Foo"), -1);
}


void SignatureTest::testRenameParam()
{
    Signature sig("test");
    QVERIFY(!sig.renameParam("", ""));

    sig.addParameter("testParam", Location::regOf(25));
    QVERIFY(sig.renameParam("testParam", ""));
    QCOMPARE(sig.getParamName(0), QString());

    QVERIFY(sig.renameParam("", ""));
    QVERIFY(sig.renameParam("", "foo"));
    QVERIFY(!sig.renameParam("bar", "baz"));
    QCOMPARE(sig.getParamName(0), QString("foo"));
}


void SignatureTest::testGetArgumentExp()
{
    Signature sig("test");

    sig.addParameter(Location::regOf(25));
    QVERIFY(*sig.getArgumentExp(0) == *Location::regOf(25));
}


void SignatureTest::testEllipsis()
{
    Signature sig("test");

    QVERIFY(!sig.hasEllipsis());
    sig.setHasEllipsis(true);
    QVERIFY(sig.hasEllipsis());
    sig.setHasEllipsis(false);
    QVERIFY(!sig.hasEllipsis());
}


void SignatureTest::testIsNoReturn()
{
    Signature sig("test");
    QVERIFY(!sig.isNoReturn());
}


void SignatureTest::testIsPromoted()
{
    Signature sig("test");
    QVERIFY(!sig.isPromoted());
}


void SignatureTest::testPromote()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testGetStackRegister()
{
    Signature sig("test");
    QCOMPARE(sig.getStackRegister(), -1);
}


void SignatureTest::testIsStackLocal()
{
    Signature sig("test");

    QVERIFY(sig.isStackLocal(28, Location::memOf(Location::regOf(28))));
    QVERIFY(!sig.isStackLocal(28, Location::regOf(28)));

    SharedExp spPlus4  = Binary::get(opPlus, Location::regOf(28), Const::get(4));
    SharedExp spMinus4 = Binary::get(opMinus, Location::regOf(28), Const::get(4));
    QVERIFY(!sig.isStackLocal(28, Location::memOf(spPlus4)));
    QVERIFY(sig.isStackLocal(28, Location::memOf(spMinus4)));

    spPlus4  = Binary::get(opMinus, Location::regOf(28), Const::get(-4));
    spMinus4 = Binary::get(opPlus, Location::regOf(28), Const::get(-4));
    QVERIFY(!sig.isStackLocal(28, Location::memOf(spPlus4)));
    QVERIFY(sig.isStackLocal(28, Location::memOf(spMinus4)));

    // Check if the subscript is ignored correctly
    QVERIFY(!sig.isStackLocal(28, RefExp::get(Location::memOf(spPlus4), nullptr)));
    QVERIFY(sig.isStackLocal(28, RefExp::get(Location::memOf(spMinus4), nullptr)));

    SharedExp spMinusPi = Binary::get(opMinus, Location::regOf(28), Const::get(3.14156));
    QVERIFY(!sig.isStackLocal(28, Location::memOf(spMinusPi)));
}


void SignatureTest::testIsAddrOfStackLocal()
{
    Signature sig("test");

    QVERIFY(sig.isAddrOfStackLocal(28, Location::regOf(28)));
    QVERIFY(!sig.isAddrOfStackLocal(28, Location::memOf(Location::regOf(28))));

    SharedExp spPlus4  = Binary::get(opPlus, Location::regOf(28), Const::get(4));
    SharedExp spMinus4 = Binary::get(opPlus, Location::regOf(28), Const::get(-4));
    QVERIFY(!sig.isAddrOfStackLocal(28, spPlus4));
    QVERIFY(sig.isAddrOfStackLocal(28, spMinus4));

    spPlus4  = Binary::get(opMinus, Location::regOf(28), Const::get(-4));
    spMinus4 = Binary::get(opMinus, Location::regOf(28), Const::get(4));
    QVERIFY(!sig.isAddrOfStackLocal(28, spPlus4));
    QVERIFY(sig.isAddrOfStackLocal(28, spMinus4));

    SharedExp spMinusPi = Binary::get(opMinus, Location::regOf(28), Const::get(3.14156));
    QVERIFY(!sig.isAddrOfStackLocal(28, spMinusPi));

    // m[sp{4} - 10] is not a stack local
    Assign asgn(Location::regOf(28), Location::regOf(24));
    asgn.setNumber(4);

    SharedExp sp4Minus10 = Binary::get(opMinus, RefExp::get(Location::regOf(28), &asgn), Const::get(10));
    QVERIFY(!sig.isAddrOfStackLocal(28, sp4Minus10));
}


void SignatureTest::testIsLocalOffsetNegative()
{
    Signature sig("test");
    QVERIFY(sig.isLocalOffsetNegative());
}


void SignatureTest::testIsLocalOffsetPositive()
{
    Signature sig("test");
    QVERIFY(!sig.isLocalOffsetPositive());
}


void SignatureTest::testIsOpCompatStackLocal()
{
    Signature sig("test");

    QVERIFY(sig.isOpCompatStackLocal(opMinus));
    QVERIFY(!sig.isOpCompatStackLocal(opPlus));
    QVERIFY(!sig.isOpCompatStackLocal(opAddrOf)); // neither plus nor minus
}


void SignatureTest::testGetProven()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testIsPreserved()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testGetLibraryDefines()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testGetABIDefines()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testSetPreferredName()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testSetPreferredReturn()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testAddPreferredParameter()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testGetPreferredReturn()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testGetPreferredName()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testGetNumPreferredParams()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testGetPreferredParam()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testCompare()
{
    QSKIP("Not implemented.");
}


QTEST_GUILESS_MAIN(SignatureTest)
