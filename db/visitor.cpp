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
	std::vector<Exp*>& args = stmt->getArguments();
	int i, n = args.size();
	for (i=0; i < n; i++)
		args[i]->accept(&sc);
	std::vector<Exp*>& impargs = stmt->getImplicitArguments();
	n = impargs.size();
	for (i=0; i < n; i++)
		impargs[i]->accept(&sc);
	n = stmt->getNumReturns();
	for (i=0; i < n; i++) {
		Exp* r = stmt->getReturnExp(i);
		r->accept(&sc);
	}
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
	int n = stmt->getNumReturns();
	for (int i=0; i < n; i++) {
		Exp* r = stmt->getReturnExp(i);
		r->accept(&sc);
	}
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

void PhiStripper::visit(PhiAssign* s, bool& recur) {
	del = true;
	recur = true;
}

Exp* RefStripper::preVisit(RefExp* e, bool& recur) {
	recur = false;
	return e->getSubExp1();		// Do the actual stripping of references!
}

Exp* CallRefsFixer::postVisit(RefExp* r) {
	Exp* ret = r;
	// If child was modified, simplify now
	if (!(unchanged & ~mask)) ret = r->simplify();
	mask >>= 1;
	// Note: r will always == ret here, so the below is safe
	Statement* def = r->getRef();
	CallStatement *call = dynamic_cast<CallStatement*>(def);
	if (call) {
		Exp *e = call->getProven(r->getSubExp1());
		if (e) {
			e = call->substituteParams(e->clone());
			assert(e);
			if (VERBOSE)
				LOG << "fixcall refs replacing " << r << " with " << e
					<< "\n";
			// e = e->simplify();	// No: simplify the parent
			unchanged &= ~mask;
			mod = true;
			return e;
		} else {
			Exp* subExp1 = r->getSubExp1();
			if (call->findReturn(subExp1) == -1) {
				if (VERBOSE && !subExp1->isPC()) {
					LOG << "nothing proven about " << subExp1 <<
						" and yet it is referenced by " << r <<
						", and not in returns of " << "\n" <<
						"	" << call << "\n";
				}
			}
		}
	}
	return ret;
}

#if 0
Exp* CallRefsFixer::postVisit(PhiExp* p) {
	Exp* ret = p;
	// If child was modified, simplify now
	if (!(unchanged & mask)) ret = p->simplify();
	mask >>= 1;

	std::vector<Statement*> remove;
	std::vector<Statement*> insert;
	unsigned n = p->getNumRefs();

	bool oneIsGlobalFunc = false;
	Prog *prog = NULL;
	unsigned int i;
	for (i=0; i < n; i++) {
		Statement* u = p->getAt(i);
		if (u) {
			CallStatement *call = dynamic_cast<CallStatement*>(u);
			if (call)
				prog = call->getProc()->getProg();
		}
	}
	if (prog)
		oneIsGlobalFunc = p->hasGlobalFuncParam(prog);

	for (i=0; i < n; i++) {
		Statement* u = p->getAt(i);
		CallStatement *call = dynamic_cast<CallStatement*>(u);
		if (call) {
			Exp* subExp1 = p->getSubExp1();
			Exp *e = call->getProven(subExp1);
			if (call->isComputed() && oneIsGlobalFunc) {
				e = subExp1->clone();
				if (VERBOSE)
					LOG << "ignoring ref in phi to computed call with function pointer param " << e << "\n";
			}
			if (e) {
				e = call->substituteParams(e->clone());
				if (e && e->getOper() == opSubscript &&
					*e->getSubExp1() == *subExp1) {
					if (VERBOSE)
						LOG << "fixcall refs replacing param " << i << " in "
							<< p << " with " << e << "\n";
					p->putAt(i, ((RefExp*)e)->getRef());
					mod = true;
				} else {
					if (VERBOSE)
						LOG << "cant update phi ref to " << e << "\n";
				}
			} else {
				if (call->findReturn(subExp1) == -1) {
					if (VERBOSE) {
						LOG << "nothing proven about " << subExp1 <<
							" and yet it is referenced by " << p <<
							", and not in returns of " << "\n" <<
							"	" << call << "\n";
					}
				}
			}
		}
	}
	return ret;
}
#endif

Exp* CallRefsFixer::postVisit(Unary *e)	   {
	bool isAddrOfMem = e->isAddrOf() && e->getSubExp1()->isMemOf();
	if (isAddrOfMem) return e;
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}
Exp* CallRefsFixer::postVisit(Binary *e)	{
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplifyArith()->simplify();
	mask >>= 1;
	return ret;
}
Exp* CallRefsFixer::postVisit(Ternary *e)	 {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}
Exp* CallRefsFixer::postVisit(TypedExp *e)	  {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}
Exp* CallRefsFixer::postVisit(FlagDef *e)	 {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}
Exp* CallRefsFixer::postVisit(Location *e)	  {
	Exp* ret = e;
	if (!(unchanged & mask)) ret = e->simplify();
	mask >>= 1;
	return ret;
}
Exp* CallRefsFixer::postVisit(Const *e)	   {
	mask >>= 1;
	return e;
}
Exp* CallRefsFixer::postVisit(TypeVal *e)	 {
	mask >>= 1;
	return e;
}
Exp* CallRefsFixer::postVisit(Terminal *e)	  {
	mask >>= 1;
	return e;
}

// Add used locations finder
bool UsedLocsFinder::visit(Location* e, bool& override) {
	used->insert(e);		// All locations visited are used
	if (e->isMemOf()) {
		// Example: m[r28{10} - 4]	we use r28{10}
		Exp* child = e->getSubExp1();
		child->accept(this);
	}
	override = false;
	return true;
}

bool UsedLocsFinder::visit(Terminal* e) {
	switch (e->getOper()) {
		case opPC:
		case opFlags:
		case opFflags:
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
	used->insert(e);		 // This location is used
	// However, e's subexpression is NOT used ...
	override = true;
	// ... unless that is a m[x], in which case x (not m[x]) is used
	Exp* refd = e->getSubExp1();
	if (refd->isMemOf()) {
		Exp* x = refd->getSubExp1();
		x->accept(this);
	}
	return true;
}

#if 0
bool UsedLocsFinder::visit(PhiExp* e, bool& override) {
	StatementVec& stmtVec = e->getRefs();
	Exp* subExp1 = e->getSubExp1();
	StatementVec::iterator uu;
	for (uu = stmtVec.begin(); uu != stmtVec.end(); uu++) {
		Exp* temp = new RefExp(subExp1, *uu);
		// Note: the below is not really correct; it is kept for compatibility
		// with the pre-visitor code.
		// A phi of m[blah] uses blah, but a phi should never be the only place
		// it is used, so that should be OK.
		// Should really be temp->accept(this);
		used->insert(temp);
	}
	override = true;
	return true;
}
#endif

bool UsedLocsVisitor::visit(Assign* s, bool& override) {
	Exp* lhs = s->getLeft();
	Exp* rhs = s->getRight();
	if (rhs) rhs->accept(ev);
	// Special logic for the LHS
	if (lhs->isMemOf()) {
		Exp* child = ((Location*)lhs)->getSubExp1();
		child->accept(ev);
	}
	override = true;				// Don't do the usual accept logic
	return true;					// Continue the recursion
}
bool UsedLocsVisitor::visit(PhiAssign* s, bool& override) {
	Exp* lhs = s->getLeft();
	// Special logic for the LHS
	if (lhs->isMemOf()) {
		Exp* child = ((Location*)lhs)->getSubExp1();
		child->accept(ev);
	}
	StatementVec& stmtVec = s->getRefs();
	StatementVec::iterator uu;
	for (uu = stmtVec.begin(); uu != stmtVec.end(); uu++) {
		Exp* temp = new RefExp(lhs, *uu);
		temp->accept(ev);
	}

	override = true;				// Don't do the usual accept logic
	return true;					// Continue the recursion
}
bool UsedLocsVisitor::visit(ImplicitAssign* s, bool& override) {
	Exp* lhs = s->getLeft();
	// Special logic for the LHS
	if (lhs->isMemOf()) {
		Exp* child = ((Location*)lhs)->getSubExp1();
		child->accept(ev);
	}
	override = true;				// Don't do the usual accept logic
	return true;					// Continue the recursion
}

bool UsedLocsVisitor::visit(CallStatement* s, bool& override) {
	Exp* pDest = s->getDest();
	if (pDest)
		pDest->accept(ev);
	std::vector<Exp*>::iterator it;
	std::vector<Exp*>& arguments = s->getArguments();
	for (it = arguments.begin(); it != arguments.end(); it++)
		(*it)->accept(ev);
	if (!final) {
		// Ignore the implicit arguments when final
		int n = s->getNumImplicitArguments();
		for (int i=0; i < n; i++)
			s->getImplicitArgumentExp(i)->accept(ev);
	}
	// For the final pass, also only consider the first return
	int n = s->getNumReturns();
	if (final) {
		if (n != 0) {
			Exp* r = s->getReturnExp(0);
			// If of form m[x] then x is used
			if (r->isMemOf()) {
				Exp* x = ((Location*)r)->getSubExp1();
				x->accept(ev);
			}
		}
	} else {
		// Otherwise, consider all returns. If of form m[x] then x is used
		for (int i=0; i < n; i++) {
			Exp* r = s->getReturnExp(i);
			if (r->isMemOf()) {
				Exp* x = ((Location*)r)->getSubExp1();
				x->accept(ev);
			}
		} 
	}
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
		x->accept(ev);					// ... then x is used
	}
	override = true;			// Don't do the normal accept logic
	return true;				// Continue the recursion
}

//
// Expression subscripter
//
Exp* ExpSubscripter::preVisit(Location* e, bool& recur) {
	if (*e == *search) {
		recur = e->isMemOf();	// Don't double subscript unless m[...]
		return new RefExp(e, def);
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
	Exp* base = e->getSubExp1();
	if (*base == *search) {
		recur = false;		// Don't recurse; would double subscript
		e->setDef(def);
		return e;
	}
	recur = true;
	return e;
}

// The Statement subscripter class
void StmtSubscripter::visit(Assign* s, bool& recur) {
	Exp* rhs = s->getRight();
	s->setRight(rhs->accept(mod));
	// Don't subscript the LHS of an assign, ever
	Exp* lhs = s->getLeft();
	if (lhs->isMemOf()) {
		Exp*& child = ((Location*)lhs)->refSubExp1();
		child = child->accept(mod);
	}
	recur = false;
}
void StmtSubscripter::visit(PhiAssign* s, bool& recur) {
	Exp* lhs = s->getLeft();
	if (lhs->isMemOf()) {
		Exp*& child = ((Location*)lhs)->refSubExp1();
		child = child->accept(mod);
	}
	recur = false;
}
void StmtSubscripter::visit(ImplicitAssign* s, bool& recur) {
	Exp* lhs = s->getLeft();
	if (lhs->isMemOf()) {
		Exp*& child = ((Location*)lhs)->refSubExp1();
		child = child->accept(mod);
	}
	recur = false;
}

void StmtSubscripter::visit(CallStatement* s, bool& recur) {
	Exp* pDest = s->getDest();
	if (pDest)
		s->setDest(pDest->accept(mod));
	// Subscript the ordinary arguments
	std::vector<Exp*>& arguments = s->getArguments();
	int n = arguments.size();
	for (int i=0; i < n; i++)
		arguments[i] = arguments[i]->accept(mod);
	// Subscript the implicit arguments
	std::vector<Exp*>& implicits = s->getImplicitArguments();
	n = implicits.size();
	for (int i=0; i < n; i++)
		implicits[i] = implicits[i]->accept(mod);
	// Returns are like the LHS of an assignment; don't subscript them
	// directly (only if m[x], and then only subscript the x's)
	n = s->getNumReturns();
	for (int i=0; i < n; i++) {
		Exp* r = s->getReturnExp(i);
		if (r->isMemOf()) {
			Exp*& x = ((Location*)r)->refSubExp1();
			x = x->accept(mod);
		}
	}
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

bool StmtConstCaster::visit(Assign *stmt) {
	Exp* e = stmt->getLeft();
	stmt->setLeft(e->accept(ecc));
	if (ecc->isChanged()) return false;
	e = stmt->getRight();
	stmt->setRight(e->accept(ecc));
	return !ecc->isChanged();
}
bool StmtConstCaster::visit(PhiAssign *stmt) {
	Exp* e = stmt->getLeft();
	stmt->setLeft(e->accept(ecc));
	return !ecc->isChanged();
}
bool StmtConstCaster::visit(ImplicitAssign *stmt) {
	Exp* e = stmt->getLeft();
	stmt->setLeft(e->accept(ecc));
	return !ecc->isChanged();
}
bool StmtConstCaster::visit(GotoStatement *stmt) {
	Exp* e = stmt->getDest();
	stmt->setDest(e->accept(ecc));
	return !ecc->isChanged();
}
bool StmtConstCaster::visit(BranchStatement *stmt) {
	Exp* e = stmt->getDest();
	stmt->setDest(e->accept(ecc));
	if (ecc->isChanged()) return false;
	e = stmt->getCondExpr();
	stmt->setCondExpr(e->accept(ecc));
	return !ecc->isChanged();
}
bool StmtConstCaster::visit(CaseStatement *stmt) {
	SWITCH_INFO* si = stmt->getSwitchInfo();
	if (si) {
		si->pSwitchVar = si->pSwitchVar->accept(ecc);
	}
	return !ecc->isChanged();
}
bool StmtConstCaster::visit(CallStatement *stmt) {
	std::vector<Exp*> args;
	args = stmt->getArguments();
	int i, n = args.size();
	for (i=0; i < n; i++) {
		args[i] = args[i]->accept(ecc);
		if (ecc->isChanged()) return true;
	}
	std::vector<Exp*>& impargs = stmt->getImplicitArguments();
	n = impargs.size();
	for (i=0; i < n; i++) {
		impargs[i] = impargs[i]->accept(ecc);
		if (ecc->isChanged()) return true;
	}
	std::vector<Exp*>& returns = stmt->getReturns();
	n = returns.size();
	for (i=0; i < n; i++) {
		returns[i] = returns[i]->accept(ecc);
		if (ecc->isChanged()) return true;
	}
	return true;
}
bool StmtConstCaster::visit(ReturnStatement *stmt) {
	std::vector<Exp*>& returns = stmt->getReturns();
	int i, n = returns.size();
	for (i=0; i < n; i++) {
		returns[i] = returns[i]->accept(ecc);
		if (ecc->isChanged()) return true;
	}
	return true;
}
bool StmtConstCaster::visit(BoolAssign *stmt) {
	Exp* e = stmt->getLeft();
	stmt->setLeft(e->accept(ecc));
	if (ecc->isChanged()) return false;
	e = stmt->getCondExpr();
	stmt->setCondExpr(e->accept(ecc));
	return !ecc->isChanged();
}

