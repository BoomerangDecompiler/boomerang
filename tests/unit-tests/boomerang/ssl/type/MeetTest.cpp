#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "MeetTest.h"


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


void MeetTest::testMeet()
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


#define TEST_MEET(firstName, secondName, firstOp, secondOp, result) \
    QTest::newRow(firstName " M " secondName) << SharedTypeWrapper(firstOp) << SharedTypeWrapper(secondOp) << SharedTypeWrapper(result); \
    QTest::newRow(secondName " M " firstName) << SharedTypeWrapper(secondOp) << SharedTypeWrapper(firstOp) << SharedTypeWrapper(result);


void MeetTest::testMeet_data()
{
    QTest::addColumn<SharedTypeWrapper>("firstOp");
    QTest::addColumn<SharedTypeWrapper>("secondOp");
    QTest::addColumn<SharedTypeWrapper>("tgtResult");

    std::shared_ptr<ArrayType> intArr(new ArrayType(IntegerType::get(64, Sign::Signed), 3));

    // 64 bit int
    TEST_MEET("i64", "v",   IntegerType::get(64, Sign::Signed), VoidType::get(),                      IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "i64", IntegerType::get(64, Sign::Signed), IntegerType::get(64, Sign::Signed),   IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "j64", IntegerType::get(64, Sign::Signed), IntegerType::get(64, Sign::Unknown),  IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "u64", IntegerType::get(64, Sign::Signed), IntegerType::get(64, Sign::Unsigned), IntegerType::get(64, Sign::Unknown));
    TEST_MEET("i64", "i32", IntegerType::get(64, Sign::Signed), IntegerType::get(32, Sign::Signed),   IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "j32", IntegerType::get(64, Sign::Signed), IntegerType::get(32, Sign::Unknown),  IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "u32", IntegerType::get(64, Sign::Signed), IntegerType::get(32, Sign::Unsigned), IntegerType::get(64, Sign::Unknown));
    TEST_MEET("i64", "i16", IntegerType::get(64, Sign::Signed), IntegerType::get(16, Sign::Signed),   IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "j16", IntegerType::get(64, Sign::Signed), IntegerType::get(16, Sign::Unknown),  IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "u16", IntegerType::get(64, Sign::Signed), IntegerType::get(16, Sign::Unsigned), IntegerType::get(64, Sign::Unknown));
    TEST_MEET("i64", "i8",  IntegerType::get(64, Sign::Signed), IntegerType::get(8, Sign::Signed),    IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "j8",  IntegerType::get(64, Sign::Signed), IntegerType::get(8, Sign::Unknown),   IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "u8",  IntegerType::get(64, Sign::Signed), IntegerType::get(8, Sign::Unsigned),  IntegerType::get(64, Sign::Unknown));
    TEST_MEET("i64", "s64", IntegerType::get(64, Sign::Signed), SizeType::get(64),                    IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "s32", IntegerType::get(64, Sign::Signed), SizeType::get(32),                    IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "s16", IntegerType::get(64, Sign::Signed), SizeType::get(16),                    IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "s8",  IntegerType::get(64, Sign::Signed), SizeType::get(8),                     IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "c",   IntegerType::get(64, Sign::Signed), CharType::get(),                      IntegerType::get(64, Sign::Signed));
    TEST_MEET("i64", "f64", IntegerType::get(64, Sign::Signed), FloatType::get(64),                   UnionType::get({ IntegerType::get(64, Sign::Signed), FloatType::get(64) }));
    TEST_MEET("i64", "f32", IntegerType::get(64, Sign::Signed), FloatType::get(32),                   UnionType::get({ IntegerType::get(64, Sign::Signed), FloatType::get(32) }));
    TEST_MEET("i64", "p",   IntegerType::get(64, Sign::Signed), PointerType::get(VoidType::get()),    UnionType::get({ IntegerType::get(64, Sign::Signed), PointerType::get(VoidType::get()) }));
    TEST_MEET("i64", "a3i64", IntegerType::get(64, Sign::Signed), intArr,                 intArr);

    TEST_MEET("j64", "i64", IntegerType::get(64, Sign::Unknown), IntegerType::get(64, Sign::Signed),  IntegerType::get(64, Sign::Signed));
    TEST_MEET("j64", "u64", IntegerType::get(64, Sign::Unknown), IntegerType::get(64, Sign::Unsigned), IntegerType::get(64, Sign::Unsigned));

    // size
    TEST_MEET("s32", "i32", SizeType::get(32), IntegerType::get(32, Sign::Signed), IntegerType::get(32, Sign::Signed));
    TEST_MEET("s32", "s16", SizeType::get(32), SizeType::get(16), SizeType::get(32));
    TEST_MEET("s16", "f32", SizeType::get(16), FloatType::get(32), UnionType::get({ SizeType::get(16), FloatType::get(32) }));
    TEST_MEET("s16", "v",   SizeType::get(16), VoidType::get(), SizeType::get(16));

    // pointer types
    TEST_MEET("pi32", "v",    PointerType::get(IntegerType::get(32, Sign::Signed)), VoidType::get(), PointerType::get(IntegerType::get(32, Sign::Signed)));
    TEST_MEET("pi32", "pv",   PointerType::get(IntegerType::get(32, Sign::Signed)), PointerType::get(VoidType::get()), PointerType::get(IntegerType::get(32, Sign::Signed)));
    TEST_MEET("pi32", "pu32", PointerType::get(IntegerType::get(32, Sign::Signed)), PointerType::get(IntegerType::get(32, Sign::Unsigned)), PointerType::get(IntegerType::get(32, Sign::Unknown)));
    TEST_MEET("pi32", "i32",  PointerType::get(IntegerType::get(32, Sign::Signed)), IntegerType::get(32, Sign::Signed),
              UnionType::get({ PointerType::get(IntegerType::get(32, Sign::Signed)), IntegerType::get(32, Sign::Signed) }));

    // Union types
    std::shared_ptr<UnionType> ut = UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) });

    TEST_MEET("u", "i32", ut, IntegerType::get(32, Sign::Signed), ut);
    TEST_MEET("u", "j32", ut, IntegerType::get(32, Sign::Unknown), ut);
    TEST_MEET("u", "f32", ut, FloatType::get(32), ut);
    TEST_MEET("u", "u32", ut, IntegerType::get(32, Sign::Unsigned), UnionType::get({ IntegerType::get(32, Sign::Unsigned), FloatType::get(32) }));
    TEST_MEET("u", "u",   ut, UnionType::get({ PointerType::get(VoidType::get()) }),
              UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32), PointerType::get(VoidType::get()) }));

    // floating point types
    TEST_MEET("f32", "f32", FloatType::get(32), FloatType::get(32), FloatType::get(32));
    TEST_MEET("f32", "v",   FloatType::get(32), VoidType::get(),    FloatType::get(32));
    TEST_MEET("f32", "f64", FloatType::get(32), FloatType::get(64), FloatType::get(64)); // Maybe this should result in a union
}

QTEST_GUILESS_MAIN(MeetTest)
