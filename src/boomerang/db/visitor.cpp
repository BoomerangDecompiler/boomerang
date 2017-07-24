/*
 * Copyright (C) 2004-2006, Mike Van Emmerik and Trent Waddington
 */

/***************************************************************************/ /**
 * \file       visitor.cpp
 * \brief   Provides the implementation for the various visitor and modifier classes.
 ******************************************************************************/
#include "boomerang/db/visitor.h"


#include "boomerang/util/Log.h"
#include "boomerang/util/Log.h" // For VERBOSE

#include "boomerang/db/proc.h"
#include "boomerang/db/signature.h"
#include "boomerang/db/prog.h"
#include "boomerang/db/statements/phiassign.h"
#include "boomerang/db/statements/implicitassign.h"
#include "boomerang/db/statements/callstatement.h"
#include "boomerang/db/statements/casestatement.h"
#include "boomerang/db/statements/boolassign.h"
#include "boomerang/db/statements/branchstatement.h"
#include "boomerang/db/statements/imprefstatement.h"

#include <QtCore/QDebug>
#include <sstream>

// FixProcVisitor class

bool FixProcVisitor::visit(const std::shared_ptr<Location>& l, bool& override)
{
    l->setProc(proc); // Set the proc, but only for Locations
    override = false; // Use normal accept logic
    return true;
}


// GetProcVisitor class

bool GetProcVisitor::visit(const std::shared_ptr<Location>& l, bool& override)
{
    proc     = l->getProc();
    override = false;
    return proc == nullptr; // Continue recursion only if failed so far
}


// SetConscripts class

bool SetConscripts::visit(const std::shared_ptr<Const>& c)
{
    if (!m_bInLocalGlobal) {
        if (m_bClear) {
            c->setConscript(0);
        }
        else {
            c->setConscript(++m_curConscript);
        }
    }

    m_bInLocalGlobal = false;
    return true; // Continue recursion
}


bool SetConscripts::visit(const std::shared_ptr<Location>& l, bool& override)
{
    OPER op = l->getOper();

    if ((op == opLocal) || (op == opGlobal) || (op == opRegOf) || (op == opParam)) {
        m_bInLocalGlobal = true;
    }

    override = false;
    return true; // Continue recursion
}


bool SetConscripts::visit(const std::shared_ptr<Binary>& b, bool& override)
{
    OPER op = b->getOper();

    if (op == opSize) {
        m_bInLocalGlobal = true;
    }

    override = false;
    return true; // Continue recursion
}


bool StmtVisitor::visit(RTL * /*rtl*/)
{
    // Mostly, don't do anything at the RTL level
    return true;
}


bool StmtConscriptSetter::visit(Assign *stmt)
{
    SetConscripts sc(m_curConscript, m_clear);

    stmt->getLeft()->accept(&sc);
    stmt->getRight()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(PhiAssign *stmt)
{
    SetConscripts sc(m_curConscript, m_clear);

    stmt->getLeft()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(ImplicitAssign *stmt)
{
    SetConscripts sc(m_curConscript, m_clear);

    stmt->getLeft()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(CallStatement *stmt)
{
    SetConscripts  sc(m_curConscript, m_clear);
    StatementList& args = stmt->getArguments();

    StatementList::iterator ss;

    for (ss = args.begin(); ss != args.end(); ++ss) {
        (*ss)->accept(this);
    }

    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(CaseStatement *stmt)
{
    SetConscripts sc(m_curConscript, m_clear);
    SWITCH_INFO   *si = stmt->getSwitchInfo();

    if (si) {
        si->pSwitchVar->accept(&sc);
        m_curConscript = sc.getLast();
    }

    return true;
}


bool StmtConscriptSetter::visit(ReturnStatement *stmt)
{
    SetConscripts sc(m_curConscript, m_clear);

    ReturnStatement::iterator rr;

    for (rr = stmt->begin(); rr != stmt->end(); ++rr) {
        (*rr)->accept(this);
    }

    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(BoolAssign *stmt)
{
    SetConscripts sc(m_curConscript, m_clear);

    stmt->getCondExpr()->accept(&sc);
    stmt->getLeft()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(BranchStatement *stmt)
{
    SetConscripts sc(m_curConscript, m_clear);

    stmt->getCondExpr()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


bool StmtConscriptSetter::visit(ImpRefStatement *stmt)
{
    SetConscripts sc(m_curConscript, m_clear);

    stmt->getAddressExp()->accept(&sc);
    m_curConscript = sc.getLast();
    return true;
}


void PhiStripper::visit(PhiAssign * /*s*/, bool& recur)
{
    m_del = true;
    recur = true;
}


SharedExp CallBypasser::postVisit(const std::shared_ptr<RefExp>& r)
{
    // If child was modified, simplify now
    SharedExp ret = r;

    if (!(m_unchanged & m_mask)) {
        ret = r->simplify();
    }

    m_mask >>= 1;
    // Note: r (the pointer) will always == ret (also the pointer) here, so the below is safe and avoids a cast
    Instruction   *def  = r->getDef();
    CallStatement *call = dynamic_cast<CallStatement *>(def);

    if (call) {
        assert(std::dynamic_pointer_cast<RefExp>(ret));
        bool ch;
        ret = call->bypassRef(std::static_pointer_cast<RefExp>(ret), ch);

        if (ch) {
            m_unchanged &= ~m_mask;
            m_mod        = true;
            // Now have to recurse to do any further bypassing that may be required
            // E.g. bypass the two recursive calls in fibo?? FIXME: check!
            return ret->accept(new CallBypasser(m_enclosingStmt));
        }
    }

    // Else just leave as is (perhaps simplified)
    return ret;
}


SharedExp CallBypasser::postVisit(const std::shared_ptr<Location>& e)
{
    // Hack to preserve a[m[x]]. Can likely go when ad hoc TA goes.
    bool isAddrOfMem = e->isAddrOf() && e->getSubExp1()->isMemOf();

    if (isAddrOfMem) {
        return e;
    }

    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postVisit(const std::shared_ptr<Location>& e)
{
    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postVisit(const std::shared_ptr<RefExp>& e)
{
    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postVisit(const std::shared_ptr<Unary>& e)
{
    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postVisit(const std::shared_ptr<Binary>& e)
{
    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplifyArith()->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postVisit(const std::shared_ptr<Ternary>& e)
{
    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postVisit(const std::shared_ptr<TypedExp>& e)
{
    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postVisit(const std::shared_ptr<FlagDef>& e)
{
    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplify();
    }

    m_mask >>= 1;
    return ret;
}


SharedExp SimpExpModifier::postVisit(const std::shared_ptr<Const>& e)
{
    m_mask >>= 1;
    return e;
}


SharedExp SimpExpModifier::postVisit(const std::shared_ptr<TypeVal>& e)
{
    m_mask >>= 1;
    return e;
}


SharedExp SimpExpModifier::postVisit(const std::shared_ptr<Terminal>& e)
{
    m_mask >>= 1;
    return e;
}


bool UsedLocsFinder::visit(const std::shared_ptr<Location>& e, bool& override)
{
    if (!m_memOnly) {
        m_used->insert(e->shared_from_this());       // All locations visited are used
    }

    if (e->isMemOf()) {
        // Example: m[r28{10} - 4]    we use r28{10}
        SharedExp child = e->access<Exp, 1>();
        // Care! Need to turn off the memOnly flag for work inside the m[...], otherwise everything will get ignored
        bool wasMemOnly = m_memOnly;
        m_memOnly = false;
        child->accept(this);
        m_memOnly = wasMemOnly;
        override  = true; // Already looked inside child
    }
    else {
        override = false;
    }

    return true; // Continue looking for other locations
}


bool UsedLocsFinder::visit(const std::shared_ptr<Terminal>& e)
{
    if (m_memOnly) {
        return true; // Only interested in m[...]
    }

    switch (e->getOper())
    {
    case opPC:
    case opFlags:
    case opFflags:
    case opDefineAll:
    // Fall through
    // The carry flag can be used in some SPARC idioms, etc
    case opDF:
    case opCF:
    case opZF:
    case opNF:
    case opOF: // also these
        m_used->insert(e);
        break;

    default:
        break;
    }

    return true; // Always continue recursion
}


bool UsedLocsFinder::visit(const std::shared_ptr<RefExp>& arg, bool& override)
{
    if (m_memOnly) {
        override = false; // Look inside the ref for m[...]
        return true;      // Don't count this reference
    }

    std::shared_ptr<RefExp> e = arg;

    if (m_used->find(e) == m_used->end()) {
        // e = (RefExp *)arg.clone();
        m_used->insert(e);       // This location is used
    }

    // However, e's subexpression is NOT used ...
    override = true;
    // ... unless that is a m[x], array[x] or .x, in which case x (not m[x]/array[x]/refd.x) is used
    SharedExp refd = e->getSubExp1();

    if (refd->isMemOf()) {
        refd->getSubExp1()->accept(this);
    }
    else if (refd->isArrayIndex()) {
        refd->getSubExp1()->accept(this);
        refd->getSubExp2()->accept(this);
    }
    else if (refd->isMemberOf()) {
        refd->getSubExp1()->accept(this);
    }

    return true;
}


bool UsedLocalFinder::visit(const std::shared_ptr<Location>& e, bool& override)
{
    override = false;
    if (e->isLocal())
    {
        used->insert(e); // Found a local
    }

    return true;         // Continue looking for other locations
}


bool UsedLocalFinder::visit(const std::shared_ptr<TypedExp>& e, bool& override)
{
    override = false;
    SharedType ty = e->getType();

    // Assumption: (cast)exp where cast is of pointer type means that exp is the address of a local
    if (ty->resolvesToPointer()) {
        SharedExp sub = e->getSubExp1();
        SharedExp mof = Location::memOf(sub);

        if (!proc->findLocal(mof, ty).isNull()) {
            used->insert(mof);
            override = true;
        }
    }

    return true;
}


bool UsedLocalFinder::visit(const std::shared_ptr<Terminal>& e)
{
    if (e->getOper() == opDefineAll) {
        all = true;
    }

    QString sym = proc->findFirstSymbol(e);

    if (!sym.isNull()) {
        used->insert(e);
    }

    return true; // Always continue recursion
}


bool UsedLocsVisitor::visit(Assign *s, bool& override)
{
    SharedExp lhs = s->getLeft();
    SharedExp rhs = s->getRight();

    if (rhs) {
        rhs->accept(ev);
    }

    // Special logic for the LHS. Note: PPC can have r[tmp + 30] on LHS
    if (lhs->isMemOf() || lhs->isRegOf()) {
        SharedExp child = lhs->getSubExp1(); // m[xxx] uses xxx
        // Care! Don't want the memOnly flag when inside a m[...]. Otherwise, nothing will be found
        // Also beware that ev may be a UsedLocalFinder now
        UsedLocsFinder *ulf = dynamic_cast<UsedLocsFinder *>(ev);

        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            child->accept(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    }
    else if ((lhs->getOper() == opArrayIndex) || (lhs->getOper() == opMemberAccess)) {
        SharedExp subExp1 = lhs->getSubExp1(); // array(base, index) and member(base, offset)?? use
        subExp1->accept(ev);                   // base and index
        SharedExp subExp2 = lhs->getSubExp2();
        subExp2->accept(ev);
    }
    else if (lhs->getOper() == opAt) {   // foo@[first:last] uses foo, first, and last
        SharedExp subExp1 = lhs->getSubExp1();
        subExp1->accept(ev);
        SharedExp subExp2 = lhs->getSubExp2();
        subExp2->accept(ev);
        SharedExp subExp3 = lhs->getSubExp3();
        subExp3->accept(ev);
    }

    override = true; // Don't do the usual accept logic
    return true;     // Continue the recursion
}


bool UsedLocsVisitor::visit(PhiAssign *s, bool& override)
{
    SharedExp lhs = s->getLeft();

    // Special logic for the LHS
    if (lhs->isMemOf()) {
        SharedExp      child = lhs->getSubExp1();
        UsedLocsFinder *ulf  = dynamic_cast<UsedLocsFinder *>(ev);

        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            child->accept(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    }
    else if ((lhs->getOper() == opArrayIndex) || (lhs->getOper() == opMemberAccess)) {
        SharedExp subExp1 = lhs->getSubExp1();
        subExp1->accept(ev);
        SharedExp subExp2 = lhs->getSubExp2();
        subExp2->accept(ev);
    }

    for (const auto& v : *s) {
        // Note: don't make the RefExp based on lhs, since it is possible that the lhs was renamed in fromSSA()
        // Use the actual expression in the PhiAssign
        // Also note that it's possible for uu->e to be nullptr. Suppose variable a can be assigned to along in-edges
        // 0, 1, and 3; inserting the phi parameter at index 3 will cause a null entry at 2
        assert(v.second.e);
        auto temp = RefExp::get(v.second.e, (Instruction *)v.second.def());
        temp->accept(ev);
    }

    override = true; // Don't do the usual accept logic
    return true;     // Continue the recursion
}


bool UsedLocsVisitor::visit(ImplicitAssign *s, bool& override)
{
    SharedExp lhs = s->getLeft();

    // Special logic for the LHS
    if (lhs->isMemOf()) {
        SharedExp      child = lhs->getSubExp1();
        UsedLocsFinder *ulf  = dynamic_cast<UsedLocsFinder *>(ev);

        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            child->accept(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    }
    else if ((lhs->getOper() == opArrayIndex) || (lhs->getOper() == opMemberAccess)) {
        SharedExp subExp1 = lhs->getSubExp1();
        subExp1->accept(ev);
        SharedExp subExp2 = lhs->getSubExp2();
        subExp2->accept(ev);
    }

    override = true; // Don't do the usual accept logic
    return true;     // Continue the recursion
}


bool UsedLocsVisitor::visit(CallStatement *s, bool& override)
{
    SharedExp pDest = s->getDest();

    if (pDest) {
        pDest->accept(ev);
    }

    StatementList::iterator it;
    StatementList&          arguments = s->getArguments();

    for (it = arguments.begin(); it != arguments.end(); it++) {
        // Don't want to ever collect anything from the lhs
        ((Assign *)*it)->getRight()->accept(ev);
    }

    if (m_countCol) {
        DefCollector::iterator dd;
        DefCollector           *col = s->getDefCollector();

        for (dd = col->begin(); dd != col->end(); ++dd) {
            (*dd)->accept(this);
        }
    }

    override = true; // Don't do the normal accept logic
    return true;     // Continue the recursion
}


bool UsedLocsVisitor::visit(ReturnStatement *s, bool& override)
{
    // For the final pass, only consider the first return
    ReturnStatement::iterator rr;

    for (rr = s->begin(); rr != s->end(); ++rr) {
        (*rr)->accept(this);
    }

    // Also consider the reaching definitions to be uses, so when they are the only non-empty component of this
    // ReturnStatement, they can get propagated to.
    if (m_countCol) { // But we need to ignore these "uses" unless propagating
        DefCollector::iterator dd;
        DefCollector           *col = s->getCollector();

        for (dd = col->begin(); dd != col->end(); ++dd) {
            (*dd)->accept(this);
        }
    }

    // Insert a phantom use of "everything" here, so that we can find out if any childless calls define something that
    // may end up being returned
    // FIXME: Not here! Causes locals to never get removed. Find out where this belongs, if anywhere:
    // ((UsedLocsFinder*)ev)->getLocSet()->insert(Terminal::get(opDefineAll));

    override = true; // Don't do the normal accept logic
    return true;     // Continue the recursion
}


bool UsedLocsVisitor::visit(BoolAssign *s, bool& override)
{
    SharedExp pCond = s->getCondExpr();

    if (pCond) {
        pCond->accept(ev); // Condition is used
    }

    SharedExp lhs = s->getLeft();
    assert(lhs);

    if (lhs->isMemOf()) { // If dest is of form m[x]...
        SharedExp      x    = lhs->getSubExp1();
        UsedLocsFinder *ulf = dynamic_cast<UsedLocsFinder *>(ev);

        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            x->accept(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    }
    else if ((lhs->getOper() == opArrayIndex) || (lhs->getOper() == opMemberAccess)) {
        SharedExp subExp1 = lhs->getSubExp1();
        subExp1->accept(ev);
        SharedExp subExp2 = lhs->getSubExp2();
        subExp2->accept(ev);
    }

    override = true; // Don't do the normal accept logic
    return true;     // Continue the recursion
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<Location>& e, bool& recur)
{
    if (*e == *m_search) {
        recur = e->isMemOf();         // Don't double subscript unless m[...]
        return RefExp::get(e, m_def); // Was replaced by postVisit below
    }

    recur = true;
    return e;
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<Binary>& e, bool& recur)
{
    // array[index] is like m[addrexp]: requires a subscript
    if (e->isArrayIndex() && (*e == *m_search)) {
        recur = true;                 // Check the index expression
        return RefExp::get(e, m_def); // Was replaced by postVisit below
    }

    recur = true;
    return e;
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<Terminal>& e)
{
    if (*e == *m_search) {
        return RefExp::get(e, m_def);
    }

    return e;
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<RefExp>& e, bool& recur)
{
    recur = false; // Don't look inside... not sure about this
    return e;
}


void StmtSubscripter::visit(Assign *s, bool& recur)
{
    SharedExp rhs = s->getRight();

    s->setRight(rhs->accept(m_mod));
    // Don't subscript the LHS of an assign, ever
    SharedExp lhs = s->getLeft();

    if (lhs->isMemOf() || lhs->isRegOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->accept(m_mod));
    }

    recur = false;
}


void StmtSubscripter::visit(PhiAssign *s, bool& recur)
{
    SharedExp lhs = s->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->accept(m_mod));
    }

    recur = false;
}


void StmtSubscripter::visit(ImplicitAssign *s, bool& recur)
{
    SharedExp lhs = s->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->accept(m_mod));
    }

    recur = false;
}


void StmtSubscripter::visit(BoolAssign *s, bool& recur)
{
    SharedExp lhs = s->getLeft();

    if (lhs->isMemOf()) {
        lhs->setSubExp1(lhs->getSubExp1()->accept(m_mod));
    }

    SharedExp rhs = s->getCondExpr();
    s->setCondExpr(rhs->accept(m_mod));
    recur = false;
}


void StmtSubscripter::visit(CallStatement *s, bool& recur)
{
    SharedExp pDest = s->getDest();

    if (pDest) {
        s->setDest(pDest->accept(m_mod));
    }

    // Subscript the ordinary arguments
    StatementList&          arguments = s->getArguments();
    StatementList::iterator ss;

    for (ss = arguments.begin(); ss != arguments.end(); ++ss) {
        (*ss)->accept(this);
    }

    // Returns are like the LHS of an assignment; don't subscript them directly (only if m[x], and then only subscript
    // the x's)
    recur = false; // Don't do the usual accept logic
}


SharedExp SizeStripper::preVisit(const std::shared_ptr<Binary>& b, bool& recur)
{
    recur = true; // Visit the binary's children

    if (b->isSizeCast()) {
        // Could be a size cast of a size cast
        return b->getSubExp2()->stripSizes();
    }

    return b;
}


SharedExp ExpConstCaster::preVisit(const std::shared_ptr<Const>& c)
{
    if (c->getConscript() == m_num) {
        m_changed = true;
        return std::make_shared<TypedExp>(m_ty, c);
    }

    return c;
}


bool ConstFinder::visit(const std::shared_ptr<Const>& e)
{
    m_constList.push_back(e);
    return true;
}


bool ConstFinder::visit(const std::shared_ptr<Location>& e, bool& override)
{
    if (e->isMemOf()) {
        override = false; // We DO want to see constants in memofs
    }
    else {
        override = true; // Don't consider register numbers, global names, etc
    }

    return true;
}


SharedExp ImplicitConverter::postVisit(const std::shared_ptr<RefExp>& e)
{
    if (e->getDef() == nullptr) {
        e->setDef(m_cfg->findImplicitAssign(e->getSubExp1()));
    }

    return e;
}


void StmtImplicitConverter::visit(PhiAssign *s, bool& recur)
{
    // The LHS could be a m[x] where x has a null subscript; must do first
    s->setLeft(s->getLeft()->accept(m_mod));

    for (auto& v : *s) {
        assert(v.second.e != nullptr);

        if (v.second.def() == nullptr) {
            v.second.def(m_cfg->findImplicitAssign(v.second.e));
        }
    }

    recur = false; // Already done LHS
}


SharedExp Localiser::preVisit(const std::shared_ptr<RefExp>& e, bool& recur)
{
    recur    = false; // Don't recurse into already subscripted variables
    m_mask <<= 1;
    return e;
}


SharedExp Localiser::preVisit(const std::shared_ptr<Location>& e, bool& recur)
{
    recur    = true;
    m_mask <<= 1;
    return e;
}


SharedExp Localiser::postVisit(const std::shared_ptr<Location>& e)
{
    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplify();
    }

    m_mask >>= 1;
    SharedExp r = call->findDefFor(ret);

    if (r) {
        ret = r->clone();

        ret          = ret->bypass();
        m_unchanged &= ~m_mask;
        m_mod        = true;
    }
    else {
        ret = RefExp::get(ret, nullptr); // No definition reaches, so subscript with {-}
    }

    return ret;
}


SharedExp Localiser::postVisit(const std::shared_ptr<Terminal>& e)
{
    SharedExp ret = e;

    if (!(m_unchanged & m_mask)) {
        ret = e->simplify();
    }

    m_mask >>= 1;
    SharedExp r = call->findDefFor(ret);

    if (r) {
        ret          = r->clone()->bypass();
        m_unchanged &= ~m_mask;
        m_mod        = true;
    }
    else {
        ret = RefExp::get(ret, nullptr); // No definition reaches, so subscript with {-}
    }

    return ret;
}


bool ComplexityFinder::visit(const std::shared_ptr<Location>& e, bool& override)
{
    if (proc && (proc->findFirstSymbol(e) != nullptr)) {
        // This is mapped to a local. Count it as zero, not about 3 (m[r28+4] -> memof, regof, plus)
        override = true;
        return true;
    }

    if (e->isMemOf() || e->isArrayIndex()) {
        count++; // Count the more complex unaries
    }

    override = false;
    return true;
}


bool ComplexityFinder::visit(const std::shared_ptr<Unary>& /*e*/, bool& override)
{
    count++;
    override = false;
    return true;
}


bool ComplexityFinder::visit(const std::shared_ptr<Binary>& /*e*/, bool& override)
{
    count++;
    override = false;
    return true;
}


bool ComplexityFinder::visit(const std::shared_ptr<Ternary>& /*e*/, bool& override)
{
    count++;
    override = false;
    return true;
}


bool MemDepthFinder::visit(const std::shared_ptr<Location>& e, bool& override)
{
    if (e->isMemOf()) {
        ++depth;
    }

    override = false;
    return true;
}


SharedExp ExpPropagator::postVisit(const std::shared_ptr<RefExp>& e)
{
    // No need to call e->canRename() here, because if e's base expression is not suitable for renaming, it will never
    // have been renamed, and we never would get here
    if (!Instruction::canPropagateToExp(*e)) { // Check of the definition statement is suitable for propagating
        return e;
    }

    Instruction *def = e->getDef();
    SharedExp   res  = e;

    if (def && def->isAssign()) {
        SharedExp lhs = ((Assign *)def)->getLeft();
        SharedExp rhs = ((Assign *)def)->getRight();
        bool      ch;
        res = e->searchReplaceAll(RefExp(lhs, def), rhs->clone(), ch);

        if (ch) {
            change       = true;    // Record this change
            m_unchanged &= ~m_mask; // Been changed now (so simplify parent)

            if (res->isSubscript()) {
                res = postVisit(std::static_pointer_cast<RefExp>(res)); // Recursively propagate more if possible
            }
        }
    }

    return res;
}


bool PrimitiveTester::visit(const std::shared_ptr<Location>& /*e*/, bool& override)
{
    // We reached a bare (unsubscripted) location. This is certainly not primitive
    override = true;
    result   = false;
    return false; // No need to continue searching
}


bool PrimitiveTester::visit(const std::shared_ptr<RefExp>& e, bool& override)
{
    Instruction *def = e->getDef();

    // If defined by a call, e had better not be a memory location (crude approximation for now)
    if ((def == nullptr) || (def->getNumber() == 0) || (def->isCall() && !e->getSubExp1()->isMemOf())) {
        // Implicit definitions are always primitive
        // The results of calls are always primitive
        override = true; // Don't recurse into the reference
        return true;     // Result remains true
    }

    // For now, all references to other definitions will be considered non primitive. I think I'll have to extend this!
    result   = false;
    override = true; // Regareless of outcome, don't recurse into the reference
    return true;
}


bool ExpHasMemofTester::visit(const std::shared_ptr<Location>& e, bool& override)
{
    if (e->isMemOf()) {
        override = true; // Don't recurse children (not needed surely)
        result   = true; // Found a memof
        return false;    // Don't continue searching the expression
    }

    override = false;
    return true;
}


bool TempToLocalMapper::visit(const std::shared_ptr<Location>& e, bool& override)
{
    if (e->isTemp()) {
        // We have a temp subexpression; get its name
        QString    tempName = e->access<Const, 1>()->getStr();
        SharedType ty       = Type::getTempType(tempName); // Types for temps strictly depend on the name
        // This call will do the mapping from the temp to a new local:
        proc->getSymbolExp(e, ty, true);
    }

    override = true; // No need to examine the string
    return true;
}


ExpRegMapper::ExpRegMapper(UserProc *p)
    : m_proc(p)
{
    m_prog = m_proc->getProg();
}


bool ExpRegMapper::visit(const std::shared_ptr<RefExp>& e, bool& override)
{
    SharedExp base = e->getSubExp1();

    if (base->isRegOf() || m_proc->isLocalOrParamPattern(base)) { // Don't convert if e.g. a global
        m_proc->checkLocalFor(e);
    }

    override = true; // Don't examine the r[] inside
    return true;
}


bool StmtRegMapper::common(Assignment *stmt, bool& override)
{
    // In case lhs is a reg or m[reg] such that reg is otherwise unused
    SharedExp lhs = stmt->getLeft();
    auto      re  = RefExp::get(lhs, stmt);

    re->accept((ExpRegMapper *)ev);
    override = false;
    return true;
}


bool StmtRegMapper::visit(Assign *stmt, bool& override)
{
    return common(stmt, override);
}


bool StmtRegMapper::visit(PhiAssign *stmt, bool& override)
{
    return common(stmt, override);
}


bool StmtRegMapper::visit(ImplicitAssign *stmt, bool& override)
{
    return common(stmt, override);
}


bool StmtRegMapper::visit(BoolAssign *stmt, bool& override)
{
    return common(stmt, override);
}


SharedExp ConstGlobalConverter::preVisit(const std::shared_ptr<RefExp>& e, bool& recur)
{
    Instruction *def = e->getDef();

    if (!def || def->isImplicit()) {
        SharedExp base = e->getSubExp1();
        SharedExp addr = base->isMemOf() ? base->getSubExp1() : nullptr;

        if (base->isMemOf() && addr && addr->isIntConst()) {
            // We have a m[K]{-}
            Address K     = addr->access<Const>()->getAddr();
            int     value = m_prog->readNative4(K);
            recur = false;
            return Const::get(value);
        }
        else if (base->isGlobal()) {
            // We have a glo{-}
            QString gname    = base->access<Const, 1>()->getStr();
            Address gloValue = m_prog->getGlobalAddr(gname);
            int     value    = m_prog->readNative4(gloValue);
            recur = false;
            return Const::get(value);
        }
        else if (base->isArrayIndex()) {
            SharedExp idx = base->getSubExp2();
            SharedExp glo = base->getSubExp1();
            if (idx && idx->isIntConst() && glo && glo->isGlobal()) {
                // We have a glo[K]{-}
                int        K        = idx->access<Const>()->getInt();
                QString    gname    = glo->access<Const, 1>()->getStr();
                Address    gloValue = m_prog->getGlobalAddr(gname);
                SharedType gloType  = m_prog->getGlobal(gname)->getType();

                assert(gloType->isArray());
                SharedType componentType = gloType->as<ArrayType>()->getBaseType();
                int        value         = m_prog->readNative4(gloValue + K * (componentType->getSize() / 8));
                recur = false;
                return Const::get(value);
            }
        }
    }

    recur = true;
    return e;
}


bool ExpDestCounter::visit(const std::shared_ptr<RefExp>& e, bool& override)
{
    if (Instruction::canPropagateToExp(*e)) {
        m_destCounts[e->clone()]++;
    }

    override = false; // Continue searching my children
    return true;      // Continue visiting the rest of Exp* e
}


bool StmtDestCounter::visit(PhiAssign * /*stmt*/, bool& override)
{
    override = false;
    return true;
}


bool FlagsFinder::visit(const std::shared_ptr<Binary>& e, bool& override)
{
    if (e->isFlagCall()) {
        m_found = true;
        return false; // Don't continue searching
    }

    override = false;
    return true;
}


bool BadMemofFinder::visit(const std::shared_ptr<Location>& e, bool& override)
{
    if (e->isMemOf()) {
        m_found = true;       // A bare memof
        return false;
    }

    override = false;
    return true; // Continue searching
}


bool BadMemofFinder::visit(const std::shared_ptr<RefExp>& e, bool& override)
{
    SharedExp base = e->getSubExp1();

    if (base->isMemOf()) {
        // Beware: it may be possible to have a bad memof inside a subscripted one
        SharedExp addr = base->getSubExp1();
        addr->accept(this);

        if (m_found) {
            return false; // Don't continue searching
        }

#if NEW                   // FIXME: not ready for this until have incremental propagation
        const char *sym = proc->lookupSym(e);

        if (sym == nullptr) {
            found    = true; // Found a memof that is not a symbol
            override = true; // Don't look inside the refexp
            return false;
        }
#endif
    }

    override = true; // Don't look inside the refexp
    return true;     // It has a symbol; noting bad foound yet but continue searching
}


// CastInserters. More cases to be implemented.

// Check the type of the address expression of memof to make sure it is compatible with the given memofType.
// memof may be changed internally to include a TypedExp, which will emit as a cast
void ExpCastInserter::checkMemofType(const SharedExp& memof, SharedType memofType)
{
    SharedExp addr = memof->getSubExp1();

    if (addr->isSubscript()) {
        SharedExp  addrBase     = addr->getSubExp1();
        SharedType actType      = addr->access<RefExp>()->getDef()->getTypeFor(addrBase);
        SharedType expectedType = PointerType::get(memofType);

        if (!actType->isCompatibleWith(*expectedType)) {
            memof->setSubExp1(std::make_shared<TypedExp>(expectedType, addrBase));
        }
    }
}


SharedExp ExpCastInserter::postVisit(const std::shared_ptr<RefExp>& e)
{
    SharedExp base = e->getSubExp1();

    if (base->isMemOf()) {
        // Check to see if the address expression needs type annotation
        Instruction *def = e->getDef();

        if (!def) {
            qDebug() << "ExpCastInserter::postVisit RefExp def is null";
            return e;
        }

        SharedType memofType = def->getTypeFor(base);
        checkMemofType(base, memofType);
    }

    return e;
}


static SharedExp checkSignedness(SharedExp e, int reqSignedness)
{
    SharedType ty             = e->ascendType();
    int        currSignedness = 0;
    bool       isInt          = ty->resolvesToInteger();

    if (isInt) {
        currSignedness = ty->as<IntegerType>()->getSignedness();
        currSignedness = (currSignedness >= 0) ? 1 : -1;
    }

    // if (!isInt || currSignedness != reqSignedness) { // }
    // Don't want to cast e.g. floats to integer
    if (isInt && (currSignedness != reqSignedness)) {
        std::shared_ptr<IntegerType> newtype;

        if (!isInt) {
            newtype = IntegerType::get(STD_SIZE, reqSignedness);
        }
        else {
            newtype = IntegerType::get(std::static_pointer_cast<IntegerType>(ty)->getSize(), reqSignedness); // Transfer size
        }

        newtype->setSigned(reqSignedness);
        return std::make_shared<TypedExp>(newtype, e);
    }

    return e;
}


SharedExp ExpCastInserter::postVisit(const std::shared_ptr<Binary>& e)
{
    OPER op = e->getOper();

    switch (op)
    {
    // This case needed for e.g. test/pentium/switch_gcc:
    case opLessUns:
    case opGtrUns:
    case opLessEqUns:
    case opGtrEqUns:
    case opShiftR:
        e->setSubExp1(checkSignedness(e->getSubExp1(), -1));

        if (op != opShiftR) { // The shift amount (second operand) is sign agnostic
            e->setSubExp2(checkSignedness(e->getSubExp2(), -1));
        }

        break;

    // This case needed for e.g. test/sparc/minmax2, if %g1 is declared as unsigned int
    case opLess:
    case opGtr:
    case opLessEq:
    case opGtrEq:
    case opShiftRA:
        e->setSubExp1(checkSignedness(e->getSubExp1(), +1));

        if (op != opShiftRA) {
            e->setSubExp2(checkSignedness(e->getSubExp2(), +1));
        }

        break;

    default:
        break;
    }

    return e;
}


SharedExp ExpCastInserter::postVisit(const std::shared_ptr<Const>& e)
{
    if (e->isIntConst()) {
        bool       naturallySigned = e->getInt() < 0;
        SharedType ty = e->getType();

        if (naturallySigned && ty->isInteger() && !ty->as<IntegerType>()->isSigned()) {
            return std::make_shared<TypedExp>(IntegerType::get(ty->as<IntegerType>()->getSize(), -1), e);
        }
    }

    return e;
}


bool StmtCastInserter::visit(Assign *s)
{
    return common(s);
}


bool StmtCastInserter::visit(PhiAssign *s)
{
    return common(s);
}


bool StmtCastInserter::visit(ImplicitAssign *s)
{
    return common(s);
}


bool StmtCastInserter::visit(BoolAssign *s)
{
    return common(s);
}


bool StmtCastInserter::common(Assignment *s)
{
    SharedExp lhs = s->getLeft();

    if (lhs->isMemOf()) {
        SharedType memofType = s->getType();
        ExpCastInserter::checkMemofType(lhs, memofType);
    }

    return true;
}


SharedExp ExpSsaXformer::postVisit(const std::shared_ptr<RefExp>& e)
{
    QString sym = m_proc->lookupSymFromRefAny(e);

    if (!sym.isNull()) {
        return Location::local(sym, m_proc);
    }

    // We should not get here: all locations should be replaced with Locals or Parameters
    // LOG << "ERROR! Could not find local or parameter for " << e << " !!\n";
    return e->getSubExp1(); // At least strip off the subscript
}


// Common code for the left hand side of assignments
void StmtSsaXformer::commonLhs(Assignment *as)
{
    SharedExp lhs = as->getLeft();

    lhs = lhs->accept((ExpSsaXformer *)m_mod); // In case the LHS has say m[r28{0}+8] -> m[esp+8]
    QString sym = m_proc->lookupSymFromRefAny(RefExp::get(lhs, as));

    if (!sym.isNull()) {
        as->setLeft(Location::local(sym, m_proc));
    }
}


void StmtSsaXformer::visit(BoolAssign *s, bool& recur)
{
    commonLhs(s);
    SharedExp pCond = s->getCondExpr();
    pCond = pCond->accept((ExpSsaXformer *)m_mod);
    s->setCondExpr(pCond);
    recur = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(Assign *s, bool& recur)
{
    commonLhs(s);
    SharedExp rhs = s->getRight();
    rhs = rhs->accept(m_mod);
    s->setRight(rhs);
    recur = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(ImplicitAssign *s, bool& recur)
{
    commonLhs(s);
    recur = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(PhiAssign *s, bool& recur)
{
    commonLhs(s);

    UserProc *_proc = ((ExpSsaXformer *)m_mod)->getProc();

    for (auto& v : *s) {
        assert(v.second.e != nullptr);
        QString sym = _proc->lookupSymFromRefAny(RefExp::get(v.second.e, v.second.def()));

        if (!sym.isNull()) {
            v.second.e = Location::local(sym, _proc); // Some may be parameters, but hopefully it won't matter
        }
    }

    recur = false; // TODO: verify recur setting
}


void StmtSsaXformer::visit(CallStatement *s, bool& recur)
{
    SharedExp pDest = s->getDest();

    if (pDest) {
        pDest = pDest->accept((ExpSsaXformer *)m_mod);
        s->setDest(pDest);
    }

    StatementList&          arguments = s->getArguments();
    StatementList::iterator ss;

    for (ss = arguments.begin(); ss != arguments.end(); ++ss) {
        (*ss)->accept(this);
    }

    // Note that defines have statements (assignments) within a statement (this call). The fromSSA logic, which needs
    // to subscript definitions on the left with the statement pointer, won't work if we just call the assignment's
    // fromSSA() function
    StatementList& defines = s->getDefines();

    for (ss = defines.begin(); ss != defines.end(); ++ss) {
        Assignment *as = ((Assignment *)*ss);
        // FIXME: use of fromSSAleft is deprecated
        SharedExp e = as->getLeft()->fromSSAleft(((ExpSsaXformer *)m_mod)->getProc(), s);
        // FIXME: this looks like a HACK that can go:
        Function *procDest = s->getDestProc();

        if (procDest && procDest->isLib() && e->isLocal()) {
            UserProc   *_proc = s->getProc(); // Enclosing proc
            SharedType lty    = _proc->getLocalType(e->access<Const, 1>()->getStr());
            SharedType ty     = as->getType();

            if (ty && lty && (*ty != *lty)) {
                LOG << "local " << e << " has type " << lty->getCtype() << " that doesn't agree with type of define "
                    << ty->getCtype() << " of a library, why?\n";
                _proc->setLocalType(e->access<Const, 1>()->getStr(), ty);
            }
        }

        as->setLeft(e);
    }

    // Don't think we'll need this anyway:
    // defCol.fromSSAform(ig);

    // However, need modifications of the use collector; needed when say eax is renamed to local5, otherwise
    // local5 is removed from the results of the call
    s->useColFromSsaForm(s);
    recur = false; // TODO: verify recur setting
}


DfaLocalMapper::DfaLocalMapper(UserProc *_proc)
    : m_proc(_proc)
{
    m_sig  = m_proc->getSignature();
    m_prog = m_proc->getProg();
    change = false;
}


bool DfaLocalMapper::processExp(const SharedExp& e)
{
    if (m_proc->isLocalOrParamPattern(e)) { // Check if this is an appropriate pattern for local variables
        if (m_sig->isStackLocal(m_prog, e)) {
            change = true;                  // We've made a mapping
            // We have probably not even run TA yet, so doing a full descendtype here would be silly
            // Note also that void is compatible with all types, so the symbol effectively covers all types
            m_proc->getSymbolExp(e, VoidType::get(), true);
        }

        return false; // set recur false: Don't dig inside m[x] to make m[a[m[x]]] !
    }

    return true;
}


SharedExp DfaLocalMapper::preVisit(const std::shared_ptr<Location>& e, bool& recur)
{
    recur = true;

    if (e->isMemOf() && (m_proc->findFirstSymbol(e) == nullptr)) { // Need the 2nd test to ensure change set correctly
        recur = processExp(e);
    }

    return e;
}


SharedExp DfaLocalMapper::preVisit(const std::shared_ptr<Binary>& e, bool& recur)
{
#if 1
    // Check for sp -/+ K
    SharedExp memOf_e = Location::memOf(e);

    if (m_proc->findFirstSymbol(memOf_e) != nullptr) {
        recur = false; // Already done; don't recurse
        return e;
    }
    else {
        recur = processExp(memOf_e);              // Process m[this]

        if (!recur) {                             // If made a change this visit,
            return Unary::get(opAddrOf, memOf_e); // change to a[m[this]]
        }
    }
#endif
    return e;
}


SharedExp DfaLocalMapper::preVisit(const std::shared_ptr<TypedExp>& e, bool& recur)
{
    // Assume it's already been done correctly, so don't recurse into this
    recur = false;
    return e;
}
