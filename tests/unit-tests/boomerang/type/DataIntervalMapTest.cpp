#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DataIntervalMapTest.h"


#include "boomerang/type/DataIntervalMap.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/util/log/Log.h"

#include <QDebug>


#define HELLO_WINDOWS    getFullSamplePath("windows/hello.exe")


// TODO: untangle the dynamic-size types from static size types ( Int vs Boolean etc. )
// The following two are for compForAddress()
struct ComplexTypeComp
{
    bool isArray;
    struct
    {
        QString  memberName; // Member name if offset
        unsigned index;      // Constant index if array
    } u;
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
            ctc.u.memberName = compCurType->getMemberNameByOffset(bitOffset);
            res->push_back(ctc);
            curType = compCurType->getMemberTypeByOffset(bitOffset);
        }
        else if (curType->isArray()) {
            curType = curType->as<ArrayType>()->getBaseType();
            Type::Size baseSize = curType->getSize();
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


void DataIntervalMapTest::testDataInterval()
{
    Prog            *prog = new Prog("test", &m_project);
    Module          *m    = prog->getOrInsertModule("test");
    UserProc        *proc = static_cast<UserProc *>(m->createFunction("test", Address(0x123)));
    DataIntervalMap dim(proc);

    proc->setSignature(Signature::instantiate(Machine::PENTIUM, CallConv::C, "testProc"));

    dim.insertItem(Address(0x00001000), "first", IntegerType::get(32, Sign::Signed));
    dim.insertItem(Address(0x00001004), "second", FloatType::get(64));
    QCOMPARE(dim.toString(), QString(
        "0x00001000-0x00001004 first int\n"
        "0x00001004-0x0000100c second double\n"));

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
    ct->addMember(IntegerType::get(16, Sign::Signed), "short1");
    ct->addMember(IntegerType::get(16, Sign::Signed), "short2");
    ct->addMember(IntegerType::get(32, Sign::Signed), "int1");
    ct->addMember(FloatType::get(32), "float1");
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


void DataIntervalMapTest::testDataIntervalOverlaps()
{
    Prog            *prog = new Prog("test", &m_project);
    Module          *m    = prog->getOrInsertModule("test");
    UserProc        *proc = static_cast<UserProc *>(m->createFunction("test", Address(0x00000100)));
    DataIntervalMap dim(proc);

    proc->setSignature(Signature::instantiate(Machine::PENTIUM, CallConv::C, "test"));

    dim.insertItem(Address(0x00001000), "firstInt", IntegerType::get(32, Sign::Signed));
    dim.insertItem(Address(0x00001004), "firstFloat", FloatType::get(32));
    dim.insertItem(Address(0x00001008), "secondInt", IntegerType::get(32, Sign::Signed));
    dim.insertItem(Address(0x0000100C), "secondFloat", FloatType::get(32));

    auto ct = CompoundType::get();
    ct->addMember(IntegerType::get(32, Sign::Signed), "int3");
    ct->addMember(FloatType::get(32), "float3");
    dim.insertItem(Address(0x00001010), "newStruct", ct);
    QCOMPARE(dim.toString(), QString(
        "0x00001000-0x00001004 firstInt int\n"
        "0x00001004-0x00001008 firstFloat float\n"
        "0x00001008-0x0000100c secondInt int\n"
        "0x0000100c-0x00001010 secondFloat float\n"
        "0x00001010-0x00001018 newStruct struct { int int3; float float3; }\n"));

    // First insert a new struct over the top of the existing middle pair
    auto ctu = CompoundType::get();
    ctu->addMember(IntegerType::get(32, Sign::Unknown), "newInt"); // This int has UNKNOWN sign
    ctu->addMember(FloatType::get(32), "newFloat");
    dim.insertItem(Address(0x00001008), "replacementStruct", ctu);

    const TypedVariable *var = dim.find(Address(0x1008));
    QCOMPARE(var->type->getCtype(), QString("struct { int newInt; float newFloat; }"));
    QCOMPARE(dim.toString(), QString(
        "0x00001000-0x00001004 firstInt int\n"
        "0x00001004-0x00001008 firstFloat float\n"
        "0x00001008-0x00001010 replacementStruct struct { int newInt; float newFloat; }\n"
        "0x00001010-0x00001018 newStruct struct { int int3; float float3; }\n"));


    // Attempt a weave; should fail
    auto ct3 = CompoundType::get();
    ct3->addMember(FloatType::get(32), "newFloat3");
    ct3->addMember(IntegerType::get(32, Sign::Unknown), "newInt3");
    QVERIFY(dim.insertItem(Address(0x00001004), "weaveStruct1", ct3) == dim.end());
    var = dim.find(Address(0x00001004));
    QVERIFY(var != nullptr);
    QCOMPARE(var->name, QString("firstFloat"));

    // Totally unaligned
    QVERIFY(dim.insertItem(Address(0x00001001), "weaveStruct2", ct3) == dim.end());
    var = dim.find(Address(0x000001001));
    QVERIFY(var != nullptr);
    QCOMPARE(var->name, QString("firstInt"));

    dim.insertItem(Address(0x00001004), "firstInt", IntegerType::get(32, Sign::Signed)); // Should fail
    var = dim.find(Address(0x00001004));
    QCOMPARE(var->name, QString("firstFloat"));

    // Set up three ints
    dim.deleteItem(Address(0x00001004));
    dim.deleteItem(Address(0x00001008));
    dim.insertItem(Address(0x00001004), "firstInt", IntegerType::get(32, Sign::Signed)); // Definitely signed
    dim.insertItem(Address(0x00001008), "firstInt", IntegerType::get(32, Sign::Unknown)); // Unknown signedess
    // then, add an array over the three integers
    auto at = ArrayType::get(IntegerType::get(32, Sign::Unknown), 3);
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


void DataIntervalMapTest::testIsClear()
{
    DataIntervalMap dim;

    QVERIFY(dim.isClear(Address::ZERO, Address(0x1000)));

    dim.insertItem(Address(0x1000), "first", IntegerType::get(32, Sign::Signed));

    QVERIFY(dim.isClear(Address::ZERO, Address(0x1000)));
    QVERIFY(!dim.isClear(Address(0x0800), Address(0x1004)));
    QVERIFY(!dim.isClear(Address(0x1000), Address(0x1004)));
    QVERIFY(!dim.isClear(Address(0x1002), Address(0x1010)));
    QVERIFY(dim.isClear(Address(0x1004), Address(0x1010)));
}


void DataIntervalMapTest::testFind()
{
    DataIntervalMap dim;

    QVERIFY(dim.find(Address(0x1000)) == nullptr);
    QVERIFY(dim.find_it(Address(0x1000)) == dim.end());

    dim.insertItem(Address(0x1000), "first", IntegerType::get(32, Sign::Signed));

    QVERIFY(dim.find(Address(0x800)) == nullptr);
    QVERIFY(dim.find(Address(0x1000)) != nullptr);
    QVERIFY(dim.find(Address(0x1002)) != nullptr);
    QVERIFY(dim.find(Address(0x1004)) == nullptr);

    QVERIFY(dim.find_it(Address(0x800)) == dim.end());
    QVERIFY(dim.find_it(Address(0x1000)) != dim.end());
    QVERIFY(dim.find_it(Address(0x1002)) != dim.end());
    QVERIFY(dim.find_it(Address(0x1004)) == dim.end());
}


void DataIntervalMapTest::testInsert()
{
    DataIntervalMap dim;

    DataIntervalMap::iterator it = dim.insertItem(Address(0x1000), "first", IntegerType::get(32, Sign::Signed));

    const TypedVariable& var = it->second;

    QVERIFY(var.baseAddr == Address(0x1000));
    QVERIFY(var.size == 32);
    QVERIFY(var.name == "first");
    QCOMPARE(var.type->toString(), IntegerType::get(32, Sign::Signed)->toString());

    // overlapped non-forced
    QVERIFY(dim.insertItem(Address(0x1002), "second", IntegerType::get(32, Sign::Signed)) == dim.end());
    QCOMPARE(dim.toString(), QString("0x00001000-0x00001004 first int\n"));

    // overlapped forced
    DataIntervalMap::iterator it2 = dim.insertItem(Address(0x1002), "second", IntegerType::get(32, Sign::Signed), true);
    QVERIFY(it2 != dim.end());
    QCOMPARE(dim.toString(), QString("0x00001002-0x00001006 second int\n"));
}


QTEST_GUILESS_MAIN(DataIntervalMapTest)
