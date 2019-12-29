#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "IRFragmentTest.h"

#include "boomerang/db/BasicBlock.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/util/Util.h"


void IRFragmentTest::testConstruct()
{
    IRFragment frag1(1, nullptr, Address(0x1000));

    QCOMPARE(frag1.getLowAddr(), Address(0x1000));
    QCOMPARE(frag1.getProc(), nullptr);
    QVERIFY(frag1.isType(FragType::Invalid));

    frag1.setType(FragType::Call);

    IRFragment frag2(2, nullptr, createRTLs(Address(0x2000), 1, 1));
    QCOMPARE(frag2.getLowAddr(), Address(0x2000));
    QCOMPARE(frag2.getProc(), nullptr);
    QVERIFY(frag2.isType(FragType::Fall));
}


void IRFragmentTest::testGetType()
{
    IRFragment frag(0, nullptr, Address::ZERO); // incomplete fragment

    QCOMPARE(frag.getType(), FragType::Invalid);
    QVERIFY(frag.isType(FragType::Invalid));
}


void IRFragmentTest::testExtent()
{
    {
        IRFragment frag1(1, nullptr, Address(0x1000));
        QCOMPARE(frag1.getLowAddr(), Address(0x1000));
        QCOMPARE(frag1.getHiAddr(), Address::INVALID);
    }

    {
        IRFragment frag2(2, nullptr, createRTLs(Address(0x1000), 1, 1));
        QCOMPARE(frag2.getLowAddr(), Address(0x1000));
        QCOMPARE(frag2.getHiAddr(), Address(0x1000));
    }

    {
        IRFragment frag3(3, nullptr, createRTLs(Address(0x1000), 2, 1));
        QCOMPARE(frag3.getLowAddr(), Address(0x1000));
        QCOMPARE(frag3.getHiAddr(),  Address(0x1001));
    }
}


void IRFragmentTest::testRemoveRTL()
{
    {
        IRFragment bb1(1, nullptr, Address(0x1000));
        bb1.removeRTL(nullptr); // check it does not crash
    }

    {
        std::unique_ptr<RTLList> rtls = createRTLs(Address(0x2000), 1, 1);
        RTL *rtlToBeRemoved = rtls->front().get();
        IRFragment bb2(1, nullptr, std::move(rtls));

        bb2.removeRTL(rtlToBeRemoved);
        QCOMPARE(bb2.getLowAddr(), Address(0x2000));
        QVERIFY(bb2.getRTLs()->empty());
    }
}


void IRFragmentTest::testGetStmt()
{

    IRFragment::RTLIterator rit;
    IRFragment::RTLRIterator rrit;
    StatementList::iterator sit;
    StatementList::reverse_iterator srit;

    {
        IRFragment bb1(1, nullptr, Address(0x1000));
        QCOMPARE(bb1.getFirstStmt(), nullptr);
        QCOMPARE(bb1.getLastStmt(), nullptr);
        QCOMPARE(bb1.getFirstStmt(rit, sit), nullptr);
        QCOMPARE(bb1.getLastStmt(rrit, srit), nullptr);
    }

    {
        IRFragment bb2(2, nullptr, createRTLs(Address(0x1000), 1, 0));
        bb2.setType(FragType::CompJump);

        const SharedStmt firstStmt = bb2.getFirstStmt(rit, sit);
        const SharedStmt lastStmt  = bb2.getLastStmt(rrit, srit);

        QCOMPARE(firstStmt, nullptr);
        QCOMPARE(lastStmt, nullptr);
        QCOMPARE(bb2.getFirstStmt(), nullptr);
        QCOMPARE(bb2.getLastStmt(), nullptr);
    }

    {
        IRFragment bb3(3, nullptr, createRTLs(Address(0x1000), 1, 1));

        const SharedStmt firstStmt = bb3.getFirstStmt(rit, sit);
        const SharedStmt lastStmt  = bb3.getLastStmt(rrit, srit);

        QVERIFY(firstStmt != nullptr);
        QVERIFY(lastStmt != nullptr);
        QVERIFY(firstStmt->isAssign());
        QCOMPARE(firstStmt, bb3.getFirstStmt());
        QCOMPARE(lastStmt, bb3.getLastStmt());
        QCOMPARE(firstStmt, lastStmt);
    }
}


void IRFragmentTest::testAddImplicit()
{
    IRFragment bb1(1, nullptr, createRTLs(Address(0x1000), 1, 1));
    const std::shared_ptr<ImplicitAssign> imp = bb1.addImplicitAssign(Terminal::get(opCF));

    QVERIFY(imp);
    QVERIFY(imp->isImplicit());
    QVERIFY(*imp->getLeft() == *Terminal::get(opCF));

    QVERIFY(bb1.getFirstStmt() == imp);

    const QString expected(
        "Fall BB:\n"
        "  in edges: \n"
        "  out edges: \n"
        "0x00000000    0 *v* %CF := -\n"
        "0x00001000    0 *v*  := \n"
    );

    QCOMPARE(bb1.toString(), expected);

    // add same implicit assign twice
    bb1.addImplicitAssign(Terminal::get(OPER::opCF));

    QCOMPARE(bb1.toString(), expected);
}


void IRFragmentTest::testAddPhi()
{
    IRFragment bb1(1, nullptr, createRTLs(Address(0x1000), 1, 1));
    const std::shared_ptr<PhiAssign> phi = bb1.addPhi(Terminal::get(OPER::opCF));
    QVERIFY(phi);
    QVERIFY(phi->isPhi());
    QVERIFY(*phi->getLeft() == *Terminal::get(opCF));
    QVERIFY(bb1.getFirstStmt() == phi);

    const QString expected(
        "Fall BB:\n"
        "  in edges: \n"
        "  out edges: \n"
        "0x00000000    0 *v* %CF := phi{}\n"
        "0x00001000    0 *v*  := \n"
    );

    QCOMPARE(bb1.toString(), expected);

    // add same implicit assign twice
    bb1.addPhi(Terminal::get(OPER::opCF));

    QCOMPARE(bb1.toString(), expected);
}


void IRFragmentTest::testAddImplicitOverPhi()
{
    std::unique_ptr<RTLList> rtls(new RTLList);
    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));
    IRFragment bb1(1, nullptr, std::move(rtls));

    QVERIFY(nullptr != bb1.addPhi(Terminal::get(opCF)));
    QVERIFY(nullptr == bb1.addImplicitAssign(Terminal::get(opCF)));

    QString expected(
        "Fall BB:\n"
        "  in edges: \n"
        "  out edges: \n"
        "0x00000000    0 *v* %CF := phi{}\n"
        "0x00001000    0 BRANCH *no dest*, condition equals\n"
        "\n"
    );

    QCOMPARE(bb1.toString(), expected);
}


void IRFragmentTest::testAddPhiOverImplict()
{
    std::unique_ptr<RTLList> rtls(new RTLList);
    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));
    IRFragment bb1(1, nullptr, std::move(rtls));

    QVERIFY(nullptr != bb1.addImplicitAssign(Terminal::get(opCF)));
    QVERIFY(nullptr == bb1.addPhi(Terminal::get(opCF)));

    QString expected(
        "Fall BB:\n"
        "  in edges: \n"
        "  out edges: \n"
        "0x00000000    0 *v* %CF := -\n"
        "0x00001000    0 BRANCH *no dest*, condition equals\n"
        "\n"
    );

    QCOMPARE(bb1.toString(), expected);
}


void IRFragmentTest::testGetCallDestProc()
{
    QSKIP("Not implemented");
//     {
//         IRFragment bb1(nullptr, Address(0x1000));
//         QVERIFY(bb1.getCallDestProc() == nullptr);
//     }

//     {
//         LibProc proc(Address(0x5000), "test", nullptr);
//
//         std::unique_ptr<RTLList> rtls(new RTLList);
//         std::shared_ptr<CallStatement> call(new CallStatement);
//         call->setDestProc(&proc);
//
//         rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { call })));
//         IRFragment bb2(nullptr, std::move(rtls));
//
//         QVERIFY(bb2.getCallDestProc() == &proc);
//     }
}


void IRFragmentTest::testGetCond()
{
    {
        IRFragment bb1(1, nullptr, Address(0x1000));
        QVERIFY(bb1.getCond() == nullptr);
    }

    {
        std::unique_ptr<RTLList> rtls(new RTLList);
        std::shared_ptr<BranchStatement> branch(new BranchStatement);
        branch->setCondExpr(Terminal::get(opZF));

        rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { branch })));
        IRFragment bb2(2, nullptr, std::move(rtls));

        QVERIFY(bb2.getCond() != nullptr);
        QVERIFY(*bb2.getCond() == *Terminal::get(opZF));
    }
}


void IRFragmentTest::testSetCond()
{
    std::unique_ptr<RTLList> rtls(new RTLList);
    std::shared_ptr<BranchStatement> branch(new BranchStatement);
    branch->setCondExpr(Terminal::get(opZF));

    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { branch })));
    IRFragment bb2(2, nullptr, std::move(rtls));

    bb2.setCond(nullptr);
    QVERIFY(bb2.getCond() == nullptr);

    bb2.setCond(Terminal::get(opOF));
    QVERIFY(*bb2.getCond() == *Terminal::get(opOF));
}


void IRFragmentTest::testGetDest()
{
    {
        IRFragment bb1(1, nullptr, Address(0x1000));
        QVERIFY(bb1.getDest() == nullptr);
    }

    {
        std::unique_ptr<RTLList> rtls(new RTLList);
        std::shared_ptr<GotoStatement> jump(new GotoStatement(Address(0x2000)));
        rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { jump })));
        IRFragment frag2(2, nullptr, std::move(rtls));

        QVERIFY(frag2.getDest() != nullptr);
        QCOMPARE(frag2.getDest()->toString(), QString("0x2000"));
    }
}


void IRFragmentTest::testHasStatement()
{
    {
        IRFragment bb1(1, nullptr, Address(0x1000));
        QVERIFY(!bb1.hasStatement(nullptr));
    }

    {
        std::unique_ptr<RTLList> rtls(new RTLList);
        std::shared_ptr<GotoStatement> jump(new GotoStatement(Address(0x2000)));
        rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { jump })));

        IRFragment bb2(2, nullptr, std::move(rtls));
        QVERIFY(bb2.hasStatement(jump));

        std::shared_ptr<GotoStatement> jump2(new GotoStatement(Address(0x2000)));
        QVERIFY(!bb2.hasStatement(jump2));
    }
}


void IRFragmentTest::testSimplify()
{
    QSKIP("Not implmented");

//     UserProc proc(Address(0x1000), "test", nullptr);
//
//     std::unique_ptr<RTLList> rtls(new RTLList);
//     rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));
//     IRFragment *bb1 = proc.getCFG()->createFragment(std::move(rtls), nullptr);
//
//     rtls.reset(new RTLList);
//     rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x2000), { std::make_shared<CallStatement>() })));
//     BasicBlock *bb2 = proc.getCFG()->createBB(BBType::Twoway, std::move(rtls));
//
//     proc.getCFG()->addEdge(bb1, bb2);
//     proc.getCFG()->addEdge(bb1, bb2);
//
//     bb1->getIR()->simplify();
//     QCOMPARE(bb1->getType(), BBType::Oneway);
//     QCOMPARE(bb1->getNumSuccessors(), 1);
//     QVERIFY(bb1->isPredecessorOf(bb2));
//     QVERIFY(bb2->isSuccessorOf(bb1));
}


void IRFragmentTest::testUpdateAddresses()
{
    {
        IRFragment bb1(1, nullptr, Address(0x1000));
        bb1.updateAddresses();

        QVERIFY(bb1.getLowAddr() == Address(0x1000));
        QVERIFY(bb1.getHiAddr()  == Address::INVALID);
    }

    {
        std::unique_ptr<RTLList> rtls(new RTLList);
        std::shared_ptr<GotoStatement> jump(new GotoStatement(Address(0x2000)));

        rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x2000), { jump })));
        IRFragment bb1(1, nullptr, std::move(rtls));

        QVERIFY(bb1.getLowAddr() == Address(0x2000));
        QVERIFY(bb1.getHiAddr()  == Address(0x2000));
    }
}


void IRFragmentTest::testIsEmpty()
{
    {
        IRFragment bb1(1, nullptr, Address(0x1000));
        QVERIFY(bb1.isEmpty());
    }

    {
        IRFragment bb1(1, nullptr, createRTLs(Address(0x1000), 1, 0));
        QVERIFY(bb1.isEmpty());
    }

    {
        IRFragment bb1(1, nullptr, createRTLs(Address(0x1000), 1, 0));
        bb1.getRTLs()->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1001))));
        QVERIFY(bb1.isEmpty());
    }

    {
        IRFragment bb1(1, nullptr, createRTLs(Address(0x1000), 1, 0));
        std::shared_ptr<GotoStatement> jump(new GotoStatement(Address(0x2000)));
        bb1.getRTLs()->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1002), { jump })));

        QVERIFY(!bb1.isEmpty());
    }
}


void IRFragmentTest::testIsEmptyJump()
{
    {
        IRFragment bb1(1, nullptr, Address(0x1000));
        QVERIFY(!bb1.isEmptyJump());
    }

    {
        auto bbRTLs = std::unique_ptr<RTLList>(new RTLList);
        bbRTLs->push_back(std::make_unique<RTL>(Address(0x1000)));
        IRFragment bb1(1, nullptr, std::move(bbRTLs));
        QVERIFY(!bb1.isEmptyJump());
    }

    {
        auto bbRTLs = std::unique_ptr<RTLList>(new RTLList);
        bbRTLs->push_back(std::make_unique<RTL>(Address(0x1000)));
        std::shared_ptr<BranchStatement> jump(new BranchStatement());
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1001), { jump })));

        IRFragment bb1(1, nullptr, std::move(bbRTLs));
        QVERIFY(!bb1.isEmptyJump());
    }

    {
        auto bbRTLs = std::unique_ptr<RTLList>(new RTLList);
        bbRTLs->push_back(std::make_unique<RTL>(Address(0x1000)));
        std::shared_ptr<GotoStatement> jump(new GotoStatement(Address(0x2000)));
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1001), { jump })));

        IRFragment bb1(1, nullptr, std::move(bbRTLs));
        QVERIFY(bb1.isEmptyJump());
    }

    {
        auto bbRTLs = std::unique_ptr<RTLList>(new RTLList);
        bbRTLs->push_back(std::make_unique<RTL>(Address(0x1000)));
        std::shared_ptr<GotoStatement> jump(new GotoStatement(Address(0x2000)));
        std::shared_ptr<Assign> asgn(new Assign(Terminal::get(opNil), Terminal::get(opNil)));
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1001), { asgn, jump })));

        IRFragment bb1(1, nullptr, std::move(bbRTLs));
        QVERIFY(!bb1.isEmptyJump());
    }
}


QTEST_GUILESS_MAIN(IRFragmentTest)
