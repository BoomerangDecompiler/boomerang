#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "TypeTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/Signature.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/core/Project.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/FloatType.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/frontend/pentium/pentiumfrontend.h"

#include <QTextStream>
#include <QDebug>

#define HELLO_WINDOWS    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/windows/hello.exe"))



// TODO: untangle the dynamic-size types from static size types ( Int vs Boolean etc. )
// The following two are for compForAddress()
struct ComplexTypeComp
{
    bool isArray;
    struct
    {
        QString  memberName; // Member name if offset
        unsigned index;      // Constant index if array
    }
         u;
};

typedef std::list<ComplexTypeComp> ComplexTypeCompList;


/// From a complex type like an array of structs with a float, return a list of components so you
/// can construct e.g. myarray1[8].mystruct2.myfloat7
std::unique_ptr<ComplexTypeCompList> compForAddress(Address addr, DataIntervalMap& dim)
{
    const TypedVariable *var = dim.find(addr);
    std::unique_ptr<ComplexTypeCompList> res(new ComplexTypeCompList);

    if (var == nullptr) {
        return res;
    }

    Address    startCurrent = var->baseAddr;
    SharedType curType      = var->type;

    while (startCurrent < addr) {
        size_t bitOffset = (addr - startCurrent).value() * 8;

        if (curType->isCompound()) {
            auto     compCurType = curType->as<CompoundType>();
            unsigned rem         = compCurType->getOffsetRemainder(bitOffset);
            startCurrent = addr - (rem / 8);
            ComplexTypeComp ctc;
            ctc.isArray      = false;
            ctc.u.memberName = compCurType->getNameAtOffset(bitOffset);
            res->push_back(ctc);
            curType = compCurType->getTypeAtOffset(bitOffset);
        }
        else if (curType->isArray()) {
            curType = curType->as<ArrayType>()->getBaseType();
            unsigned baseSize = curType->getSize();
            unsigned index    = bitOffset / baseSize;
            startCurrent += index * baseSize / 8;
            ComplexTypeComp ctc;
            ctc.isArray = true;
            ctc.u.index = index;
            res->push_back(ctc);
        }
        else {
            LOG_ERROR("TYPE ERROR: no struct or array at byte address %1", addr);
            return res;
        }
    }

    return res;
}


void TypeTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


void TypeTest::cleanupTestCase()
{
    Boomerang::destroy();
}


void TypeTest::testTypeLong()
{
    auto t = IntegerType::get(64, -1);

    QCOMPARE(t->getCtype(), QString("unsigned long long"));
}


void TypeTest::testNotEqual()
{
    auto t1(IntegerType::get(32, -1));
    auto t2(IntegerType::get(32, -1));
    auto t3(IntegerType::get(16, -1));

    QVERIFY(!(*t1 != *t2));
    QVERIFY(*t2 != *t3);
}


void TypeTest::testCompound()
{
    Project project;
    QVERIFY(project.loadBinaryFile(HELLO_WINDOWS));
    Prog *prog = project.getProg();
    IFrontEnd *fe = prog->getFrontEnd();

    fe->readLibraryCatalog(); // Read definitions

    std::shared_ptr<Signature> paintSig = fe->getLibSignature("BeginPaint");

    SharedType paramType = paintSig->getParamType(1);
    QCOMPARE(paintSig->getParamType(1)->getCtype(), QString("LPPAINTSTRUCT"));
    SharedType paintStructType = paramType->as<PointerType>()->getPointsTo();
    QCOMPARE(paintStructType->getCtype(), QString("PAINTSTRUCT"));

    // Offset 8 should have a RECT
    SharedType subTy    = paintStructType->as<CompoundType>()->getTypeAtOffset(8 * 8);
    QString    expected = "struct { "
                          "int left; "
                          "int top; "
                          "int right; "
                          "int bottom; "
                          "}";
    QCOMPARE(subTy->getCtype(true), expected);

    // Name at offset 0x0C should be bottom
    QCOMPARE(subTy->as<CompoundType>()->getNameAtOffset(0x0C * 8), QString("bottom"));

    // Now figure out the name at offset 8+C
    QCOMPARE(paintStructType->as<CompoundType>()->getNameAtOffset((8 + 0x0C) * 8), QString("rcPaint"));

    // Also at offset 8
    QCOMPARE(paintStructType->as<CompoundType>()->getNameAtOffset((8 + 0) * 8), QString("rcPaint"));

    // Also at offset 8+4
    QCOMPARE(paintStructType->as<CompoundType>()->getNameAtOffset((8 + 4) * 8), QString("rcPaint"));

    // And at offset 8+8
    QCOMPARE(paintStructType->as<CompoundType>()->getNameAtOffset((8 + 8) * 8), QString("rcPaint"));
}


void TypeTest::testDataInterval()
{
    Prog            *prog = new Prog("test", nullptr);
    Module          *m    = prog->getOrInsertModule("test");
    UserProc        *proc = static_cast<UserProc *>(m->createFunction("test", Address(0x123)));
    DataIntervalMap dim(proc);

    proc->setSignature(Signature::instantiate(Platform::PENTIUM, CallConv::C, "testProc"));

    dim.insertItem(Address(0x00001000), "first", IntegerType::get(32, 1));
    dim.insertItem(Address(0x00001004), "second", FloatType::get(64));
    QCOMPARE(dim.prints(),
             "0x00001000-0x00001004 first int\n"
             "0x00001004-0x0000100c second double\n");

    const TypedVariable *var = dim.find(Address(0x00001000));
    QVERIFY(var != nullptr);
    QCOMPARE(var->name, QString("first"));

    var = dim.find(Address(0x00001003));
    QVERIFY(var != nullptr);
    QCOMPARE(var->name, QString("first"));

    var = dim.find(Address(0x00001004));
    QVERIFY(var);
    QCOMPARE(var->name, QString("second"));

    var = dim.find(Address(0x00001007));
    QVERIFY(var);
    QCOMPARE(var->name, QString("second"));

    auto ct(CompoundType::get());
    ct->addType(IntegerType::get(16, 1), "short1");
    ct->addType(IntegerType::get(16, 1), "short2");
    ct->addType(IntegerType::get(32, 1), "int1");
    ct->addType(FloatType::get(32), "float1");
    dim.insertItem(Address(0x00001010), "struct1", ct);

    std::unique_ptr<ComplexTypeCompList> ctcl = compForAddress(Address(0x00001012), dim);
    unsigned             ua   = ctcl->size();
    unsigned             ue   = 1;
    QCOMPARE(ua, ue);
    ComplexTypeComp& ctc = ctcl->front();
    ue = 0;
    ua = ctc.isArray;
    QCOMPARE(ua, ue);
    QCOMPARE(ctc.u.memberName, QString("short2"));

    // An array of 10 struct1's
    auto at = ArrayType::get(ct, 10);
    dim.insertItem(Address(0x00001020), "array1", at);
    std::unique_ptr<ComplexTypeCompList> ctcl2 = compForAddress(Address(0x00001020 + 0x3C + 8), dim);
    // Should be 2 components: [5] and .float1
    QCOMPARE(ctcl2->size(), static_cast<size_t>(2));

    ComplexTypeComp& ctc0 = ctcl2->front();
    ComplexTypeComp& ctc1 = ctcl2->back();
    QCOMPARE(ctc0.isArray, true);
    QCOMPARE(ctc0.u.index, 5U);
    QCOMPARE(ctc1.isArray, false);
    QCOMPARE(ctc1.u.memberName, QString("float1"));

    delete prog;
}


void TypeTest::testDataIntervalOverlaps()
{
    Prog            *prog = new Prog("test", nullptr);
    Module          *m    = prog->getOrInsertModule("test");
    UserProc        *proc = static_cast<UserProc *>(m->createFunction("test", Address(0x00000100)));
    DataIntervalMap dim(proc);

    proc->setSignature(Signature::instantiate(Platform::PENTIUM, CallConv::C, "test"));

    dim.insertItem(Address(0x00001000), "firstInt", IntegerType::get(32, 1));
    dim.insertItem(Address(0x00001004), "firstFloat", FloatType::get(32));
    dim.insertItem(Address(0x00001008), "secondInt", IntegerType::get(32, 1));
    dim.insertItem(Address(0x0000100C), "secondFloat", FloatType::get(32));

    auto ct = CompoundType::get();
    ct->addType(IntegerType::get(32, 1), "int3");
    ct->addType(FloatType::get(32), "float3");
    dim.insertItem(Address(0x00001010), "newStruct", ct);
    QCOMPARE(dim.prints(),
             "0x00001000-0x00001004 firstInt int\n"
             "0x00001004-0x00001008 firstFloat float\n"
             "0x00001008-0x0000100c secondInt int\n"
             "0x0000100c-0x00001010 secondFloat float\n"
             "0x00001010-0x00001018 newStruct struct { int int3; float float3; }\n");

    // First insert a new struct over the top of the existing middle pair
    auto ctu = CompoundType::get();
    ctu->addType(IntegerType::get(32, 0), "newInt"); // This int has UNKNOWN sign
    ctu->addType(FloatType::get(32), "newFloat");
    dim.insertItem(Address(0x00001008), "replacementStruct", ctu);

    const TypedVariable *var = dim.find(Address(0x1008));
    QCOMPARE(var->type->getCtype(), QString("struct { int newInt; float newFloat; }"));
    QCOMPARE(dim.prints(),
             "0x00001000-0x00001004 firstInt int\n"
             "0x00001004-0x00001008 firstFloat float\n"
             "0x00001008-0x00001010 replacementStruct struct { int newInt; float newFloat; }\n"
             "0x00001010-0x00001018 newStruct struct { int int3; float float3; }\n");


    // Attempt a weave; should fail
    auto ct3 = CompoundType::get();
    ct3->addType(FloatType::get(32), "newFloat3");
    ct3->addType(IntegerType::get(32, 0), "newInt3");
    QVERIFY(dim.insertItem(Address(0x00001004), "weaveStruct1", ct3) == dim.end());
    var = dim.find(Address(0x00001004));
    QVERIFY(var != nullptr);
    QCOMPARE(var->name, QString("firstFloat"));

    // Totally unaligned
    QVERIFY(dim.insertItem(Address(0x00001001), "weaveStruct2", ct3) == dim.end());
    var = dim.find(Address(0x000001001));
    QVERIFY(var != nullptr);
    QCOMPARE(var->name, QString("firstInt"));

    dim.insertItem(Address(0x00001004), "firstInt", IntegerType::get(32, 1)); // Should fail
    var = dim.find(Address(0x00001004));
    QCOMPARE(var->name, QString("firstFloat"));

    // Set up three ints
    dim.deleteItem(Address(0x00001004));
    dim.deleteItem(Address(0x00001008));
    dim.insertItem(Address(0x00001004), "firstInt", IntegerType::get(32, 1)); // Definitely signed
    dim.insertItem(Address(0x00001008), "firstInt", IntegerType::get(32, 0)); // Unknown signedess
    // then, add an array over the three integers
    auto at = ArrayType::get(IntegerType::get(32, 0), 3);
    dim.insertItem(Address(0x00001000), "newArray", at);

    var = dim.find(Address(0x00001005)); // Check middle element
    QCOMPARE(var->name, QString("newArray"));
    var = dim.find(Address(0x00001000)); // Check first
    QCOMPARE(var->name, QString("newArray"));
    var = dim.find(Address(0x100B));     // Check last
    QCOMPARE(var->name, QString("newArray"));

    // Already have an array of 3 ints at 0x1000. Put a new array completely before, then with only one word overlap
    QVERIFY(dim.insertItem(Address(0x00000F00), "newArray2", at) != dim.end());
    var = dim.find(Address(0x00001000)); // Should still be newArray at 0x1000
    QVERIFY(var != nullptr);
    QCOMPARE(var->name, QString("newArray"));

    var = dim.find(Address(0x00000F00));
    QVERIFY(var != nullptr);
    QCOMPARE(var->name, QString("newArray2"));

    // insertion should fail
    QVERIFY(dim.insertItem(Address(0x00000FF8), "newArray3", at) == dim.end());
    var = dim.find(Address(0x00000FF8));
    QVERIFY(var == nullptr);

    delete prog;
}


QTEST_MAIN(TypeTest)
