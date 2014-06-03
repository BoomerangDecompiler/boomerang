/***************************************************************************/ /**
  * \file       TypeTest.cc
  * OVERVIEW:   Provides the implementation for the TypeTest class, which tests the Type class and some utility
  *functions
  ******************************************************************************/
/*
 * $Revision$
 *
 * 09 Apr 02 - Mike: Created
 * 22 Aug 03 - Mike: Extended for Constraint tests
 * 25 Juk 05 - Mike: DataIntervalMap tests
 */

#include "TypeTest.h"
#include "BinaryFile.h" // Ugh - needed before frontend.h
#include "pentiumfrontend.h"
#include "signature.h"
#include "boomerang.h"
#include "log.h"
#include "prog.h"
#include "proc.h"



#define HELLO_WINDOWS "test/windows/hello.exe"

/***************************************************************************/ /**
  * FUNCTION:        TypeTest::setUp
  * OVERVIEW:        Set up anything needed before all tests
  * NOTE:            Called before any tests
  *
  ******************************************************************************/
static bool logset = false;
void TypeTest::setUp() {
    if (!logset) {
        logset = true;
        Boomerang::get()->setLogger(new NullLogger());
    }
}

/***************************************************************************/ /**
  * FUNCTION:        TypeTest::tearDown
  * OVERVIEW:        Delete objects created in setUp
  * NOTE:            Called after all tests
  *
  ******************************************************************************/
void TypeTest::tearDown() {}

/***************************************************************************/ /**
  * FUNCTION:        TypeTest::testTypeLong
  * OVERVIEW:        Test type unsigned long
  ******************************************************************************/
void TypeTest::testTypeLong() {

    QString expected("unsigned long long");
    IntegerType *t = IntegerType::get(64, -1);
    QString actual(t->getCtype());
    QCOMPARE(expected, actual);

}

/***************************************************************************/ /**
  * FUNCTION:        TypeTest::testNotEqual
  * OVERVIEW:        Test type inequality
  ******************************************************************************/
void TypeTest::testNotEqual() {

    IntegerType *t1(IntegerType::get(32, -1));
    IntegerType *t2(IntegerType::get(32, -1));
    IntegerType *t3(IntegerType::get(16, -1));
    QVERIFY(!(*t1 != *t2));
    QVERIFY(*t2 != *t3);
}

/***************************************************************************/ /**
  * FUNCTION:        TypeTest::testNotEqual
  * OVERVIEW:        Test type inequality
  ******************************************************************************/
void TypeTest::testCompound() {
    BinaryFileFactory bff;
    QObject *pBF = bff.Load(HELLO_WINDOWS);
    FrontEnd *pFE = new PentiumFrontEnd(pBF, new Prog, &bff);
    pFE->readLibraryCatalog(); // Read definitions

    Signature *paintSig = pFE->getLibSignature("BeginPaint");
    // Second argument should be an LPPAINTSTRUCT
    Type *ty = paintSig->getParamType(1);
    QString p = ty->getCtype();
    QString expected("LPPAINTSTRUCT");
    QString actual(p);
    QCOMPARE(expected, actual);

    // Get the type pointed to
    ty = ty->asPointer()->getPointsTo();
    p = ty->getCtype();
    expected = "PAINTSTRUCT";
    actual = p;
    QCOMPARE(expected, actual);

    // Offset 8 should have a RECT
    Type *subTy = ty->asCompound()->getTypeAtOffset(8 * 8);
    p = subTy->getCtype();
    expected = "RECT";
    actual = p;
    QCOMPARE(expected, actual);

    // Name at offset C should be bottom
    p = subTy->asCompound()->getNameAtOffset(0x0C * 8);
    expected = "bottom";
    actual = p;
    QCOMPARE(expected, actual);

    // Now figure out the name at offset 8+C
    p = ty->asCompound()->getNameAtOffset((8 + 0x0C) * 8);
    expected = "rcPaint";
    actual = p;
    QCOMPARE(expected, actual);

    // Also at offset 8
    p = ty->asCompound()->getNameAtOffset((8 + 0) * 8);
    actual = p;
    QCOMPARE(expected, actual);

    // Also at offset 8+4
    p = ty->asCompound()->getNameAtOffset((8 + 4) * 8);
    actual = p;
    QCOMPARE(expected, actual);

    // And at offset 8+8
    p = ty->asCompound()->getNameAtOffset((8 + 8) * 8);
    actual = p;
    QCOMPARE(expected, actual);

    delete pFE;
}

/***************************************************************************/ /**
  * FUNCTION:        TypeTest::testDataInterval
  * OVERVIEW:        Test the DataIntervalMap class
  ******************************************************************************/
void TypeTest::testDataInterval() {
    DataIntervalMap dim;

    Prog *prog = new Prog;
    UserProc *proc = (UserProc *)prog->newProc("test", ADDRESS::g(0x123));
    std::string name("test");
    proc->setSignature(Signature::instantiate(PLAT_PENTIUM, CONV_C, name.c_str()));
    dim.setProc(proc);

    dim.addItem(ADDRESS::g(0x1000), "first", IntegerType::get(32, 1));
    dim.addItem(ADDRESS::g(0x1004), "second", FloatType::get(64));
    QString actual(dim.prints());
    QString expected("0x1000 first int\n"
                         "0x1004 second double\n");
    QCOMPARE(expected, actual);

    DataIntervalEntry *pdie = dim.find(ADDRESS::g(0x1000));
    expected = "first";
    QVERIFY(pdie);
    actual = pdie->second.name;
    QCOMPARE(expected, actual);

    pdie = dim.find(ADDRESS::g(0x1003));
    QVERIFY(pdie);
    actual = pdie->second.name;
    QCOMPARE(expected, actual);

    pdie = dim.find(ADDRESS::g(0x1004));
    QVERIFY(pdie);
    expected = "second";
    actual = pdie->second.name;
    QCOMPARE(expected, actual);

    pdie = dim.find(ADDRESS::g(0x1007));
    QVERIFY(pdie);
    actual = pdie->second.name;
    QCOMPARE(expected, actual);

    CompoundType ct;
    ct.addType(IntegerType::get(16, 1), "short1");
    ct.addType(IntegerType::get(16, 1), "short2");
    ct.addType(IntegerType::get(32, 1), "int1");
    ct.addType(FloatType::get(32), "float1");
    dim.addItem(ADDRESS::g(0x1010), "struct1", &ct);

    ComplexTypeCompList &ctcl = ct.compForAddress(ADDRESS::g(0x1012), dim);
    unsigned ua = ctcl.size();
    unsigned ue = 1;
    QCOMPARE(ue, ua);
    ComplexTypeComp &ctc = ctcl.front();
    ue = 0;
    ua = ctc.isArray;
    QCOMPARE(ue, ua);
    expected = "short2";
    actual = ctc.u.memberName;
    QCOMPARE(expected, actual);

    // An array of 10 struct1's
    ArrayType at(&ct, 10);
    dim.addItem(ADDRESS::g(0x1020), "array1", &at);
    ComplexTypeCompList &ctcl2 = at.compForAddress(ADDRESS::g(0x1020 + 0x3C + 8), dim);
    // Should be 2 components: [5] and .float1
    ue = 2;
    ua = ctcl2.size();
    QCOMPARE(ue, ua);
    ComplexTypeComp &ctc0 = ctcl2.front();
    ComplexTypeComp &ctc1 = ctcl2.back();
    ue = 1;
    ua = ctc0.isArray;
    QCOMPARE(ue, ua);
    ue = 5;
    ua = ctc0.u.index;
    QCOMPARE(ue, ua);
    ue = 0;
    ua = ctc1.isArray;
    QCOMPARE(ue, ua);
    expected = "float1";
    actual = ctc1.u.memberName;
    QCOMPARE(expected, actual);
}

/***************************************************************************/ /**
  * FUNCTION:        TypeTest::testDataIntervalOverlaps
  * OVERVIEW:        Test the DataIntervalMap class with overlapping addItems
  ******************************************************************************/
void TypeTest::testDataIntervalOverlaps() {
    DataIntervalMap dim;

    Prog *prog = new Prog;
    UserProc *proc = (UserProc *)prog->newProc("test", ADDRESS::g(0x123));
    std::string name("test");
    proc->setSignature(Signature::instantiate(PLAT_PENTIUM, CONV_C, name.c_str()));
    dim.setProc(proc);

    dim.addItem(ADDRESS::g(0x1000), "firstInt", IntegerType::get(32, 1));
    dim.addItem(ADDRESS::g(0x1004), "firstFloat", FloatType::get(32));
    dim.addItem(ADDRESS::g(0x1008), "secondInt", IntegerType::get(32, 1));
    dim.addItem(ADDRESS::g(0x100C), "secondFloat", FloatType::get(32));
    CompoundType ct;
    ct.addType(IntegerType::get(32, 1), "int3");
    ct.addType(FloatType::get(32), "float3");
    dim.addItem(ADDRESS::g(0x1010), "existingStruct", &ct);

    // First insert a new struct over the top of the existing middle pair
    CompoundType ctu;
    ctu.addType(IntegerType::get(32, 0), "newInt"); // This int has UNKNOWN sign
    ctu.addType(FloatType::get(32), "newFloat");
    dim.addItem(ADDRESS::g(0x1008), "replacementStruct", &ctu);

    DataIntervalEntry *pdie = dim.find(ADDRESS::g(0x1008));
    QString expected = "struct { int newInt; float newFloat; }";
    QString actual = pdie->second.type->getCtype();
    QCOMPARE(expected, actual);

    // Attempt a weave; should fail
    CompoundType ct3;
    ct3.addType(FloatType::get(32), "newFloat3");
    ct3.addType(IntegerType::get(32, 0), "newInt3");
    dim.addItem(ADDRESS::g(0x1004), "weaveStruct1", &ct3);
    pdie = dim.find(ADDRESS::g(0x1004));
    expected = "firstFloat";
    actual = pdie->second.name;
    QCOMPARE(expected, actual);

    // Totally unaligned
    dim.addItem(ADDRESS::g(0x1001), "weaveStruct2", &ct3);
    pdie = dim.find(ADDRESS::g(0x1001));
    expected = "firstInt";
    actual = pdie->second.name;
    QCOMPARE(expected, actual);

    dim.addItem(ADDRESS::g(0x1004), "firstInt", IntegerType::get(32, 1)); // Should fail
    pdie = dim.find(ADDRESS::g(0x1004));
    expected = "firstFloat";
    actual = pdie->second.name;
    QCOMPARE(expected, actual);

    // Set up three ints
    dim.deleteItem(ADDRESS::g(0x1004));
    dim.addItem(ADDRESS::g(0x1004), "firstInt", IntegerType::get(32, 1)); // Definately signed
    dim.deleteItem(ADDRESS::g(0x1008));
    dim.addItem(ADDRESS::g(0x1008), "firstInt", IntegerType::get(32, 0)); // Unknown signedess
    // then, add an array over the three integers
    ArrayType at(IntegerType::get(32, 0), 3);
    dim.addItem(ADDRESS::g(0x1000), "newArray", &at);
    pdie = dim.find(ADDRESS::g(0x1005)); // Check middle element
    expected = "newArray";
    actual = pdie->second.name;
    QCOMPARE(expected, actual);
    pdie = dim.find(ADDRESS::g(0x1000)); // Check first
    actual = pdie->second.name;
    QCOMPARE(expected, actual);
    pdie = dim.find(ADDRESS::g(0x100B)); // Check last
    actual = pdie->second.name;
    QCOMPARE(expected, actual);

    // Already have an array of 3 ints at 0x1000. Put a new array completely before, then with only one word overlap
    dim.addItem(ADDRESS::g(0xF00), "newArray2", &at);
    pdie = dim.find(ADDRESS::g(0x1000)); // Should still be newArray at 0x1000
    actual = pdie->second.name;
    QCOMPARE(expected, actual);

    pdie = dim.find(ADDRESS::g(0xF00));
    expected = "newArray2";
    actual = pdie->second.name;
    QCOMPARE(expected, actual);

    dim.addItem(ADDRESS::g(0xFF8), "newArray3", &at); // Should fail
    pdie = dim.find(ADDRESS::g(0xFF8));
    QVERIFY(nullptr==(void *)pdie); // Expect nullptr
}
QTEST_MAIN(TypeTest)
