#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DfaTest.h"


/**
 * \file DfaTest.cpp
 * Provides the implementation for the DfaTest class, which
 * tests the data flow based type analysis code
 */

#include "boomerang/core/Boomerang.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/SizeType.h"
#include "boomerang/type/type/FloatType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/type/type/UnionType.h"
#include "boomerang/type/type/CharType.h"
#include "boomerang/util/Log.h"

#include <QtCore/QDebug>
#include <sstream>


/// HACK to work around limitations of QMetaType
struct SharedTypeWrapper
{
public:
    SharedTypeWrapper() : ty(nullptr) {}
    SharedTypeWrapper(SharedType _ty) : ty(_ty) {}

    SharedType operator->() { return ty; }
    SharedType operator*() { return ty; }
    operator SharedType() { return ty; }

public:
    SharedType ty;
};

Q_DECLARE_METATYPE(SharedTypeWrapper);


void DfaTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE);

    qRegisterMetaType<SharedTypeWrapper>();
}


void DfaTest::testMeetInt()
{

    QFETCH(SharedTypeWrapper, firstOp);
    QFETCH(SharedTypeWrapper, secondOp);
    QFETCH(SharedTypeWrapper, tgtResult);

    SharedType oldFirstOp = firstOp->clone();
    SharedType oldSecondOp = secondOp->clone();

    bool changed = false;
    SharedType result = firstOp->meetWith(secondOp, changed, false);
    QCOMPARE(*result, *(tgtResult.ty));
    QCOMPARE(changed, result->getCtype() != firstOp->getCtype());

    // verify that the source types themselves are not changed by meet
    QCOMPARE(firstOp->getCtype(), oldFirstOp->getCtype());
    QCOMPARE(secondOp->getCtype(), oldSecondOp->getCtype());
}

#define TEST_MEET(name, firstOp, secondOp, result) \
    QTest::newRow(name) << SharedTypeWrapper(firstOp) << SharedTypeWrapper(secondOp) << SharedTypeWrapper(result)


void DfaTest::testMeetInt_data()
{
    QTest::addColumn<SharedTypeWrapper>("firstOp");
    QTest::addColumn<SharedTypeWrapper>("secondOp");
    QTest::addColumn<SharedTypeWrapper>("tgtResult");

    std::shared_ptr<ArrayType> intArr(new ArrayType(IntegerType::get(64, 1), 3));

    // 64 bit int
    TEST_MEET("i64 M v",   IntegerType::get(64, 1), VoidType::get(),          IntegerType::get(64, 1));
    TEST_MEET("i64 M i64", IntegerType::get(64, 1), IntegerType::get(64, 1),  IntegerType::get(64, 1));
    TEST_MEET("i64 M j64", IntegerType::get(64, 1), IntegerType::get(64, 0),  IntegerType::get(64, 1));
    TEST_MEET("i64 M u64", IntegerType::get(64, 1), IntegerType::get(64, -1), IntegerType::get(64, 0));
    TEST_MEET("i64 M i32", IntegerType::get(64, 1), IntegerType::get(64, 1),  IntegerType::get(64, 1));
    TEST_MEET("i64 M j32", IntegerType::get(64, 1), IntegerType::get(64, 0),  IntegerType::get(64, 1));
    TEST_MEET("i64 M u32", IntegerType::get(64, 1), IntegerType::get(64, -1), IntegerType::get(64, 0));
    TEST_MEET("i64 M i16", IntegerType::get(64, 1), IntegerType::get(64, 1),  IntegerType::get(64, 1));
    TEST_MEET("i64 M j16", IntegerType::get(64, 1), IntegerType::get(64, 0),  IntegerType::get(64, 1));
    TEST_MEET("i64 M u16", IntegerType::get(64, 1), IntegerType::get(64, -1), IntegerType::get(64, 0));
    TEST_MEET("i64 M i8",  IntegerType::get(64, 1), IntegerType::get(64, 1),  IntegerType::get(64, 1));
    TEST_MEET("i64 M j8",  IntegerType::get(64, 1), IntegerType::get(64, 0),  IntegerType::get(64, 1));
    TEST_MEET("i64 M u8",  IntegerType::get(64, 1), IntegerType::get(64, -1), IntegerType::get(64, 0));
    TEST_MEET("i64 M s64", IntegerType::get(64, 1), SizeType::get(64),        IntegerType::get(64, 1));
    TEST_MEET("i64 M s32", IntegerType::get(64, 1), SizeType::get(32),        IntegerType::get(64, 1));
    TEST_MEET("i64 M s16", IntegerType::get(64, 1), SizeType::get(16),        IntegerType::get(64, 1));
    TEST_MEET("i64 M s8",  IntegerType::get(64, 1), SizeType::get(8),         IntegerType::get(64, 1));
    TEST_MEET("i64 M c",   IntegerType::get(64, 1), CharType::get(),          UnionType::get({ IntegerType::get(64, 1), CharType::get() }));
    TEST_MEET("i64 M f64", IntegerType::get(64, 1), FloatType::get(64),       UnionType::get({ IntegerType::get(64, 1), FloatType::get(64) }));
    TEST_MEET("i64 M f32", IntegerType::get(64, 1), FloatType::get(32),       UnionType::get({ IntegerType::get(64, 1), FloatType::get(32) }));
    TEST_MEET("i64 M p",   IntegerType::get(64, 1), PointerType::get(VoidType::get()), UnionType::get({ IntegerType::get(64, 1), PointerType::get(VoidType::get()) }));
    TEST_MEET("i64 M a3i64", IntegerType::get(64, 1), intArr,                 intArr);

    TEST_MEET("j64 M i64", IntegerType::get(64, 0), IntegerType::get(64, 1),  IntegerType::get(64, 1));
    TEST_MEET("j64 M u64", IntegerType::get(64, 0), IntegerType::get(64, -1), IntegerType::get(64, -1));

    // size
    TEST_MEET("s32 M i32", SizeType::get(32), IntegerType::get(32, 1), IntegerType::get(32, 1));
    TEST_MEET("s32 M s16", SizeType::get(32), SizeType::get(16), SizeType::get(32));
    TEST_MEET("s16 M f32", SizeType::get(16), FloatType::get(32), UnionType::get({ SizeType::get(16), FloatType::get(32) }));
    TEST_MEET("s16 M v",   SizeType::get(16), VoidType::get(), SizeType::get(16));

    // pointer types
    TEST_MEET("pi32 M v",    PointerType::get(IntegerType::get(32, 1)), VoidType::get(), PointerType::get(IntegerType::get(32, 1)));
    TEST_MEET("pi32 M pv",   PointerType::get(IntegerType::get(32, 1)), PointerType::get(VoidType::get()), PointerType::get(IntegerType::get(32, 1)));
    TEST_MEET("pi32 M pu32", PointerType::get(IntegerType::get(32, 1)), PointerType::get(IntegerType::get(32, -1)), PointerType::get(IntegerType::get(32, 0)));
    TEST_MEET("pi32 M i32",  PointerType::get(IntegerType::get(32, 1)), IntegerType::get(32, 1), UnionType::get({ PointerType::get(IntegerType::get(32, 1)), IntegerType::get(32, 1) }));
}


void DfaTest::testMeetUnion()
{
	auto i32  = IntegerType::get(32, 1);
	auto j32  = IntegerType::get(32, 0);
	auto u32  = IntegerType::get(32, -1);
	auto u1   = UnionType::get();
	auto u2   = UnionType::get();
	auto flt  = FloatType::get(32);
	auto flt2 = FloatType::get(32);

	u1->addType(i32, "bow");
	u1->addType(flt, "wow");
	u2->addType(flt2, "gorm");
	QCOMPARE(u1->getCtype(), QString("union { float wow; int bow; }"));

	bool ch  = false;
	auto res = u1->meetWith(j32, ch, false);
	QVERIFY(ch == false);
	QCOMPARE(res->getCtype(), QString("union { float wow; int bow; }"));

	ch  = false;
	res = u1->meetWith(flt, ch, false);
	QVERIFY(ch == false);
	QCOMPARE(res->getCtype(), QString("union { float wow; int bow; }"));

	res = u1->meetWith(u2, ch, false);
	QVERIFY(ch == false);
	QCOMPARE(u1->getCtype(), QString("union { float wow; int bow; }"));

	// Note: this test relies on the int in the union having signedness 1
	res = u1->meetWith(u32, ch, false);
	QVERIFY(ch == true);
	QCOMPARE(u1->getCtype(), QString("union { /*signed?*/int bow; float wow; }"));
}


QTEST_MAIN(DfaTest)
