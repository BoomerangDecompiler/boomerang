#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "RangeVisitor.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Unary.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/ImpRefStatement.h"
#include "boomerang/db/statements/JunctionStatement.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/passes/range/Range.h"
#include "boomerang/passes/range/RangeMap.h"
#include "boomerang/passes/range/RangePrivateData.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/util/Log.h"


RangeVisitor::RangeVisitor(RangePrivateData *t, std::list<Statement *>& ex_paths)
    : tgt(t)
    , execution_paths(ex_paths)
{}


void RangeVisitor::processRange(Statement* i)
{
    RangeMap output = getInputRanges(i);

    updateRanges(i, output);
}


RangeMap RangeVisitor::getInputRanges(Statement *insn)
{
    if (!insn->isFirstStatementInBB()) {
        RangeMap SavedInputRanges = tgt->getRanges(insn->getPreviousStatementInBB());
        tgt->setSavedRanges(insn, SavedInputRanges);
        return SavedInputRanges;
    }

    assert(insn->getBB() && insn->getBB()->getNumPredecessors() <= 1);
    RangeMap input;

    if (insn->getBB()->getNumPredecessors() == 0) {
        // setup input for start of procedure
        Range ra24(1, 0, 0, Unary::get(opInitValueOf, Location::regOf(24)));
        Range ra25(1, 0, 0, Unary::get(opInitValueOf, Location::regOf(25)));
        Range ra26(1, 0, 0, Unary::get(opInitValueOf, Location::regOf(26)));
        Range ra27(1, 0, 0, Unary::get(opInitValueOf, Location::regOf(27)));
        Range ra28(1, 0, 0, Unary::get(opInitValueOf, Location::regOf(28)));
        Range ra29(1, 0, 0, Unary::get(opInitValueOf, Location::regOf(29)));
        Range ra30(1, 0, 0, Unary::get(opInitValueOf, Location::regOf(30)));
        Range ra31(1, 0, 0, Unary::get(opInitValueOf, Location::regOf(31)));
        Range rpc(1, 0, 0, Unary::get(opInitValueOf, Terminal::get(opPC)));
        input.addRange(Location::regOf(24), ra24);
        input.addRange(Location::regOf(25), ra25);
        input.addRange(Location::regOf(26), ra26);
        input.addRange(Location::regOf(27), ra27);
        input.addRange(Location::regOf(28), ra28);
        input.addRange(Location::regOf(29), ra29);
        input.addRange(Location::regOf(30), ra30);
        input.addRange(Location::regOf(31), ra31);
        input.addRange(Terminal::get(opPC), rpc);
    }
    else {
        BasicBlock *pred = insn->getBB()->getPredecessors()[0];
        Statement  *last = pred->getLastStmt();
        assert(last);

        if (pred->getNumSuccessors() != 2) {
            input = tgt->getRanges(last);
        }
        else {
            assert(pred->getNumSuccessors() == 2);
            assert(last->isBranch());
            input = getRangesForOutEdgeTo((BranchStatement *)last, insn->getBB());
        }
    }

    tgt->setSavedRanges(insn, input);
    return input;
}


void RangeVisitor::updateRanges(Statement* insn, RangeMap& output, bool notTaken)
{
    if (insn->isBranch()) {
        BranchStatement *self_branch = (BranchStatement *)insn;

        if (!output.isSubset(notTaken ? tgt->getBranchRange(self_branch) : tgt->getRanges(insn))) {
            if (notTaken) {
                tgt->setBranchRange(self_branch, output);
            }
            else {
                tgt->setRanges(insn, output);
            }

            if (insn->isLastStatementInBB()) {
                if (insn->getBB()->getNumSuccessors()) {
                    uint32_t arc = 0;

                    if (insn->getBB()->getSuccessor(0)->getLowAddr() != self_branch->getFixedDest()) {
                        arc = 1;
                    }

                    if (notTaken) {
                        arc ^= 1;
                    }

                    execution_paths.push_back(insn->getBB()->getSuccessor(arc)->getFirstStmt());
                }
            }
            else {
                execution_paths.push_back(insn->getNextStatementInBB());
            }
        }
    }
    else if (!notTaken) {
        if (!output.isSubset(tgt->getRanges(insn))) {
            tgt->setRanges(insn, output);

            if (insn->isLastStatementInBB()) {
                if (insn->getBB()->getNumSuccessors()) {
                    execution_paths.push_back(insn->getBB()->getSuccessor(0)->getFirstStmt());
                }
            }
            else {
                execution_paths.push_back(insn->getNextStatementInBB());
            }
        }
    }
}


bool RangeVisitor::visit(Assign* insn)
{
    static Unary search_term(opTemp, Terminal::get(opWild));
    static Unary search_regof(opRegOf, Terminal::get(opWild));
    RangeMap     output = getInputRanges(insn);
    auto         a_lhs  = insn->getLeft()->clone();

    if (a_lhs->isFlags()) {
        // special hacks for flags
        assert(insn->getRight()->isFlagCall());
        auto a_rhs = insn->getRight()->clone();

        if (a_rhs->getSubExp2()->getSubExp1()->isMemOf()) {
            a_rhs->getSubExp2()->getSubExp1()->setSubExp1(
                output.substInto(a_rhs->getSubExp2()->getSubExp1()->getSubExp1()));
        }

        if (!a_rhs->getSubExp2()->getSubExp2()->isTerminal() &&
                a_rhs->getSubExp2()->getSubExp2()->getSubExp1()->isMemOf()) {
            a_rhs->getSubExp2()->getSubExp2()->getSubExp1()->setSubExp1(
                output.substInto(a_rhs->getSubExp2()->getSubExp2()->getSubExp1()->getSubExp1()));
        }

        Range ra(1, 0, 0, a_rhs);
        output.addRange(a_lhs, ra);
    }
    else {
        if (a_lhs->isMemOf()) {
            a_lhs->setSubExp1(output.substInto(a_lhs->getSubExp1()->clone()));
        }

        auto a_rhs = output.substInto(insn->getRight()->clone());

        if (a_rhs->isMemOf() && (a_rhs->getSubExp1()->getOper() == opInitValueOf) &&
                a_rhs->getSubExp1()->getSubExp1()->isRegOfK() &&
                (a_rhs->access<Const, 1, 1, 1>()->getInt() == 28)) {
            a_rhs = Unary::get(opInitValueOf, Terminal::get(opPC)); // nice hack
        }

        if (DEBUG_RANGE_ANALYSIS) {
            LOG_VERBOSE("a_rhs is %1", a_rhs);
        }

        if (a_rhs->isMemOf() && a_rhs->getSubExp1()->isIntConst()) {
            Address c = a_rhs->access<Const, 1>()->getAddr();

            if (insn->getProc()->getProg()->isDynamicLinkedProcPointer(c)) {
                const QString& nam(insn->getProc()->getProg()->getDynamicProcName(c));

                if (!nam.isEmpty()) {
                    a_rhs = Const::get(nam);

                    if (DEBUG_RANGE_ANALYSIS) {
                        LOG_VERBOSE("a_rhs is a dynamic proc pointer to %1", nam);
                    }
                }
            }
            else if (Boomerang::get()->getImage()->isReadOnly(c)) {
                switch (insn->getType()->getSize())
                {
                case 8:
                    a_rhs = Const::get(Boomerang::get()->getImage()->readNative1(c));
                    break;

                case 16:
                    a_rhs = Const::get(Boomerang::get()->getImage()->readNative2(c));
                    break;

                case 32:
                    a_rhs = Const::get(Boomerang::get()->getImage()->readNative4(c));
                    break;

                default:
                    LOG_ERROR("Unhandled type size %1 for reading native address", insn->getType()->getSize());
                }
            }
            else if (DEBUG_RANGE_ANALYSIS) {
                LOG_VERBOSE("Address %1 is not dynamically linked proc pointer or in read only memory", c);
            }
        }

        if (((a_rhs->getOper() == opPlus) || (a_rhs->getOper() == opMinus)) && a_rhs->getSubExp2()->isIntConst() &&
                output.hasRange(a_rhs->getSubExp1())) {
            Range& r = output.getRange(a_rhs->getSubExp1());
            int    c = a_rhs->access<Const, 2>()->getInt();

            if (a_rhs->getOper() == opPlus) {
                Range ra(1, r.getLowerBound() != Range::MIN ? r.getLowerBound() + c : Range::MIN,
                         r.getUpperBound() != Range::MAX ? r.getUpperBound() + c : Range::MAX, r.getBase());
                output.addRange(a_lhs, ra);
            }
            else {
                Range ra(1, r.getLowerBound() != Range::MIN ? r.getLowerBound() - c : Range::MIN,
                         r.getUpperBound() != Range::MAX ? r.getUpperBound() - c : Range::MAX, r.getBase());
                output.addRange(a_lhs, ra);
            }
        }
        else {
            if (output.hasRange(a_rhs)) {
                output.addRange(a_lhs, output.getRange(a_rhs));
            }
            else {
                SharedExp result;

                if ((a_rhs->getMemDepth() == 0) && !a_rhs->search(search_regof, result) &&
                        !a_rhs->search(search_term, result)) {
                    if (a_rhs->isIntConst()) {
                        Range ra(1, a_rhs->access<Const>()->getInt(), a_rhs->access<Const>()->getInt(), Const::get(0));
                        output.addRange(a_lhs, ra);
                    }
                    else {
                        Range ra(1, 0, 0, a_rhs);
                        output.addRange(a_lhs, ra);
                    }
                }
                else {
                    Range empty;
                    output.addRange(a_lhs, empty);
                }
            }
        }
    }

    if (DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("Added %1 -> %2", a_lhs, output.getRange(a_lhs));
    }

    updateRanges(insn, output);

    if (DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("%1", insn);
    }

    return true;
}


bool RangeVisitor::visit(PhiAssign* stmt)
{
    processRange(stmt);
    return true;
}


bool RangeVisitor::visit(ImplicitAssign* stmt)
{
    processRange(stmt);
    return true;
}


bool RangeVisitor::visit(BoolAssign* stmt)
{
    processRange(stmt);
    return true;
}


bool RangeVisitor::visit(GotoStatement* stmt)
{
    processRange(stmt);
    return true;
}


bool RangeVisitor::visit(BranchStatement* stmt)
{
    RangeMap output = getInputRanges(stmt);

    SharedExp e = nullptr;
    // try to hack up a useful expression for this branch
    OPER op = stmt->getCondExpr()->getOper();

    if ((op == opLess) || (op == opLessEq) || (op == opGtr) || (op == opGtrEq) || (op == opLessUns) || (op == opLessEqUns) ||
            (op == opGtrUns) || (op == opGtrEqUns) || (op == opEquals) || (op == opNotEqual)) {
        if (stmt->getCondExpr()->getSubExp1()->isFlags() && output.hasRange(stmt->getCondExpr()->getSubExp1())) {
            Range& r = output.getRange(stmt->getCondExpr()->getSubExp1());

            if (r.getBase()->isFlagCall() && (r.getBase()->getSubExp2()->getOper() == opList) &&
                    (r.getBase()->getSubExp2()->getSubExp2()->getOper() == opList)) {
                e = Binary::get(op, r.getBase()->getSubExp2()->getSubExp1()->clone(),
                                r.getBase()->getSubExp2()->getSubExp2()->getSubExp1()->clone());

                if (DEBUG_RANGE_ANALYSIS) {
                    LOG_VERBOSE("Calculated condition %1", e);
                }
            }
        }
    }

    if (e) {
        limitOutputWithCondition(stmt, output, e);
    }

    updateRanges(stmt, output);
    output = getInputRanges(stmt);

    if (e) {
        limitOutputWithCondition(stmt, output, (Unary::get(opNot, e))->simplify());
    }

    updateRanges(stmt, output, true);

    if (DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("%1", stmt);
    }

    return true;
}


bool RangeVisitor::visit(CaseStatement* stmt)
{
    processRange(stmt);
    return true;
}


bool RangeVisitor::visit(CallStatement* stmt)
{
    if (!stmt) {
        return true;
    }

    RangeMap output = getInputRanges(stmt);

    if (stmt->getDestProc() == nullptr) {
        // note this assumes the call is only to one proc.. could be bad.
        auto d = output.substInto(stmt->getDest()->clone());

        if (d && (d->isIntConst() || d->isStrConst())) {
            if (d->isIntConst()) {
                Address dest = d->access<Const>()->getAddr();
                stmt->setDestProc(stmt->getProc()->getProg()->createFunction(dest));
            }
            else {
                stmt->setDestProc(stmt->getProc()->getProg()->getLibraryProc(d->access<Const>()->getStr()));
            }

            if (stmt->getDestProc()) {
                auto sig = stmt->getDestProc()->getSignature();
                stmt->setDest(d);

                StatementList newArguments;
                for (size_t i = 0; i < sig->getNumParams(); i++) {
                    SharedConstExp a   = sig->getParamExp(i);
                    Assign *as = new Assign(VoidType::get(),
                                            a->clone(),
                                            a->clone());
                    as->setProc(stmt->getProc());
                    as->setBB(stmt->getBB());
                    newArguments.append(as);
                }

                stmt->setArguments(newArguments);

                stmt->setSignature(stmt->getDestProc()->getSignature()->clone());
                stmt->setIsComputed(false);
                stmt->getProc()->undoComputedBB(stmt);
                stmt->getProc()->addCallee(stmt->getDestProc());

                LOG_MSG("Replaced indirect call with call to %1", stmt->getProc()->getName());
            }
        }
    }

    if (output.hasRange(Location::regOf(28))) {
        Range& r = output.getRange(Location::regOf(28));
        int    c = 4;

        if (stmt->getDestProc() == nullptr) {
            LOG_MSG("Using push count hack to guess number of params");
            Statement *prev = stmt->getPreviousStatementInBB();

            while (prev) {
                if (prev->isAssign() && ((Assign *)prev)->getLeft()->isMemOf() &&
                        ((Assign *)prev)->getLeft()->getSubExp1()->isRegOfK() &&
                        (((Assign *)prev)->getLeft()->access<Const, 1, 1>()->getInt() == 28) &&
                        (((Assign *)prev)->getRight()->getOper() != opPC)) {
                    c += 4;
                }

                prev = prev->getPreviousStatementInBB();
            }
        }
        else if (stmt->getDestProc()->getSignature()->getConvention() == CallConv::Pascal) {
            c += stmt->getDestProc()->getSignature()->getNumParams() * 4;
        }
        else if (!strncmp(qPrintable(stmt->getDestProc()->getName()), "__imp_", 6)) {
            Statement *first = ((UserProc *)stmt->getDestProc())->getCFG()->getEntryBB()->getFirstStmt();

            if (!first || !first->isCall()) {
                assert(false);
                return false;
            }

            Function *d = ((CallStatement *)first)->getDestProc();

            if (d && (d->getSignature()->getConvention() == CallConv::Pascal)) {
                c += d->getSignature()->getNumParams() * 4;
            }
        }
        else if (!stmt->getDestProc()->isLib()) {
            UserProc *p = (UserProc *)stmt->getDestProc();

            if (!p) {
                assert(false);
                return false;
            }

            LOG_VERBOSE("== checking for number of bytes popped ==");
            LOG_VERBOSE("%1", p->prints());
            LOG_VERBOSE("== end it ==");

            SharedExp eq = p->getProven(Location::regOf(28));

            if (eq) {
                LOG_VERBOSE("Found proven %1", eq);

                if ((eq->getOper() == opPlus) && (*eq->getSubExp1() == *Location::regOf(28)) &&
                        eq->getSubExp2()->isIntConst()) {
                    c = eq->access<Const, 2>()->getInt();
                }
                else {
                    eq = nullptr;
                }
            }

            BasicBlock *retbb = p->getCFG()->findRetNode();

            if (retbb && (eq == nullptr)) {
                Statement *last = retbb->getLastStmt();

                if (!last) {
                    assert(false);
                    return false;
                }

                if (last && last->isReturn()) {
                    last->setBB(retbb);
                    last = last->getPreviousStatementInBB();
                }

                if (last == nullptr) {
                    // call followed by a ret, sigh
                    for (size_t i = 0; i < retbb->getNumPredecessors(); i++) {
                        last = retbb->getPredecessors()[i]->getLastStmt();

                        if (last->isCall()) {
                            break;
                        }
                    }

                    if (last && last->isCall()) {
                        Function *d = ((CallStatement *)last)->getDestProc();

                        if (d && (d->getSignature()->getConvention() == CallConv::Pascal)) {
                            c += d->getSignature()->getNumParams() * 4;
                        }
                    }

                    last = nullptr;
                }

                if (last && last->isAssign()) {
                    // LOG << "checking last statement " << last << " for number of bytes popped\n";
                    Assign *a = (Assign *)last;
                    assert(a->getLeft()->isRegOfK() && (a->getLeft()->access<Const, 1>()->getInt() == 28));
                    auto t = a->getRight()->clone()->simplifyArith();
                    assert(t->getOper() == opPlus && t->getSubExp1()->isRegOfK() &&
                           (t->access<Const, 1, 1>()->getInt() == 28));
                    assert(t->getSubExp2()->isIntConst());
                    c = t->access<Const, 2>()->getInt();
                }
            }
        }

        Range ra(r.getStride(), r.getLowerBound() == Range::MIN ? Range::MIN : r.getLowerBound() + c,
                 r.getUpperBound() == Range::MAX ? Range::MAX : r.getUpperBound() + c, r.getBase());
        output.addRange(Location::regOf(28), ra);
    }

    updateRanges(stmt, output);
    return true;
}


bool RangeVisitor::visit(ReturnStatement* stmt)
{
    processRange(stmt);
    return true;
}


bool RangeVisitor::visit(ImpRefStatement* stmt)
{
    processRange(stmt);
    return true;
}


bool RangeVisitor::visit(JunctionStatement* stmt)
{
    RangeMap input;

    if (DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("unioning {");
    }

    for (size_t i = 0; i < stmt->getBB()->getNumPredecessors(); i++) {
        Statement *last = stmt->getBB()->getPredecessors()[i]->getLastStmt();

        if (DEBUG_RANGE_ANALYSIS) {
            LOG_VERBOSE("  in BB: address %1 %2", stmt->getBB()->getPredecessors()[i]->getLowAddr(), last);
        }

        if (last->isBranch()) {
            input.unionWith(getRangesForOutEdgeTo((BranchStatement *)last, stmt->getBB()));
        }
        else {
            if (last->isCall()) {
                Function *d = ((CallStatement *)last)->getDestProc();

                if (d && !d->isLib() && (((UserProc *)d)->getCFG()->findRetNode() == nullptr)) {
                    if (DEBUG_RANGE_ANALYSIS) {
                        LOG_VERBOSE("Ignoring ranges from call to proc with no ret node");
                    }
                }
                else {
                    input.unionWith(tgt->getRanges(last));
                }
            }
            else {
                input.unionWith(tgt->getRanges(last));
            }
        }
    }

    if (DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("}");
    }

    if (!input.isSubset(tgt->getRanges(stmt))) {
        RangeMap output = input;

        if (output.hasRange(Location::regOf(28))) {
            Range& r = output.getRange(Location::regOf(28));

            if ((r.getLowerBound() != r.getUpperBound()) && (r.getLowerBound() != Range::MIN)) {
                if (VERBOSE) {
                    LOG_ERROR("Stack height assumption violated %1", r.toString());
                    LOG_ERROR(" my BB: ", stmt->getBB()->getLowAddr());

                    QString     procStr;
                    QTextStream ost(&procStr);
                    stmt->getProc()->print(ost);
                    LOG_VERBOSE(procStr);
                }

                assert(false);
            }
        }

        if (stmt->isLoopJunction()) {
            output = tgt->getRanges(stmt);
            output.widenWith(input);
        }

        updateRanges(stmt, output);
    }

    if (DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("%1", stmt);
    }

    return true;
}


RangeMap& RangeVisitor::getRangesForOutEdgeTo(BranchStatement* b, BasicBlock* out)
{
    assert(b->getFixedDest() != Address::INVALID);

    if (out->getLowAddr() == b->getFixedDest()) {
        return tgt->getRanges(b);
    }

    return tgt->getBranchRange(b);
}


void RangeVisitor::limitOutputWithCondition(BranchStatement* stmt, RangeMap& output, const SharedExp& e)
{
    Q_UNUSED(stmt);
    assert(e);

    if (!output.hasRange(e->getSubExp1())) {
        return;
    }

    Range& r = output.getRange(e->getSubExp1());

    if (!(e->getSubExp2()->isIntConst() && r.getBase()->isIntConst() && (r.getBase()->access<Const>()->getInt() == 0))) {
        return;
    }

    int c = e->access<Const, 2>()->getInt();

    switch (e->getOper())
    {
    case opLess:
    case opLessUns:
    {
        Range ra(r.getStride(), r.getLowerBound() >= c ? c - 1 : r.getLowerBound(),
                 r.getUpperBound() >= c ? c - 1 : r.getUpperBound(), r.getBase());
        output.addRange(e->getSubExp1(), ra);
        break;
    }

    case opLessEq:
    case opLessEqUns:
    {
        Range ra(r.getStride(), r.getLowerBound() > c ? c : r.getLowerBound(),
                 r.getUpperBound() > c ? c : r.getUpperBound(), r.getBase());
        output.addRange(e->getSubExp1(), ra);
        break;
    }

    case opGtr:
    case opGtrUns:
    {
        Range ra(r.getStride(), r.getLowerBound() <= c ? c + 1 : r.getLowerBound(),
                 r.getUpperBound() <= c ? c + 1 : r.getUpperBound(), r.getBase());
        output.addRange(e->getSubExp1(), ra);
        break;
    }

    case opGtrEq:
    case opGtrEqUns:
    {
        Range ra(r.getStride(), r.getLowerBound() < c ? c : r.getLowerBound(),
                 r.getUpperBound() < c ? c : r.getUpperBound(), r.getBase());
        output.addRange(e->getSubExp1(), ra);
        break;
    }

    case opEquals:
    {
        Range ra(r.getStride(), c, c, r.getBase());
        output.addRange(e->getSubExp1(), ra);
        break;
    }

    case opNotEqual:
    {
        Range ra(r.getStride(), r.getLowerBound() == c ? c + 1 : r.getLowerBound(),
                 r.getUpperBound() == c ? c - 1 : r.getUpperBound(), r.getBase());
        output.addRange(e->getSubExp1(), ra);
        break;
    }

    default:
        break;
    }
}

