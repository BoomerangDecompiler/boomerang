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
#include "boomerang/ssl/type/BooleanType.h"
#include "boomerang/ssl/type/CharType.h"
#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/FuncType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/NamedType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/UnionType.h"
#include "boomerang/ssl/type/VoidType.h"

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
    QCOMPARE(result->toString(), tgtResult.ty->toString());
    QCOMPARE(changed, result->getCtype() != firstOp->getCtype());

    // verify that the source types themselves are not changed by meet
    QCOMPARE(firstOp.ty->toString(), oldFirstOp->toString());
    QCOMPARE(secondOp.ty->toString(), oldSecondOp->toString());
}


#define TEST_MEET(firstOp, secondOp, result) \
    QTest::newRow(qPrintable(QString("%1 M %2").arg(firstOp->getCtype()).arg(secondOp->getCtype()))) << SharedTypeWrapper(firstOp) << SharedTypeWrapper(secondOp) << SharedTypeWrapper(result); \
    QTest::newRow(qPrintable(QString("%1 M %2").arg(secondOp->getCtype()).arg(firstOp->getCtype()))) << SharedTypeWrapper(secondOp) << SharedTypeWrapper(firstOp) << SharedTypeWrapper(result);


void MeetTest::testMeet_data()
{
    QTest::addColumn<SharedTypeWrapper>("firstOp");
    QTest::addColumn<SharedTypeWrapper>("secondOp");
    QTest::addColumn<SharedTypeWrapper>("tgtResult");

    // void
    TEST_MEET(
        VoidType::get(),
        VoidType::get(),
        VoidType::get());

    TEST_MEET(
        VoidType::get(),
        ArrayType::get(VoidType::get()),
        ArrayType::get(VoidType::get()));

    TEST_MEET(
        VoidType::get(),
        BooleanType::get(),
        BooleanType::get());

    TEST_MEET(
        VoidType::get(),
        CharType::get(),
        CharType::get());

    TEST_MEET(
        VoidType::get(),
        CompoundType::get(),
        CompoundType::get());

    TEST_MEET(
        VoidType::get(),
        FloatType::get(32),
        FloatType::get(32));

    TEST_MEET(
        VoidType::get(),
        FloatType::get(64),
        FloatType::get(64));

    TEST_MEET(
        VoidType::get(),
        FuncType::get(),
        FuncType::get());

    TEST_MEET(
        VoidType::get(),
        IntegerType::get(32, Sign::Signed),
        IntegerType::get(32, Sign::Signed));

    TEST_MEET(
        VoidType::get(),
        IntegerType::get(32, Sign::Unknown),
        IntegerType::get(32, Sign::Unknown));

    TEST_MEET(
        VoidType::get(),
        IntegerType::get(16, Sign::Unsigned),
        IntegerType::get(16, Sign::Unsigned));

    TEST_MEET(
        VoidType::get(),
        NamedType::get("foo"),
        NamedType::get("foo"));

    TEST_MEET(
        VoidType::get(),
        PointerType::get(VoidType::get()),
        PointerType::get(VoidType::get()));

    TEST_MEET(
        VoidType::get(),
        SizeType::get(16),
        SizeType::get(16));

    TEST_MEET(
        VoidType::get(),
        UnionType::get({ IntegerType::get(32, Sign::Signed), VoidType::get() }),
        IntegerType::get(32, Sign::Signed));


    // Array
    TEST_MEET(
        ArrayType::get(VoidType::get()),
        ArrayType::get(VoidType::get()),
        ArrayType::get(VoidType::get()));

    TEST_MEET(
        ArrayType::get(VoidType::get()),
        ArrayType::get(IntegerType::get(32, Sign::Signed)),
        ArrayType::get(IntegerType::get(32, Sign::Signed)));

    TEST_MEET(
        ArrayType::get(IntegerType::get(32, Sign::Signed)),
        ArrayType::get(IntegerType::get(32, Sign::Unsigned)),
        ArrayType::get(IntegerType::get(32, Sign::Unknown)));

    TEST_MEET(
        ArrayType::get(VoidType::get()),
        ArrayType::get(VoidType::get(), 10),
        ArrayType::get(VoidType::get(), 10));

    TEST_MEET(
        ArrayType::get(IntegerType::get(16, Sign::Unsigned)),
        IntegerType::get(16, Sign::Unsigned),
        ArrayType::get(IntegerType::get(16, Sign::Unsigned)));

    TEST_MEET(
        ArrayType::get(IntegerType::get(8, Sign::Unsigned), 4),
        FloatType::get(32),
        UnionType::get({ ArrayType::get(IntegerType::get(8, Sign::Unsigned), 4), FloatType::get(32) }));


    // Boolean
    TEST_MEET(
        BooleanType::get(),
        BooleanType::get(),
        BooleanType::get());

    TEST_MEET(
        BooleanType::get(),
        CharType::get(),
        UnionType::get({ BooleanType::get(), CharType::get() }));


    // Char
    TEST_MEET(
        CharType::get(),
        CharType::get(),
        CharType::get());

    TEST_MEET(
        CharType::get(),
        IntegerType::get(8, Sign::Unknown),
        IntegerType::get(8, Sign::Unknown));

    TEST_MEET(
        CharType::get(),
        SizeType::get(8),
        CharType::get());

    TEST_MEET(
        CharType::get(),
        SizeType::get(32),
        CharType::get());


    // Compound
    TEST_MEET(
        CompoundType::get(),
        CompoundType::get(),
        CompoundType::get());


    // Float
    TEST_MEET(
        FloatType::get(64),
        FloatType::get(64),
        FloatType::get(64));

    TEST_MEET(
        FloatType::get(64),
        FloatType::get(32),
        FloatType::get(64));

    TEST_MEET(
        FloatType::get(32),
        SizeType::get(32),
        FloatType::get(32));

    TEST_MEET(
        FloatType::get(32),
        SizeType::get(64),
        UnionType::get({ FloatType::get(32), SizeType::get(64) }));


    // Func
    TEST_MEET(
        FuncType::get(),
        FuncType::get(),
        FuncType::get());


    // Integer
    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(64, Sign::Unknown),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(64, Sign::Unsigned),
        IntegerType::get(64, Sign::Unknown));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(32, Sign::Signed),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(32, Sign::Unknown),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(32, Sign::Unsigned),
        IntegerType::get(64, Sign::Unknown));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(16, Sign::Signed),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(16, Sign::Unknown),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(16, Sign::Unsigned),
        IntegerType::get(64, Sign::Unknown));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(8, Sign::Signed),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(8, Sign::Unknown),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(8, Sign::Unsigned),
        IntegerType::get(64, Sign::Unknown));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        SizeType::get(64),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        SizeType::get(32),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        SizeType::get(16),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        SizeType::get(8),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        CharType::get(),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        FloatType::get(64),
        UnionType::get({ IntegerType::get(64, Sign::Signed), FloatType::get(64) }));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        FloatType::get(32),
        UnionType::get({ IntegerType::get(64, Sign::Signed), FloatType::get(32) }));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        PointerType::get(VoidType::get()),
        UnionType::get({ IntegerType::get(64, Sign::Signed), PointerType::get(VoidType::get()) }));

    TEST_MEET(
        IntegerType::get(64, Sign::Signed),
        ArrayType::get(IntegerType::get(64, Sign::Signed), 3),
        ArrayType::get(IntegerType::get(64, Sign::Signed), 3));

    TEST_MEET(
        IntegerType::get(64, Sign::Unknown),
        IntegerType::get(64, Sign::Signed),
        IntegerType::get(64, Sign::Signed));

    TEST_MEET(
        IntegerType::get(64, Sign::Unknown),
        IntegerType::get(64, Sign::Unsigned),
        IntegerType::get(64, Sign::Unsigned));


    // size
    TEST_MEET(
        SizeType::get(32),
        IntegerType::get(32, Sign::Signed),
        IntegerType::get(32, Sign::Signed));

    TEST_MEET(
        SizeType::get(32),
        SizeType::get(16),
        SizeType::get(32));

    TEST_MEET(
        SizeType::get(16),
        FloatType::get(32),
        UnionType::get({ SizeType::get(16), FloatType::get(32) }));


    // Pointer
    TEST_MEET(
        PointerType::get(IntegerType::get(32, Sign::Signed)),
        PointerType::get(VoidType::get()),
        PointerType::get(IntegerType::get(32, Sign::Signed)));

    TEST_MEET(
        PointerType::get(IntegerType::get(32, Sign::Signed)),
        PointerType::get(IntegerType::get(32, Sign::Unsigned)),
        PointerType::get(IntegerType::get(32, Sign::Unknown)));

    TEST_MEET(
        PointerType::get(IntegerType::get(32, Sign::Signed)),
        IntegerType::get(32, Sign::Signed),
        UnionType::get({ PointerType::get(IntegerType::get(32, Sign::Signed)), IntegerType::get(32, Sign::Signed) }));

    TEST_MEET(
        PointerType::get(VoidType::get()),
        SizeType::get(32),
        PointerType::get(VoidType::get()));


    // Union
    TEST_MEET(
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }),
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }),
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }));

    TEST_MEET(
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }),
        IntegerType::get(32, Sign::Signed),
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }));

    TEST_MEET(
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }),
        IntegerType::get(32, Sign::Unknown),
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }));

    TEST_MEET(
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }),
        FloatType::get(32),
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }));

    TEST_MEET(
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }),
        IntegerType::get(32, Sign::Unsigned),
        UnionType::get({ IntegerType::get(32, Sign::Unsigned), FloatType::get(32) }));

    TEST_MEET(
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }),
        UnionType::get({ PointerType::get(VoidType::get()) }),
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32), PointerType::get(VoidType::get()) }));

    TEST_MEET(
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) }),
        PointerType::get(UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32) })),
        UnionType::get({ IntegerType::get(32, Sign::Signed), FloatType::get(32), PointerType::get(VoidType::get()) }));
}

QTEST_GUILESS_MAIN(MeetTest)
