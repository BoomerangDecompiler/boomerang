#include "RangeAnalysis.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/db/Visitor.h"
#include "boomerang/type/Type.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/exp/ExpHelp.h"
#include "boomerang/db/Visitor.h"

#include "boomerang/db/statements/JunctionStatement.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/ImpRefStatement.h"

#include "boomerang/type/Type.h"

#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"

class Range : public Printable
{
protected:
    int m_stride;
    int m_lowerBound;
    int m_upperBound;
    SharedExp m_base;

public:
    Range();
    Range(int stride, int lowerBound, int upperBound, SharedExp base);

    SharedExp getBase() const { return m_base; }
    int getStride() const { return m_stride; }
    int getLowerBound() const { return m_lowerBound; }
    int getUpperBound() const { return m_upperBound; }
    void unionWith(Range& r);
    void widenWith(Range& r);
    QString toString() const override;
    bool operator==(Range& other);

    static const int MAX = INT32_MAX;
    static const int MIN = INT32_MIN;
};


class RangeMap : public Printable
{
protected:
    std::map<SharedExp, Range, lessExpStar> ranges;

public:
    RangeMap() {}
    void addRange(SharedExp loc, Range& r) { ranges[loc] = r; }
    bool hasRange(const SharedExp& loc) { return ranges.find(loc) != ranges.end(); }
    Range& getRange(const SharedExp& loc);
    void unionwith(RangeMap& other);
    void widenwith(RangeMap& other);
    QString toString() const override;
    void print() const;
    SharedExp substInto(SharedExp e, ExpSet *only = nullptr) const;
    void killAllMemOfs();

    void clear() { ranges.clear(); }

    /// return true if this range map is a subset of the other range map
    bool isSubset(RangeMap& other) const;

    bool empty() const { return ranges.empty(); }
};


struct RangePrivateData
{
    RangeMap&                             getRanges(Instruction *insn)
    {
        return Ranges[insn];
    }

    void                                  setRanges(Instruction *insn, RangeMap r)
    {
        Ranges[insn] = r;
    }

    void                                  clearRanges()
    {
        SavedInputRanges.clear();
    }

    RangeMap&                             getBranchRange(BranchStatement *s)
    {
        return BranchRanges[s];
    }

    void                                  setBranchRange(BranchStatement *s, RangeMap& rm)
    {
        BranchRanges[s] = rm;
    }

    void setSavedRanges(Instruction *insn, RangeMap map);
    RangeMap                              getSavedRanges(Instruction *insn);

public:
    std::map<Instruction *, RangeMap>     SavedInputRanges; ///< overestimation of ranges of locations
    std::map<Instruction *, RangeMap>     Ranges;           ///< saved overestimation of ranges of locations
    std::map<BranchStatement *, RangeMap> BranchRanges;
};


RangeAnalysis::RangeAnalysis()
    : RangeData(new RangePrivateData)
{
}


void RangeAnalysis::addJunctionStatements(Cfg& cfg)
{
    for (BasicBlock *pbb : cfg) {
        if ((pbb->getNumInEdges() > 1) && ((pbb->getFirstStmt() == nullptr) || !pbb->getFirstStmt()->isJunction())) {
            assert(pbb->getRTLs());
            JunctionStatement *j = new JunctionStatement();
            j->setBB(pbb);
            pbb->getRTLs()->front()->push_front(j);
        }
    }
}


void RangeAnalysis::clearRanges()
{
    RangeData->clearRanges();
}


class RangeVisitor : public StmtVisitor
{
public:
    RangePrivateData          *tgt;
    std::list<Instruction *>& execution_paths;
    RangeVisitor(RangePrivateData *t, std::list<Instruction *>& ex_paths)
        : tgt(t)
        , execution_paths(ex_paths)
    {
    }

    void processRange(Instruction *i)
    {
        RangeMap output = getInputRanges(i);

        updateRanges(i, output);
    }

    RangeMap getInputRanges(Instruction *insn)
    {
        if (!insn->isFirstStatementInBB()) {
            RangeMap SavedInputRanges = tgt->getRanges(insn->getPreviousStatementInBB());
            tgt->setSavedRanges(insn, SavedInputRanges);
            return SavedInputRanges;
        }

        assert(insn->getBB() && insn->getBB()->getNumInEdges() <= 1);
        RangeMap input;

        if (insn->getBB()->getNumInEdges() == 0) {
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
            BasicBlock  *pred = insn->getBB()->getInEdges()[0];
            Instruction *last = pred->getLastStmt();
            assert(last);

            if (pred->getNumOutEdges() != 2) {
                input = tgt->getRanges(last);
            }
            else {
                assert(pred->getNumOutEdges() == 2);
                assert(last->isBranch());
                input = getRangesForOutEdgeTo((BranchStatement *)last, insn->getBB());
            }
        }

        tgt->setSavedRanges(insn, input);
        return input;
    }

    void updateRanges(Instruction *insn, RangeMap& output, bool notTaken = false)
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
                    if (insn->getBB()->getNumOutEdges()) {
                        uint32_t arc = 0;

                        if (insn->getBB()->getOutEdge(0)->getLowAddr() != self_branch->getFixedDest()) {
                            arc = 1;
                        }

                        if (notTaken) {
                            arc ^= 1;
                        }

                        execution_paths.push_back(insn->getBB()->getOutEdge(arc)->getFirstStmt());
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
                    if (insn->getBB()->getNumOutEdges()) {
                        execution_paths.push_back(insn->getBB()->getOutEdge(0)->getFirstStmt());
                    }
                }
                else {
                    execution_paths.push_back(insn->getNextStatementInBB());
                }
            }
        }
    }

    bool visit(Assign *insn) override
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

    bool visit(PhiAssign *stmt) override { processRange(stmt); return true; }
    bool visit(ImplicitAssign *stmt) override { processRange(stmt); return true; }
    bool visit(BoolAssign *stmt) override { processRange(stmt); return true; }
    bool visit(GotoStatement *stmt) override { processRange(stmt);  return true; }
    bool visit(BranchStatement *stmt) override
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

    bool visit(CaseStatement *stmt) override { processRange(stmt); return true; }
    bool visit(CallStatement *stmt) override
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
                    stmt->setDestProc(stmt->getProc()->getProg()->createProc(dest));
                }
                else {
                    stmt->setDestProc(stmt->getProc()->getProg()->getLibraryProc(d->access<Const>()->getStr()));
                }

                if (stmt->getDestProc()) {
                    auto sig = stmt->getDestProc()->getSignature();
                    stmt->setDest(d);
                    stmt->getArguments().clear();

                    for (size_t i = 0; i < sig->getNumParams(); i++) {
                        auto   a   = sig->getParamExp(i);
                        Assign *as = new Assign(VoidType::get(), a->clone(), a->clone());
                        if (as) {
                            as->setProc(stmt->getProc());
                            as->setBB(stmt->getBB());
                            stmt->getArguments().append(as);
                        }
                    }

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
                Instruction *prev = stmt->getPreviousStatementInBB();

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
                Instruction *first = ((UserProc *)stmt->getDestProc())->getCFG()->getEntryBB()->getFirstStmt();
                if (!first || !first->isCall()) {
                    assert(false);
                    return false;
                }

                Function *d = ((CallStatement *)first)->getDestProc();

                if (d && d->getSignature()->getConvention() == CallConv::Pascal) {
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
                    Instruction *last = retbb->getLastStmt();
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
                        for (size_t i = 0; i < retbb->getNumInEdges(); i++) {
                            last = retbb->getInEdges()[i]->getLastStmt();

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

    bool visit(ReturnStatement *stmt) override { processRange(stmt); return true; }
    bool visit(ImpRefStatement *stmt) override { processRange(stmt); return true; }
    bool visit(JunctionStatement *stmt) override
    {
        RangeMap input;

        if (DEBUG_RANGE_ANALYSIS) {
            LOG_VERBOSE("unioning {");
        }

        for (size_t i = 0; i < stmt->getBB()->getNumInEdges(); i++) {
            Instruction *last = stmt->getBB()->getInEdges()[i]->getLastStmt();

            if (DEBUG_RANGE_ANALYSIS) {
                LOG_VERBOSE("  in BB: address %1 %2", stmt->getBB()->getInEdges()[i]->getLowAddr(), last);
            }

            if (last->isBranch()) {
                input.unionwith(getRangesForOutEdgeTo((BranchStatement *)last, stmt->getBB()));
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
                        input.unionwith(tgt->getRanges(last));
                    }
                }
                else {
                    input.unionwith(tgt->getRanges(last));
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

                        QString procStr;
                        QTextStream ost(&procStr);
                        stmt->getProc()->print(ost);
                        LOG_VERBOSE(procStr);
                    }

                    assert(false);
                }
            }

            if (stmt->isLoopJunction()) {
                output = tgt->getRanges(stmt);
                output.widenwith(input);
            }

            updateRanges(stmt, output);
        }

        if (DEBUG_RANGE_ANALYSIS) {
            LOG_VERBOSE("%1",stmt);
        }

        return true;
    }

private:
    RangeMap& getRangesForOutEdgeTo(BranchStatement *b, BasicBlock *out)
    {
        assert(b->getFixedDest() != Address::INVALID);

        if (out->getLowAddr() == b->getFixedDest()) {
            return tgt->getRanges(b);
        }

        return tgt->getBranchRange(b);
    }

    void limitOutputWithCondition(BranchStatement *stmt, RangeMap& output, const SharedExp& e)
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
};


bool RangeAnalysis::runOnFunction(Function& F)
{
    if (F.isLib()) {
        return false;
    }

    LOG_VERBOSE("Performing range analysis on %1", F.getName());

    UserProc& UF((UserProc&)F);
    assert(UF.getCFG());

    // this helps
    UF.getCFG()->sortByAddress();

    addJunctionStatements(*UF.getCFG());
    UF.getCFG()->establishDFTOrder();

    clearRanges();

    UF.debugPrintAll("Before performing range analysis");

    std::list<Instruction *> execution_paths;
    std::list<Instruction *> junctions;

    assert(UF.getCFG()->getEntryBB());
    assert(UF.getCFG()->getEntryBB()->getFirstStmt());
    execution_paths.push_back(UF.getCFG()->getEntryBB()->getFirstStmt());

    int          watchdog = 0;
    RangeVisitor rv(this->RangeData, execution_paths);

    while (!execution_paths.empty()) {
        while (!execution_paths.empty()) {
            Instruction *stmt = execution_paths.front();
            execution_paths.pop_front();

            if (stmt == nullptr) {
                continue; // ??
            }

            if (stmt->isJunction()) {
                junctions.push_back(stmt);
            }
            else {
                stmt->accept(&rv);
            }
        }

        if (watchdog > 45) {
            LOG_MSG("Processing execution paths resulted in %1 junctions to process", junctions.size());
        }

        while (!junctions.empty()) {
            Instruction *junction = junctions.front();
            junctions.pop_front();

            if (watchdog > 45) {
                LOG_MSG("Processing junction %1", junction);
            }

            assert(junction->isJunction());
            junction->accept(&rv);
        }

        watchdog++;

        if (watchdog > 10) {
            LOG_MSG("  watchdog %1", watchdog);

            if (watchdog > 45) {
                LOG_MSG("%1 execution paths remaining.", execution_paths.size());
                LOG_SEPARATE(UF.getName(), "=== After range analysis watchdog %1 ===", watchdog, UF.getName());
                LOG_SEPARATE(UF.getName(), "%1", UF);
                LOG_SEPARATE(UF.getName(), "=== End after range analysis watchdog %1 for %2 ===", watchdog, UF.getName());
            }
        }

        if (watchdog > 50) {
            LOG_MSG("  watchdog expired");
            break;
        }
    }

    UF.debugPrintAll("After range analysis");

    UF.getCFG()->removeJunctionStatements();
    logSuspectMemoryDefs(UF);
    return true;
}


RangeMap RangePrivateData::getSavedRanges(Instruction *insn)
{
    return SavedInputRanges[insn];
}


void RangePrivateData::setSavedRanges(Instruction *insn, RangeMap map)
{
    SavedInputRanges[insn] = map;
}


void RangeAnalysis::logSuspectMemoryDefs(UserProc& UF)
{
    StatementList stmts;

    UF.getStatements(stmts);

    for (Instruction *st : stmts) {
        if (!st->isAssign()) {
            continue;
        }

        Assign *a = (Assign *)st;

        if (!a->getLeft()->isMemOf()) {
            continue;
        }

        RangeMap& rm = RangeData->getRanges(st);
        SharedExp p  = rm.substInto(a->getLeft()->getSubExp1()->clone());

        if (rm.hasRange(p)) {
            Range& r = rm.getRange(p);
            LOG_VERBOSE("Got p %1 with range %2", p, r.toString());

            if ((r.getBase()->getOper() == opInitValueOf) && r.getBase()->getSubExp1()->isRegOfK() &&
                (r.getBase()->access<Const, 1, 1>()->getInt() == 28)) {
                RTL *rtl = a->getBB()->getRTLWithStatement(a);
                LOG_VERBOSE("Interesting stack reference at address %1: %2", rtl->getAddress(), a);
            }
        }
    }
}


Range::Range()
    : m_stride(1)
    , m_lowerBound(MIN)
    , m_upperBound(MAX)
{
    m_base = Const::get(0);
}


Range::Range(int _stride, int _lowerBound, int _upperBound, SharedExp _base)
    : m_stride(_stride)
    , m_lowerBound(_lowerBound)
    , m_upperBound(_upperBound)
    , m_base(_base)
{
    if ((m_lowerBound == m_upperBound) && (m_lowerBound == 0) && ((m_base->getOper() == opMinus) || (m_base->getOper() == opPlus)) &&
        m_base->getSubExp2()->isIntConst()) {
        this->m_lowerBound = m_base->access<Const, 2>()->getInt();

        if (m_base->getOper() == opMinus) {
            this->m_lowerBound = -this->m_lowerBound;
        }

        this->m_upperBound = this->m_lowerBound;
        this->m_base       = m_base->getSubExp1();
    }
    else {
        if (m_base == nullptr) {
            // NOTE: was "base = Const::get(0);"
            this->m_base = Const::get(0);
        }

        if (m_lowerBound > m_upperBound) {
            this->m_upperBound = m_lowerBound;
        }

        if (m_upperBound < m_lowerBound) {
            this->m_lowerBound = m_upperBound;
        }
    }
}


QString Range::toString() const
{
    QString     res;
    QTextStream os(&res);

    assert(m_lowerBound <= m_upperBound);

    if (m_base->isIntConst() && (m_base->access<Const>()->getInt() == 0) && (m_lowerBound == MIN) && (m_upperBound == MAX)) {
        os << "T";
        return res;
    }

    bool needPlus = false;

    if (m_lowerBound == m_upperBound) {
        if (!m_base->isIntConst() || (m_base->access<Const>()->getInt() != 0)) {
            if (m_lowerBound != 0) {
                os << m_lowerBound;
                needPlus = true;
            }
        }
        else {
            needPlus = true;
            os << m_lowerBound;
        }
    }
    else {
        if (m_stride != 1) {
            os << m_stride;
        }

        os << "[";

        if (m_lowerBound == MIN) {
            os << "-inf";
        }
        else {
            os << m_lowerBound;
        }

        os << ", ";

        if (m_upperBound == MAX) {
            os << "inf";
        }
        else {
            os << m_upperBound;
        }

        os << "]";
        needPlus = true;
    }

    if (!m_base->isIntConst() || (m_base->access<Const>()->getInt() != 0)) {
        if (needPlus) {
            os << " + ";
        }

        m_base->print(os);
    }

    return res;
}


void Range::unionWith(Range& r)
{
    if (DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("unioning %1 with %2 got...", toString(), r);
    }

    assert(m_base && r.m_base);

    if ((m_base->getOper() == opMinus) && (r.m_base->getOper() == opMinus) && (*m_base->getSubExp1() == *r.m_base->getSubExp1()) &&
        m_base->getSubExp2()->isIntConst() && r.m_base->getSubExp2()->isIntConst()) {
        int c1 = m_base->access<Const, 2>()->getInt();
        int c2 = r.m_base->access<Const, 2>()->getInt();

        if (c1 != c2) {
            if ((m_lowerBound == r.m_lowerBound) && (m_upperBound == r.m_upperBound) && (m_lowerBound == 0)) {
                m_lowerBound = std::min(-c1, -c2);
                m_upperBound = std::max(-c1, -c2);
                m_base       = m_base->getSubExp1();

                if (DEBUG_RANGE_ANALYSIS) {
                    LOG_VERBOSE("%1", toString());
                }

                return;
            }
        }
    }

    if (!(*m_base == *r.m_base)) {
        m_stride     = 1;
        m_lowerBound = MIN;
        m_upperBound = MAX;
        m_base       = Const::get(0);

        if (DEBUG_RANGE_ANALYSIS) {
            LOG_VERBOSE("%1", toString());
        }

        return;
    }

    if (m_stride != r.m_stride) {
        m_stride = std::min(m_stride, r.m_stride);
    }

    if (m_lowerBound != r.m_lowerBound) {
        m_lowerBound = std::min(m_lowerBound, r.m_lowerBound);
    }

    if (m_upperBound != r.m_upperBound) {
        m_upperBound = std::max(m_upperBound, r.m_upperBound);
    }

    if (VERBOSE && DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("%1", toString());
    }
}


void Range::widenWith(Range& r)
{
    if (VERBOSE && DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("Widening %1 with %2 got...", toString(), r);
    }

    if (!(*m_base == *r.m_base)) {
        m_stride     = 1;
        m_lowerBound = MIN;
        m_upperBound = MAX;
        m_base       = Const::get(0);

        if (VERBOSE && DEBUG_RANGE_ANALYSIS) {
            LOG_VERBOSE("%1", toString());
        }

        return;
    }

    // ignore stride for now
    if (r.getLowerBound() < m_lowerBound) {
        m_lowerBound = MIN;
    }

    if (r.getUpperBound() > m_upperBound) {
        m_upperBound = MAX;
    }

    if (DEBUG_RANGE_ANALYSIS) {
        LOG_MSG(this->toString());
    }
}


Range& RangeMap::getRange(const SharedExp& loc)
{
    if (ranges.find(loc) == ranges.end()) {
        return *(new Range(1, Range::MIN, Range::MAX, Const::get(0)));
    }

    return ranges[loc];
}


void RangeMap::unionwith(RangeMap& other)
{
    for (auto& elem : other.ranges) {
        if (ranges.find((elem).first) == ranges.end()) {
            ranges[(elem).first] = (elem).second;
        }
        else {
            ranges[(elem).first].unionWith((elem).second);
        }
    }
}


void RangeMap::widenwith(RangeMap& other)
{
    for (auto& elem : other.ranges) {
        if (ranges.find((elem).first) == ranges.end()) {
            ranges[(elem).first] = (elem).second;
        }
        else {
            ranges[(elem).first].widenWith((elem).second);
        }
    }
}


QString RangeMap::toString() const
{
    QStringList res;

    for (const std::pair<SharedExp, Range>& elem : ranges) {
        res += QString("%1 -> %2").arg(elem.first->toString()).arg(elem.second.toString());
    }

    return res.join(", ");
}


SharedExp RangeMap::substInto(SharedExp e, ExpSet *only) const
{
    bool changes;
    int  count = 0;

    do {
        changes = false;

        for (const std::pair<SharedExp, Range>& elem : ranges) {
            if (only && (only->find(elem.first) == only->end())) {
                continue;
            }

            bool      change = false;
            SharedExp eold   = nullptr;

            if (DEBUG_RANGE_ANALYSIS) {
                eold = e->clone();
            }

            if (elem.second.getLowerBound() == elem.second.getUpperBound()) {
                // The following likely contains a bug, the replace argument is using direct pointer to
                // elem.second.getBase() instead of a clone.
                e = e->searchReplaceAll(*elem.first,
                                        (Binary::get(opPlus, elem.second.getBase(),
                                                     Const::get(elem.second.getLowerBound())))->simplify(),
                                        change);
            }

            if (change) {
                e = e->simplify()->simplifyArith();

                if (DEBUG_RANGE_ANALYSIS) {
                    LOG_VERBOSE("Applied %1 to %2 to get %3", elem.first, eold, e);
                }

                changes = true;
            }
        }

        count++;
        assert(count < 5);
    } while (changes);

    return e;
}


void RangeMap::killAllMemOfs()
{
    for (auto& elem : ranges) {
        if ((elem).first->isMemOf()) {
            Range empty;
            (elem).second.unionWith(empty);
        }
    }
}


bool Range::operator==(Range& other)
{
    return m_stride == other.m_stride && m_lowerBound == other.m_lowerBound && m_upperBound == other.m_upperBound &&
           *m_base == *other.m_base;
}


bool RangeMap::isSubset(RangeMap& other) const
{
    for (std::pair<SharedExp, Range> it : ranges) {
        if (other.ranges.find(it.first) == other.ranges.end()) {
            if (DEBUG_RANGE_ANALYSIS) {
                LOG_VERBOSE("Could not find %1 in other, not a subset", it.first);
            }

            return false;
        }

        Range& r = other.ranges[it.first];

        if (!(it.second == r)) {
            if (DEBUG_RANGE_ANALYSIS) {
                LOG_VERBOSE("Range for %1 in other %2 is not equal to range in this %3, not a subset",
                            it.first, r, it.second);
            }

            return false;
        }
    }

    return true;
}
