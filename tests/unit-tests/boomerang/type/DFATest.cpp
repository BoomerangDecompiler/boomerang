#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DFATest.h"


#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/UnionType.h"
#include "boomerang/ssl/type/CharType.h"

#include <QDebug>
#include <sstream>


void DfaTest::testMeet()
{
    QFETCH(SharedTypeWrapper, firstOp);
    QFETCH(SharedTypeWrapper, secondOp);
    QFETCH(SharedTypeWrapper, tgtResult);

    SharedType oldFirstOp  = firstOp->clone();
    SharedType oldSecondOp = secondOp->clone();

    bool       changed = false;
    SharedType result  = firstOp->meetWith(secondOp, changed, false);
    QCOMPARE(result->toString(), tgtResult.ty->toString()); // we are just comparing types here, not variable names
    QCOMPARE(changed, result->getCtype() != firstOp->getCtype());

    // verify that the source types themselves are not changed by meet
    QCOMPARE(firstOp.ty->toString(), oldFirstOp->toString());
    QCOMPARE(secondOp.ty->toString(), oldSecondOp->toString());
}


#define TEST_MEET(name, firstOp, secondOp, result) \
    QTest::newRow(name) << SharedTypeWrapper(firstOp) << SharedTypeWrapper(secondOp) << SharedTypeWrapper(result)


void DfaTest::testMeet_data()
{
    QTest::addColumn<SharedTypeWrapper>("firstOp");
    QTest::addColumn<SharedTypeWrapper>("secondOp");
    QTest::addColumn<SharedTypeWrapper>("tgtResult");

    std::shared_ptr<ArrayType> intArr(new ArrayType(IntegerType::get(64, Sign::Signed), 3));

    // 64 bit int
    TEST_MEET("i64 M v",   IntegerType::get(64, Sign::Signed), VoidType::get(),                      IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M i64", IntegerType::get(64, Sign::Signed), IntegerType::get(64, Sign::Signed),   IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M j64", IntegerType::get(64, Sign::Signed), IntegerType::get(64, Sign::Unknown),  IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M u64", IntegerType::get(64, Sign::Signed), IntegerType::get(64, Sign::Unsigned), IntegerType::get(64, Sign::Unknown));
    TEST_MEET("i64 M i32", IntegerType::get(64, Sign::Signed), IntegerType::get(32, Sign::Signed),   IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M j32", IntegerType::get(64, Sign::Signed), IntegerType::get(32, Sign::Unknown),  IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M u32", IntegerType::get(64, Sign::Signed), IntegerType::get(32, Sign::Unsigned), IntegerType::get(64, Sign::Unknown));
    TEST_MEET("i64 M i16", IntegerType::get(64, Sign::Signed), IntegerType::get(16, Sign::Signed),   IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M j16", IntegerType::get(64, Sign::Signed), IntegerType::get(16, Sign::Unknown),  IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M u16", IntegerType::get(64, Sign::Signed), IntegerType::get(16, Sign::Unsigned), IntegerType::get(64, Sign::Unknown));
    TEST_MEET("i64 M i8",  IntegerType::get(64, Sign::Signed), IntegerType::get(8, Sign::Signed),    IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M j8",  IntegerType::get(64, Sign::Signed), IntegerType::get(8, Sign::Unknown),   IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M u8",  IntegerType::get(64, Sign::Signed), IntegerType::get(8, Sign::Unsigned),  IntegerType::get(64, Sign::Unknown));
    TEST_MEET("i64 M s64", IntegerType::get(64, Sign::Signed), SizeType::get(64),                    IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M s32", IntegerType::get(64, Sign::Signed), SizeType::get(32),                    IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M s16", IntegerType::get(64, Sign::Signed), SizeType::get(16),                    IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M s8",  IntegerType::get(64, Sign::Signed), SizeType::get(8),                     IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64 M c",   IntegerType::get(64, Sign::Signed), CharType::get(),                      UnionType::get({ IntegerType::get(64, Sign::Signed), CharType::get() }));
    TEST_MEET("i64 M f64", IntegerType::get(64, Sign::Signed), FloatType::get(64),                   UnionType::get({ IntegerType::get(64, Sign::Signed), FloatType::get(64) }));
    TEST_MEET("i64 M f32", IntegerType::get(64, Sign::Signed), FloatType::get(32),                   UnionType::get({ IntegerType::get(64, Sign::Signed), FloatType::get(32) }));
    TEST_MEET("i64 M p",   IntegerType::get(64, Sign::Signed), PointerType::get(VoidType::get()),    UnionType::get({ IntegerType::get(64, Sign::Signed), PointerType::get(VoidType::get()) }));
    TEST_MEET("i64 M a3i64", IntegerType::get(64, Sign::Signed), intArr,                 intArr);

    TEST_MEET("j64 M i64", IntegerType::get(64, Sign::Unknown), IntegerType::get(64, Sign::Signed),  IntegerType::get(64, Sign::Signed));
    TEST_MEET("j64 M u64", IntegerType::get(64, Sign::Unknown), IntegerType::get(64, Sign::Unsigned), IntegerType::get(64, Sign::Unsigned));

    // size
    TEST_MEET("s32 M i32", SizeType::get(32), IntegerType::get(32, Sign::Signed), IntegerType::get(32, Sign::Signed));
    TEST_MEET("s32 M s16", SizeType::get(32), SizeType::get(16), SizeType::get(32));
    TEST_MEET("s16 M f32", SizeType::get(16), FloatType::get(32), UnionType::get({ SizeType::get(16), FloatType::get(32) }));
    TEST_MEET("s16 M v",   SizeType::get(16), VoidType::get(), SizeType::get(16));

    // pointer types
    TEST_MEET("pi32 M v",    PointerType::get(IntegerType::get(32, Sign::Signed)), VoidType::get(), PointerType::get(IntegerType::get(32, Sign::Signed)));
    TEST_MEET("pi32 M pv",   PointerType::get(IntegerType::get(32, Sign::Signed)), PointerType::get(VoidType::get()), PointerType::get(IntegerType::get(32, Sign::Signed)));
    TEST_MEET("pi32 M pu32", PointerType::get(IntegerType::get(32, Sign::Signed)), PointerType::get(IntegerType::get(32, Sign::Unsigned)), PointerType::get(IntegerType::get(32, Sign::Unknown)));
    TEST_MEET("pi32 M i32",  PointerType::get(IntegerType::get(32, Sign::Signed)), IntegerType::get(32, Sign::Signed),
              UnionType::get({ PointerType::get(IntegerType::get(32, Sign::Signed)), IntegerType::get(32, Sign::Signed) }));

    // Union types
    std::shared_ptr<UnionType> ut = UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) });

    TEST_MEET("u M i32", ut, IntegerType::get(32, Sign::Signed), ut);
    TEST_MEET("u M j32", ut, IntegerType::get(32, Sign::Unknown), ut);
    TEST_MEET("u M f32", ut, FloatType::get(32), ut);
    TEST_MEET("u M u32", ut, IntegerType::get(32, Sign::Unsigned), UnionType::get({ IntegerType::get(32, Sign::Unsigned), FloatType::get(32) }));
    TEST_MEET("u M u",   ut, UnionType::get({ PointerType::get(VoidType::get()) }),
              UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32), PointerType::get(VoidType::get()) }));

    // floating point types
    TEST_MEET("f32 M f32", FloatType::get(32), FloatType::get(32), FloatType::get(32));
    TEST_MEET("f32 M v",   FloatType::get(32), VoidType::get(),    FloatType::get(32));
    TEST_MEET("f32 M f64", FloatType::get(32), FloatType::get(64), FloatType::get(64)); // Maybe this should result in a union
}

QTEST_GUILESS_MAIN(DfaTest)
