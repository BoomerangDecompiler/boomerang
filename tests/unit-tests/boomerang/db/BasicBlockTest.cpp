#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BasicBlockTest.h"


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


void BasicBlockTest::testConstruct()
{
    LibProc proc(Address(0x5000), "test", nullptr);

    BasicBlock bb1(Address(0x1000), &proc);

    QVERIFY(bb1.getLowAddr() == Address(0x1000));
    QVERIFY(bb1.getFunction() == &proc);
    QVERIFY(bb1.isIncomplete());
    QVERIFY(bb1.isType(BBType::Invalid));

    bb1.setType(BBType::Call);
    BasicBlock bb2(bb1);

    QVERIFY(bb2.getLowAddr() == Address(0x1000));
    QVERIFY(bb2.getFunction() == &proc);
    QVERIFY(bb2.isIncomplete());
    QVERIFY(bb2.isType(BBType::Call));

    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::make_unique<RTL>(Address(0x2000), nullptr));

    BasicBlock bb3(BBType::Fall, std::move(bbRTLs), &proc);
    QVERIFY(bb3.getLowAddr() == Address(0x2000));
    QVERIFY(bb3.getFunction() == &proc);
    QVERIFY(!bb3.isIncomplete());
    QVERIFY(bb3.isType(BBType::Fall));
}


void BasicBlockTest::testAssign()
{
    LibProc proc(Address(0x5000), "test", nullptr);

    BasicBlock bb1(Address(0x1000), &proc);

    BasicBlock bb2 = bb1;
    QVERIFY(bb2.getLowAddr() == Address(0x1000));
    QVERIFY(bb2.getFunction() == &proc);
    QVERIFY(bb2.isIncomplete());
    QVERIFY(bb2.isType(BBType::Invalid));

    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    bbRTLs->push_back(std::make_unique<RTL>(Address(0x2000), nullptr));

    BasicBlock bb3(BBType::Fall, std::move(bbRTLs), &proc);

    BasicBlock bb4 = bb3;
    QCOMPARE(bb4.toString(), bb3.toString());

    BasicBlock bb5 = bb2;

    QVERIFY(bb5.getLowAddr() == Address(0x1000));
    QVERIFY(bb5.getFunction() == &proc);
    QVERIFY(bb5.isIncomplete());
    QVERIFY(bb5.isType(BBType::Invalid));
}


void BasicBlockTest::testGetType()
{
    BasicBlock bb(Address::ZERO, nullptr); // incomplete BB

    QVERIFY(bb.getType() == BBType::Invalid);
    QVERIFY(bb.isType(BBType::Invalid));
}


void BasicBlockTest::testExtent()
{
    {
        BasicBlock bb1(Address(0x1000), nullptr);
        QCOMPARE(bb1.getLowAddr(), Address(0x1000));
        QCOMPARE(bb1.getHiAddr(), Address::INVALID);
    }

    {
        std::unique_ptr<RTLList> rtls(new RTLList);
        rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));


        BasicBlock bb2(BBType::Invalid, std::move(rtls), nullptr);
        QCOMPARE(bb2.getLowAddr(), Address(0x1000));
        QCOMPARE(bb2.getHiAddr(), Address(0x1000));
    }

    {
        std::unique_ptr<RTLList> rtls(new RTLList);
        rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));

        BasicBlock bb3(BBType::Twoway, std::move(rtls), nullptr);
        QCOMPARE(bb3.getLowAddr(), Address(0x1000));
        QCOMPARE(bb3.getHiAddr(),  Address(0x1000));
    }
}


void BasicBlockTest::testIncomplete()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    QCOMPARE(bb1.isIncomplete(), true);

    std::unique_ptr<RTLList> rtls1(new RTLList);
    rtls1->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));

    BasicBlock bb2(BBType::Twoway, std::move(rtls1), nullptr);
    QCOMPARE(bb2.isIncomplete(), false);

    std::unique_ptr<RTLList> rtls2(new RTLList);
    rtls2->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));

    BasicBlock bb3(Address(0x1000), nullptr);
    bb3.completeBB(std::move(rtls2));
    QCOMPARE(bb3.isIncomplete(), false);
}


void BasicBlockTest::testGetPredecessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    QCOMPARE(bb1.getPredecessor(0), static_cast<BasicBlock *>(nullptr)); // out of range

    BasicBlock pred1(Address::ZERO, nullptr);
    bb1.addPredecessor(&pred1);
    QCOMPARE(bb1.getPredecessor(0), &pred1);
    QCOMPARE(bb1.getPredecessor(1), static_cast<BasicBlock *>(nullptr));
}


void BasicBlockTest::testGetSuccessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    QCOMPARE(bb1.getSuccessor(0), static_cast<BasicBlock *>(nullptr)); // out of range

    BasicBlock succ1(Address::ZERO, nullptr);
    bb1.addSuccessor(&succ1);
    QCOMPARE(bb1.getSuccessor(0), &succ1);
    QCOMPARE(bb1.getSuccessor(1), static_cast<BasicBlock *>(nullptr));
}


void BasicBlockTest::testSetPredecessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock pred1(Address::ZERO, nullptr);
    BasicBlock pred2(Address::ZERO, nullptr);

    QCOMPARE(bb1.getNumPredecessors(), 0); // not added
    bb1.addPredecessor(&pred1);
    bb1.setPredecessor(0, &pred2);
    QCOMPARE(bb1.getNumPredecessors(), 1);
    QCOMPARE(bb1.getPredecessor(0), &pred2);

    bb1.setPredecessor(0, nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 1);
    QCOMPARE(bb1.getPredecessor(0), static_cast<BasicBlock *>(nullptr));
}


void BasicBlockTest::testSetSuccessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock succ1(Address::ZERO, nullptr);
    BasicBlock succ2(Address::ZERO, nullptr);

    QCOMPARE(bb1.getNumSuccessors(), 0); // not added
    bb1.addSuccessor(&succ1);
    bb1.setSuccessor(0, &succ2);
    QCOMPARE(bb1.getNumSuccessors(), 1);
    QCOMPARE(bb1.getSuccessor(0), &succ2);

    bb1.setSuccessor(0, nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 1);
    QCOMPARE(bb1.getSuccessor(0), static_cast<BasicBlock *>(nullptr));
}


void BasicBlockTest::testAddPredecessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock pred1(Address::ZERO, nullptr);

    bb1.addPredecessor(nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 1);

    bb1.addPredecessor(&pred1);
    QCOMPARE(bb1.getNumPredecessors(), 2);
    QCOMPARE(bb1.getPredecessor(1), &pred1);

    bb1.addPredecessor(&pred1);
    QCOMPARE(bb1.getNumPredecessors(), 3);
    QCOMPARE(bb1.getPredecessor(2), &pred1);
}


void BasicBlockTest::testAddSuccessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock succ1(Address::ZERO, nullptr);

    bb1.addSuccessor(nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 1);

    bb1.addSuccessor(&succ1);
    QCOMPARE(bb1.getNumSuccessors(), 2);
    QCOMPARE(bb1.getSuccessor(1), &succ1);

    bb1.addSuccessor(&succ1);
    QCOMPARE(bb1.getNumSuccessors(), 3);
    QCOMPARE(bb1.getSuccessor(2), &succ1);
}


void BasicBlockTest::testRemovePredecessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);

    bb1.removePredecessor(nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 0);

    BasicBlock pred1(Address::ZERO, nullptr);

    bb1.addPredecessor(&pred1);
    bb1.removePredecessor(nullptr);
    QCOMPARE(bb1.getNumPredecessors(), 1);
    bb1.removePredecessor(&pred1);
    QCOMPARE(bb1.getNumPredecessors(), 0);
}


void BasicBlockTest::testRemoveSuccessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);

    bb1.removeSuccessor(nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 0);

    BasicBlock succ1(Address::ZERO, nullptr);

    bb1.addSuccessor(&succ1);
    bb1.removeSuccessor(nullptr);
    QCOMPARE(bb1.getNumSuccessors(), 1);
    bb1.removeSuccessor(&succ1);
    QCOMPARE(bb1.getNumSuccessors(), 0);
}


void BasicBlockTest::testIsPredecessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock bb2(Address::ZERO, nullptr);

    bb1.addSuccessor(&bb1);
    QVERIFY(bb1.isPredecessorOf(&bb1));
    QVERIFY(!bb1.isPredecessorOf(nullptr));
    QVERIFY(!bb1.isPredecessorOf(&bb2));
    bb1.addSuccessor(&bb2);
    QVERIFY(bb1.isPredecessorOf(&bb2));
}


void BasicBlockTest::testIsSuccessor()
{
    BasicBlock bb1(Address::ZERO, nullptr);
    BasicBlock bb2(Address::ZERO, nullptr);

    bb1.addPredecessor(&bb1);
    QVERIFY(bb1.isSuccessorOf(&bb1));
    QVERIFY(!bb1.isSuccessorOf(nullptr));
    QVERIFY(!bb1.isSuccessorOf(&bb2));
    bb1.addPredecessor(&bb2);
    QVERIFY(bb1.isSuccessorOf(&bb2));
}


void BasicBlockTest::testRemoveRTL()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    bb1.getIR()->removeRTL(nullptr); // check it does not crash

    std::unique_ptr<RTLList> rtls(new RTLList);
    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x2000), { std::make_shared<BranchStatement>() })));

    RTL *rtlToBeRemoved = rtls->front().get();
    BasicBlock bb2(BBType::Twoway, std::move(rtls), nullptr);

    bb2.getIR()->removeRTL(rtlToBeRemoved);
    QVERIFY(bb2.getLowAddr() == Address(0x2000));
    QVERIFY(bb2.isIncomplete());
}


void BasicBlockTest::testCompleteBB()
{
    {
        BasicBlock bb1(Address(0x1000), nullptr);
        QVERIFY(bb1.isIncomplete());

        std::unique_ptr<RTLList> rtls(new RTLList);
        rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x2000), { std::make_shared<BranchStatement>() })));

        bb1.completeBB(std::move(rtls));

        QVERIFY(!bb1.isIncomplete());
    }

    {
        BasicBlock bb2(Address(0x1000), nullptr);
        QVERIFY(bb2.isIncomplete());

        std::unique_ptr<RTLList> rtls(new RTLList);
        rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x2000))));
        bb2.completeBB(std::move(rtls));

        QVERIFY(!bb2.isIncomplete());
    }
}


void BasicBlockTest::testGetStmt()
{

    IRFragment::RTLIterator rit;
    IRFragment::RTLRIterator rrit;
    StatementList::iterator sit;
    StatementList::reverse_iterator srit;

    BasicBlock bb1(Address(0x1000), nullptr);
    QVERIFY(bb1.getIR()->getFirstStmt() == nullptr);
    QVERIFY(bb1.getIR()->getLastStmt() == nullptr);
    QVERIFY(bb1.getIR()->getFirstStmt(rit, sit) == nullptr);
    QVERIFY(bb1.getIR()->getLastStmt(rrit, srit) == nullptr);


    std::unique_ptr<RTLList> rtls(new RTLList);
    rtls->push_back(std::make_unique<RTL>(Address(0x1000)));
    BasicBlock bb2(BBType::CompJump, std::move(rtls), nullptr);

    SharedStmt firstStmt = bb2.getIR()->getFirstStmt(rit, sit);
    SharedStmt lastStmt  = bb2.getIR()->getLastStmt(rrit, srit);

    QVERIFY(firstStmt == nullptr);
    QVERIFY(lastStmt == nullptr);
    QVERIFY(bb2.getIR()->getFirstStmt() == nullptr);
    QVERIFY(bb2.getIR()->getLastStmt() == nullptr);

    bb2.getIR()->getRTLs()->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));

    firstStmt = bb2.getIR()->getFirstStmt(rit, sit);
    lastStmt  = bb2.getIR()->getLastStmt(rrit, srit);

    QVERIFY(firstStmt->isBranch());
    QVERIFY(firstStmt == bb2.getIR()->getFirstStmt());
    QVERIFY(lastStmt  == bb2.getIR()->getLastStmt());
    QVERIFY(firstStmt == lastStmt);
}


void BasicBlockTest::testAddImplicit()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    std::unique_ptr<RTLList> rtls(new RTLList);
    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));
    bb1.completeBB(std::move(rtls));

    std::shared_ptr<ImplicitAssign> imp = bb1.getIR()->addImplicitAssign(Terminal::get(opCF));
    QVERIFY(imp);
    QVERIFY(imp->isImplicit());
    QVERIFY(*imp->getLeft() == *Terminal::get(opCF));

    QString expected("Invalid BB:\n"
        "  in edges: \n"
        "  out edges: \n"
        "0x00000000    0 *v* %CF := -\n"
        "0x00001000    0 BRANCH *no dest*, condition equals\n"
        "\n"
    );

    QCOMPARE(bb1.toString(), expected);

    // add same implicit assign twice
    bb1.getIR()->addImplicitAssign(Terminal::get(OPER::opCF));

    QCOMPARE(bb1.toString(), expected);
}


void BasicBlockTest::testAddPhi()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    std::unique_ptr<RTLList> rtls(new RTLList);
    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));
    bb1.completeBB(std::move(rtls));

    std::shared_ptr<PhiAssign> phi = bb1.getIR()->addPhi(Terminal::get(OPER::opCF));
    QVERIFY(phi);
    QVERIFY(phi->isPhi());
    QVERIFY(*phi->getLeft() == *Terminal::get(opCF));

    QString expected("Invalid BB:\n"
        "  in edges: \n"
        "  out edges: \n"
        "0x00000000    0 *v* %CF := phi{}\n"
        "0x00001000    0 BRANCH *no dest*, condition equals\n"
        "\n"
    );

    QCOMPARE(bb1.toString(), expected);

    // add same implicit assign twice
    bb1.getIR()->addPhi(Terminal::get(OPER::opCF));

    QCOMPARE(bb1.toString(), expected);
}


void BasicBlockTest::testAddImplicitOverPhi()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    std::unique_ptr<RTLList> rtls(new RTLList);
    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));
    bb1.completeBB(std::move(rtls));

    QVERIFY(nullptr != bb1.getIR()->addPhi(Terminal::get(opCF)));
    QVERIFY(nullptr == bb1.getIR()->addImplicitAssign(Terminal::get(opCF)));

    QString expected("Invalid BB:\n"
        "  in edges: \n"
        "  out edges: \n"
        "0x00000000    0 *v* %CF := phi{}\n"
        "0x00001000    0 BRANCH *no dest*, condition equals\n"
        "\n"
    );

    QCOMPARE(bb1.toString(), expected);
}


void BasicBlockTest::testAddPhiOverImplict()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    std::unique_ptr<RTLList> rtls(new RTLList);
    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));
    bb1.completeBB(std::move(rtls));

    QVERIFY(nullptr != bb1.getIR()->addImplicitAssign(Terminal::get(opCF)));
    QVERIFY(nullptr == bb1.getIR()->addPhi(Terminal::get(opCF)));

    QString expected("Invalid BB:\n"
        "  in edges: \n"
        "  out edges: \n"
        "0x00000000    0 *v* %CF := -\n"
        "0x00001000    0 BRANCH *no dest*, condition equals\n"
        "\n"
    );

    QCOMPARE(bb1.toString(), expected);
}


void BasicBlockTest::testGetCallDestProc()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    QVERIFY(bb1.getIR()->getCallDestProc() == nullptr);

    LibProc proc(Address(0x5000), "test", nullptr);

    std::unique_ptr<RTLList> rtls(new RTLList);
    std::shared_ptr<CallStatement> call(new CallStatement);
    call->setDestProc(&proc);

    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { call })));
    BasicBlock bb2(BBType::Call, std::move(rtls), nullptr);

    QVERIFY(bb2.getIR()->getCallDestProc() == &proc);
}


void BasicBlockTest::testGetCond()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    QVERIFY(bb1.getIR()->getCond() == nullptr);

    std::unique_ptr<RTLList> rtls(new RTLList);
    std::shared_ptr<BranchStatement> branch(new BranchStatement);
    branch->setCondExpr(Terminal::get(opZF));

    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { branch })));
    BasicBlock bb2(BBType::Twoway, std::move(rtls), nullptr);

    QVERIFY(bb2.getIR()->getCond() != nullptr);
    QVERIFY(*bb2.getIR()->getCond() == *Terminal::get(opZF));
}


void BasicBlockTest::testSetCond()
{
    std::unique_ptr<RTLList> rtls(new RTLList);
    std::shared_ptr<BranchStatement> branch(new BranchStatement);
    branch->setCondExpr(Terminal::get(opZF));

    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { branch })));
    BasicBlock bb2(BBType::Twoway, std::move(rtls), nullptr);

    bb2.getIR()->setCond(nullptr);
    QVERIFY(bb2.getIR()->getCond() == nullptr);

    bb2.getIR()->setCond(Terminal::get(opOF));
    QVERIFY(*bb2.getIR()->getCond() == *Terminal::get(opOF));
}


void BasicBlockTest::testGetDest()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    QVERIFY(bb1.getIR()->getDest() == nullptr);

    std::unique_ptr<RTLList> rtls(new RTLList);
    std::shared_ptr<GotoStatement> jump(new GotoStatement(Address(0x2000)));

    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { jump })));
    BasicBlock bb2(BBType::Oneway, std::move(rtls), nullptr);

    QVERIFY(bb2.getIR()->getDest() != nullptr);
    QCOMPARE(bb2.getIR()->getDest()->toString(), QString("0x2000"));
}


void BasicBlockTest::testHasStatement()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    QVERIFY(!bb1.getIR()->hasStatement(nullptr));

    std::unique_ptr<RTLList> rtls(new RTLList);
    std::shared_ptr<GotoStatement> jump(new GotoStatement(Address(0x2000)));

    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { jump })));
    BasicBlock bb2(BBType::Oneway, std::move(rtls), nullptr);
    QVERIFY(bb2.getIR()->hasStatement(jump));

    std::shared_ptr<GotoStatement> jump2(new GotoStatement(Address(0x2000)));
    QVERIFY(!bb2.getIR()->hasStatement(jump2));
}


void BasicBlockTest::testSimplify()
{
    UserProc proc(Address(0x1000), "test", nullptr);

    std::unique_ptr<RTLList> rtls(new RTLList);
    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000), { std::make_shared<BranchStatement>() })));
    BasicBlock *bb1 = proc.getCFG()->createBB(BBType::Twoway, std::move(rtls));

    rtls.reset(new RTLList);
    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x2000), { std::make_shared<CallStatement>() })));
    BasicBlock *bb2 = proc.getCFG()->createBB(BBType::Twoway, std::move(rtls));

    proc.getCFG()->addEdge(bb1, bb2);
    proc.getCFG()->addEdge(bb1, bb2);

    bb1->getIR()->simplify();
    QCOMPARE(bb1->getType(), BBType::Oneway);
    QCOMPARE(bb1->getNumSuccessors(), 1);
    QVERIFY(bb1->isPredecessorOf(bb2));
    QVERIFY(bb2->isSuccessorOf(bb1));
}


void BasicBlockTest::testUpdateBBAddresses()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    bb1.updateBBAddresses();

    QVERIFY(bb1.getLowAddr() == Address(0x1000));
    QVERIFY(bb1.getHiAddr()  == Address::INVALID);

    std::unique_ptr<RTLList> rtls(new RTLList);
    std::shared_ptr<GotoStatement> jump(new GotoStatement(Address(0x2000)));

    rtls->push_back(std::unique_ptr<RTL>(new RTL(Address(0x2000), { jump })));
    bb1.completeBB(std::move(rtls));
    bb1.updateBBAddresses();

    QVERIFY(bb1.getLowAddr() == Address(0x2000));
    QVERIFY(bb1.getHiAddr()  == Address(0x2000));
}


void BasicBlockTest::testIsEmpty()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    QVERIFY(bb1.getIR()->isEmpty());

    auto bbRTLs = std::unique_ptr<RTLList>(new RTLList);
    bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1000))));

    bb1.completeBB(std::move(bbRTLs));
    QVERIFY(bb1.getIR()->isEmpty());

    bb1.getIR()->getRTLs()->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1001))));
    QVERIFY(bb1.getIR()->isEmpty());

    std::shared_ptr<GotoStatement> jump(new GotoStatement(Address(0x2000)));
    bb1.getIR()->getRTLs()->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1002), { jump })));

    QVERIFY(!bb1.getIR()->isEmpty());
}


void BasicBlockTest::testIsEmptyJump()
{
    BasicBlock bb1(Address(0x1000), nullptr);
    QVERIFY(!bb1.getIR()->isEmptyJump());

    auto bbRTLs = std::unique_ptr<RTLList>(new RTLList);
    bbRTLs->push_back(std::make_unique<RTL>(Address(0x1000)));
    bb1.completeBB(std::move(bbRTLs));
    QVERIFY(!bb1.getIR()->isEmptyJump());

    std::shared_ptr<GotoStatement> jump(new GotoStatement(Address(0x2000)));
    bb1.getIR()->getRTLs()->push_back(std::unique_ptr<RTL>(new RTL(Address(0x1001), { jump })));
    QVERIFY(bb1.getIR()->isEmptyJump());

    std::shared_ptr<Assign> asgn(new Assign(Terminal::get(opNil), Terminal::get(opNil)));
    bb1.getIR()->getRTLs()->back()->push_front(asgn);
    QVERIFY(!bb1.getIR()->isEmptyJump());
}


QTEST_GUILESS_MAIN(BasicBlockTest)
