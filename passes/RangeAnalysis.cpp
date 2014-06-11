#include "RangeAnalysis.h"

#include "proc.h"
#include "boomerang.h"
#include "log.h"
#include "cfg.h"
#include "basicblock.h"
#include "rtl.h"
#include "type.h"
#include "prog.h"
#include "statement.h"
#include "signature.h"
#include "visitor.h"
#include "exp.h"
#include "exphelp.h"

class Range {
  protected:
    int stride, lowerBound, upperBound;
    Exp *base;

  public:
    Range();
    Range(int stride, int lowerBound, int upperBound, Exp *base);

    Exp *getBase() { return base; }
    int getStride() { return stride; }
    int getLowerBound() { return lowerBound; }
    int getUpperBound() { return upperBound; }
    void unionWith(Range &r);
    void widenWith(Range &r);
    void print(QTextStream &os) const;
    bool operator==(Range &other);

    static const int MAX = 2147483647;
    static const int MIN = -2147483647;
};
class RangeMap {
  protected:
    std::map<Exp *, Range, lessExpStar> ranges;

  public:
    RangeMap() {}
    void addRange(Exp *loc, Range &r) { ranges[loc] = r; }
    bool hasRange(Exp *loc) { return ranges.find(loc) != ranges.end(); }
    Range &getRange(Exp *loc);
    void unionwith(RangeMap &other);
    void widenwith(RangeMap &other);
    void print(QTextStream &os) const;
    Exp *substInto(Exp *e, std::set<Exp *, lessExpStar> *only = nullptr);
    void killAllMemOfs();
    void clear() { ranges.clear(); }
    bool isSubset(RangeMap &other);
    bool empty() const { return ranges.empty(); }
};
struct RangePrivateData {
    std::map<Instruction *,RangeMap> SavedInputRanges; //!< overestimation of ranges of locations
    std::map<Instruction *,RangeMap> Ranges;           //!< saved overestimation of ranges of locations
    std::map<BranchStatement *,RangeMap> BranchRanges;
    RangeMap &getRanges(Instruction *insn);
    void setRanges(Instruction *insn,RangeMap r);
    void clearRanges() {
        SavedInputRanges.clear();
    }
    RangeMap &getBranchRange(BranchStatement *s) {
        return BranchRanges[s];
    }
    void setBranchRange(BranchStatement *s,RangeMap &rm) {
        BranchRanges[s]=rm;
    }
public:
    void setSavedRanges(Instruction *insn, RangeMap map);
    RangeMap getSavedRanges(Instruction *insn);
};
RangeAnalysis::RangeAnalysis() : RangeData(new RangePrivateData)
{
}
/***************************************************************************/ /**
  * \brief Add Junction statements
  *******************************************************************************/
void RangeAnalysis::addJunctionStatements(Cfg &cfg) {
    for (BasicBlock *pbb : cfg) {
        if (pbb->getNumInEdges() > 1 && (pbb->getFirstStmt() == nullptr || !pbb->getFirstStmt()->isJunction())) {
            assert(pbb->getRTLs());
            JunctionStatement *j = new JunctionStatement();
            j->setBB(pbb);
            pbb->getRTLs()->front()->push_front(j);
        }
    }
}
void RangeAnalysis::clearRanges() {
    RangeData->clearRanges();
}
struct rangeVisitor : public StmtVisitor {
    RangePrivateData *tgt;
    std::list<Instruction *> &execution_paths;
    void processRange(Instruction *i) {
        RangeMap output = getInputRanges(i);
        updateRanges(i,output);
    }
    RangeMap getInputRanges(Instruction *insn) {
        if (!insn->isFirstStatementInBB()) {
            RangeMap SavedInputRanges = tgt->getRanges(insn->getPreviousStatementInBB());
            tgt->setSavedRanges(insn,SavedInputRanges);
            return SavedInputRanges;
        }

        assert(insn->getBB() && insn->getBB()->getNumInEdges() <= 1);
        RangeMap input;
        if (insn->getBB()->getNumInEdges() == 0) {
            // setup input for start of procedure
            Range ra24(1, 0, 0, new Unary(opInitValueOf, Location::regOf(24)));
            Range ra25(1, 0, 0, new Unary(opInitValueOf, Location::regOf(25)));
            Range ra26(1, 0, 0, new Unary(opInitValueOf, Location::regOf(26)));
            Range ra27(1, 0, 0, new Unary(opInitValueOf, Location::regOf(27)));
            Range ra28(1, 0, 0, new Unary(opInitValueOf, Location::regOf(28)));
            Range ra29(1, 0, 0, new Unary(opInitValueOf, Location::regOf(29)));
            Range ra30(1, 0, 0, new Unary(opInitValueOf, Location::regOf(30)));
            Range ra31(1, 0, 0, new Unary(opInitValueOf, Location::regOf(31)));
            Range rpc(1, 0, 0, new Unary(opInitValueOf, Terminal::get(opPC)));
            input.addRange(Location::regOf(24), ra24);
            input.addRange(Location::regOf(25), ra25);
            input.addRange(Location::regOf(26), ra26);
            input.addRange(Location::regOf(27), ra27);
            input.addRange(Location::regOf(28), ra28);
            input.addRange(Location::regOf(29), ra29);
            input.addRange(Location::regOf(30), ra30);
            input.addRange(Location::regOf(31), ra31);
            input.addRange(Terminal::get(opPC), rpc);
        } else {
            BasicBlock *pred = insn->getBB()->getInEdges()[0];
            Instruction *last = pred->getLastStmt();
            assert(last);
            if (pred->getNumOutEdges() != 2) {
                input = tgt->getRanges(last);
            } else {
                assert(pred->getNumOutEdges() == 2);
                assert(last->isBranch());
                input = getRangesForOutEdgeTo((BranchStatement *)last,insn->getBB());
            }
        }
        tgt->setSavedRanges(insn,input);
        return input;
    }
    void updateRanges(Instruction *insn,RangeMap &output, bool notTaken=false) {
        if (insn->isBranch()) {
            BranchStatement *self_branch = (BranchStatement *)insn;
            if (!output.isSubset(notTaken ? tgt->getBranchRange(self_branch) : tgt->getRanges(insn)) ) {
                if (notTaken)
                    tgt->setBranchRange(self_branch,output);
                else
                    tgt->setRanges(insn,output);
                if (insn->isLastStatementInBB()) {
                    if (insn->getBB()->getNumOutEdges()) {
                        uint32_t arc = 0;
                        if (insn->getBB()->getOutEdge(0)->getLowAddr() != self_branch->getFixedDest())
                            arc = 1;
                        if (notTaken)
                            arc ^= 1;
                        execution_paths.push_back(insn->getBB()->getOutEdge(arc)->getFirstStmt());
                    }
                } else
                    execution_paths.push_back(insn->getNextStatementInBB());
            }
        } else if (!notTaken) {
            if (!output.isSubset(tgt->getRanges(insn))) {
                tgt->setRanges(insn,output);
                if (insn->isLastStatementInBB()) {
                    if (insn->getBB()->getNumOutEdges()) {
                        execution_paths.push_back(insn->getBB()->getOutEdge(0)->getFirstStmt());
                    }
                } else
                    execution_paths.push_back(insn->getNextStatementInBB());
            }
        }
    }

    bool visit(Assign *insn) {
        static Unary search_term(opTemp, Terminal::get(opWild));
        static Unary search_regof(opRegOf, Terminal::get(opWild));
        RangeMap output = getInputRanges(insn);
        Exp *a_lhs = insn->getLeft()->clone();
        if (a_lhs->isFlags()) {
            // special hacks for flags
            assert(insn->getRight()->isFlagCall());
            Exp *a_rhs = insn->getRight()->clone();
            if (a_rhs->getSubExp2()->getSubExp1()->isMemOf())
                a_rhs->getSubExp2()->getSubExp1()->setSubExp1(
                    output.substInto(a_rhs->getSubExp2()->getSubExp1()->getSubExp1()));
            if (!a_rhs->getSubExp2()->getSubExp2()->isTerminal() &&
                a_rhs->getSubExp2()->getSubExp2()->getSubExp1()->isMemOf())
                a_rhs->getSubExp2()->getSubExp2()->getSubExp1()->setSubExp1(
                    output.substInto(a_rhs->getSubExp2()->getSubExp2()->getSubExp1()->getSubExp1()));
            Range ra(1, 0, 0, a_rhs);
            output.addRange(a_lhs, ra);
        } else {
            if (a_lhs->isMemOf())
                a_lhs->setSubExp1(output.substInto(a_lhs->getSubExp1()->clone()));
            Exp *a_rhs = output.substInto(insn->getRight()->clone());
            if (a_rhs->isMemOf() && a_rhs->getSubExp1()->getOper() == opInitValueOf &&
                a_rhs->getSubExp1()->getSubExp1()->isRegOfK() &&
                ((Const *)a_rhs->getSubExp1()->getSubExp1()->getSubExp1())->getInt() == 28)
                a_rhs = new Unary(opInitValueOf, Terminal::get(opPC)); // nice hack
            if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                LOG << "a_rhs is " << a_rhs << "\n";
            if (a_rhs->isMemOf() && a_rhs->getSubExp1()->isIntConst()) {
                ADDRESS c = ((Const *)a_rhs->getSubExp1())->getAddr();
                if (insn->getProc()->getProg()->isDynamicLinkedProcPointer(c)) {
                    const char *nam = insn->getProc()->getProg()->GetDynamicProcName(c);
                    if (nam) {
                        a_rhs = Const::get(nam);
                        if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                            LOG << "a_rhs is a dynamic proc pointer to " << nam << "\n";
                    }
                } else if (insn->getProc()->getProg()->isReadOnly(c)) {
                    switch (insn->getType()->getSize()) {
                    case 8:
                        a_rhs = Const::get(insn->getProc()->getProg()->readNative1(c));
                        break;
                    case 16:
                        a_rhs = Const::get(insn->getProc()->getProg()->readNative2(c));
                        break;
                    case 32:
                        a_rhs = Const::get(insn->getProc()->getProg()->readNative4(c));
                        break;
                    default:
                        LOG << "error: unhandled type size " << (int)insn->getType()->getSize() << " for reading native address\n";
                    }
                } else if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                    LOG << c << " is not dynamically linked proc pointer or in read only memory\n";
            }
            if ((a_rhs->getOper() == opPlus || a_rhs->getOper() == opMinus) && a_rhs->getSubExp2()->isIntConst() &&
                output.hasRange(a_rhs->getSubExp1())) {
                Range &r = output.getRange(a_rhs->getSubExp1());
                int c = ((Const *)a_rhs->getSubExp2())->getInt();
                if (a_rhs->getOper() == opPlus) {
                    Range ra(1, r.getLowerBound() != Range::MIN ? r.getLowerBound() + c : Range::MIN,
                             r.getUpperBound() != Range::MAX ? r.getUpperBound() + c : Range::MAX, r.getBase());
                    output.addRange(a_lhs, ra);
                } else {
                    Range ra(1, r.getLowerBound() != Range::MIN ? r.getLowerBound() - c : Range::MIN,
                             r.getUpperBound() != Range::MAX ? r.getUpperBound() - c : Range::MAX, r.getBase());
                    output.addRange(a_lhs, ra);
                }
            } else {
                if (output.hasRange(a_rhs)) {
                    output.addRange(a_lhs, output.getRange(a_rhs));
                } else {
                    Exp *result;
                    if (a_rhs->getMemDepth() == 0 && !a_rhs->search(search_regof, result) &&
                        !a_rhs->search(search_term, result)) {
                        if (a_rhs->isIntConst()) {
                            Range ra(1, ((Const *)a_rhs)->getInt(), ((Const *)a_rhs)->getInt(), Const::get(0));
                            output.addRange(a_lhs, ra);
                        } else {
                            Range ra(1, 0, 0, a_rhs);
                            output.addRange(a_lhs, ra);
                        }
                    } else {
                        Range empty;
                        output.addRange(a_lhs, empty);
                    }
                }
            }
        }
        if (DEBUG_RANGE_ANALYSIS)
            LOG_VERBOSE(1) << "added " << a_lhs << " -> " << output.getRange(a_lhs) << "\n";
        updateRanges(output, execution_paths);
        if (DEBUG_RANGE_ANALYSIS)
            LOG_VERBOSE(1) << insn << "\n";
        return true;
    }

    bool visit(PhiAssign * stmt) { processRange(stmt); return true; }
    bool visit(ImplicitAssign * stmt) { processRange(stmt); return true; }
    bool visit(BoolAssign * stmt) { processRange(stmt); return true; }
    bool visit(GotoStatement * stmt) {processRange(stmt);  return true; }
    bool visit(BranchStatement * stmt) {
        RangeMap output = getInputRanges(stmt);

        Exp *e = nullptr;
        // try to hack up a useful expression for this branch
        OPER op = stmt->getCondExpr()->getOper();
        if (op == opLess || op == opLessEq || op == opGtr || op == opGtrEq || op == opLessUns || op == opLessEqUns ||
            op == opGtrUns || op == opGtrEqUns || op == opEquals || op == opNotEqual) {
            if (stmt->pCond->getSubExp1()->isFlags() && output.hasRange(stmt->getCondExpr()->getSubExp1())) {
                Range &r = output.getRange(stmt->getCondExpr()->getSubExp1());
                if (r.getBase()->isFlagCall() && r.getBase()->getSubExp2()->getOper() == opList &&
                    r.getBase()->getSubExp2()->getSubExp2()->getOper() == opList) {
                    e = Binary::get(op, r.getBase()->getSubExp2()->getSubExp1()->clone(),
                                    r.getBase()->getSubExp2()->getSubExp2()->getSubExp1()->clone());
                    if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                        LOG << "calculated condition " << e << "\n";
                }
            }
        }

        if (e)
            limitOutputWithCondition(stmt,output, e);
        updateRanges(output, execution_paths);
        output = getInputRanges();
        if (e)
            limitOutputWithCondition(stmt,output, (new Unary(opNot, e))->simplify());
        updateRanges(output, execution_paths, true);

        if (VERBOSE && DEBUG_RANGE_ANALYSIS)
            LOG << stmt << "\n";
    }
    bool visit(CaseStatement * stmt) {processRange(stmt); return true; }
    bool visit(CallStatement * stmt) {
        RangeMap output = getInputRanges(stmt);
        ;
        if (stmt->getDestProc() == nullptr) {
            // note this assumes the call is only to one proc.. could be bad.
            Exp *d = output.substInto(stmt->getDest()->clone());
            if (d->isIntConst() || d->isStrConst()) {
                if (d->isIntConst()) {
                    ADDRESS dest = ((Const *)d)->getAddr();
                    stmt->procDest = stmt->proc->getProg()->setNewProc(dest);
                } else {
                    stmt->procDest = stmt->proc->getProg()->getLibraryProc(((Const *)d)->getStr());
                }
                if (stmt->procDest) {
                    Signature *sig = stmt->procDest->getSignature();
                    stmt->pDest = d;
                    stmt->arguments.clear();
                    for (size_t i = 0; i < sig->getNumParams(); i++) {
                        Exp *a = sig->getParamExp(i);
                        Assign *as = new Assign(VoidType::get(), a->clone(), a->clone());
                        as->setProc(stmt->proc);
                        as->setBB(stmt->Parent);
                        stmt->arguments.append(as);
                    }
                    stmt->signature = stmt->procDest->getSignature()->clone();
                    stmt->m_isComputed = false;
                    stmt->proc->undoComputedBB(this);
                    stmt->proc->addCallee(stmt->procDest);
                    LOG << "replaced indirect call with call to " << stmt->procDest->getName() << "\n";
                }
            }
        }

        if (output.hasRange(Location::regOf(28))) {
            Range &r = output.getRange(Location::regOf(28));
            int c = 4;
            if (stmt->procDest == nullptr) {
                LOG << "using push count hack to guess number of params\n";
                Instruction *prev = stmt->getPreviousStatementInBB();
                while (prev) {
                    if (prev->isAssign() && ((Assign *)prev)->getLeft()->isMemOf() &&
                        ((Assign *)prev)->getLeft()->getSubExp1()->isRegOfK() &&
                        ((Const *)((Assign *)prev)->getLeft()->getSubExp1()->getSubExp1())->getInt() == 28 &&
                        ((Assign *)prev)->getRight()->getOper() != opPC) {
                        c += 4;
                    }
                    prev = prev->getPreviousStatementInBB();
                }
            } else if (stmt->getDestProc()->getSignature()->getConvention() == CONV_PASCAL)
                c += stmt->getDestProc()->getSignature()->getNumParams() * 4;
            else if (!strncmp(qPrintable(stmt->getDestProc()->getName()), "__imp_", 6)) {
                Instruction *first = ((UserProc *)stmt->getDestProc())->getCFG()->getEntryBB()->getFirstStmt();
                assert(first && first->isCall());
                Function *d = ((CallStatement *)first)->getDestProc();
                if (d->getSignature()->getConvention() == CONV_PASCAL)
                    c += d->getSignature()->getNumParams() * 4;
            } else if (!stmt->getDestProc()->isLib()) {
                UserProc *p = (UserProc *)stmt->getDestProc();
                LOG_VERBOSE(1) << "== checking for number of bytes popped ==\n" << *p << "== end it ==\n";
                Exp *eq = p->getProven(Location::regOf(28));
                if (eq) {
                    LOG_VERBOSE(1) << "found proven " << eq << "\n";
                    if (eq->getOper() == opPlus && *eq->getSubExp1() == *Location::regOf(28) &&
                        eq->getSubExp2()->isIntConst()) {
                        c = ((Const *)eq->getSubExp2())->getInt();
                    } else
                        eq = nullptr;
                }
                BasicBlock *retbb = p->getCFG()->findRetNode();
                if (retbb && eq == nullptr) {
                    Instruction *last = retbb->getLastStmt();
                    assert(last);
                    if (last->isReturn()) {
                        last->setBB(retbb);
                        last = last->getPreviousStatementInBB();
                    }
                    if (last == nullptr) {
                        // call followed by a ret, sigh
                        for (size_t i = 0; i < retbb->getNumInEdges(); i++) {
                            last = retbb->getInEdges()[i]->getLastStmt();
                            if (last->isCall())
                                break;
                        }
                        if (last->isCall()) {
                            Function *d = ((CallStatement *)last)->getDestProc();
                            if (d && d->getSignature()->getConvention() == CONV_PASCAL)
                                c += d->getSignature()->getNumParams() * 4;
                        }
                        last = nullptr;
                    }
                    if (last && last->isAssign()) {
                        // LOG << "checking last statement " << last << " for number of bytes popped\n";
                        Assign *a = (Assign *)last;
                        assert(a->getLeft()->isRegOfK() && ((Const *)a->getLeft()->getSubExp1())->getInt() == 28);
                        Exp *t = a->getRight()->clone()->simplifyArith();
                        assert(t->getOper() == opPlus && t->getSubExp1()->isRegOfK() &&
                               ((Const *)t->getSubExp1()->getSubExp1())->getInt() == 28);
                        assert(t->getSubExp2()->isIntConst());
                        c = ((Const *)t->getSubExp2())->getInt();
                    }
                }
            }
            Range ra(r.getStride(), r.getLowerBound() == Range::MIN ? Range::MIN : r.getLowerBound() + c,
                     r.getUpperBound() == Range::MAX ? Range::MAX : r.getUpperBound() + c, r.getBase());
            output.addRange(Location::regOf(28), ra);
        }
        updateRanges(stmt,output, execution_paths);
        return true;
    }

    bool visit(ReturnStatement * stmt) {processRange(stmt); return true; }
    bool visit(ImpRefStatement * stmt) {processRange(stmt); return true; }
    bool visit(JunctionStatement * stmt) {
        RangeMap input;
        if (DEBUG_RANGE_ANALYSIS)
            LOG_VERBOSE(1) << "unioning {\n";
        for (size_t i = 0; i < stmt->Parent->getNumInEdges(); i++) {
            Instruction *last = stmt->Parent->getInEdges()[i]->getLastStmt();
            if (DEBUG_RANGE_ANALYSIS)
                LOG_VERBOSE(1) << "  in BB: " << stmt->Parent->getInEdges()[i]->getLowAddr() << " " << last << "\n";
            if (last->isBranch()) {
                input.unionwith(getRangesForOutEdgeTo((BranchStatement *)last,stmt->Parent));
            } else {
                if (last->isCall()) {
                    Function *d = ((CallStatement *)last)->getDestProc();
                    if (d && !d->isLib() && ((UserProc *)d)->getCFG()->findRetNode() == nullptr) {
                        if (DEBUG_RANGE_ANALYSIS)
                            LOG_VERBOSE(1) << "ignoring ranges from call to proc with no ret node\n";
                    } else
                        input.unionwith(tgt->getRanges(last));
                } else
                    input.unionwith(tgt->getRanges(last));
            }
        }
        if (DEBUG_RANGE_ANALYSIS)
            LOG_VERBOSE(1) << "}\n";

        if (!input.isSubset(tgt->getRanges(stmt))) {
            RangeMap output = input;

            if (output.hasRange(Location::regOf(28))) {
                Range &r = output.getRange(Location::regOf(28));
                if (r.getLowerBound() != r.getUpperBound() && r.getLowerBound() != Range::MIN) {
                    LOG_VERBOSE(1) << "stack height assumption violated " << r << " my bb: " << Parent->getLowAddr()
                                   << "\n";
                    LOG << *proc;
                    assert(false);
                }
            }

            if (stmt->isLoopJunction()) {
                output = tgt->getRanges(stmt);
                output.widenwith(input);
            }

            updateRanges(stmt,output, execution_paths);
        }

        if (DEBUG_RANGE_ANALYSIS)
            LOG_VERBOSE(1) << stmt << "\n";
        return true;
    }
private:
    RangeMap &getRangesForOutEdgeTo(BranchStatement *b,BasicBlock *out) {
        assert(b->getFixedDest() != NO_ADDRESS);
        if (out->getLowAddr() == b->getFixedDest())
            return tgt->getRanges(b);
        return tgt->getBranchRange(b);
    }
    void limitOutputWithCondition(BranchStatement *stmt,RangeMap &output, Exp *e) {
        assert(e);
        if (output.hasRange(e->getSubExp1())) {
            Range &r = output.getRange(e->getSubExp1());
            if (e->getSubExp2()->isIntConst() && r.getBase()->isIntConst() && ((Const *)r.getBase())->getInt() == 0) {
                int c = ((Const *)e->getSubExp2())->getInt();
                switch (e->getOper()) {
                case opLess:
                case opLessUns: {
                    Range ra(r.getStride(), r.getLowerBound() >= c ? c - 1 : r.getLowerBound(),
                             r.getUpperBound() >= c ? c - 1 : r.getUpperBound(), r.getBase());
                    output.addRange(e->getSubExp1(), ra);
                    break;
                }
                case opLessEq:
                case opLessEqUns: {
                    Range ra(r.getStride(), r.getLowerBound() > c ? c : r.getLowerBound(),
                             r.getUpperBound() > c ? c : r.getUpperBound(), r.getBase());
                    output.addRange(e->getSubExp1(), ra);
                    break;
                }
                case opGtr:
                case opGtrUns: {
                    Range ra(r.getStride(), r.getLowerBound() <= c ? c + 1 : r.getLowerBound(),
                             r.getUpperBound() <= c ? c + 1 : r.getUpperBound(), r.getBase());
                    output.addRange(e->getSubExp1(), ra);
                    break;
                }
                case opGtrEq:
                case opGtrEqUns: {
                    Range ra(r.getStride(), r.getLowerBound() < c ? c : r.getLowerBound(),
                             r.getUpperBound() < c ? c : r.getUpperBound(), r.getBase());
                    output.addRange(e->getSubExp1(), ra);
                    break;
                }
                case opEquals: {
                    Range ra(r.getStride(), c, c, r.getBase());
                    output.addRange(e->getSubExp1(), ra);
                    break;
                }
                case opNotEqual: {
                    Range ra(r.getStride(), r.getLowerBound() == c ? c + 1 : r.getLowerBound(),
                             r.getUpperBound() == c ? c - 1 : r.getUpperBound(), r.getBase());
                    output.addRange(e->getSubExp1(), ra);
                    break;
                }
                default:
                    break;
                }
            }
        }
    }

};
/**
  *
  * \brief Range analysis (for procedure).
  *
  */
bool RangeAnalysis::runOnFunction(Function &F)
{
    if(F.isLib())
        return false;
    LOG_STREAM() << "performing range analysis on " << getName() << "\n";
    UserProc &UF(F);
    // this helps
    UF.cfg->sortByAddress();

    addJunctionStatements(UF.cfg);
    UF.cfg->establishDFTOrder();

    clearRanges();

    UF.debugPrintAll("Before performing range analysis");

    std::list<Instruction *> execution_paths;
    std::list<Instruction *> junctions;

    assert(UF.cfg->getEntryBB());
    assert(UF.cfg->getEntryBB()->getFirstStmt());
    execution_paths.push_back(UF.cfg->getEntryBB()->getFirstStmt());

    int watchdog = 0;

    while (execution_paths.size()) {
        while (execution_paths.size()) {
            Instruction *stmt = execution_paths.front();
            execution_paths.pop_front();
            if (stmt == nullptr)
                continue; // ??
            if (stmt->isJunction())
                junctions.push_back(stmt);
            else
                stmt->rangeAnalysis(execution_paths);
        }
        if (watchdog > 45)
            LOG << "processing execution paths resulted in " << (int)junctions.size() << " junctions to process\n";
        while (junctions.size()) {
            Instruction *junction = junctions.front();
            junctions.pop_front();
            if (watchdog > 45)
                LOG << "processing junction " << junction << "\n";
            assert(junction->isJunction());
            junction->rangeAnalysis(execution_paths);
        }

        watchdog++;
        if (watchdog > 10) {
            LOG << "  watchdog " << watchdog << "\n";
            if (watchdog > 45) {
                LOG << (int)execution_paths.size() << " execution paths remaining.\n";
                LOG_SEPARATE(UF.getName()) << "=== After range analysis watchdog " << watchdog << " for " << UF.getName()
                                        << " ===\n" << *this << "=== end after range analysis watchdog " << watchdog
                                        << " for " << UF.getName() << " ===\n\n";
            }
        }
        if (watchdog > 50) {
            LOG << "  watchdog expired\n";
            break;
        }
    }
    UF.debugPrintAll("After range analysis");

    UF.cfg->removeJunctionStatements();
    logSuspectMemoryDefs(UF);

}
RangeMap RangePrivateData::getSavedRanges(Instruction *insn) {
    return SavedInputRanges[insn];
}
void RangePrivateData::setSavedRanges(Instruction *insn,RangeMap map) {
    SavedInputRanges[insn] = map;
}
/***************************************************************************/ /**
  *
  * \brief Detect and log possible buffer overflows
  *
  ******************************************************************************/
void RangeAnalysis::logSuspectMemoryDefs(UserProc &UF) {
    StatementList stmts;
    UF.getStatements(stmts);
    for (Instruction *st : stmts) {
        if (!st->isAssign())
            continue;
        Assign *a = (Assign *)st;
        if (!a->getLeft()->isMemOf())
            continue;
        RangeMap &rm = RangeData->getRanges(st);
        Exp *p = rm.substInto(a->getLeft()->getSubExp1()->clone());
        if (rm.hasRange(p)) {
            Range &r = rm.getRange(p);
            LOG << "got p " << p << " with range " << r << "\n";
            if (r.getBase()->getOper() == opInitValueOf && r.getBase()->getSubExp1()->isRegOfK() &&
                ((Const *)r.getBase()->getSubExp1()->getSubExp1())->getInt() == 28) {
                RTL *rtl = a->getBB()->getRTLWithStatement(a);
                LOG << "interesting stack reference at " << rtl->getAddress() << " " << a << "\n";
            }
        }
    }
}



Range::Range() : stride(1), lowerBound(MIN), upperBound(MAX) { base = Const::get(0); }

Range::Range(int stride, int lowerBound, int upperBound, Exp *base)
    : stride(stride), lowerBound(lowerBound), upperBound(upperBound), base(base) {
    if (lowerBound == upperBound && lowerBound == 0 && (base->getOper() == opMinus || base->getOper() == opPlus) &&
        base->getSubExp2()->isIntConst()) {
        this->lowerBound = ((Const *)base->getSubExp2())->getInt();
        if (base->getOper() == opMinus)
            this->lowerBound = -this->lowerBound;
        this->upperBound = this->lowerBound;
        this->base = base->getSubExp1();
    } else {
        if (base == nullptr)
            // NOTE: was "base = new Const(0);"
            this->base = new Const(0);
        if (lowerBound > upperBound)
            this->upperBound = lowerBound;
        if (upperBound < lowerBound)
            this->lowerBound = upperBound;
    }
}

void Range::print(QTextStream &os) const {
    assert(lowerBound <= upperBound);
    if (base->isIntConst() && ((Const *)base)->getInt() == 0 && lowerBound == MIN && upperBound == MAX) {
        os << "T";
        return;
    }
    bool needPlus = false;
    if (lowerBound == upperBound) {
        if (!base->isIntConst() || ((Const *)base)->getInt() != 0) {
            if (lowerBound != 0) {
                os << lowerBound;
                needPlus = true;
            }
        } else {
            needPlus = true;
            os << lowerBound;
        }
    } else {
        if (stride != 1)
            os << stride;
        os << "[";
        if (lowerBound == MIN)
            os << "-inf";
        else
            os << lowerBound;
        os << ", ";
        if (upperBound == MAX)
            os << "inf";
        else
            os << upperBound;
        os << "]";
        needPlus = true;
    }
    if (!base->isIntConst() || ((Const *)base)->getInt() != 0) {
        if (needPlus)
            os << " + ";
        base->print(os);
    }
}

void Range::unionWith(Range &r) {
    if (VERBOSE && DEBUG_RANGE_ANALYSIS)
        LOG << "unioning " << this << " with " << r << " got ";
    assert(base && r.base);
    if (base->getOper() == opMinus && r.base->getOper() == opMinus && *base->getSubExp1() == *r.base->getSubExp1() &&
        base->getSubExp2()->isIntConst() && r.base->getSubExp2()->isIntConst()) {
        int c1 = ((Const *)base->getSubExp2())->getInt();
        int c2 = ((Const *)r.base->getSubExp2())->getInt();
        if (c1 != c2) {
            if (lowerBound == r.lowerBound && upperBound == r.upperBound && lowerBound == 0) {
                lowerBound = std::min(-c1, -c2);
                upperBound = std::max(-c1, -c2);
                base = base->getSubExp1();
                if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                    LOG << this << "\n";
                return;
            }
        }
    }
    if (!(*base == *r.base)) {
        stride = 1;
        lowerBound = MIN;
        upperBound = MAX;
        base = Const::get(0);
        if (VERBOSE && DEBUG_RANGE_ANALYSIS)
            LOG << this << "\n";
        return;
    }
    if (stride != r.stride)
        stride = std::min(stride, r.stride);
    if (lowerBound != r.lowerBound)
        lowerBound = std::min(lowerBound, r.lowerBound);
    if (upperBound != r.upperBound)
        upperBound = std::max(upperBound, r.upperBound);
    if (VERBOSE && DEBUG_RANGE_ANALYSIS)
        LOG << this << "\n";
}

void Range::widenWith(Range &r) {
    if (VERBOSE && DEBUG_RANGE_ANALYSIS)
        LOG << "widening " << this << " with " << r << " got ";
    if (!(*base == *r.base)) {
        stride = 1;
        lowerBound = MIN;
        upperBound = MAX;
        base = Const::get(0);
        if (VERBOSE && DEBUG_RANGE_ANALYSIS)
            LOG << this << "\n";
        return;
    }
    // ignore stride for now
    if (r.getLowerBound() < lowerBound)
        lowerBound = MIN;
    if (r.getUpperBound() > upperBound)
        upperBound = MAX;
    if (VERBOSE && DEBUG_RANGE_ANALYSIS)
        LOG << this << "\n";
}
Range &RangeMap::getRange(Exp *loc) {
    if (ranges.find(loc) == ranges.end()) {
        return *(new Range(1, Range::MIN, Range::MAX, Const::get(0)));
    }
    return ranges[loc];
}

void RangeMap::unionwith(RangeMap &other) {
    for (auto &elem : other.ranges) {
        if (ranges.find((elem).first) == ranges.end()) {
            ranges[(elem).first] = (elem).second;
        } else {
            ranges[(elem).first].unionWith((elem).second);
        }
    }
}

void RangeMap::widenwith(RangeMap &other) {
    for (auto &elem : other.ranges) {
        if (ranges.find((elem).first) == ranges.end()) {
            ranges[(elem).first] = (elem).second;
        } else {
            ranges[(elem).first].widenWith((elem).second);
        }
    }
}

void RangeMap::print(QTextStream &os) const {
    for (auto it = ranges.begin(); it != ranges.end(); it++) {
        if (it != ranges.begin())
            os << ", ";
        (*it).first->print(os);
        os << " -> ";
        (*it).second.print(os);
    }
}

Exp *RangeMap::substInto(Exp *e, std::set<Exp *, lessExpStar> *only) const {
    bool changes;
    int count = 0;
    do {
        changes = false;
        for (auto &elem : ranges) {
            if (only && only->find((elem).first) == only->end())
                continue;
            bool change = false;
            Exp *eold = nullptr;
            if (DEBUG_RANGE_ANALYSIS)
                eold = e->clone();
            if ((elem).second.getLowerBound() == (elem).second.getUpperBound()) {
                e = e->searchReplaceAll(*(elem).first,
                                        (Binary::get(opPlus, (elem).second.getBase(),
                                                     new Const((elem).second.getLowerBound())))->simplify(),
                                        change);
            }
            if (change) {
                e = e->simplify()->simplifyArith();
                if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                    LOG << "applied " << (elem).first << " to " << eold << " to get " << e << "\n";
                changes = true;
            }
        }
        count++;
        assert(count < 5);
    } while (changes);
    return e;
}

void RangeMap::killAllMemOfs() {
    for (auto &elem : ranges) {
        if ((elem).first->isMemOf()) {
            Range empty;
            (elem).second.unionWith(empty);
        }
    }
}

bool Range::operator==(Range &other) {
    return stride == other.stride && lowerBound == other.lowerBound && upperBound == other.upperBound &&
           *base == *other.base;
}

// return true if this range map is a subset of the other range map
bool RangeMap::isSubset(RangeMap &other) {
    for (std::pair<Exp *, Range> it : ranges) {
        if (other.ranges.find(it.first) == other.ranges.end()) {
            if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                LOG << "did not find " << it.first << " in other, not a subset\n";
            return false;
        }
        Range &r = other.ranges[it.first];
        if (!(it.second == r)) {
            if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                LOG << "range for " << it.first << " in other " << r << " is not equal to range in this " << it.second
                    << ", not a subset\n";
            return false;
        }
    }
    return true;
}
//Log &Log::operator<<(const Range *r) {
//    QString tgt;
//    QTextStream st(&tgt);
//    r->print(st);
//    *this << tgt;
//    return *this;
//}

//Log &Log::operator<<(const Range &r) {
//    QString tgt;
//    QTextStream st(&tgt);
//    r.print(st);
//    *this << tgt;
//    return *this;
//}

//Log &Log::operator<<(const RangeMap &r) {
//    QString tgt;
//    QTextStream st(&tgt);
//    r.print(st);
//    *this << tgt;
//    return *this;
//}
