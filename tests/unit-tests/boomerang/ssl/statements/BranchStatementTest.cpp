#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BranchStatementTest.h"


#include "boomerang/db/LowLevelCFG.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/util/LocationSet.h"


Q_DECLARE_METATYPE(BranchType)


void BranchStatementTest::testClone()
{
    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement(Address(0x1000)));
        SharedStmt clone = bs->clone();

        QVERIFY(&(*clone) != &(*bs));
        QVERIFY(clone->isBranch());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != bs->getID());

        std::shared_ptr<BranchStatement> bsClone = clone->as<BranchStatement>();
        QVERIFY(bsClone->getDest() != nullptr);
        QCOMPARE(*bsClone->getDest(), *bs->getDest());
        QCOMPARE(bsClone->getCondExpr(), nullptr);
        QCOMPARE(bsClone->isComputed(), bs->isComputed());

        QCOMPARE(bsClone->getCondType(), bs->getCondType());
        QCOMPARE(bsClone->isFloatBranch(), bs->isFloatBranch());
    }

    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement(Address(0x1000)));
        bs->setCondType(BranchType::JE, true);
        SharedStmt clone = bs->clone();

        QVERIFY(&(*clone) != &(*bs));
        QVERIFY(clone->isBranch());
        QVERIFY(clone->getID() != (uint32)-1);
        QVERIFY(clone->getID() != bs->getID());

        std::shared_ptr<BranchStatement> bsClone = clone->as<BranchStatement>();
        QVERIFY(bsClone->getDest() != nullptr);
        QVERIFY(bsClone->getCondExpr() != nullptr);

        QVERIFY(&(*bsClone->getDest())     != &(*bs->getDest()));
        QVERIFY(&(*bsClone->getCondExpr()) != &(*bs->getCondExpr()));

        QCOMPARE(*bsClone->getDest(),     *bs->getDest());
        QCOMPARE(*bsClone->getCondExpr(), *bs->getCondExpr());

        QCOMPARE(bsClone->isComputed(), bs->isComputed());
        QCOMPARE(bsClone->getCondType(), bs->getCondType());
        QCOMPARE(bsClone->isFloatBranch(), bs->isFloatBranch());
    }
}


void BranchStatementTest::testGetDefinitions()
{
    {
        LocationSet defs;
        std::shared_ptr<BranchStatement> bs(new BranchStatement(Address(0x1000)));

        bs->getDefinitions(defs, false);

        QVERIFY(defs.empty());
    }
}


void BranchStatementTest::testDefinesLoc()
{
    {
        const SharedExp condExp = Binary::get(opEquals, Location::regOf(REG_X86_EAX), Const::get(0));

        std::shared_ptr<BranchStatement> bs(new BranchStatement(Address(0x1000)));
        bs->setCondExpr(condExp);

        QVERIFY(!bs->definesLoc(Const::get(Address(0x1000))));
        QVERIFY(!bs->definesLoc(nullptr));
        QVERIFY(!bs->definesLoc(Location::regOf(REG_X86_ESP)));
        QVERIFY(!bs->definesLoc(condExp));
        QVERIFY(!bs->definesLoc(Location::regOf(REG_X86_EAX)));
    }
}


void BranchStatementTest::testSearch()
{
    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement(Address(0x0800)));

        SharedExp result;
        QVERIFY(!bs->search(*Const::get(0x1000), result));
    }

    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement(Address(0x1000)));

        SharedExp result;
        QVERIFY(!bs->search(*Const::get(0), result));
        QVERIFY(bs->search(*Const::get(0x1000), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Const::get(Address(0x1000)));

        bs->setCondExpr(Binary::get(opEquals, Location::regOf(REG_X86_ECX), Const::get(0)));
        QVERIFY(bs->search(*Terminal::get(opWildRegOf), result));
        QVERIFY(result != nullptr);
        QCOMPARE(*result, *Location::regOf(REG_X86_ECX));
    }
}


void BranchStatementTest::testSearchAll()
{
    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement(Address(0x0800)));

        std::list<SharedExp> result;
        QVERIFY(!bs->searchAll(*Const::get(0x1000), result));
        QVERIFY(result.empty());
    }

    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement(Address(0x1000)));
        bs->setCondExpr(nullptr);

        std::list<SharedExp> result;
        QVERIFY(!bs->searchAll(*Const::get(0), result));
        QVERIFY(result.empty());

        QVERIFY(bs->searchAll(*Const::get(0x1000), result));
        QCOMPARE(result, { bs->getDest() });
    }

    {
        SharedExp ecx = Location::regOf(REG_X86_ECX);

        std::shared_ptr<BranchStatement> bs(new BranchStatement(Address(0x1000)));
        bs->setCondExpr(Binary::get(opEquals, ecx, Const::get(0)));

        std::list<SharedExp> result;
        QVERIFY(bs->searchAll(*Terminal::get(opWildRegOf), result));
        QCOMPARE(result, { ecx });
    }
}


void BranchStatementTest::testSearchAndReplace()
{
    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement(Location::regOf(REG_X86_ECX)));

        auto bsClone = bs->clone()->as<GotoStatement>();
        QVERIFY(!bs->searchAndReplace(*Const::get(Address(0x1000)), Const::get(0x800)));
        QCOMPARE(bs->toString(), bsClone->toString());

        bsClone->setDest(Address(0x1000));

        QVERIFY(bs->searchAndReplace(*Location::regOf(REG_X86_ECX), Const::get(Address(0x1000))));
        QCOMPARE(bs->toString(), bsClone->toString());
    }

    {
        std::shared_ptr<BranchStatement> bs(new BranchStatement(Location::regOf(REG_X86_EAX)));
        bs->setCondExpr(Binary::get(opEquals, Location::regOf(REG_X86_EAX), Const::get(0)));

        auto bsClone = bs->clone()->as<BranchStatement>();
        bsClone->setDest(Location::regOf(REG_X86_ECX));
        bsClone->setCondExpr(Binary::get(opEquals, Location::regOf(REG_X86_ECX), Const::get(0)));

        QVERIFY(bs->searchAndReplace(*Location::regOf(REG_X86_EAX), Location::regOf(REG_X86_ECX)));
        QCOMPARE(bs->toString(), bsClone->toString());
    }
}


void BranchStatementTest::testFallTakenFragment()
{
    Prog prog("test", &m_project);
    UserProc *proc = static_cast<UserProc *>(prog.getOrCreateFunction(Address(0x1000)));
    ProcCFG *cfg = proc->getCFG();

    BasicBlock *bb0 = prog.getCFG()->createBB(BBType::Twoway, createInsns(Address(0x1000), 1));
    BasicBlock *bb1 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1001), 1));
    BasicBlock *bb2 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1002), 1));
    BasicBlock *bb3 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1003), 1));
    BasicBlock *bb4 = prog.getCFG()->createBB(BBType::Oneway, createInsns(Address(0x1004), 1));

    std::shared_ptr<BranchStatement> bs(new BranchStatement(Address(0x1000)));
    QVERIFY(bs->getFallFragment() == nullptr);
    QVERIFY(bs->getTakenFragment() == nullptr);

    IRFragment *frag0 = cfg->createFragment(FragType::Twoway, createRTLs(Address(0x1000), 1, 1), bb0);
    IRFragment *frag1 = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1001), 1, 1), bb1);
    IRFragment *frag2 = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1002), 1, 1), bb2);
    IRFragment *frag3 = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1003), 1, 1), bb3);
    IRFragment *frag4 = cfg->createFragment(FragType::Oneway, createRTLs(Address(0x1003), 1, 1), bb4);

    frag0->getRTLs()->back()->append(bs);

    bs->setFallFragment(frag1);
    QVERIFY(bs->getFallFragment() == nullptr);
    QVERIFY(bs->getTakenFragment() == nullptr);

    bs->setTakenFragment(frag1);
    QVERIFY(bs->getFallFragment() == nullptr);
    QVERIFY(bs->getTakenFragment() == nullptr);

    bs->setFragment(frag0);

    bs->setFallFragment(frag1);
    QVERIFY(bs->getFallFragment() == nullptr);
    QVERIFY(bs->getTakenFragment() == nullptr);

    bs->setTakenFragment(frag1);
    QVERIFY(bs->getFallFragment() == nullptr);
    QVERIFY(bs->getTakenFragment() == nullptr);

    cfg->addEdge(frag0, frag2);
    cfg->addEdge(frag0, frag1);

    bs->setFallFragment(frag1);
    QVERIFY(bs->getFallFragment() == frag1);
    QVERIFY(bs->getTakenFragment() == frag2);

    bs->setTakenFragment(frag2);
    QVERIFY(bs->getFallFragment() == frag1);
    QVERIFY(bs->getTakenFragment() == frag2);

    bs->setFallFragment(frag3);
    QVERIFY(bs->getFallFragment() == frag3);
    QVERIFY(bs->getTakenFragment() == frag2);

    bs->setTakenFragment(frag4);
    QVERIFY(bs->getFallFragment() == frag3);
    QVERIFY(bs->getTakenFragment() == frag4);
}


void BranchStatementTest::testToString()
{
    QFETCH(BranchType, condTy);
    QFETCH(bool, isFloat);
    QFETCH(QString, expected);

    std::shared_ptr<BranchStatement> bs(new BranchStatement(Address(0x1000)));
    bs->setCondType(condTy, isFloat);

    QCOMPARE(bs->toString(), expected);
}


#define TEST_TOSTRING(bt, isFloat, expected) \
    do { \
        QTest::addRow(#bt ", " #isFloat) << bt << isFloat << expected; \
    } while (false);


void BranchStatementTest::testToString_data()
{
    QTest::addColumn<BranchType>("condTy");
    QTest::addColumn<bool>("isFloat");
    QTest::addColumn<QString>("expected");

    TEST_TOSTRING(BranchType::JE,     false, "   0 BRANCH 0x00001000, condition equals\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JNE,    false, "   0 BRANCH 0x00001000, condition not equals\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JSL,    false, "   0 BRANCH 0x00001000, condition signed less\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JSLE,   false, "   0 BRANCH 0x00001000, condition signed less or equals\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JSGE,   false, "   0 BRANCH 0x00001000, condition signed greater or equals\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JSG,    false, "   0 BRANCH 0x00001000, condition signed greater\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JUL,    false, "   0 BRANCH 0x00001000, condition unsigned less\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JULE,   false, "   0 BRANCH 0x00001000, condition unsigned less or equals\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JUGE,   false, "   0 BRANCH 0x00001000, condition unsigned greater or equals\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JUG,    false, "   0 BRANCH 0x00001000, condition unsigned greater\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JMI,    false, "   0 BRANCH 0x00001000, condition minus\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JPOS,   false, "   0 BRANCH 0x00001000, condition plus\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JOF,    false, "   0 BRANCH 0x00001000, condition overflow\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JNOF,   false, "   0 BRANCH 0x00001000, condition no overflow\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JPAR,   false, "   0 BRANCH 0x00001000, condition parity\nHigh level: %flags");
    TEST_TOSTRING(BranchType::JNPAR,  false, "   0 BRANCH 0x00001000, condition no parity\nHigh level: %flags");

    TEST_TOSTRING(BranchType::JE,     true, "   0 BRANCH 0x00001000, condition equals float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JNE,    true, "   0 BRANCH 0x00001000, condition not equals float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JSL,    true, "   0 BRANCH 0x00001000, condition signed less float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JSLE,   true, "   0 BRANCH 0x00001000, condition signed less or equals float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JSGE,   true, "   0 BRANCH 0x00001000, condition signed greater or equals float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JSG,    true, "   0 BRANCH 0x00001000, condition signed greater float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JUL,    true, "   0 BRANCH 0x00001000, condition unsigned less float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JULE,   true, "   0 BRANCH 0x00001000, condition unsigned less or equals float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JUGE,   true, "   0 BRANCH 0x00001000, condition unsigned greater or equals float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JUG,    true, "   0 BRANCH 0x00001000, condition unsigned greater float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JMI,    true, "   0 BRANCH 0x00001000, condition minus float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JPOS,   true, "   0 BRANCH 0x00001000, condition plus float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JOF,    true, "   0 BRANCH 0x00001000, condition overflow float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JNOF,   true, "   0 BRANCH 0x00001000, condition no overflow float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JPAR,   true, "   0 BRANCH 0x00001000, condition parity float\nHigh level: %fflags");
    TEST_TOSTRING(BranchType::JNPAR,  true, "   0 BRANCH 0x00001000, condition no parity float\nHigh level: %fflags");
}


QTEST_GUILESS_MAIN(BranchStatementTest)
