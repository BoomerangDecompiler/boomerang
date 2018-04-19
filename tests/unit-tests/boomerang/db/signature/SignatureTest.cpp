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


#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/util/StatementList.h"


void SignatureTest::testClone()
{
    std::shared_ptr<Signature> sig(new Signature("test"));
    sig->addParameter("firstParam", Location::regOf(PENT_REG_EDX), IntegerType::get(32, 1));
    sig->addReturn(IntegerType::get(32, 1), Location::regOf(PENT_REG_EAX));

    std::shared_ptr<Signature> cloned = sig->clone();
    QCOMPARE(cloned->getName(), QString("test"));
    QCOMPARE(cloned->getNumParams(), 1);
    QCOMPARE(cloned->getParamName(0), QString("firstParam"));
    QCOMPARE(cloned->getNumReturns(), 1);
    QVERIFY(*cloned->getReturnType(0) == *IntegerType::get(32, 1));
}


void SignatureTest::testCompare()
{
    Signature sig1("test1");
    Signature sig2("test2");
    QVERIFY(sig1 == sig2);

    sig1.addParameter(Location::regOf(PENT_REG_EDX));
    QVERIFY(sig1 != sig2);

    sig2.addParameter(Location::regOf(PENT_REG_EAX));
    QVERIFY(sig1 != sig2); // different paarameters

    sig2.addParameter(Location::regOf(PENT_REG_EDX));
    sig1.addParameter(Location::regOf(PENT_REG_EAX));
    QVERIFY(sig1 != sig2); // swapped parameters

    sig1.removeParameter(0);
    sig1.removeParameter(0);
    sig2.removeParameter(0);
    sig2.removeParameter(0);

    QVERIFY(sig1 == sig2);

    sig1.addReturn(VoidType::get(), Location::regOf(PENT_REG_ESP));
    QVERIFY(sig1 != sig2);
    sig2.addReturn(IntegerType::get(32, 1), Location::regOf(PENT_REG_ECX));
    QVERIFY(sig1 != sig2);
}


void SignatureTest::testAddReturn()
{
    Signature sig("test");
    sig.addReturn(IntegerType::get(32, 1), Location::regOf(PENT_REG_EAX));
    QVERIFY(*sig.getReturnExp(0) == *Location::regOf(PENT_REG_EAX));
}


void SignatureTest::testGetReturnExp()
{
    Signature sig("test");

    sig.addReturn(Location::regOf(PENT_REG_EAX));
    QVERIFY(*sig.getReturnExp(0) == *Location::regOf(PENT_REG_EAX));
}


void SignatureTest::testGetReturnType()
{
    Signature sig("test");

    sig.addReturn(Location::regOf(PENT_REG_EAX));
    QVERIFY(*sig.getReturnType(0) == *PointerType::get(VoidType::get()));

    sig.addReturn(IntegerType::get(32, 1), Location::regOf(PENT_REG_ECX));
    QVERIFY(*sig.getReturnType(1) == *IntegerType::get(32, 1));
}


void SignatureTest::testGetNumReturns()
{
    Signature sig("test");
    QCOMPARE(sig.getNumReturns(), 0);

    sig.addReturn(Location::regOf(PENT_REG_EAX));
    QCOMPARE(sig.getNumReturns(), 1);
}


void SignatureTest::testFindReturn()
{
    Signature sig("test");
    QCOMPARE(sig.findReturn(nullptr), -1);

    sig.addReturn(IntegerType::get(32, 1), Location::regOf(PENT_REG_EAX));
    QCOMPARE(sig.findReturn(Location::regOf(PENT_REG_EAX)), 0);
    QCOMPARE(sig.findReturn(Location::regOf(PENT_REG_ECX)), -1);
}


void SignatureTest::testAddParameter()
{
    Signature sig("test");

    sig.addParameter(Location::regOf(PENT_REG_ECX));
    QCOMPARE(sig.getNumParams(), 1);
    QVERIFY(*sig.getParamType(0) == *VoidType::get());

    sig.addParameter(Location::regOf(PENT_REG_ECX), IntegerType::get(32, 1));
    QCOMPARE(sig.getNumParams(), 2);
    QVERIFY(*sig.getParamType(1) == *IntegerType::get(32, 1));

    // test parameter name collision detection
    sig.setParamName(1, "param2");
    sig.addParameter("", Location::regOf(PENT_REG_EBX)); // name = "param3" (taken) -> "param4"
    QCOMPARE(sig.getParamName(2), QString("param3"));
}


void SignatureTest::testRemoveParameter()
{
    Signature sig("test");

    // verify it does not crash
    sig.removeParameter(nullptr);
    QCOMPARE(sig.getNumParams(), 0);

    sig.removeParameter(0);
    QCOMPARE(sig.getNumParams(), 0);

    sig.addParameter(Location::regOf(PENT_REG_ECX));
    sig.removeParameter(0);
    QCOMPARE(sig.getNumParams(), 0);

    sig.addParameter(Location::regOf(PENT_REG_ECX), IntegerType::get(32, 1));
    sig.addParameter(Location::regOf(PENT_REG_EDX));
    sig.removeParameter(Location::regOf(PENT_REG_ECX));
    QCOMPARE(sig.getNumParams(), 1);
    QVERIFY(*sig.getParamExp(0) == *Location::regOf(PENT_REG_EDX));
}


void SignatureTest::testSetNumParams()
{
    Signature sig("test");

    sig.setNumParams(0);
    QCOMPARE(sig.getNumParams(), 0);

    sig.addParameter("foo", Location::regOf(PENT_REG_ECX));
    sig.addParameter("bar", Location::regOf(PENT_REG_EAX));

    sig.setNumParams(1);
    QCOMPARE(sig.getNumParams(), 1);
}


void SignatureTest::testGetParamName()
{
    Signature sig("test");

    sig.addParameter("testParam", Location::regOf(PENT_REG_ECX), VoidType::get());
    QCOMPARE(sig.getParamName(0), QString("testParam"));
}


void SignatureTest::testGetParamExp()
{
    Signature sig("test");

    sig.addParameter(Location::regOf(PENT_REG_ECX));
    QVERIFY(*sig.getParamExp(0) == *Location::regOf(PENT_REG_ECX));
}


void SignatureTest::testGetParamType()
{
    Signature sig("test");

    QVERIFY(sig.getParamType(0) == nullptr);

    sig.addParameter(Location::regOf(PENT_REG_ECX), IntegerType::get(32, 1));
    QVERIFY(*sig.getParamType(0) == *IntegerType::get(32, 1));
}


void SignatureTest::testGetParamBoundMax()
{
    Signature sig("test");
    QCOMPARE(sig.getParamBoundMax(0), QString());

    sig.addParameter(Location::regOf(PENT_REG_ECX), IntegerType::get(32, 1));
    QCOMPARE(sig.getParamBoundMax(0), QString());

    sig.addParameter("testParam", Location::regOf(PENT_REG_EDX), IntegerType::get(32, 1), "r25");
    QCOMPARE(sig.getParamBoundMax(1), QString("r25"));
}


void SignatureTest::testSetParamType()
{
    Signature sig("test");

    sig.addParameter("testParam", Location::regOf(PENT_REG_ECX), IntegerType::get(32, 1));
    sig.setParamType(0, VoidType::get());
    QVERIFY(*sig.getParamType(0) == *VoidType::get());

    sig.setParamType("testParam", IntegerType::get(32, 0));
    QVERIFY(*sig.getParamType(0) == *IntegerType::get(32, 0));
}


void SignatureTest::testFindParam()
{
    Signature sig("test");
    QCOMPARE(sig.findParam(Location::regOf(PENT_REG_EAX)), -1);
    QCOMPARE(sig.findParam("testParam"), -1);

    sig.addParameter("testParam", Location::regOf(PENT_REG_ECX), IntegerType::get(32, 1));
    QCOMPARE(sig.findParam(Location::regOf(PENT_REG_ECX)), 0);
    QCOMPARE(sig.findParam(Location::regOf(PENT_REG_EAX)), -1);
    QCOMPARE(sig.findParam("testParam"), 0);
    QCOMPARE(sig.findParam("Foo"), -1);
}


void SignatureTest::testRenameParam()
{
    Signature sig("test");
    QVERIFY(!sig.renameParam("", ""));

    sig.addParameter("testParam", Location::regOf(PENT_REG_ECX));
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

    sig.addParameter(Location::regOf(PENT_REG_ECX));
    QVERIFY(*sig.getArgumentExp(0) == *Location::regOf(PENT_REG_ECX));
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

    QVERIFY(sig.isStackLocal(PENT_REG_ESP, Location::memOf(Location::regOf(PENT_REG_ESP))));
    QVERIFY(!sig.isStackLocal(PENT_REG_ESP, Location::regOf(PENT_REG_ESP)));

    SharedExp spPlus4  = Binary::get(opPlus, Location::regOf(PENT_REG_ESP), Const::get(4));
    SharedExp spMinus4 = Binary::get(opMinus, Location::regOf(PENT_REG_ESP), Const::get(4));
    QVERIFY(!sig.isStackLocal(PENT_REG_ESP, Location::memOf(spPlus4)));
    QVERIFY(sig.isStackLocal(PENT_REG_ESP, Location::memOf(spMinus4)));

    spPlus4  = Binary::get(opMinus, Location::regOf(PENT_REG_ESP), Const::get(-4));
    spMinus4 = Binary::get(opPlus, Location::regOf(PENT_REG_ESP), Const::get(-4));
    QVERIFY(!sig.isStackLocal(PENT_REG_ESP, Location::memOf(spPlus4)));
    QVERIFY(sig.isStackLocal(PENT_REG_ESP, Location::memOf(spMinus4)));

    // Check if the subscript is ignored correctly
    QVERIFY(!sig.isStackLocal(PENT_REG_ESP, RefExp::get(Location::memOf(spPlus4), nullptr)));
    QVERIFY(sig.isStackLocal(28, RefExp::get(Location::memOf(spMinus4), nullptr)));

    SharedExp spMinusPi = Binary::get(opMinus, Location::regOf(PENT_REG_ESP), Const::get(3.14156));
    QVERIFY(!sig.isStackLocal(PENT_REG_ESP, Location::memOf(spMinusPi)));
}


void SignatureTest::testIsAddrOfStackLocal()
{
    Signature sig("test");

    QVERIFY(sig.isAddrOfStackLocal(PENT_REG_ESP, Location::regOf(PENT_REG_ESP)));
    QVERIFY(!sig.isAddrOfStackLocal(PENT_REG_ESP, Location::memOf(Location::regOf(PENT_REG_ESP))));

    SharedExp spPlus4  = Binary::get(opPlus, Location::regOf(PENT_REG_ESP), Const::get(4));
    SharedExp spMinus4 = Binary::get(opPlus, Location::regOf(PENT_REG_ESP), Const::get(-4));
    QVERIFY(!sig.isAddrOfStackLocal(PENT_REG_ESP, spPlus4));
    QVERIFY(sig.isAddrOfStackLocal(PENT_REG_ESP, spMinus4));

    spPlus4  = Binary::get(opMinus, Location::regOf(PENT_REG_ESP), Const::get(-4));
    spMinus4 = Binary::get(opMinus, Location::regOf(PENT_REG_ESP), Const::get(4));
    QVERIFY(!sig.isAddrOfStackLocal(PENT_REG_ESP, spPlus4));
    QVERIFY(sig.isAddrOfStackLocal(PENT_REG_ESP, spMinus4));

    SharedExp spMinusPi = Binary::get(opMinus, Location::regOf(PENT_REG_ESP), Const::get(3.14156));
    QVERIFY(!sig.isAddrOfStackLocal(PENT_REG_ESP, spMinusPi));

    // m[sp{4} - 10] is not a stack local
    Assign asgn(Location::regOf(PENT_REG_ESP), Location::regOf(PENT_REG_EAX));
    asgn.setNumber(4);

    SharedExp sp4Minus10 = Binary::get(opMinus, RefExp::get(Location::regOf(PENT_REG_ESP), &asgn), Const::get(10));
    QVERIFY(!sig.isAddrOfStackLocal(PENT_REG_ESP, sp4Minus10));

    // verify a[...] and m[...] cancel out
    QVERIFY(sig.isAddrOfStackLocal(PENT_REG_ESP, Unary::get(opAddrOf, Location::memOf(spMinus4))));
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
    Signature sig("test");
    QVERIFY(sig.getProven(SharedExp()) == nullptr);
}


void SignatureTest::testIsPreserved()
{
    Signature sig("test");
    QVERIFY(!sig.isPreserved(SharedExp()));
}


void SignatureTest::testGetLibraryDefines()
{
    Signature sig("test");

    StatementList stmts;
    sig.getLibraryDefines(stmts);
    QVERIFY(stmts.empty());
}


void SignatureTest::testGetABIDefines()
{
    StatementList defs;

    QVERIFY(Signature::getABIDefines(Machine::PENTIUM, defs));
    QVERIFY(defs.size() == 3);
    QVERIFY(defs.findOnLeft(Location::regOf(PENT_REG_EAX)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(PENT_REG_ECX)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(PENT_REG_EDX)) != nullptr);
    qDeleteAll(defs);
    defs.clear();

    QVERIFY(Signature::getABIDefines(Machine::SPARC, defs));
    QVERIFY(defs.size() == 7);
    QVERIFY(defs.findOnLeft(Location::regOf(SPARC_REG_O0)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(SPARC_REG_O1)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(SPARC_REG_O2)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(SPARC_REG_O3)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(SPARC_REG_O4)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(SPARC_REG_O5)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(SPARC_REG_G1)) != nullptr);
    qDeleteAll(defs);
    defs.clear();

    QVERIFY(Signature::getABIDefines(Machine::PPC, defs));
    QVERIFY(defs.size() == 10);
    QVERIFY(defs.findOnLeft(Location::regOf(PPC_REG_G3)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(PPC_REG_G4)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(PPC_REG_G5)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(PPC_REG_G6)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(PPC_REG_G7)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(PPC_REG_G8)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(PPC_REG_G9)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(PPC_REG_G10)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(PPC_REG_G11)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(PPC_REG_G12)) != nullptr);
    qDeleteAll(defs);
    defs.clear();

    QVERIFY(Signature::getABIDefines(Machine::ST20, defs));
    QVERIFY(defs.size() == 3);

    QVERIFY(!Signature::getABIDefines(Machine::ST20, defs));
    QVERIFY(defs.size() == 3);

    QVERIFY(!Signature::getABIDefines(Machine::PPC, defs));
    QVERIFY(defs.size() == 3);

    // Machine::ST20
    QVERIFY(defs.findOnLeft(Location::regOf(ST20_REG_A)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(ST20_REG_B)) != nullptr);
    QVERIFY(defs.findOnLeft(Location::regOf(ST20_REG_C)) != nullptr);
    qDeleteAll(defs);
    defs.clear();

    QVERIFY(Signature::getABIDefines(Machine::UNKNOWN, defs));
    QVERIFY(defs.empty());
    QVERIFY(!Signature::getABIDefines(Machine::INVALID, defs));
    QVERIFY(defs.empty());
}


void SignatureTest::testPreferredName()
{
    Signature sig("test");

    QCOMPARE(sig.getPreferredName(), QString());
    sig.setPreferredName("Foo");
    QCOMPARE(sig.getPreferredName(), QString("Foo"));
}


QTEST_GUILESS_MAIN(SignatureTest)
