/*
 * Copyright (C) 2004-2006, Mike Van Emmerik and Trent Waddington
 */
/***************************************************************************/ /**
  * \file       visitor.cpp
  * \brief   Provides the implementation for the various visitor and modifier classes.
  ******************************************************************************/
#include "visitor.h"

#include "exp.h"
#include "statement.h"
#include "log.h"
#include "boomerang.h" // For VERBOSE
#include "proc.h"
#include "signature.h"
#include "prog.h"

#include <QtCore/QDebug>
#include <sstream>

// FixProcVisitor class

bool FixProcVisitor::visit(Location *l, bool &override) {
    l->setProc(proc); // Set the proc, but only for Locations
    override = false; // Use normal accept logic
    return true;
}

// GetProcVisitor class

bool GetProcVisitor::visit(Location *l, bool &override) {
    proc = l->getProc();
    override = false;
    return proc == nullptr; // Continue recursion only if failed so far
}

// SetConscripts class

bool SetConscripts::visit(Const *c) {
    if (!bInLocalGlobal) {
        if (bClear)
            c->setConscript(0);
        else
            c->setConscript(++curConscript);
    }
    bInLocalGlobal = false;
    return true; // Continue recursion
}

bool SetConscripts::visit(Location *l, bool &override) {
    OPER op = l->getOper();
    if (op == opLocal || op == opGlobal || op == opRegOf || op == opParam)
        bInLocalGlobal = true;
    override = false;
    return true; // Continue recursion
}

bool SetConscripts::visit(Binary *b, bool &override) {
    OPER op = b->getOper();
    if (op == opSize)
        bInLocalGlobal = true;
    override = false;
    return true; // Continue recursion
}

bool StmtVisitor::visit(RTL * /*rtl*/) {
    // Mostly, don't do anything at the RTL level
    return true;
}

bool StmtConscriptSetter::visit(Assign *stmt) {
    SetConscripts sc(curConscript, bClear);
    stmt->getLeft()->accept(&sc);
    stmt->getRight()->accept(&sc);
    curConscript = sc.getLast();
    return true;
}
bool StmtConscriptSetter::visit(PhiAssign *stmt) {
    SetConscripts sc(curConscript, bClear);
    stmt->getLeft()->accept(&sc);
    curConscript = sc.getLast();
    return true;
}
bool StmtConscriptSetter::visit(ImplicitAssign *stmt) {
    SetConscripts sc(curConscript, bClear);
    stmt->getLeft()->accept(&sc);
    curConscript = sc.getLast();
    return true;
}

bool StmtConscriptSetter::visit(CallStatement *stmt) {
    SetConscripts sc(curConscript, bClear);
    StatementList &args = stmt->getArguments();
    StatementList::iterator ss;
    for (ss = args.begin(); ss != args.end(); ++ss)
        (*ss)->accept(this);
    curConscript = sc.getLast();
    return true;
}

bool StmtConscriptSetter::visit(CaseStatement *stmt) {
    SetConscripts sc(curConscript, bClear);
    SWITCH_INFO *si = stmt->getSwitchInfo();
    if (si) {
        si->pSwitchVar->accept(&sc);
        curConscript = sc.getLast();
    }
    return true;
}

bool StmtConscriptSetter::visit(ReturnStatement *stmt) {
    SetConscripts sc(curConscript, bClear);
    ReturnStatement::iterator rr;
    for (rr = stmt->begin(); rr != stmt->end(); ++rr)
        (*rr)->accept(this);
    curConscript = sc.getLast();
    return true;
}

bool StmtConscriptSetter::visit(BoolAssign *stmt) {
    SetConscripts sc(curConscript, bClear);
    stmt->getCondExpr()->accept(&sc);
    stmt->getLeft()->accept(&sc);
    curConscript = sc.getLast();
    return true;
}

bool StmtConscriptSetter::visit(BranchStatement *stmt) {
    SetConscripts sc(curConscript, bClear);
    stmt->getCondExpr()->accept(&sc);
    curConscript = sc.getLast();
    return true;
}

bool StmtConscriptSetter::visit(ImpRefStatement *stmt) {
    SetConscripts sc(curConscript, bClear);
    stmt->getAddressExp()->accept(&sc);
    curConscript = sc.getLast();
    return true;
}

void PhiStripper::visit(PhiAssign * /*s*/, bool &recur) {
    del = true;
    recur = true;
}

Exp *CallBypasser::postVisit(RefExp *r) {
    // If child was modified, simplify now
    Exp *ret = r;
    if (!(unchanged & mask))
        ret = r->simplify();
    mask >>= 1;
    // Note: r (the pointer) will always == ret (also the pointer) here, so the below is safe and avoids a cast
    Instruction *def = r->getDef();
    CallStatement *call = dynamic_cast<CallStatement *>(def);
    if (call ) {
        bool ch;
        ret = call->bypassRef((RefExp *)ret, ch);
        if (ch) {
            unchanged &= ~mask;
            mod = true;
            // Now have to recurse to do any further bypassing that may be required
            // E.g. bypass the two recursive calls in fibo?? FIXME: check!
            return ret->accept(new CallBypasser(enclosingStmt));
        }
    }

    // Else just leave as is (perhaps simplified)
    return ret;
}

Exp *CallBypasser::postVisit(Location *e) {
    // Hack to preserve a[m[x]]. Can likely go when ad hoc TA goes.
    bool isAddrOfMem = e->isAddrOf() && e->getSubExp1()->isMemOf();
    if (isAddrOfMem)
        return e;
    Exp *ret = e;
    if (!(unchanged & mask))
        ret = e->simplify();
    mask >>= 1;
    return ret;
}

Exp *SimpExpModifier::postVisit(Location *e) {
    Exp *ret = e;
    if (!(unchanged & mask))
        ret = e->simplify();
    mask >>= 1;
    return ret;
}
Exp *SimpExpModifier::postVisit(RefExp *e) {
    Exp *ret = e;
    if (!(unchanged & mask))
        ret = e->simplify();
    mask >>= 1;
    return ret;
}
Exp *SimpExpModifier::postVisit(Unary *e) {
    Exp *ret = e;
    if (!(unchanged & mask))
        ret = e->simplify();
    mask >>= 1;
    return ret;
}
Exp *SimpExpModifier::postVisit(Binary *e) {
    Exp *ret = e;
    if (!(unchanged & mask))
        ret = e->simplifyArith()->simplify();
    mask >>= 1;
    return ret;
}
Exp *SimpExpModifier::postVisit(Ternary *e) {
    Exp *ret = e;
    if (!(unchanged & mask))
        ret = e->simplify();
    mask >>= 1;
    return ret;
}
Exp *SimpExpModifier::postVisit(TypedExp *e) {
    Exp *ret = e;
    if (!(unchanged & mask))
        ret = e->simplify();
    mask >>= 1;
    return ret;
}
Exp *SimpExpModifier::postVisit(FlagDef *e) {
    Exp *ret = e;
    if (!(unchanged & mask))
        ret = e->simplify();
    mask >>= 1;
    return ret;
}
Exp *SimpExpModifier::postVisit(Const *e) {
    mask >>= 1;
    return e;
}
Exp *SimpExpModifier::postVisit(TypeVal *e) {
    mask >>= 1;
    return e;
}
Exp *SimpExpModifier::postVisit(Terminal *e) {
    mask >>= 1;
    return e;
}

// Add used locations finder
bool UsedLocsFinder::visit(Location *e, bool &override) {
    if (!memOnly)
        used->insert(e); // All locations visited are used
    if (e->isMemOf()) {
        // Example: m[r28{10} - 4]    we use r28{10}
        Exp *child = e->getSubExp1();
        // Care! Need to turn off the memOnly flag for work inside the m[...], otherwise everything will get ignored
        bool wasMemOnly = memOnly;
        memOnly = false;
        child->accept(this);
        memOnly = wasMemOnly;
        override = true; // Already looked inside child
    } else
        override = false;
    return true; // Continue looking for other locations
}

bool UsedLocsFinder::visit(Terminal *e) {
    if (memOnly)
        return true; // Only interested in m[...]
    switch (e->getOper()) {
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
        used->insert(e);
    default:
        break;
    }
    return true; // Always continue recursion
}

bool UsedLocsFinder::visit(RefExp &arg, bool &override) {
    if (memOnly) {
        override = false; // Look inside the ref for m[...]
        return true;      // Don't count this reference
    }
    RefExp *e = &arg;
    if(used->find(e)==used->end()) {
        //e = (RefExp *)arg.clone();
        used->insert(&arg); // This location is used
    }
    // However, e's subexpression is NOT used ...
    override = true;
    // ... unless that is a m[x], array[x] or .x, in which case x (not m[x]/array[x]/refd.x) is used
    Exp *refd = e->getSubExp1();
    if (refd->isMemOf()) {
        refd->getSubExp1()->accept(this);
    } else if (refd->isArrayIndex()) {
        refd->getSubExp1()->accept(this);
        refd->getSubExp2()->accept(this);
    } else if (refd->isMemberOf()) {
        refd->getSubExp1()->accept(this);
    }
    return true;
}

bool UsedLocalFinder::visit(Location *e, bool &override) {
    override = false;
#if 0
    char* sym = proc->lookupSym(e);
    if (sym)
        override = true;                // Don't look inside this local or parameter
    if (proc->findLocal(e))
#else
    if (e->isLocal())
#endif
    used->insert(e); // Found a local
    return true;     // Continue looking for other locations
}

bool UsedLocalFinder::visit(TypedExp *e, bool &override) {
    override = false;
    SharedType ty = e->getType();
    // Assumption: (cast)exp where cast is of pointer type means that exp is the address of a local
    if (ty->resolvesToPointer()) {
        Exp *sub = e->getSubExp1();
        Exp *mof = Location::memOf(sub);
        if (!proc->findLocal(*mof, ty).isNull()) {
            used->insert(mof);
            override = true;
        }
        else
            delete mof;
    }
    return true;
}

bool UsedLocalFinder::visit(Terminal *e) {
    if (e->getOper() == opDefineAll)
        all = true;
    QString sym = proc->findFirstSymbol(e);
    if (!sym.isNull())
        used->insert(e);
    return true; // Always continue recursion
}

bool UsedLocsVisitor::visit(Assign *s, bool &override) {
    Exp *lhs = s->getLeft();
    Exp *rhs = s->getRight();
    if (rhs)
        rhs->accept(ev);
    // Special logic for the LHS. Note: PPC can have r[tmp + 30] on LHS
    if (lhs->isMemOf() || lhs->isRegOf()) {
        Exp *child = ((Location *)lhs)->getSubExp1(); // m[xxx] uses xxx
        // Care! Don't want the memOnly flag when inside a m[...]. Otherwise, nothing will be found
        // Also beware that ev may be a UsedLocalFinder now
        UsedLocsFinder *ulf = dynamic_cast<UsedLocsFinder *>(ev);
        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            child->accept(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    } else if (lhs->getOper() == opArrayIndex || lhs->getOper() == opMemberAccess) {
        Exp *subExp1 = ((Binary *)lhs)->getSubExp1(); // array(base, index) and member(base, offset)?? use
        subExp1->accept(ev);                          // base and index
        Exp *subExp2 = ((Binary *)lhs)->getSubExp2();
        subExp2->accept(ev);
    } else if (lhs->getOper() == opAt) { // foo@[first:last] uses foo, first, and last
        Exp *subExp1 = ((Ternary *)lhs)->getSubExp1();
        subExp1->accept(ev);
        Exp *subExp2 = ((Ternary *)lhs)->getSubExp2();
        subExp2->accept(ev);
        Exp *subExp3 = ((Ternary *)lhs)->getSubExp3();
        subExp3->accept(ev);
    }
    override = true; // Don't do the usual accept logic
    return true;     // Continue the recursion
}
bool UsedLocsVisitor::visit(PhiAssign *s, bool &override) {
    Exp *lhs = s->getLeft();
    // Special logic for the LHS
    if (lhs->isMemOf()) {
        Exp *child = ((Location *)lhs)->getSubExp1();
        UsedLocsFinder *ulf = dynamic_cast<UsedLocsFinder *>(ev);
        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            child->accept(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    } else if (lhs->getOper() == opArrayIndex || lhs->getOper() == opMemberAccess) {
        Exp *subExp1 = ((Binary *)lhs)->getSubExp1();
        subExp1->accept(ev);
        Exp *subExp2 = ((Binary *)lhs)->getSubExp2();
        subExp2->accept(ev);
    }

    for (const auto &v : *s) {
        // Note: don't make the RefExp based on lhs, since it is possible that the lhs was renamed in fromSSA()
        // Use the actual expression in the PhiAssign
        // Also note that it's possible for uu->e to be nullptr. Suppose variable a can be assigned to along in-edges
        // 0, 1, and 3; inserting the phi parameter at index 3 will cause a null entry at 2
        assert(v.second.e);
        RefExp *temp = RefExp::get(v.second.e, (Instruction *)v.second.def());
        temp->accept(ev);
    }

    override = true; // Don't do the usual accept logic
    return true;     // Continue the recursion
}
bool UsedLocsVisitor::visit(ImplicitAssign *s, bool &override) {
    Exp *lhs = s->getLeft();
    // Special logic for the LHS
    if (lhs->isMemOf()) {
        Exp *child = ((Location *)lhs)->getSubExp1();
        UsedLocsFinder *ulf = dynamic_cast<UsedLocsFinder *>(ev);
        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            child->accept(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    } else if (lhs->getOper() == opArrayIndex || lhs->getOper() == opMemberAccess) {
        Exp *subExp1 = ((Binary *)lhs)->getSubExp1();
        subExp1->accept(ev);
        Exp *subExp2 = ((Binary *)lhs)->getSubExp2();
        subExp2->accept(ev);
    }
    override = true; // Don't do the usual accept logic
    return true;     // Continue the recursion
}

bool UsedLocsVisitor::visit(CallStatement *s, bool &override) {
    Exp *pDest = s->getDest();
    if (pDest)
        pDest->accept(ev);
    StatementList::iterator it;
    StatementList &arguments = s->getArguments();
    for (it = arguments.begin(); it != arguments.end(); it++) {
        // Don't want to ever collect anything from the lhs
        ((Assign *)*it)->getRight()->accept(ev);
    }
    if (countCol) {
        DefCollector::iterator dd;
        DefCollector *col = s->getDefCollector();
        for (dd = col->begin(); dd != col->end(); ++dd)
            (*dd)->accept(this);
    }
    override = true; // Don't do the normal accept logic
    return true;     // Continue the recursion
}

bool UsedLocsVisitor::visit(ReturnStatement *s, bool &override) {
    // For the final pass, only consider the first return
    ReturnStatement::iterator rr;
    for (rr = s->begin(); rr != s->end(); ++rr)
        (*rr)->accept(this);
    // Also consider the reaching definitions to be uses, so when they are the only non-empty component of this
    // ReturnStatement, they can get propagated to.
    if (countCol) { // But we need to ignore these "uses" unless propagating
        DefCollector::iterator dd;
        DefCollector *col = s->getCollector();
        for (dd = col->begin(); dd != col->end(); ++dd)
            (*dd)->accept(this);
    }

    // Insert a phantom use of "everything" here, so that we can find out if any childless calls define something that
    // may end up being returned
    // FIXME: Not here! Causes locals to never get removed. Find out where this belongs, if anywhere:
    //((UsedLocsFinder*)ev)->getLocSet()->insert(new Terminal(opDefineAll));

    override = true; // Don't do the normal accept logic
    return true;     // Continue the recursion
}

bool UsedLocsVisitor::visit(BoolAssign *s, bool &override) {
    Exp *pCond = s->getCondExpr();
    if (pCond)
        pCond->accept(ev); // Condition is used
    Exp *lhs = s->getLeft();
    assert(lhs);
    if (lhs->isMemOf()) { // If dest is of form m[x]...
        Exp *x = ((Location *)lhs)->getSubExp1();
        UsedLocsFinder *ulf = dynamic_cast<UsedLocsFinder *>(ev);
        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            x->accept(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    } else if (lhs->getOper() == opArrayIndex || lhs->getOper() == opMemberAccess) {
        Exp *subExp1 = ((Binary *)lhs)->getSubExp1();
        subExp1->accept(ev);
        Exp *subExp2 = ((Binary *)lhs)->getSubExp2();
        subExp2->accept(ev);
    }
    override = true; // Don't do the normal accept logic
    return true;     // Continue the recursion
}

//
// Expression subscripter
//
Exp *ExpSubscripter::preVisit(Location *e, bool &recur) {
    if (*e == *search) {
        recur = e->isMemOf();      // Don't double subscript unless m[...]
        return new RefExp(e, def); // Was replaced by postVisit below
    }
    recur = true;
    return e;
}

Exp *ExpSubscripter::preVisit(Binary *e, bool &recur) {
    // array[index] is like m[addrexp]: requires a subscript
    if (e->isArrayIndex() && *e == *search) {
        recur = true;              // Check the index expression
        return new RefExp(e, def); // Was replaced by postVisit below
    }
    recur = true;
    return e;
}

Exp *ExpSubscripter::preVisit(Terminal *e) {
    if (*e == *search)
        return new RefExp(e, def);
    return e;
}

Exp *ExpSubscripter::preVisit(RefExp *e, bool &recur) {
    recur = false; // Don't look inside... not sure about this
    return e;
}

// The Statement subscripter class
void StmtSubscripter::visit(Assign *s, bool &recur) {
    Exp *rhs = s->getRight();
    s->setRight(rhs->accept(mod));
    // Don't subscript the LHS of an assign, ever
    Exp *lhs = s->getLeft();
    if (lhs->isMemOf() || lhs->isRegOf()) {
        ((Location *)lhs)->setSubExp1(((Location *)lhs)->getSubExp1()->accept(mod));
    }
    recur = false;
}
void StmtSubscripter::visit(PhiAssign *s, bool &recur) {
    Exp *lhs = s->getLeft();
    if (lhs->isMemOf()) {
        ((Location *)lhs)->setSubExp1(((Location *)lhs)->getSubExp1()->accept(mod));
    }
    recur = false;
}
void StmtSubscripter::visit(ImplicitAssign *s, bool &recur) {
    Exp *lhs = s->getLeft();
    if (lhs->isMemOf()) {
        ((Location *)lhs)->setSubExp1(((Location *)lhs)->getSubExp1()->accept(mod));
    }
    recur = false;
}
void StmtSubscripter::visit(BoolAssign *s, bool &recur) {
    Exp *lhs = s->getLeft();
    if (lhs->isMemOf()) {
        ((Location *)lhs)->setSubExp1(((Location *)lhs)->getSubExp1()->accept(mod));
    }
    Exp *rhs = s->getCondExpr();
    s->setCondExpr(rhs->accept(mod));
    recur = false;
}

void StmtSubscripter::visit(CallStatement *s, bool &recur) {
    Exp *pDest = s->getDest();
    if (pDest)
        s->setDest(pDest->accept(mod));
    // Subscript the ordinary arguments
    StatementList &arguments = s->getArguments();
    StatementList::iterator ss;
    for (ss = arguments.begin(); ss != arguments.end(); ++ss)
        (*ss)->accept(this);
    // Returns are like the LHS of an assignment; don't subscript them directly (only if m[x], and then only subscript
    // the x's)
    recur = false; // Don't do the usual accept logic
}

// Size stripper
Exp *SizeStripper::preVisit(Binary *b, bool &recur) {
    recur = true; // Visit the binary's children
    if (b->isSizeCast())
        // Could be a size cast of a size cast
        return b->getSubExp2()->stripSizes();
    return b;
}

Exp *ExpConstCaster::preVisit(Const *c) {
    if (c->getConscript() == num) {
        changed = true;
        return new TypedExp(ty, c);
    }
    return c;
}

// This is the code (apart from definitions) to find all constants in a Statement
bool ConstFinder::visit(Const *e) {
    lc.push_back(e);
    return true;
}
bool ConstFinder::visit(Location *e, bool &override) {
    if (e->isMemOf())
        override = false; // We DO want to see constants in memofs
    else
        override = true; // Don't consider register numbers, global names, etc
    return true;
}

// This is in the POST visit function, because it's important to process any child expressions first.
// Otherwise, for m[r28{0} - 12]{0}, you could be adding an implicit assignment with a nullptr definition for r28.
Exp *ImplicitConverter::postVisit(RefExp *e) {
    if (e->getDef() == nullptr)
        e->setDef(m_cfg->findImplicitAssign(e->getSubExp1()));
    return e;
}

void StmtImplicitConverter::visit(PhiAssign *s, bool &recur) {
    // The LHS could be a m[x] where x has a null subscript; must do first
    s->setLeft(s->getLeft()->accept(mod));

    for (auto &v : *s) {
        assert(v.second.e != nullptr);
        if (v.second.def() == nullptr)
            v.second.def(m_cfg->findImplicitAssign(v.second.e));
    }
    recur = false; // Already done LHS
}

// Localiser. Subscript a location with the definitions that reach the call, or with {-} if none
Exp *Localiser::preVisit(RefExp *e, bool &recur) {
    recur = false; // Don't recurse into already subscripted variables
    mask <<= 1;
    return e;
}

Exp *Localiser::preVisit(Location *e, bool &recur) {
    recur = true;
    mask <<= 1;
    return e;
}

Exp *Localiser::postVisit(Location *e) {
    Exp *ret = e;
    if (!(unchanged & mask))
        ret = e->simplify();
    mask >>= 1;
    Exp *r = call->findDefFor(ret);
    if (r) {
        ret = r->clone();
        if (0 && EXPERIMENTAL) { // FIXME: check if sometimes needed
            // The trouble with the below is that you can propagate to say a call statement's argument expression and
            // not to the assignment of the actual argument. Examples: test/pentium/fromssa2, fbranch
            bool ch;
            ret = ret->propagateAllRpt(ch); // Propagate into this repeatedly, in case propagation is limited
        }
        ret = ret->bypass();
        unchanged &= ~mask;
        mod = true;
    } else
        ret = new RefExp(ret, nullptr); // No definition reaches, so subscript with {-}
    return ret;
}

// Want to be able to localise a few terminals, in particular <all>
Exp *Localiser::postVisit(Terminal *e) {
    Exp *ret = e;
    if (!(unchanged & mask))
        ret = e->simplify();
    mask >>= 1;
    Exp *r = call->findDefFor(ret);
    if (r) {
        ret = r->clone()->bypass();
        unchanged &= ~mask;
        mod = true;
    } else
        ret = new RefExp(ret, nullptr); // No definition reaches, so subscript with {-}
    return ret;
}

bool ComplexityFinder::visit(Location *e, bool &override) {
    if (proc && proc->findFirstSymbol(e) != nullptr) {
        // This is mapped to a local. Count it as zero, not about 3 (m[r28+4] -> memof, regof, plus)
        override = true;
        return true;
    }
    if (e->isMemOf() || e->isArrayIndex())
        count++; // Count the more complex unaries
    override = false;
    return true;
}
bool ComplexityFinder::visit(Unary * /*e*/, bool &override) {
    count++;
    override = false;
    return true;
}
bool ComplexityFinder::visit(Binary * /*e*/, bool &override) {
    count++;
    override = false;
    return true;
}
bool ComplexityFinder::visit(Ternary * /*e*/, bool &override) {
    count++;
    override = false;
    return true;
}

bool MemDepthFinder::visit(Location *e, bool &override) {
    if (e->isMemOf())
        ++depth;
    override = false;
    return true;
}

// Ugh! This is still a separate propagation mechanism from Statement::propagateTo().
Exp *ExpPropagator::postVisit(RefExp *e) {
    // No need to call e->canRename() here, because if e's base expression is not suitable for renaming, it will never
    // have been renamed, and we never would get here
    if (!Instruction::canPropagateToExp(*e)) // Check of the definition statement is suitable for propagating
        return e;
    Instruction *def = e->getDef();
    Exp *res = e;
    if (def && def->isAssign()) {
        Exp *lhs = ((Assign *)def)->getLeft();
        Exp *rhs = ((Assign *)def)->getRight();
        bool ch;
        res = e->searchReplaceAll(RefExp(lhs, def), rhs->clone(), ch);
        if (ch) {
            change = true;      // Record this change
            unchanged &= ~mask; // Been changed now (so simplify parent)
            if (res->isSubscript())
                res = postVisit((RefExp *)res); // Recursively propagate more if possible
        }
    }
    return res;
}

// Return true if e is a primitive expression; basically, an expression you can propagate to without causing
// memory expression problems. See Mike's thesis for details
// Algorithm: if find any unsubscripted location, not primitive
//   Implicit definitions are primitive (but keep searching for non primitives)
//   References to the results of calls are considered primitive... but only if bypassed?
//   Other references considered non primitive
// Start with result=true, must find primitivity in all components
bool PrimitiveTester::visit(Location * /*e*/, bool &override) {
    // We reached a bare (unsubscripted) location. This is certainly not primitive
    override = true;
    result = false;
    return false; // No need to continue searching
}

bool PrimitiveTester::visit(RefExp &e, bool &override) {
    Instruction *def = e.getDef();
    // If defined by a call, e had better not be a memory location (crude approximation for now)
    if (def == nullptr || def->getNumber() == 0 || (def->isCall() && !e.getSubExp1()->isMemOf())) {
        // Implicit definitions are always primitive
        // The results of calls are always primitive
        override = true; // Don't recurse into the reference
        return true;     // Result remains true
    }

    // For now, all references to other definitions will be considered non primitive. I think I'll have to extend this!
    result = false;
    override = true; // Regareless of outcome, don't recurse into the reference
    return true;
}

bool ExpHasMemofTester::visit(Location *e, bool &override) {
    if (e->isMemOf()) {
        override = true; // Don't recurse children (not needed surely)
        result = true;   // Found a memof
        return false;    // Don't continue searching the expression
    }
    override = false;
    return true;
}

bool TempToLocalMapper::visit(Location *e, bool &override) {
    if (e->isTemp()) {
        // We have a temp subexpression; get its name
        QString tempName = ((Const *)e->getSubExp1())->getStr();
        SharedType ty = Type::getTempType(tempName); // Types for temps strictly depend on the name
        // This call will do the mapping from the temp to a new local:
        proc->getSymbolExp(e, ty, true);
    }
    override = true; // No need to examine the string
    return true;
}

ExpRegMapper::ExpRegMapper(UserProc *p) : proc(p) { prog = proc->getProg(); }

// The idea here is to map the default of a register to a symbol with the type of that first use. If the register is
// not involved in any conflicts, it will use this name by default
bool ExpRegMapper::visit(RefExp &e, bool &override) {
    Exp *base = e.getSubExp1();
    if (base->isRegOf() || proc->isLocalOrParamPattern(base)) // Don't convert if e.g. a global
        proc->checkLocalFor(e);
    override = true; // Don't examine the r[] inside
    return true;
}

bool StmtRegMapper::common(Assignment *stmt, bool &override) {
    // In case lhs is a reg or m[reg] such that reg is otherwise unused
    Exp *lhs = stmt->getLeft();
    RefExp *re = new RefExp(lhs, stmt);
    re->accept((ExpRegMapper *)ev);
    override = false;
    return true;
}

bool StmtRegMapper::visit(Assign *stmt, bool &override) { return common(stmt, override); }
bool StmtRegMapper::visit(PhiAssign *stmt, bool &override) { return common(stmt, override); }
bool StmtRegMapper::visit(ImplicitAssign *stmt, bool &override) { return common(stmt, override); }
bool StmtRegMapper::visit(BoolAssign *stmt, bool &override) { return common(stmt, override); }

// Constant global converter. Example: m[m[r24{16} + m[0x8048d60]{-}]{-}]{-} -> m[m[r24{16} + 32]{-}]{-}
// Allows some complex variations to be matched to standard indirect call forms
Exp *ConstGlobalConverter::preVisit(RefExp *e, bool &recur) {
    Instruction *def = e->getDef();
    Exp *base, *addr, *idx, *glo;
    if (def == nullptr || def->isImplicit()) {
        if ((base = e->getSubExp1(), base->isMemOf()) &&
            (addr = ((Location *)base)->getSubExp1(), addr->isIntConst())) {
            // We have a m[K]{-}
            ADDRESS K = ((Const *)addr)->getAddr(); // TODO: use getAddr
            int value = prog->readNative4(K);
            recur = false;
            return new Const(value);
        } else if (base->isGlobal()) {
            // We have a glo{-}
            QString gname = ((Const *)(base->getSubExp1()))->getStr();
            ADDRESS gloValue = prog->getGlobalAddr(gname);
            int value = prog->readNative4(gloValue);
            recur = false;
            return new Const(value);
        } else if (base->isArrayIndex() && (idx = ((Binary *)base)->getSubExp2(), idx->isIntConst()) &&
                   (glo = ((Binary *)base)->getSubExp1(), glo->isGlobal())) {
            // We have a glo[K]{-}
            int K = ((Const *)idx)->getInt();
            QString gname = ((Const *)(glo->getSubExp1()))->getStr();
            ADDRESS gloValue = prog->getGlobalAddr(gname);
            SharedType gloType = prog->getGlobal(gname)->getType();
            assert(gloType->isArray());
            SharedType componentType = gloType->asArray()->getBaseType();
            int value = prog->readNative4(gloValue + K * (componentType->getSize() / 8));
            recur = false;
            return new Const(value);
        }
    }
    recur = true;
    return e;
}

bool ExpDestCounter::visit(RefExp &e, bool &override) {
    if (Instruction::canPropagateToExp(e))
        destCounts[e.clone()]++;
    override = false; // Continue searching my children
    return true;      // Continue visiting the rest of Exp* e
}

bool StmtDestCounter::visit(PhiAssign * /*stmt*/, bool &override) {
    override = false;
    return true;
}

bool FlagsFinder::visit(Binary *e, bool &override) {
    if (e->isFlagCall()) {
        found = true;
        return false; // Don't continue searching
    }
    override = false;
    return true;
}

// Search for bare memofs (not subscripted) in the expression
bool BadMemofFinder::visit(Location *e, bool &override) {
    if (e->isMemOf()) {
        found = true; // A bare memof
        return false;
    }
    override = false;
    return true; // Continue searching
}

bool BadMemofFinder::visit(RefExp &e, bool &override) {
    Exp *base = e.getSubExp1();
    if (base->isMemOf()) {
        // Beware: it may be possible to have a bad memof inside a subscripted one
        Exp *addr = ((Location *)base)->getSubExp1();
        addr->accept(this);
        if (found)
            return false; // Don't continue searching
#if NEW                   // FIXME: not ready for this until have incremental propagation
        const char *sym = proc->lookupSym(e);
        if (sym == nullptr) {
            found = true;    // Found a memof that is not a symbol
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
void ExpCastInserter::checkMemofType(Exp *memof, SharedType memofType) {
    Exp *addr = ((Unary *)memof)->getSubExp1();
    if (addr->isSubscript()) {
        Exp *addrBase = ((RefExp *)addr)->getSubExp1();
        SharedType actType = ((RefExp *)addr)->getDef()->getTypeFor(addrBase);
        SharedType expectedType = PointerType::get(memofType);
        if (!actType->isCompatibleWith(*expectedType)) {
            ((Unary *)memof)->setSubExp1(new TypedExp(expectedType, addrBase));
        }
    }
}

Exp *ExpCastInserter::postVisit(RefExp *e) {
    Exp *base = e->getSubExp1();
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

static Exp *checkSignedness(Exp *e, int reqSignedness) {
    SharedType ty = e->ascendType();
    int currSignedness = 0;
    bool isInt = ty->resolvesToInteger();
    if (isInt) {
        currSignedness = ty->asInteger()->getSignedness();
        currSignedness = (currSignedness >= 0) ? 1 : -1;
    }
    // if (!isInt || currSignedness != reqSignedness) { // }
    // Don't want to cast e.g. floats to integer
    if (isInt && currSignedness != reqSignedness) {
        std::shared_ptr<IntegerType> newtype;
        if (!isInt)
            newtype = IntegerType::get(STD_SIZE, reqSignedness);
        else
            newtype = IntegerType::get(std::static_pointer_cast<IntegerType>(ty)->getSize(), reqSignedness); // Transfer size
        newtype->setSigned(reqSignedness);
        return new TypedExp(newtype, e);
    }
    return e;
}

Exp *ExpCastInserter::postVisit(Binary *e) {
    OPER op = e->getOper();
    switch (op) {
    // This case needed for e.g. test/pentium/switch_gcc:
    case opLessUns:
    case opGtrUns:
    case opLessEqUns:
    case opGtrEqUns:
    case opShiftR:
        e->setSubExp1(checkSignedness(e->getSubExp1(), -1));
        if (op != opShiftR) // The shift amount (second operand) is sign agnostic
            e->setSubExp2(checkSignedness(e->getSubExp2(), -1));
        break;
    // This case needed for e.g. test/sparc/minmax2, if %g1 is declared as unsigned int
    case opLess:
    case opGtr:
    case opLessEq:
    case opGtrEq:
    case opShiftRA:
        e->setSubExp1(checkSignedness(e->getSubExp1(), +1));
        if (op != opShiftRA)
            e->setSubExp2(checkSignedness(e->getSubExp2(), +1));
        break;
    default:
        break;
    }
    return e;
}

Exp *ExpCastInserter::postVisit(Const *e) {
    if (e->isIntConst()) {
        bool naturallySigned = e->getInt() < 0;
        SharedType ty = e->getType();
        if (naturallySigned && ty->isInteger() && !ty->asInteger()->isSigned()) {
            return new TypedExp(IntegerType::get(ty->asInteger()->getSize(), -1), e);
        }
    }
    return e;
}

bool StmtCastInserter::visit(Assign *s) { return common(s); }
bool StmtCastInserter::visit(PhiAssign *s) { return common(s); }
bool StmtCastInserter::visit(ImplicitAssign *s) { return common(s); }
bool StmtCastInserter::visit(BoolAssign *s) { return common(s); }
bool StmtCastInserter::common(Assignment *s) {
    Exp *lhs = s->getLeft();
    if (lhs->isMemOf()) {
        SharedType memofType = s->getType();
        ExpCastInserter::checkMemofType(lhs, memofType);
    }
    return true;
}

Exp *ExpSsaXformer::postVisit(RefExp *e) {
    QString sym = proc->lookupSymFromRefAny(*e);
    if (!sym.isNull())
        return Location::local(sym, proc);
    // We should not get here: all locations should be replaced with Locals or Parameters
    // LOG << "ERROR! Could not find local or parameter for " << e << " !!\n";
    return e->getSubExp1(); // At least strip off the subscript
}

// Common code for the left hand side of assignments
void StmtSsaXformer::commonLhs(Assignment *as) {
    Exp *lhs = as->getLeft();
    lhs = lhs->accept((ExpSsaXformer *)mod); // In case the LHS has say m[r28{0}+8] -> m[esp+8]
    RefExp re(lhs, as);
    QString sym = proc->lookupSymFromRefAny(re);
    if (!sym.isNull())
        as->setLeft(Location::local(sym, proc));
}

void StmtSsaXformer::visit(BoolAssign *s, bool &recur) {
    commonLhs(s);
    Exp *pCond = s->getCondExpr();
    pCond = pCond->accept((ExpSsaXformer *)mod);
    s->setCondExpr(pCond);
    recur = false; // TODO: verify recur setting
}

void StmtSsaXformer::visit(Assign *s, bool &recur) {
    commonLhs(s);
    Exp *rhs = s->getRight();
    rhs = rhs->accept(mod);
    s->setRight(rhs);
    recur = false; // TODO: verify recur setting
}

void StmtSsaXformer::visit(ImplicitAssign *s, bool &recur) {
    commonLhs(s);
    recur = false; // TODO: verify recur setting
}

void StmtSsaXformer::visit(PhiAssign *s, bool &recur) {
    commonLhs(s);

    UserProc *proc = ((ExpSsaXformer *)mod)->getProc();
    for (auto &v : *s) {
        assert(v.second.e != nullptr);
        RefExp r(v.second.e, v.second.def());
        QString sym = proc->lookupSymFromRefAny(r);
        if (!sym.isNull())
            v.second.e = Location::local(sym, proc); // Some may be parameters, but hopefully it won't matter
    }
    recur = false; // TODO: verify recur setting
}

void StmtSsaXformer::visit(CallStatement *s, bool &recur) {
    Exp *pDest = s->getDest();
    if (pDest) {
        pDest = pDest->accept((ExpSsaXformer *)mod);
        s->setDest(pDest);
    }
    StatementList &arguments = s->getArguments();
    StatementList::iterator ss;
    for (ss = arguments.begin(); ss != arguments.end(); ++ss)
        (*ss)->accept(this);
    // Note that defines have statements (assignments) within a statement (this call). The fromSSA logic, which needs
    // to subscript definitions on the left with the statement pointer, won't work if we just call the assignment's
    // fromSSA() function
    StatementList &defines = s->getDefines();
    for (ss = defines.begin(); ss != defines.end(); ++ss) {
        Assignment *as = ((Assignment *)*ss);
        // FIXME: use of fromSSAleft is deprecated
        Exp *e = as->getLeft()->fromSSAleft(((ExpSsaXformer *)mod)->getProc(), s);
        // FIXME: this looks like a HACK that can go:
        Function *procDest = s->getDestProc();
        if (procDest && procDest->isLib() && e->isLocal()) {
            UserProc *proc = s->getProc(); // Enclosing proc
            SharedType lty = proc->getLocalType(((Const *)e->getSubExp1())->getStr());
            SharedType ty = as->getType();
            if (ty && lty && *ty != *lty) {
                LOG << "local " << e << " has type " << lty->getCtype() << " that doesn't agree with type of define "
                    << ty->getCtype() << " of a library, why?\n";
                proc->setLocalType(((Const *)e->getSubExp1())->getStr(), ty);
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

// Map expressions to locals, using the (so far DFA based) type analysis information
// Basically, descend types, and when you get to m[...] compare with the local high level pattern;
// when at a sum or difference, check for the address of locals high level pattern that is a pointer

// Map expressions to locals, some with names like param3
DfaLocalMapper::DfaLocalMapper(UserProc *proc) : proc(proc) {
    sig = proc->getSignature();
    prog = proc->getProg();
    change = false;
}

// Common processing for the two main cases (visiting a Location or a Binary)
bool DfaLocalMapper::processExp(Exp *e) {
    if (proc->isLocalOrParamPattern(e)) { // Check if this is an appropriate pattern for local variables
        if (sig->isStackLocal(prog, e)) {
            change = true; // We've made a mapping
            // We have probably not even run TA yet, so doing a full descendtype here would be silly
            // Note also that void is compatible with all types, so the symbol effectively covers all types
            proc->getSymbolExp(e, VoidType::get(), true);
#if 0
        } else {
            QString name = QString("tparam%1").arg(proc->nextParamNum());
            proc->mapSymbolTo(e, Location::param(name, proc));
#endif
        }
        return false; // set recur false: Don't dig inside m[x] to make m[a[m[x]]] !
    }
    return true;
}

Exp *DfaLocalMapper::preVisit(Location *e, bool &recur) {

    recur = true;
    if (e->isMemOf() && proc->findFirstSymbol(e) == nullptr) { // Need the 2nd test to ensure change set correctly
        recur = processExp(e);
    }
    return e;
}

Exp *DfaLocalMapper::preVisit(Binary *e, bool &recur) {
#if 1
    // Check for sp -/+ K
    Exp *memOf_e = Location::memOf(e);
    if (proc->findFirstSymbol(memOf_e) != nullptr) {
        recur = false; // Already done; don't recurse
        return e;
    } else {
        recur = processExp(memOf_e);             // Process m[this]
        if (!recur)                              // If made a change this visit,
            return new Unary(opAddrOf, memOf_e); // change to a[m[this]]
    }
#endif
    return e;
}

Exp *DfaLocalMapper::preVisit(TypedExp *e, bool &recur) {
    // Assume it's already been done correctly, so don't recurse into this
    recur = false;
    return e;
}
