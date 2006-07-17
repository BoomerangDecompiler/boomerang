/*
 * Copyright (C) 2004, Mike Van Emmerik and Trent Waddington
 */
/*==============================================================================
 * FILE:	   visitor.cpp
 * OVERVIEW:   Provides the implementation for the various visitor and modifier
 *			   classes.
 *============================================================================*/
/*
 * $Revision$
 *
 * 14 Jun 04 - Mike: Created, from work started by Trent in 2003
 */

#include "visitor.h"
#include "exp.h"
#include "statement.h"
#include "log.h"
#include "boomerang.h"		// For VERBOSE
#include "proc.h"
#include "signature.h"
#include "prog.h"


// FixProcVisitor class

bool FixProcVisitor::visit(Location* l, bool& override) {
	l->setProc(proc);		// Set the proc, but only for Locations
	override = false;		// Use normal accept logic
	return true;
}

// GetProcVisitor class

bool GetProcVisitor::visit(Location* l, bool& override) {
	proc = l->getProc();
	override = false;
	return proc == NULL;		// Continue recursion only if failed so far
}

// SetConscripts class

bool SetConscripts::visit(Const* c) {
	if (!bInLocalGlobal) {
		if (bClear)
			c->setConscript(0);
		else
			c->setConscript(++curConscript);
	}
	bInLocalGlobal = false;
	return true;	   // Continue recursion
}

bool SetConscripts::visit(Location* l, bool& override) {
	OPER op = l->getOper();
	if (op == opLocal || op == opGlobal || op == opRegOf || op == opParam)
		bInLocalGlobal = true;
	override = false;
	return true;	   // Continue recursion
}

bool SetConscripts::visit(Binary* b, bool& override) {
	OPER op = b->getOper();
	if (op == opSize)
		bInLocalGlobal = true;
	override = false;
	return true;	   // Continue recursion
}


bool StmtVisitor::visit(RTL* rtl) {
	// Mostly, don't do anything at the RTL level
	return true;
} 

bool StmtConscriptSetter::visit(Assign* stmt) {
	SetConscripts sc(curConscript, bClear);
	stmt->getLeft()->accept(&sc);
	stmt->getRight()->accept(&sc);
	curConscript = sc.getLast();
	return true;
}
bool StmtConscriptSetter::visit(PhiAssign* stmt) {
	SetConscripts sc(curConscript, bClear);
	stmt->getLeft()->accept(&sc);
	curConscript = sc.getLast();
	return true;
}
bool StmtConscriptSetter::visit(ImplicitAssign* stmt) {
	SetConscripts sc(curConscript, bClear);
	stmt->getLeft()->accept(&sc);
	curConscript = sc.getLast();
	return true;
}

bool StmtConscriptSetter::visit(CallStatement* stmt) {
	SetConscripts sc(curConscript, bClear);
	StatementList& args = stmt->getArguments();
	StatementList::iterator ss;
	for (ss = args.begin(); ss != args.end(); ++ss)
		(*ss)->accept(this);
	curConscript = sc.getLast();
	return true;
}

bool StmtConscriptSetter::visit(CaseStatement* stmt) {
	SetConscripts sc(curConscript, bClear);
	SWITCH_INFO* si = stmt->getSwitchInfo();
	if (si) {
		si->pSwitchVar->accept(&sc);
		curConscript = sc.getLast();
	}
	return true;
}

bool StmtConscriptSetter::visit(ReturnStatement* stmt) {
	SetConscripts sc(curConscript, bClear);
	ReturnStatement::iterator rr;
	for (rr = stmt->begin(); rr != stmt->end(); ++rr)
		(*rr)->accept(this);
	curConscript = sc.getLast();
	return true;
}

bool StmtConscriptSetter::visit(BoolAssign* stmt) {
	SetConscripts sc(curConscript, bClear);
	stmt->getCondExpr()->accept(&sc);
	stmt->getLeft()->accept(&sc);
	curConscript = sc.getLast();
	return true;
}

bool StmtConscriptSetter::visit(BranchStatement* stmt) {
	SetConscripts sc(curConscript, bClear);
	stmt->getCondExpr()->accept(&sc);
	curConscript = sc.getLast();
	return true;
}

bool StmtConscriptSetter::visit(ImpRefStatement* stmt) {
	SetConscripts sc(curConscript, bClear);
	stmt->getAddressExp()->accept(&sc);
	curConscript = sc.getLast();
	return true;
}

void PhiStripper::visit(PhiAssign* s, bool& recur) {
	del = true;
	recur = true;
}

Exp* CallBypasser::postVisit(RefExp* r) {
	// If child was modified, simplify now
	Exp* ret = r;
	if (!(unchanged & mask)) ret = r->simplify();
	mask >>= 1;
	// Note: r (the pointer) will always == ret (also the pointer) here, so the below is safe and avoids a cast
	Statement* def = r->getDef();
	CallStatement* call = (CallStatement*)def;
	if (call && call->isCall()) {
		bool ch;
		ret = call->bypassRef((RefExp*)ret, ch);
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


Exp* CallBypasser::postVisit(Location *e)	   {
	// Hack to preserve a[m[x]]. Can likely go when ad hoc TA goes.
	bool isAddrOfMem = e->isAddrOf() && e->getSubExp1()->isMemOf();
	if (isAddrOfMem) return e;
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}


Exp* SimpExpModifier::postVisit(Location *e)	   {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}
Exp* SimpExpModifier::postVisit(RefExp *e)	   {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}
Exp* SimpExpModifier::postVisit(Unary *e)	   {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}
Exp* SimpExpModifier::postVisit(Binary *e)	{
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplifyArith()->simplify();
	mask >>= 1;
	return ret;
}
Exp* SimpExpModifier::postVisit(Ternary *e)	 {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}
Exp* SimpExpModifier::postVisit(TypedExp *e)	  {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}
Exp* SimpExpModifier::postVisit(FlagDef *e)	 {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}
Exp* SimpExpModifier::postVisit(Const *e)	   {
	mask >>= 1;
	return e;
}
Exp* SimpExpModifier::postVisit(TypeVal *e)	 {
	mask >>= 1;
	return e;
}
Exp* SimpExpModifier::postVisit(Terminal *e)	  {
	mask >>= 1;
	return e;
}

// Add used locations finder
bool UsedLocsFinder::visit(Location* e, bool& override) {
	if (!memOnly)
		used->insert(e);				// All locations visited are used
	if (e->isMemOf()) {
		// Example: m[r28{10} - 4]	we use r28{10}
		Exp* child = e->getSubExp1();
		// Care! Need to turn off the memOnly flag for work inside the m[...], otherwise everyting will get ignored
		bool wasMemOnly = memOnly;
		memOnly = false;
		child->accept(this);
		memOnly = wasMemOnly;
		override = true;					// Already looked inside child
	}
	else
		override = false;
	return true;						// Continue looking for other locations
}

bool UsedLocsFinder::visit(Terminal* e) {
	if (memOnly)
		return true;					// Only interested in m[...]
	switch (e->getOper()) {
		case opPC:
		case opFlags:
		case opFflags:
		case opDefineAll:
		// Fall through
		// The carry flag can be used in some SPARC idioms, etc
		case opDF: case opCF: case opZF: case opNF: case opOF:	// also these
			used->insert(e);
		default:
			break;
	}
	return true;		// Always continue recursion
}

bool UsedLocsFinder::visit(RefExp* e, bool& override) {
	if (memOnly) {
		override = false;				// Look inside the ref for m[...]
		return true;					// Don't count this reference
	}
	used->insert(e);		 // This location is used
	// However, e's subexpression is NOT used ...
	override = true;
	// ... unless that is a m[x], array[x] or .x, in which case x (not m[x]/array[x]/refd.x) is used
	Exp* refd = e->getSubExp1();
	if (refd->isMemOf()) {
		Exp* x = ((Location*)refd)->getSubExp1();
		x->accept(this);
	}
	else if (refd->isArrayIndex()) {
		Exp* x1 = ((Binary*)refd)->getSubExp1();
		x1->accept(this);
		Exp* x2 = ((Binary*)refd)->getSubExp2();
		x2->accept(this);
	} else if (refd->isMemberOf()) {
		Exp* x = ((Binary*)refd)->getSubExp1();
		x->accept(this);
	}
	return true;
}

bool UsedLocsVisitor::visit(Assign* s, bool& override) {
	Exp* lhs = s->getLeft();
	Exp* rhs = s->getRight();
	if (rhs) rhs->accept(ev);
	// Special logic for the LHS. Note: PPC can have r[tmp + 30] on LHS
	if (lhs->isMemOf() || lhs->isRegOf()) {
		Exp* child = ((Location*)lhs)->getSubExp1();	// m[xxx] uses xxx
		// Care! Don't want the memOnly flag when inside a m[...]. Otherwise, nothing will be found
		UsedLocsFinder* ulf = (UsedLocsFinder*)ev;
		bool wasMemOnly = ulf->isMemOnly();
		ulf->setMemOnly(false);
		child->accept(ev);
		ulf->setMemOnly(wasMemOnly);
	} else if (lhs->getOper() == opArrayIndex || lhs->getOper() == opMemberAccess) {
		Exp* subExp1 = ((Binary*)lhs)->getSubExp1();	// array(base, index) and member(base, offset)?? use
		subExp1->accept(ev);							// base and index
		Exp* subExp2 = ((Binary*)lhs)->getSubExp2();
		subExp2->accept(ev);
	} else if (lhs->getOper() == opAt) {				// foo@[first:last] uses foo, first, and last
		Exp* subExp1 = ((Ternary*)lhs)->getSubExp1();
		subExp1->accept(ev);
		Exp* subExp2 = ((Ternary*)lhs)->getSubExp2();
		subExp2->accept(ev);
		Exp* subExp3 = ((Ternary*)lhs)->getSubExp3();
		subExp3->accept(ev);
	}
	override = true;				// Don't do the usual accept logic
	return true;					// Continue the recursion
}
bool UsedLocsVisitor::visit(PhiAssign* s, bool& override) {
	Exp* lhs = s->getLeft();
	// Special logic for the LHS
	if (lhs->isMemOf()) {
		Exp* child = ((Location*)lhs)->getSubExp1();
		UsedLocsFinder* ulf = (UsedLocsFinder*)ev;
		bool wasMemOnly = ulf->isMemOnly();
		ulf->setMemOnly(false);
		child->accept(ev);
		ulf->setMemOnly(wasMemOnly);
	} else if (lhs->getOper() == opArrayIndex || lhs->getOper() == opMemberAccess) {
		Exp* subExp1 = ((Binary*)lhs)->getSubExp1();
		subExp1->accept(ev);
		Exp* subExp2 = ((Binary*)lhs)->getSubExp2();
		subExp2->accept(ev);
	}
	PhiAssign::iterator uu;
	for (uu = s->begin(); uu != s->end(); uu++) {
		// Note: don't make the RefExp based on lhs, since it is possible that the lhs was renamed in fromSSA()
		// Use the actual expression in the PhiAssign
		// Also note that it's possible for uu->e to be NULL. Suppose variable a can be assigned to along in-edges
		// 0, 1, and 3; inserting the phi parameter at index 3 will cause a null entry at 2
		if (uu->e) {
			RefExp* temp = new RefExp(uu->e, uu->def);
			temp->accept(ev);
		}
	}

	override = true;				// Don't do the usual accept logic
	return true;					// Continue the recursion
}
bool UsedLocsVisitor::visit(ImplicitAssign* s, bool& override) {
	Exp* lhs = s->getLeft();
	// Special logic for the LHS
	if (lhs->isMemOf()) {
		Exp* child = ((Location*)lhs)->getSubExp1();
		UsedLocsFinder* ulf = (UsedLocsFinder*)ev;
		bool wasMemOnly = ulf->isMemOnly();
		ulf->setMemOnly(false);
		child->accept(ev);
		ulf->setMemOnly(wasMemOnly);
	} else if (lhs->getOper() == opArrayIndex || lhs->getOper() == opMemberAccess) {
		Exp* subExp1 = ((Binary*)lhs)->getSubExp1();
		subExp1->accept(ev);
		Exp* subExp2 = ((Binary*)lhs)->getSubExp2();
		subExp2->accept(ev);
	}
	override = true;				// Don't do the usual accept logic
	return true;					// Continue the recursion
}

bool UsedLocsVisitor::visit(CallStatement* s, bool& override) {
	Exp* pDest = s->getDest();
	if (pDest)
		pDest->accept(ev);
	StatementList::iterator it;
	StatementList& arguments = s->getArguments();
	for (it = arguments.begin(); it != arguments.end(); it++) {
		// Don't want to ever collect anything from the lhs
		((Assign*)*it)->getRight()->accept(ev);
	}
	if (countCol) {
		DefCollector::iterator dd;
		DefCollector* col = s->getDefCollector();
		for (dd = col->begin(); dd != col->end(); ++dd)
			(*dd)->accept(this);
	}
	override = true;			// Don't do the normal accept logic
	return true;				// Continue the recursion
}

bool UsedLocsVisitor::visit(ReturnStatement* s, bool& override) {
	// For the final pass, only consider the first return
	ReturnStatement::iterator rr;
	for (rr = s->begin(); rr != s->end(); ++rr)
		(*rr)->accept(this);
	// Also consider the reaching definitions to be uses, so when they are the only non-empty component of this
	// ReturnStatement, they can get propagated to.
	if (countCol) { 					// But we need to ignore these "uses" unless propagating
		DefCollector::iterator dd;
		DefCollector* col = s->getCollector();
		for (dd = col->begin(); dd != col->end(); ++dd)
			(*dd)->accept(this);
	}

	// Insert a phantom use of "everything" here, so that we can find out if any childless calls define something that
	// may end up being returned
	// FIXME: Not here! Causes locals to never get removed. Find out where this belongs, if anywhere:
	//((UsedLocsFinder*)ev)->getLocSet()->insert(new Terminal(opDefineAll));

	override = true;			// Don't do the normal accept logic
	return true;				// Continue the recursion
}

bool UsedLocsVisitor::visit(BoolAssign* s, bool& override) {
	Exp* pCond = s->getCondExpr();
	if (pCond)
		pCond->accept(ev);				// Condition is used
	Exp* lhs = s->getLeft();
	if (lhs && lhs->isMemOf()) {	// If dest is of form m[x]...
		Exp* x = ((Location*)lhs)->getSubExp1();
		UsedLocsFinder* ulf = (UsedLocsFinder*)ev;
		bool wasMemOnly = ulf->isMemOnly();
		ulf->setMemOnly(false);
		x->accept(ev);
		ulf->setMemOnly(wasMemOnly);
	} else if (lhs->getOper() == opArrayIndex || lhs->getOper() == opMemberAccess) {
		Exp* subExp1 = ((Binary*)lhs)->getSubExp1();
		subExp1->accept(ev);
		Exp* subExp2 = ((Binary*)lhs)->getSubExp2();
		subExp2->accept(ev);
	}
	override = true;			// Don't do the normal accept logic
	return true;				// Continue the recursion
}

//
// Expression subscripter
//
Exp* ExpSubscripter::preVisit(Location* e, bool& recur) {
	if (*e == *search) {
		recur = e->isMemOf();			// Don't double subscript unless m[...]
		return new RefExp(e, def);		// Was replaced by postVisit below
	}
	recur = true;
	return e;
}

Exp* ExpSubscripter::preVisit(Binary* e, bool& recur) {
	// array[index] is like m[addrexp]: requires a subscript
	if (e->isArrayIndex() && *e == *search) {
		recur = true;					// Check the index expression
		return new RefExp(e, def);		// Was replaced by postVisit below
	}
	recur = true;
	return e;
}

Exp* ExpSubscripter::preVisit(Terminal* e) {
	if (*e == *search)
		return new RefExp(e, def);
	return e;
}

Exp* ExpSubscripter::preVisit(RefExp* e, bool& recur) {
	recur = false;			// Don't look inside... not sure about this
	return e;
}

// The Statement subscripter class
void StmtSubscripter::visit(Assign* s, bool& recur) {
	Exp* rhs = s->getRight();
	s->setRight(rhs->accept(mod));
	// Don't subscript the LHS of an assign, ever
	Exp* lhs = s->getLeft();
	if (lhs->isMemOf() || lhs->isRegOf()) {
        ((Location*)lhs)->setSubExp1(((Location*)lhs)->getSubExp1()->accept(mod));
	}
	recur = false;
}
void StmtSubscripter::visit(PhiAssign* s, bool& recur) {
	Exp* lhs = s->getLeft();
	if (lhs->isMemOf()) {
        ((Location*)lhs)->setSubExp1(((Location*)lhs)->getSubExp1()->accept(mod));
	}
	recur = false;
}
void StmtSubscripter::visit(ImplicitAssign* s, bool& recur) {
	Exp* lhs = s->getLeft();
	if (lhs->isMemOf()) {
        ((Location*)lhs)->setSubExp1(((Location*)lhs)->getSubExp1()->accept(mod));
	}
	recur = false;
}
void StmtSubscripter::visit(BoolAssign* s, bool& recur) {
	Exp* lhs = s->getLeft();
	if (lhs->isMemOf()) {
        ((Location*)lhs)->setSubExp1(((Location*)lhs)->getSubExp1()->accept(mod));
	}
	Exp* rhs = s->getCondExpr();
	s->setCondExpr(rhs->accept(mod));
	recur = false;
}

void StmtSubscripter::visit(CallStatement* s, bool& recur) {
	Exp* pDest = s->getDest();
	if (pDest)
		s->setDest(pDest->accept(mod));
	// Subscript the ordinary arguments
	StatementList& arguments = s->getArguments();
	StatementList::iterator ss;
	for (ss = arguments.begin(); ss != arguments.end(); ++ss)
		(*ss)->accept(this);
	// Returns are like the LHS of an assignment; don't subscript them directly (only if m[x], and then only subscript
	// the x's)
	recur = false;			// Don't do the usual accept logic
}


// Size stripper
Exp* SizeStripper::preVisit(Binary* b, bool& recur) {
	recur = true;			// Visit the binary's children
	if (b->isSizeCast())
		// Could be a size cast of a size cast
		return b->getSubExp2()->stripSizes();
	return b;
}

Exp* ExpConstCaster::preVisit(Const* c) {
	if (c->getConscript() == num) {
		changed = true;
		return new TypedExp(ty, c);
	}
	return c;
}


// This is the code (apart from definitions) to find all constants in a Statement
bool ConstFinder::visit(Const* e) {
	lc.push_back(e);
	return true;
}
bool ConstFinder::visit(Location* e, bool& override) {
	if (e->isMemOf())
		override = false;		// We DO want to see constants in memofs
	else
		override = true;		// Don't consider register numbers, global names, etc
	return true;			
}

// This is in the POST visit function, because it's important to process any child expressions first.
// Otherwise, for m[r28{0} - 12]{0}, you could be adding an implicit assignment with a NULL definition for r28.
Exp* ImplicitConverter::postVisit(RefExp* e) {
	if (e->getDef() == NULL)
		e->setDef(cfg->findImplicitAssign(e->getSubExp1()));
	return e;
}

void StmtImplicitConverter::visit(PhiAssign* s, bool& recur) {
	// The LHS could be a m[x] where x has a null subscript; must do first
	s->setLeft(s->getLeft()->accept(mod));
	PhiAssign::iterator uu;
	for (uu = s->begin(); uu != s->end(); uu++) {
		if (uu->e == NULL) continue;
		if (uu->def == NULL)
			uu->def = cfg->findImplicitAssign(uu->e);
	}
	recur = false;		// Already done LHS
}

// Localiser. Subscript a location with the definitions that reach the call, or with {-} if none
Exp* Localiser::preVisit(RefExp* e, bool& recur) {
	recur = false;				// Don't recurse into already subscripted variables
	mask <<= 1;
	return e;
}

Exp* Localiser::preVisit(Location* e, bool& recur) {
	recur = true;
	int d = e->getMemDepth();
	if (d <= depth)				// Don't recurse if depth already too low, or equal
		recur = false;
	mask <<= 1;
	return e;
}

Exp* Localiser::postVisit(Location* e) {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	int d = ret->getMemDepth();
	if (d != depth && depth != -1) return e;	// Only subscript at the requested depth, or any if depth == -1
	Exp* r = call->findDefFor(ret);
	if (r) {
		ret = r->clone();
		if (EXPERIMENTAL) {
			// The trouble with the below is that you can propagate to say a call statement's argument expression and
			// not to the assignment of the actual argument. Examples: test/pentium/fromssa2, fbranch
			bool ch;
			ret = ret->propagateAllRpt(ch);		// Propagate into this repeatedly, in case propagation is limited
		}
		ret = ret->bypass();
		unchanged &= ~mask;
		mod = true;
	} else
		ret = new RefExp(ret, NULL);				// No definition reaches, so subscript with {-}
	return ret;
}

// Want to be able to localise a few terminals, in particular <all>
Exp* Localiser::postVisit(Terminal* e) {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	if (depth >= 1) return ret;
	Exp* r = call->findDefFor(ret);
	if (r) {
		ret = r->clone()->bypass();
		unchanged &= ~mask;
		mod = true;
	} else
		ret = new RefExp(ret, NULL);				// No definition reaches, so subscript with {-}
	return ret;
}

bool ComplexityFinder::visit(Location* e,		bool& override) {
	if (proc && proc->lookupSym(e) != NULL) {
		// This is mapped to a local. Count it as zero, not about 3 (m[r28+4] -> memof, regof, plus)
		override = true;
		return true;
	}
	if (e->isMemOf() || e->isArrayIndex())
		count++;				// Count the more complex unaries
	override = false;
	return true;
}
bool ComplexityFinder::visit(Unary* e,		bool& override) {count++; override = false; return true;}
bool ComplexityFinder::visit(Binary* e,		bool& override) {count++; override = false; return true;}
bool ComplexityFinder::visit(Ternary* e,	bool& override) {count++; override = false; return true;}

// Ugh! This is still a separate propagation mechanism from Statement::propagateTo().
Exp* ExpPropagator::postVisit(RefExp* e) {
	// No need to call e->canRename() here, because if e's base expression is not suitable for renaming, it will never
	// have been renamed, and we never would get here
	if (!Statement::canPropagateToExp(e))		// Check of the definition statement is suitable for propagating
		return e;
	Statement* def = e->getDef();
	Exp* res = e;
	if (def && def->isAssign()) {
		Exp* lhs = ((Assign*)def)->getLeft();
		Exp* rhs = ((Assign*)def)->getRight();
		bool ch;
		res = e->searchReplaceAll(new RefExp(lhs, def), rhs->clone(), ch);
		if (ch) {
			change = true;						// Record this change
			unchanged &= ~mask;					// Been changed now (so simplify parent)
			if (res->isSubscript())
				res = postVisit((RefExp*)res);	// Recursively propagate more if possible
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
bool PrimitiveTester::visit(Location* e, bool& override) {
	// We reached a bare (unsubscripted) location. This is certainly not primitive
	override = true;
	result = false;
	return false;			// No need to continue searching
}

bool PrimitiveTester::visit(RefExp* e, bool& override) {
	Statement* def = e->getDef();
	// If defined by a call, e had better not be a memory location (crude approximation for now)
	if (def == NULL || def->getNumber() == 0 || def->isCall() && !e->getSubExp1()->isMemOf()) {
		// Implicit definitions are always primitive
		// The results of calls are always primitive
		override = true;	// Don't recurse into the reference
		return true;		// Result remains true
	}

	// For now, all references to other definitions will be considered non primitive. I think I'll have to extend this!
	result = false;
	override = true;		// Regareless of outcome, don't recurse into the reference
	return true;
}

bool ExpHasMemofTester::visit(Location *e, bool& override) {
	if (e->isMemOf()) {
		override = true;	// Don't recurse children (not needed surely)
		result = true;		// Found a memof
		return false;		// Don't continue searching the expression
	}
	override = false;
	return true;
}

bool TempToLocalMapper::visit(Location *e, bool& override) {
	if (e->isTemp()) {
		// We have a temp subexpression; get its name
		char* tempName = ((Const*)e->getSubExp1())->getStr();
		Type* ty = Type::getTempType(tempName);		// Types for temps strictly depend on the name
		// This call will do the mapping from the temp to a new local:
		proc->getSymbolExp(e, ty, true);
	}
	override = true;		// No need to examine the string
	return true;
}

// The idea of this mapper is to simply map the first occurrence of a register to a local with that name.
// If there are conflicting types or overlapping live ranges, other locals will be generated by the fromSSA algorithm.
// For example, r24{20} -> eax, r24{22} -> local4 // eax{22}
ExpRegMapper::ExpRegMapper(UserProc* p) : proc(p), lastType(NULL) {
	prog = proc->getProg();
}

bool ExpRegMapper::visit(Location *e, bool& override) {
	if (e->isRegOf()) {
		// We have a regiter definition (else would be picked up in the RefExp version).
		// Check to see if it is in the symbol map yet
		if (proc->lookupSym(e) == NULL) {
			// No, get its name from the front end
			int regNum = ((Const*)e->getSubExp1())->getInt();
			char* regName = (char*) prog->getRegName(regNum);
			if (regName && regName[0] == '%')
				regName++;			// Skip the %
			// Create a new local; use the type saved from the LHS of the last assignment
			proc->addLocal(lastType, regName, e);
		}
	}
	override = true;		// No need to examine the string
	return true;
}

bool ExpRegMapper::visit(RefExp *e, bool& override) {
	Exp* base = e->getSubExp1();
	if (base->isRegOf()) {
		// We have a regiter use.
		// Check to see if it is in the symbol map yet
		if (proc->lookupSym(base) == NULL) {
			// No, get its name from the front end
			int regNum = ((Const*)((Unary*)base->getSubExp1()))->getInt();
			char* regName = (char*) prog->getRegName(regNum);
			if (regName && regName[0] == '%')
				regName++;			// Skip the %
			Statement* def = e->getDef();
			Type* ty = def->getTypeFor(base);
			proc->addLocal(ty, regName, base);		 // Create a new local
		}
	}
	override = true;		// Don't examine the r[] inside
	return true;
}

bool StmtRegMapper::common(Assignment *stmt, bool& override) {
	Exp* lhs = stmt->getLeft();
	// In case there is a m[reg] such that reg is otherwise unused
	if (lhs->isMemOf()) {
		Exp* base = ((Location*)lhs)->getSubExp1();
		base->accept((ExpRegMapper*)ev);
	}
	// Remember the type of this assignment, for the expression visitor
	((ExpRegMapper*)ev)->setLastType(stmt->getType());
	override = false;
	return true;
}

bool StmtRegMapper::visit(		  Assign* stmt, bool& override) {return common(stmt, override);}
bool StmtRegMapper::visit(	   PhiAssign* stmt, bool& override) {return common(stmt, override);}
bool StmtRegMapper::visit(ImplicitAssign* stmt, bool& override) {return common(stmt, override);}
bool StmtRegMapper::visit(	  BoolAssign* stmt, bool& override) {return common(stmt, override);}

// Constant global converter. Example: m[m[r24{16} + m[0x8048d60]{-}]{-}]{-} -> m[m[r24{16} + 32]{-}]{-}
// Allows some complex variations to be matched to standard indirect call forms
Exp* ConstGlobalConverter::preVisit(RefExp* e, bool& recur) {
	Statement* def = e->getDef();
	Exp *base, *addr, *idx, *glo;
	if (def == NULL || def->isImplicit()) {
		if (	(base = e->getSubExp1(), base->isMemOf()) &&
				(addr = ((Location*)base)->getSubExp1(), addr->isIntConst())) {
			// We have a m[K]{-}
			int K = ((Const*)addr)->getInt();
			int value = prog->readNative4(K);
			recur = false;
			return new Const(value);
		} else if (base->isGlobal()) {
			// We have a glo{-}
			char* gname = ((Const*)(base->getSubExp1()))->getStr();
			ADDRESS gloValue = prog->getGlobalAddr(gname);
			int value = prog->readNative4(gloValue);
			recur = false;
			return new Const(value);
		} else if (base->isArrayIndex() &&
					(idx = ((Binary*)base)->getSubExp2(), idx->isIntConst()) &&
					(glo = ((Binary*)base)->getSubExp1(), glo->isGlobal())) {
			// We have a glo[K]{-}
			int K = ((Const*)idx)->getInt();
			char* gname = ((Const*)(glo->getSubExp1()))->getStr();
			ADDRESS gloValue = prog->getGlobalAddr(gname);
			Type* gloType = prog->getGlobal(gname)->getType();
			assert(gloType->isArray());
			Type* componentType = gloType->asArray()->getBaseType();
			int value = prog->readNative4(gloValue + K * (componentType->getSize() / 8));
			recur = false;
			return new Const(value);
		}
	}
	recur = true;
	return e;
}

bool ExpDestCounter::visit(RefExp *e, bool& override) {
	if (Statement::canPropagateToExp(e))
		destCounts[e]++;
	override = false;		// Continue searching my children
	return true;			// Continue visiting the rest of Exp* e
}

bool FlagsFinder::visit(Binary *e,	bool& override) {
	if (e->isFlagCall()) {
		found = true;
		return false;		// Don't continue searching
	}
	override = false;
	return true;
}

// Search for bare memofs (not subscripted) in the expression
bool BareMemofFinder::visit(Location* e, bool& override) {
	if (e->isMemOf()) {
		found = true;
		return false;
	}
	override = false;
	return true;			// Continue searching
}

bool BareMemofFinder::visit(RefExp* e, bool& override) {
	Exp* base = e->getSubExp1();
	if (base->isMemOf()) {
		// Beware: it may be possible to have a bare memof inside a subscripted one
		Exp* addr = ((Location*)base)->getSubExp1();
		addr->accept(this);
		if (found)
			return false;	// Don't continue searching
	}
	override = true;		// Don't look inside the refexp
	return true;			// But keep searching
}

Exp* ExpCastInserter::postVisit(Binary *e) {
	OPER op = e->getOper();
	switch (op) {
		case opLess:
		case opGtr:
		case opLessEq:
		case opGtrEq:
		case opShiftRA: {
			Type* tl = e->getSubExp1()->ascendType();
			if (!tl->isInteger() || !tl->asInteger()->isSigned()) {
				e->setSubExp1(new TypedExp(new IntegerType(tl->getSize(), 1), e->getSubExp1()));
			}
			if (op != opShiftRA) {
				Type* tr = e->getSubExp2()->ascendType();
				if (!tr->isInteger() || !tr->asInteger()->isSigned()) {
					e->setSubExp2(new TypedExp(new IntegerType(tr->getSize(), 1), e->getSubExp2()));
				}
			}
			break;
		}
		case opLessUns:
		case opGtrUns:
		case opLessEqUns:
		case opGtrEqUns:
		case opShiftR: {
			Type* tl = e->getSubExp1()->ascendType();
			if (!tl->isInteger() || !tl->asInteger()->isUnsigned()) {
				e->setSubExp1(new TypedExp(new IntegerType(tl->getSize(), -1), e->getSubExp1()));
			}
			if (op != opShiftR) {
				Type* tr = e->getSubExp2()->ascendType();
				if (!tr->isInteger() || !tr->asInteger()->isUnsigned()) {
					e->setSubExp2(new TypedExp(new IntegerType(tr->getSize(), -1), e->getSubExp2()));
				}
			}
			break;
		}
		default:
			break;
	}
	return e;
}

Exp* ExpCastInserter::postVisit(Const *e) {
	if (e->isIntConst()) {
		bool naturallySigned = e->getInt() < 0;
		Type* ty = e->getType();
		if (naturallySigned && ty->isInteger() && !ty->asInteger()->isSigned()) {
			return new TypedExp(new IntegerType(ty->asInteger()->getSize(), -1), e);
		}
	}
	return e;
}
