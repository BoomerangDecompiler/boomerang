#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementHelperTest.h"


#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/statements/StatementHelper.h"


Q_DECLARE_METATYPE(BranchType)

#define TEST_COND(name, oldCond, bt, f, expectedResult) \
    do { \
        QTest::newRow(name) << SharedExpWrapper(oldCond) << bt << f << SharedExpWrapper(expectedResult); \
    } while (false)


SharedExp makeFlagCallArgs()
{
    return Terminal::get(opNil);
}


template<typename Arg, typename... Args>
SharedExp makeFlagCallArgs(Arg arg, Args... args)
{
    return Binary::get(opList, arg, makeFlagCallArgs(args...));
}


template<typename... Args>
SharedExp makeFlagCall(const QString &name, Args... args)
{
    return Binary::get(opFlagCall, Const::get(name), makeFlagCallArgs(args...));
}


void StatementHelperTest::testCondToRelational()
{
    QFETCH(SharedExpWrapper, oldCondExp);
    QFETCH(BranchType, jtCond);
    QFETCH(bool, isFloat);
    QFETCH(SharedExpWrapper, newCondExp);

    SharedExp e  = oldCondExp->clone();
    const bool f = condToRelational(e, jtCond);

    QCOMPARE(f, isFloat);
    QCOMPARE(*e, **newCondExp);
}


void StatementHelperTest::testCondToRelational_data()
{
    const SharedExp null = Const::get(0);
    const SharedExp eax = Location::regOf(REG_X86_EAX);
    const SharedExp ecx = Location::regOf(REG_X86_ECX);
    const SharedExp edx = Location::regOf(REG_X86_EDX);

    const SharedExp st0 = Location::regOf(REG_X86_ST0);
    const SharedExp st1 = Location::regOf(REG_X86_ST1);

    QTest::addColumn<SharedExpWrapper>("oldCondExp");
    QTest::addColumn<BranchType>("jtCond");
    QTest::addColumn<bool>("isFloat");
    QTest::addColumn<SharedExpWrapper>("newCondExp");

    TEST_COND("%ecx, je",
        ecx,
        BranchType::JE,
        false,
        ecx
    );

    TEST_COND("%ecx != 0, jne",
        Binary::get(opNotEqual, ecx, null),
        BranchType::JNE,
        false,
        Binary::get(opNotEqual, ecx, null)
    );

    //
    // SUBFLAGS
    //
    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), je",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JE,
        false,
        Binary::get(opEquals, eax, ecx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jne",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JNE,
        false,
        Binary::get(opNotEqual, eax, ecx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jsl",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JSL,
        false,
        Binary::get(opLess, eax, ecx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jsle",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JSLE,
        false,
        Binary::get(opLessEq, eax, ecx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jsge",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JSGE,
        false,
        Binary::get(opGtrEq, eax, ecx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jsg",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JSG,
        false,
        Binary::get(opGtr, eax, ecx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jul",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JUL,
        false,
        Binary::get(opLessUns, eax, ecx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jule",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JULE,
        false,
        Binary::get(opLessEqUns, eax, ecx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), juge",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JUGE,
        false,
        Binary::get(opGtrEqUns, eax, ecx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jug",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JUG,
        false,
        Binary::get(opGtrUns, eax, ecx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jmi",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JMI,
        false,
        Binary::get(opLess, edx, null)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jpos",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JPOS,
        false,
        Binary::get(opGtrEq, edx, null)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jof",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JOF,
        false,
        makeFlagCall("SUBFLAGS", eax, ecx, edx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jnof",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JNOF,
        false,
        makeFlagCall("SUBFLAGS", eax, ecx, edx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jpar",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JPAR,
        false,
        makeFlagCall("SUBFLAGS", eax, ecx, edx)
    );

    TEST_COND("SUBFLAGS(%eax, %ecx, %edx), jnpar",
        makeFlagCall("SUBFLAGS", eax, ecx, edx),
        BranchType::JNPAR,
        false,
        makeFlagCall("SUBFLAGS", eax, ecx, edx)
    );

    //
    // SUBFLAGSNL
    //
    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), je",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JE,
        false,
        Binary::get(opEquals, eax, ecx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jne",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JNE,
        false,
        Binary::get(opNotEqual, eax, ecx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jsl",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JSL,
        false,
        Binary::get(opLessUns, eax, ecx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jsle",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JSLE,
        false,
        Binary::get(opLessEqUns, eax, ecx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jsge",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JSGE,
        false,
        Binary::get(opGtrEqUns, eax, ecx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jsg",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JSG,
        false,
        Binary::get(opGtrUns, eax, ecx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jul",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JUL,
        false,
        Binary::get(opLessUns, eax, ecx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jule",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JULE,
        false,
        Binary::get(opLessEqUns, eax, ecx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), juge",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JUGE,
        false,
        Binary::get(opGtrEqUns, eax, ecx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jug",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JUG,
        false,
        Binary::get(opGtrUns, eax, ecx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jmi",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JMI,
        false,
        Binary::get(opLess, edx, null)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jpos",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JPOS,
        false,
        Binary::get(opGtrEq, edx, null)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jof",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JOF,
        false,
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jnof",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JNOF,
        false,
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jpar",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JPAR,
        false,
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx)
    );

    TEST_COND("SUBFLAGSNL(%eax, %ecx, %edx), jnpar",
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx),
        BranchType::JNPAR,
        false,
        makeFlagCall("SUBFLAGSNL", eax, ecx, edx)
    );

    //
    // LOGICALFLAGS
    //
    TEST_COND("LOGICALFLAGS(%eax), je",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JE,
        false,
        Binary::get(opEquals, eax, null)
    );

    TEST_COND("LOGICALFLAGS(%eax), jne",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JNE,
        false,
        Binary::get(opNotEqual, eax, null)
    );

    TEST_COND("LOGICALFLAGS(%eax), jmi",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JMI,
        false,
        Binary::get(opLess, eax, null)
    );

    TEST_COND("LOGICALFLAGS(%eax), jpos",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JPOS,
        false,
        Binary::get(opGtrEq, eax, null)
    );

    TEST_COND("LOGICALFLAGS(%eax), jsl",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JSL,
        false,
        Binary::get(opLess, eax, null)
    );

    TEST_COND("LOGICALFLAGS(%eax), jsle",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JSLE,
        false,
        Binary::get(opLessEq, eax, null)
    );

    TEST_COND("LOGICALFLAGS(%eax), jsge",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JSGE,
        false,
        Binary::get(opGtrEq, eax, null)
    );

    TEST_COND("LOGICALFLAGS(%eax), jsg",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JSG,
        false,
        Binary::get(opGtr, eax, null)
    );

    TEST_COND("LOGICALFLAGS(%eax), jul",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JUL,
        false,
        Binary::get(opLessUns, eax, null)
    );

    TEST_COND("LOGICALFLAGS(%eax), jule",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JULE,
        false,
        Binary::get(opLessEqUns, eax, null)
    );

    TEST_COND("LOGICALFLAGS(%eax), juge",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JUGE,
        false,
        Binary::get(opGtrEqUns, eax, null)
    );

    TEST_COND("LOGICALFLAGS(%eax), jug",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JUG,
        false,
        Binary::get(opGtrUns, eax, null)
    );

    TEST_COND("LOGICALFLAGS8(tmp), jpar",
        makeFlagCall("LOGICALFLAGS8", Location::tempOf(Const::get("tmp"))),
        BranchType::JPAR,
        false,
        makeFlagCall("LOGICALFLAGS8", Location::tempOf(Const::get("tmp")))
    );

    TEST_COND("LOGICALFLAGS8(tmp{-}), jpar",
        makeFlagCall("LOGICALFLAGS8", RefExp::get(Location::tempOf(Const::get("tmp")), nullptr)),
        BranchType::JPAR,
        false,
        makeFlagCall("LOGICALFLAGS8", RefExp::get(Location::tempOf(Const::get("tmp")), nullptr))
    );

    TEST_COND("LOGICALFLAGS8(%ecx), jpar",
        makeFlagCall("LOGICALFLAGS8", ecx),
        BranchType::JPAR,
        false,
        makeFlagCall("LOGICALFLAGS8", ecx)
    );

    TEST_COND("LOGICALFLAGS8(%eax & %ecx), jpar",
        makeFlagCall("LOGICALFLAGS8", Binary::get(opBitAnd, eax, ecx)),
        BranchType::JPAR,
        false,
        makeFlagCall("LOGICALFLAGS8", Binary::get(opBitAnd, eax, ecx))
    );

    TEST_COND("LOGICALFLAGS8(%eax & 0x41), jpar",
        makeFlagCall("LOGICALFLAGS8", Binary::get(opBitAnd, eax, Const::get(0x41))),
        BranchType::JPAR,
        false,
        makeFlagCall("LOGICALFLAGS8", Binary::get(opBitAnd, eax, Const::get(0x41)))
    );

    TEST_COND("LOGICALFLAGS8(FOO(0) & 0x41), jpar",
        makeFlagCall("LOGICALFLAGS8",
                     Binary::get(opBitAnd,
                                 makeFlagCall("FOO", Const::get(0)),
                                 Const::get(0x41))),
        BranchType::JPAR,
        false,
        makeFlagCall("LOGICALFLAGS8",
                     Binary::get(opBitAnd,
                                 makeFlagCall("FOO", Const::get(0)),
                                 Const::get(0x41)))
    );

    TEST_COND("LOGICALFLAGS8(SETFFLAGS(0, 1) & 0x42), jpar",
        makeFlagCall("LOGICALFLAGS8",
                     Binary::get(opBitAnd,
                                 makeFlagCall("SETFFLAGS", Const::get(0), Const::get(1)),
                                 Const::get(0x42))),
        BranchType::JPAR,
        false,
        makeFlagCall("LOGICALFLAGS8",
                     Binary::get(opBitAnd,
                                 makeFlagCall("SETFFLAGS", Const::get(0), Const::get(1)),
                                 Const::get(0x42)))
    );

    TEST_COND("LOGICALFLAGS8(SETFFLAGS(st0) & 0x40), jpar",
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0),
                                Const::get(0x40))),
        BranchType::JPAR,
        false,
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0),
                                Const::get(0x40)))
    );

    TEST_COND("LOGICALFLAGS8(SETFFLAGS(st0, st1, st0) & 0x40), jpar",
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1, st0),
                                Const::get(0x40))),
        BranchType::JPAR,
        false,
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1, st0),
                                Const::get(0x40)))
    );

    TEST_COND("LOGICALFLAGS8(SETFFLAGS(st0, st1) & 0x0), jpar",
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x00))),
        BranchType::JPAR,
        false,
        makeFlagCall("LOGICALFLAGS8", null)
    );

    TEST_COND("LOGICALFLAGS8(SETFFLAGS(st0, st1) & 0x0), jnpar",
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x0))),
        BranchType::JNPAR,
        false,
        makeFlagCall("LOGICALFLAGS8", null)
    );

    TEST_COND("LOGICALFLAGS8(SETFFLAGS(st0, st1) & 0x1), jpar",
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x1))),
        BranchType::JPAR,
        true,
        Binary::get(opLess, st0, st1)
    );

    TEST_COND("LOGICALFLAGS8(SETFFLAGS(st0, st1) & 0x1), jnpar",
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x1))),
        BranchType::JNPAR,
        true,
        Binary::get(opGtrEq, st0, st1)
    );

    TEST_COND("LOGICALFLAGS8(SETFFLAGS(st0, st1) & 0x40), jpar",
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x40))),
        BranchType::JPAR,
        true,
        Binary::get(opEquals, st0, st1)
    );

    TEST_COND("LOGICALFLAGS8(SETFFLAGS(st0, st1) & 0x40), jnpar",
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x40))),
        BranchType::JNPAR,
        true,
        Binary::get(opEquals, st0, st1)
    );

    TEST_COND("LOGICALFLAGS8(SETFFLAGS(st0, st1) & 0x41), jpar",
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x41))),
        BranchType::JPAR,
        true,
        Binary::get(opLessEq, st0, st1)
    );

    TEST_COND("LOGICALFLAGS8(SETFFLAGS(st0, st1) & 0x41), jnpar",
        makeFlagCall("LOGICALFLAGS8",
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x41))),
        BranchType::JNPAR,
        true,
        Binary::get(opGtr, st0, st1)
    );

    TEST_COND("LOGICALFLAGS(%eax), jof",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JOF,
        false,
        makeFlagCall("LOGICALFLAGS", eax)
    );

    TEST_COND("LOGICALFLAGS(%eax), jnof",
        makeFlagCall("LOGICALFLAGS", eax),
        BranchType::JOF,
        false,
        makeFlagCall("LOGICALFLAGS", eax)
    );

    //
    // SETFFLAGS
    //
    TEST_COND("SETFFLAGS(%eax, %ecx), je",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JE,
        false,
        Binary::get(opEquals, eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jne",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JNE,
        false,
        Binary::get(opNotEqual, eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jsl",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JSL,
        false,
        Binary::get(opLess, eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jsle",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JSLE,
        false,
        Binary::get(opLessEq, eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jsge",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JSGE,
        false,
        Binary::get(opGtrEq, eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jsg",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JSG,
        false,
        Binary::get(opGtr, eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jul",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JUL,
        false,
        makeFlagCall("SETFFLAGS", eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jule",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JULE,
        false,
        makeFlagCall("SETFFLAGS", eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), juge",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JUGE,
        false,
        makeFlagCall("SETFFLAGS", eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jug",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JUG,
        false,
        makeFlagCall("SETFFLAGS", eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jmi",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JMI,
        false,
        Binary::get(opLess, eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jpos",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JPOS,
        false,
        Binary::get(opGtrEq, eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jof",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JOF,
        false,
        makeFlagCall("SETFFLAGS", eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jnof",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JNOF,
        false,
        makeFlagCall("SETFFLAGS", eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jpar",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JPAR,
        false,
        makeFlagCall("SETFFLAGS", eax, ecx)
    );

    TEST_COND("SETFFLAGS(%eax, %ecx), jnpar",
        makeFlagCall("SETFFLAGS", eax, ecx),
        BranchType::JNPAR,
        false,
        makeFlagCall("SETFFLAGS", eax, ecx)
    );

    //
    // (SETFFLAGS(...) & MASK) RELOP INTCONST
    // ((SETFFLAGS(...) & MASK) ^ 0x40) RELOP INTCONST
    //
    TEST_COND("(SETFFLAGS(st0, st1) & 0x40 == 0, je",
        Binary::get(opEquals,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x40)),
                    Const::get(0)),
        BranchType::JE,
        true,
        Binary::get(opNotEqual, st0, st1)
    );

    TEST_COND("((SETFFLAGS(st0, st1) & 0x40) ^ 0x40 == 0, je",
        Binary::get(opEquals,
                    Binary::get(opBitXor,
                                Binary::get(opBitAnd,
                                            makeFlagCall("SETFFLAGS", st0, st1),
                                            Const::get(0x40)),
                                Const::get(0x40)),
                    Const::get(0)),
        BranchType::JE,
        true,
        Binary::get(opEquals, st0, st1)
    );

    TEST_COND("(SETFFLAGS(st0, st1) & %ecx) == 0, je",
        Binary::get(opEquals,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                ecx),
                    null),
        BranchType::JE,
        false,
        Binary::get(opEquals,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                ecx),
                    null)
    );

    TEST_COND("(%eax & %ecx) == 0, je",
        Binary::get(opEquals, Binary::get(opBitAnd, eax, ecx), null),
        BranchType::JE,
        false,
        Binary::get(opEquals, Binary::get(opBitAnd, eax, ecx), null)
    );

    TEST_COND("(SETFFLAGS(st0, st1) & 0x1) == 0, je",
        Binary::get(opEquals,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(1)),
                    Const::get(0)),
        BranchType::JE,
        true,
        Binary::get(opGtrEq, st0, st1)
    );

    TEST_COND("(SETFFLAGS(st0, st1) & 0x1) != 0, jne",
        Binary::get(opNotEqual,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(1)),
                    Const::get(0)),
        BranchType::JNE,
        true,
        Binary::get(opLess, st0, st1)
    );

    TEST_COND("(SETFFLAGS(st0, st1) & 0x40) == 0, je",
        Binary::get(opEquals,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x40)),
                    null),
        BranchType::JE,
        true,
        Binary::get(opNotEqual, st0, st1)
    );

    TEST_COND("(SETFFLAGS(st0, st1) & 0x40) != 0, jne",
        Binary::get(opNotEqual,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x40)),
                    null),
        BranchType::JE,
        true,
        Binary::get(opEquals, st0, st1)
    );

    TEST_COND("(SETFFLAGS(st0, st1) & 0x41) == 0, je",
        Binary::get(opEquals,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x41)),
                    null),
        BranchType::JE,
        true,
        Binary::get(opGtr, st0, st1)
    );

    TEST_COND("(SETFFLAGS(st0, st1) & 0x41) != 0, jne",
        Binary::get(opNotEqual,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x41)),
                    null),
        BranchType::JNE,
        true,
        Binary::get(opLessEq, st0, st1)
    );

    TEST_COND("(SETFFLAGS(st0, st1) & 0x41) == 1, je",
        Binary::get(opEquals,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x41)),
                    Const::get(1)),
        BranchType::JE,
        true,
        Binary::get(opLess, st0, st1)
    );

    TEST_COND("(SETFFLAGS(st0, st1) & 0x41) != 1, jne",
        Binary::get(opNotEqual,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x41)),
                    Const::get(1)),
        BranchType::JNE,
        true,
        Binary::get(opGtrEq, st0, st1)
    );

    TEST_COND("(SETFFLAGS(st0, st1) & 0x41) == 0x40, je",
        Binary::get(opEquals,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x41)),
                    Const::get(0x40)),
        BranchType::JE,
        true,
        Binary::get(opEquals, st0, st1)
    );

    TEST_COND("(SETFFLAGS(st0, st1) & 0x41) != 0x40, jne",
        Binary::get(opNotEqual,
                    Binary::get(opBitAnd,
                                makeFlagCall("SETFFLAGS", st0, st1),
                                Const::get(0x41)),
                    Const::get(0x40)),
        BranchType::JNE,
        true,
        Binary::get(opNotEqual, st0, st1)
    );

    //
    // SAHFFLAGS
    //
    TEST_COND("SAHFFLAGS(), je",
        makeFlagCall("SAHFFLAGS"),
        BranchType::JE,
        false,
        makeFlagCall("SAHFFLAGS")
    );

    TEST_COND("SAHFFLAGS(st0, st1, st0), je",
        makeFlagCall("SAHFFLAGS", st0, st1, st0),
        BranchType::JE,
        false,
        makeFlagCall("SAHFFLAGS", st0, st1, st0)
    );

    TEST_COND("SAHFFLAGS(SETFFLAGS(st0, st1)), je",
        makeFlagCall("SAHFFLAGS", makeFlagCall("SETFFLAGS", st0, st1)),
        BranchType::JE,
        true,
        Binary::get(opEquals, st0, st1)
    );

    TEST_COND("SAHFFLAGS(SETFFLAGS(st0, st1)), jne",
        makeFlagCall("SAHFFLAGS", makeFlagCall("SETFFLAGS", st0, st1)),
        BranchType::JNE,
        true,
        Binary::get(opNotEqual, st0, st1)
    );

    TEST_COND("SAHFFLAGS(SETFFLAGS(st0, st1)), jpar",
        makeFlagCall("SAHFFLAGS", makeFlagCall("SETFFLAGS", st0, st1)),
        BranchType::JPAR,
        true,
        Terminal::get(opFalse)
    );

    TEST_COND("SAHFFLAGS(SETFFLAGS(st0, st1)), jnpar",
        makeFlagCall("SAHFFLAGS", makeFlagCall("SETFFLAGS", st0, st1)),
        BranchType::JNPAR,
        true,
        Terminal::get(opTrue)
    );

    TEST_COND("SAHFFLAGS(SETFFLAGS(st0, st1)), jul",
        makeFlagCall("SAHFFLAGS", makeFlagCall("SETFFLAGS", st0, st1)),
        BranchType::JUL,
        true,
        Binary::get(opLess, st0, st1)
    );

    TEST_COND("SAHFFLAGS(SETFFLAGS(st0, st1)), juge",
        makeFlagCall("SAHFFLAGS", makeFlagCall("SETFFLAGS", st0, st1)),
        BranchType::JUGE,
        true,
        Binary::get(opGtrEq, st0, st1)
    );

    TEST_COND("SAHFFLAGS(SETFFLAGS(st0, st1)), jule",
        makeFlagCall("SAHFFLAGS", makeFlagCall("SETFFLAGS", st0, st1)),
        BranchType::JULE,
        true,
        Binary::get(opLessEq, st0, st1)
    );

    TEST_COND("SAHFFLAGS(SETFFLAGS(st0, st1)), jug",
        makeFlagCall("SAHFFLAGS", makeFlagCall("SETFFLAGS", st0, st1)),
        BranchType::JUG,
        true,
        Binary::get(opGtr, st0, st1)
    );

    TEST_COND("SAHFFLAGS(SETFFLAGS(st0, st1)), jof",
        makeFlagCall("SAHFFLAGS", makeFlagCall("SETFFLAGS", st0, st1)),
        BranchType::JOF,
        false,
        makeFlagCall("SAHFFLAGS", makeFlagCall("SETFFLAGS", st0, st1))
    );
}


QTEST_GUILESS_MAIN(StatementHelperTest)
