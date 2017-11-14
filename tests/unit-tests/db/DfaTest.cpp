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


/// HACK to work around limitations of QMetaType which does not allow templates
struct SharedTypeWrapper
{
public:
    SharedTypeWrapper()
        : ty(nullptr) {}
    SharedTypeWrapper(SharedType _ty)
        : ty(_ty) {}

    SharedType operator->() { return ty; }
    SharedType operator*() { return ty; }
    operator SharedType() { return ty; }

public:
    SharedType ty;
};

Q_DECLARE_METATYPE(SharedTypeWrapper)


void DfaTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");

    qRegisterMetaType<SharedTypeWrapper>();
}


void DfaTest::testMeet()
{
    QFETCH(SharedTypeWrapper, firstOp);
    QFETCH(SharedTypeWrapper, secondOp);
    QFETCH(SharedTypeWrapper, tgtResult);

    SharedType oldFirstOp  = firstOp->clone();
    SharedType oldSecondOp = secondOp->clone();

    bool       changed = false;
    SharedType result  = firstOp->meetWith(secondOp, changed, false);
    QCOMPARE(*result, *(tgtResult.ty)); // we are just comparing types here, not variable names
    QCOMPARE(changed, result->getCtype() != firstOp->getCtype());

    // verify that the source types themselves are not changed by meet
    QCOMPARE(*(firstOp.ty), *oldFirstOp);
    QCOMPARE(*(secondOp.ty), *oldSecondOp);
}


#define TEST_MEET(name, firstOp, secondOp, result) \
    QTest::newRow(name) << SharedTypeWrapper(firstOp) << SharedTypeWrapper(secondOp) << SharedTypeWrapper(result)


void DfaTest::testMeet_data()
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
    TEST_MEET("i64 M i32", IntegerType::get(64, 1), IntegerType::get(32, 1),  IntegerType::get(64, 1));
    TEST_MEET("i64 M j32", IntegerType::get(64, 1), IntegerType::get(32, 0),  IntegerType::get(64, 1));
    TEST_MEET("i64 M u32", IntegerType::get(64, 1), IntegerType::get(32, -1), IntegerType::get(64, 0));
    TEST_MEET("i64 M i16", IntegerType::get(64, 1), IntegerType::get(16, 1),  IntegerType::get(64, 1));
    TEST_MEET("i64 M j16", IntegerType::get(64, 1), IntegerType::get(16, 0),  IntegerType::get(64, 1));
    TEST_MEET("i64 M u16", IntegerType::get(64, 1), IntegerType::get(16, -1), IntegerType::get(64, 0));
    TEST_MEET("i64 M i8",  IntegerType::get(64, 1), IntegerType::get(8, 1),   IntegerType::get(64, 1));
    TEST_MEET("i64 M j8",  IntegerType::get(64, 1), IntegerType::get(8, 0),   IntegerType::get(64, 1));
    TEST_MEET("i64 M u8",  IntegerType::get(64, 1), IntegerType::get(8, -1),  IntegerType::get(64, 0));
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

    // Union types
    std::shared_ptr<UnionType> ut = UnionType::get({ IntegerType::get(32, 1), FloatType::get(32) });

    TEST_MEET("u M i32", ut, IntegerType::get(32, 1), ut);
    TEST_MEET("u M j32", ut, IntegerType::get(32, 0), ut);
    TEST_MEET("u M f32", ut, FloatType::get(32), ut);
    TEST_MEET("u M u32", ut, IntegerType::get(32, -1), UnionType::get({ IntegerType::get(32, 0), FloatType::get(32) }));
    TEST_MEET("u M u",   ut, UnionType::get({ PointerType::get(VoidType::get()) }), UnionType::get({ IntegerType::get(32, 1), FloatType::get(32), PointerType::get(VoidType::get()) }));

    // floating point types
    TEST_MEET("f32 M f32", FloatType::get(32), FloatType::get(32), FloatType::get(32));
    TEST_MEET("f32 M v",   FloatType::get(32), VoidType::get(),    FloatType::get(32));
    TEST_MEET("f32 M f64", FloatType::get(32), FloatType::get(64), FloatType::get(64)); // Maybe this should result in a union
}

QTEST_MAIN(DfaTest)
