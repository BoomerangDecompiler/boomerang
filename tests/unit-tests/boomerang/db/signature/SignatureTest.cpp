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
#include "boomerang/db/exp/Location.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/type/type/IntegerType.h"


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
    QSKIP("Not implemented.");
}


void SignatureTest::testEllipsis()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testIsNoReturn()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testIsPromoted()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testPromote()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testGetStackRegister()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testIsStackLocal()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testIsAddrOfStackLocal()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testIsLocalOffsetNegative()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testIsLocalOffsetPositive()
{
    QSKIP("Not implemented.");
}


void SignatureTest::testIsOpCompatStackLocal()
{
    QSKIP("Not implemented.");
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
