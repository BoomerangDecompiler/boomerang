/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   statement.cpp
 * OVERVIEW:   Implementation of the Statement and related classes.
 *			   (Was dataflow.cpp)
 *============================================================================*/

/*
 * $Revision$	// 1.148.2.38
 * 03 Jul 02 - Trent: Created
 * 09 Jan 03 - Mike: Untabbed, reformatted
 * 03 Feb 03 - Mike: cached dataflow (uses and usedBy) (since reversed)
 * 25 Jul 03 - Mike: dataflow.cpp, hrtl.cpp -> statement.cpp
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#include <iomanip>			// For setfill etc

#include <sstream>
#include <algorithm>
#include "statement.h"
#include "exp.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "boomerang.h"
#include "rtl.h"			// For debugging code
#include "util.h"
#include "signature.h"
#include "visitor.h"
#include "dataflow.h"
#include "log.h"


extern char debug_buffer[];		 // For prints functions

#if defined(_MSC_VER) && _MSC_VER < 1310			// Ugh - MSC 7.0 doesn't have advance
#define my_advance(aa, n) \
	for (int zz = 0; zz < n; zz++) \
		aa++;
#else
#define my_advance(aa, n) \
	advance(aa, n);
#endif

void Statement::setProc(UserProc *p)
{
	proc = p;
	LocationSet exps;
	addUsedLocs(exps);
	LocationSet defs;
	getDefinitions(defs);
	exps.makeUnion(defs);
	LocationSet::iterator ll;
	for (ll = exps.begin(); ll != exps.end(); ll++) {
		Location *l = dynamic_cast<Location*>(*ll);
		if (l) {
			l->setProc(p);
		}
	}
}

Exp *Statement::getExpAtLex(unsigned int begin, unsigned int end)
{
	return NULL;
}

bool Statement::mayAlias(Exp *e1, Exp *e2, int size) { 
	if (*e1 == *e2) return true;
	// Pass the expressions both ways. Saves checking things like m[exp] vs m[exp+K] and m[exp+K] vs m[exp] explicitly
	// (only need to check one of these cases)
	bool b =  (calcMayAlias(e1, e2, size) && calcMayAlias(e2, e1, size)); 
	if (b && VERBOSE) {
		LOG << "May alias: " << e1 << " and " << e2 << " size " << size << "\n";
	}
	return b;
}

// returns true if e1 may alias e2
bool Statement::calcMayAlias(Exp *e1, Exp *e2, int size) {
	// currently only considers memory aliasing..
	if (!e1->isMemOf() || !e2->isMemOf()) {
		return false;
	}
	Exp *e1a = e1->getSubExp1();
	Exp *e2a = e2->getSubExp1();
	// constant memory accesses
	if (e1a->isIntConst() && e2a->isIntConst()) {
		ADDRESS a1 = ((Const*)e1a)->getAddr();
		ADDRESS a2 = ((Const*)e2a)->getAddr();
		int diff = a1 - a2;
		if (diff < 0) diff = -diff;
		if (diff*8 >= size) return false;
	}
	// same left op constant memory accesses
	if (e1a->getArity() == 2 && e1a->getOper() == e2a->getOper() && e1a->getSubExp2()->isIntConst() &&
			e2a->getSubExp2()->isIntConst() && *e1a->getSubExp1() == *e2a->getSubExp1()) {
		int i1 = ((Const*)e1a->getSubExp2())->getInt();
		int i2 = ((Const*)e2a->getSubExp2())->getInt();
		int diff = i1 - i2;
		if (diff < 0) diff = -diff;
		if (diff*8 >= size) return false;
	}
	// [left] vs [left +/- constant] memory accesses
	if ((e2a->getOper() == opPlus || e2a->getOper() == opMinus) && *e1a == *e2a->getSubExp1() &&
			e2a->getSubExp2()->isIntConst()) {
		int i1 = 0;
		int i2 = ((Const*)e2a->getSubExp2())->getInt();
		int diff = i1 - i2;
		if (diff < 0) diff = -diff;
		if (diff*8 >= size) return false;
	}
	// Don't need [left +/- constant ] vs [left] because called twice with
	// args reversed
	return true;
}

RangeMap Statement::getInputRanges()
{
	if (!isFirstStatementInBB()) {
		savedInputRanges = getPreviousStatementInBB()->getRanges();
		return savedInputRanges;
	}

	assert(pbb && pbb->getNumInEdges() <= 1);
	RangeMap input;
	if (pbb->getNumInEdges() == 0) {
		// setup input for start of procedure
		input.addRange(Location::regOf(24), Range(1, 0, 0, new Unary(opInitValueOf, Location::regOf(24))));
		input.addRange(Location::regOf(25), Range(1, 0, 0, new Unary(opInitValueOf, Location::regOf(25))));
		input.addRange(Location::regOf(26), Range(1, 0, 0, new Unary(opInitValueOf, Location::regOf(26))));
		input.addRange(Location::regOf(27), Range(1, 0, 0, new Unary(opInitValueOf, Location::regOf(27))));
		input.addRange(Location::regOf(28), Range(1, 0, 0, new Unary(opInitValueOf, Location::regOf(28))));
		input.addRange(Location::regOf(29), Range(1, 0, 0, new Unary(opInitValueOf, Location::regOf(29))));
		input.addRange(Location::regOf(30), Range(1, 0, 0, new Unary(opInitValueOf, Location::regOf(30))));
		input.addRange(Location::regOf(31), Range(1, 0, 0, new Unary(opInitValueOf, Location::regOf(31))));
		input.addRange(new Terminal(opPC), Range(1, 0, 0, new Unary(opInitValueOf, new Terminal(opPC))));
	} else {
		PBB pred = pbb->getInEdges()[0];
		Statement *last = pred->getLastStmt();
		assert(last);
		if (pred->getNumOutEdges() != 2) {
			input = last->getRanges();
		} else {
			assert(pred->getNumOutEdges() == 2);
			assert(last->isBranch());
			input = ((BranchStatement*)last)->getRangesForOutEdgeTo(pbb);
		}
	}

	savedInputRanges = input;

	return input;
}

void Statement::updateRanges(RangeMap &output, std::list<Statement*> &execution_paths, bool notTaken)
{
	if (!output.isSubset(notTaken ? ((BranchStatement*)this)->getRanges2Ref() : ranges)) {
		if (notTaken)
			((BranchStatement*)this)->setRanges2(output);
		else
			ranges = output;
		if (isLastStatementInBB()) {
			if (pbb->getNumOutEdges()) {
				int arc = 0;
				if (isBranch()) {
					if (pbb->getOutEdge(0)->getLowAddr() != ((BranchStatement*)this)->getFixedDest())
						arc = 1;
					if (notTaken)
						arc ^= 1;
				}
				execution_paths.push_back(pbb->getOutEdge(arc)->getFirstStmt());
			}
		} else
			execution_paths.push_back(getNextStatementInBB());
	}
}

void Statement::rangeAnalysis(std::list<Statement*> &execution_paths)
{
	RangeMap output = getInputRanges();
	updateRanges(output, execution_paths);
}

void Assign::rangeAnalysis(std::list<Statement*> &execution_paths)
{
	RangeMap output = getInputRanges();
	Exp *a_lhs = lhs->clone();
	if (a_lhs->isFlags()) {
		// special hacks for flags
		assert(rhs->isFlagCall());
		Exp *a_rhs = rhs->clone();
		if (a_rhs->getSubExp2()->getSubExp1()->isMemOf())
			a_rhs->getSubExp2()->getSubExp1()->setSubExp1(
				output.substInto(a_rhs->getSubExp2()->getSubExp1()->getSubExp1()));
		if (!a_rhs->getSubExp2()->getSubExp2()->isTerminal() &&
			a_rhs->getSubExp2()->getSubExp2()->getSubExp1()->isMemOf())
			a_rhs->getSubExp2()->getSubExp2()->getSubExp1()->setSubExp1(
				output.substInto(a_rhs->getSubExp2()->getSubExp2()->getSubExp1()->getSubExp1()));
		output.addRange(a_lhs, Range(1, 0, 0, a_rhs));		
	} else {
		if (a_lhs->isMemOf())
			a_lhs->setSubExp1(output.substInto(a_lhs->getSubExp1()->clone()));
		Exp *a_rhs = output.substInto(rhs->clone());
		if (a_rhs->isMemOf() && a_rhs->getSubExp1()->getOper() == opInitValueOf &&
			a_rhs->getSubExp1()->getSubExp1()->isRegOfK() &&
			((Const*)a_rhs->getSubExp1()->getSubExp1()->getSubExp1())->getInt() == 28)
			a_rhs = new Unary(opInitValueOf, new Terminal(opPC));   // nice hack
		if (VERBOSE)
			LOG << "a_rhs is " << a_rhs << "\n";
		if (a_rhs->isMemOf() && a_rhs->getSubExp1()->isIntConst()) {
			ADDRESS c = ((Const*)a_rhs->getSubExp1())->getInt();
			if (proc->getProg()->isDynamicLinkedProcPointer(c)) {
				char *nam = (char*)proc->getProg()->GetDynamicProcName(c);
				if (nam) {
					a_rhs = new Const(nam);
					LOG << "a_rhs is a dynamic proc pointer to " << nam << "\n";
				}
			} else if (proc->getProg()->isReadOnly(c)) {
				switch(type->getSize()) {
					case 8:
						a_rhs = new Const(proc->getProg()->readNative1(c));
						break;
					case 16:
						a_rhs = new Const(proc->getProg()->readNative2(c));
						break;
					case 32:
						a_rhs = new Const(proc->getProg()->readNative4(c));
						break;
					default:
						LOG << "error: unhandled type size " << type->getSize() << " for reading native address\n";
				}
			} else
				if (VERBOSE)
					LOG << c << " is not dynamically linked proc pointer or in read only memory\n";
		}
		if ((a_rhs->getOper() == opPlus || a_rhs->getOper() == opMinus) && 
			a_rhs->getSubExp2()->isIntConst() && output.hasRange(a_rhs->getSubExp1())) {
			Range &r = output.getRange(a_rhs->getSubExp1());
			int c = ((Const*)a_rhs->getSubExp2())->getInt();
			if (a_rhs->getOper() == opPlus) {
				output.addRange(a_lhs, Range(1, r.getLowerBound() != NEGINFINITY ? r.getLowerBound() + c : NEGINFINITY, r.getUpperBound() != INFINITY ? r.getUpperBound() + c : INFINITY, r.getBase()));
			} else {
				output.addRange(a_lhs, Range(1, r.getLowerBound() != NEGINFINITY ? r.getLowerBound() - c : NEGINFINITY, r.getUpperBound() != INFINITY ? r.getUpperBound() - c : INFINITY, r.getBase()));
			}
		} else {
			if (output.hasRange(a_rhs)) {
				output.addRange(a_lhs, output.getRange(a_rhs));
			} else {
				Exp *result;
				if (a_rhs->getMemDepth() == 0 && !a_rhs->search(new Unary(opRegOf, new Terminal(opWild)), result) &&
					!a_rhs->search(new Unary(opTemp, new Terminal(opWild)), result)) {
					if (a_rhs->isIntConst())
						output.addRange(a_lhs, Range(1, ((Const*)a_rhs)->getInt(), ((Const*)a_rhs)->getInt(), new Const(0)));
					else
						output.addRange(a_lhs, Range(1, 0, 0, a_rhs));
				} else
					output.addRange(a_lhs, Range());
			}
		}
	}
	if (VERBOSE)
		LOG << "added " << a_lhs << " -> " << output.getRange(a_lhs) << "\n";
	updateRanges(output, execution_paths);
	if (VERBOSE)
		LOG << this << "\n";
}

void BranchStatement::limitOutputWithCondition(RangeMap &output, Exp *e)
{
	assert(e);
	if (output.hasRange(e->getSubExp1())) {
		Range &r = output.getRange(e->getSubExp1());
		if (e->getSubExp2()->isIntConst() && r.getBase()->isIntConst() && ((Const*)r.getBase())->getInt() == 0) {
			int c = ((Const*)e->getSubExp2())->getInt();
			switch(e->getOper()) {
				case opLess:
				case opLessUns:
					output.addRange(e->getSubExp1(), Range(r.getStride(), r.getLowerBound() >= c ? c - 1 : r.getLowerBound(), r.getUpperBound() >= c ? c - 1 : r.getUpperBound(), r.getBase()));
					break;
				case opLessEq:
				case opLessEqUns:
					output.addRange(e->getSubExp1(), Range(r.getStride(), r.getLowerBound() > c ? c : r.getLowerBound(), r.getUpperBound() > c ? c : r.getUpperBound(), r.getBase()));
					break;
				case opGtr:
				case opGtrUns:
					output.addRange(e->getSubExp1(), Range(r.getStride(), r.getLowerBound() <= c ? c + 1 : r.getLowerBound(), r.getUpperBound() <= c ? c + 1 : r.getUpperBound(), r.getBase()));
					break;
				case opGtrEq:
				case opGtrEqUns:
					output.addRange(e->getSubExp1(), Range(r.getStride(), r.getLowerBound() < c ? c : r.getLowerBound(), r.getUpperBound() < c ? c : r.getUpperBound(), r.getBase()));
					break;
				case opEquals:
					output.addRange(e->getSubExp1(), Range(r.getStride(), c, c, r.getBase()));
					break;
				case opNotEqual:
					output.addRange(e->getSubExp1(), Range(r.getStride(), r.getLowerBound() == c ? c + 1 : r.getLowerBound(), r.getUpperBound() == c ? c - 1 : r.getUpperBound(), r.getBase()));
					break;
			}
		}
	}
}

void BranchStatement::rangeAnalysis(std::list<Statement*> &execution_paths)
{
	RangeMap output = getInputRanges();

	Exp *e = NULL;
	// try to hack up a useful expression for this branch
	OPER op = pCond->getOper();
	if (op == opLess || op == opLessEq || op == opGtr || op == opGtrEq ||
		op == opLessUns || op == opLessEqUns || op == opGtrUns || op == opGtrEqUns ||
		op == opEquals || op == opNotEqual) {
		if (pCond->getSubExp1()->isFlags() && output.hasRange(pCond->getSubExp1())) {
			Range &r = output.getRange(pCond->getSubExp1());
			if (r.getBase()->isFlagCall() && 
				r.getBase()->getSubExp2()->getOper() == opList &&
				r.getBase()->getSubExp2()->getSubExp2()->getOper() == opList) {
				e = new Binary(op, r.getBase()->getSubExp2()->getSubExp1()->clone(), r.getBase()->getSubExp2()->getSubExp2()->getSubExp1()->clone());
				if (VERBOSE)
					LOG << "calculated condition " << e << "\n";
			}
		}
	}

	if (e)
		limitOutputWithCondition(output, e);
	updateRanges(output, execution_paths);
	output = getInputRanges();
	if (e)
		limitOutputWithCondition(output, (new Unary(opNot, e))->simplify());
	updateRanges(output, execution_paths, true);

	if (VERBOSE)
		LOG << this << "\n";
}

void JunctionStatement::rangeAnalysis(std::list<Statement*> &execution_paths)
{
	RangeMap input;
	LOG << "unioning {\n";
	for (int i = 0; i < pbb->getNumInEdges(); i++) {
		Statement *last = pbb->getInEdges()[i]->getLastStmt();
		LOG << "  in BB: " << pbb->getInEdges()[i]->getLowAddr() << " " << last << "\n";
		if (last->isBranch()) {
			input.unionwith(((BranchStatement*)last)->getRangesForOutEdgeTo(pbb));
		} else {
			if (last->isCall()) {
				Proc *d = ((CallStatement*)last)->getDestProc();
				if (d && !d->isLib() && ((UserProc*)d)->getCFG()->findRetNode() == NULL) {
					LOG << "ignoring ranges from call to proc with no ret node\n";
				} else
					input.unionwith(last->getRanges());
			} else
				input.unionwith(last->getRanges());
		}
	}
	LOG << "}\n";

	if (!input.isSubset(ranges)) {
		RangeMap output = input;

		if (output.hasRange(Location::regOf(28))) {
			Range &r = output.getRange(Location::regOf(28));
			if (r.getLowerBound() != r.getUpperBound() && r.getLowerBound() != NEGINFINITY) {
				LOG << "stack height assumption violated " << r << " my bb: " << pbb->getLowAddr() << "\n";
				proc->printToLog();
				assert(false);
			}
		}

		if (isLoopJunction()) {
			output = ranges;
			output.widenwith(input);
		}

		updateRanges(output, execution_paths);
	}

	if (VERBOSE)
		LOG << this << "\n";
}

void CallStatement::rangeAnalysis(std::list<Statement*> &execution_paths)
{
	RangeMap output = getInputRanges();

	if (this->procDest == NULL) {
		// note this assumes the call is only to one proc.. could be bad.
		Exp *d = output.substInto(getDest()->clone());
		if (d->isIntConst() || d->isStrConst()) {
			if (d->isIntConst()) {
				ADDRESS dest = ((Const*)d)->getInt();
				procDest = proc->getProg()->setNewProc(dest);
			} else {
				procDest = proc->getProg()->getLibraryProc(((Const*)d)->getStr());
			}
			if (procDest) {
				Signature *sig = procDest->getSignature();
				pDest = d;
				arguments.clear();
				for (unsigned i = 0; i < sig->getNumParams(); i++) {
					Exp* a = sig->getParamExp(i);
					Assign* as = new Assign(new VoidType(), a->clone(), a->clone());
					as->setProc(proc);
					as->setBB(pbb);
					arguments.append(as);
				}
				signature = procDest->getSignature()->clone();
				m_isComputed = false;
				proc->undoComputedBB(this);
				proc->addCallee(procDest);
				LOG << "replaced indirect call with call to " << procDest->getName() << "\n";
			}
		}
	}

	if (output.hasRange(Location::regOf(28))) {
		Range &r = output.getRange(Location::regOf(28));
		int c = 4;
		if (procDest == NULL) {
			LOG << "using push count hack to guess number of params\n";
			Statement *prev = this->getPreviousStatementInBB();
			while(prev) {
				if (prev->isAssign() && ((Assign*)prev)->getLeft()->isMemOf() &&
					((Assign*)prev)->getLeft()->getSubExp1()->isRegOfK() &&
					((Const*)((Assign*)prev)->getLeft()->getSubExp1()->getSubExp1())->getInt() == 28 &&
					((Assign*)prev)->getRight()->getOper() != opPC) {
					c += 4;
				}
				prev = prev->getPreviousStatementInBB();
			}
		} else if (procDest->getSignature()->getConvention() == CONV_PASCAL)
			c += procDest->getSignature()->getNumParams() * 4;
		else if (!strncmp(procDest->getName(), "__imp_", 6)) {
			Statement *first = ((UserProc*)procDest)->getCFG()->getEntryBB()->getFirstStmt();
			assert(first && first->isCall());
			Proc *d = ((CallStatement*)first)->getDestProc();
			if (d->getSignature()->getConvention() == CONV_PASCAL)
				c += d->getSignature()->getNumParams() * 4;				
		} else if (!procDest->isLib()) {
			UserProc *p = (UserProc*)procDest;
			LOG << "== checking for number of bytes popped ==\n";
			p->printToLog();
			LOG << "== end it ==\n";
			PBB retbb = p->getCFG()->findRetNode();
			if (retbb) {
				Statement *last = retbb->getLastStmt();
				assert(last);
				if (last->isReturn()) {
					last->setBB(retbb);
					last = last->getPreviousStatementInBB();
				}
				if (last == NULL) {
					// call followed by a ret, sigh
					for (int i = 0; i < retbb->getNumInEdges(); i++) {
						last = retbb->getInEdges()[i]->getLastStmt();
						if (last->isCall())
							break;
					}
					if (last->isCall()) {
						Proc *d = ((CallStatement*)last)->getDestProc();
						if (d && d->getSignature()->getConvention() == CONV_PASCAL)
							c += d->getSignature()->getNumParams() * 4;
					}
					last = NULL;
				}
				if (last) {
					//LOG << "checking last statement " << last << " for number of bytes popped\n";
					assert(last->isAssign());
					Assign *a = (Assign*)last;
					assert(a->getLeft()->isRegOfK() && ((Const*)a->getLeft()->getSubExp1())->getInt() == 28);
					Exp *t = a->getRight()->clone()->simplifyArith();
					assert(t->getOper() == opPlus &&
						t->getSubExp1()->isRegOfK() &&
						((Const*)t->getSubExp1()->getSubExp1())->getInt() == 28);
					assert(t->getSubExp2()->isIntConst());
					c = ((Const*)t->getSubExp2())->getInt();
				}
			}
		}
		output.addRange(Location::regOf(28), Range(r.getStride(), r.getLowerBound() == NEGINFINITY ? NEGINFINITY : r.getLowerBound() + c, r.getUpperBound() == INFINITY ? INFINITY : r.getUpperBound() + c, r.getBase()));
	}
	updateRanges(output, execution_paths);
}

bool JunctionStatement::isLoopJunction()
{
	for (int i = 0; i < pbb->getNumInEdges(); i++) 
		if (pbb->isBackEdge(i))
			return true;
	return false;
}

RangeMap &BranchStatement::getRangesForOutEdgeTo(PBB out)
{
	assert(this->getFixedDest() != NO_ADDRESS);
	if (out->getLowAddr() == this->getFixedDest())
		return ranges;
	return ranges2;
}

bool Statement::isFirstStatementInBB()
{
	assert(pbb);
	assert(pbb->getRTLs());
	assert(pbb->getRTLs()->size());
	assert(pbb->getRTLs()->front());
	assert(pbb->getRTLs()->front()->getList().size());
	return this == pbb->getRTLs()->front()->getList().front();
}

bool Statement::isLastStatementInBB()
{
	assert(pbb);
	return this == pbb->getLastStmt();
}

Statement*	Statement::getPreviousStatementInBB()
{
	assert(pbb);
	std::list<RTL*> *rtls = pbb->getRTLs();
	assert(rtls);
	Statement *previous = NULL;
	for (std::list<RTL*>::iterator rit = rtls->begin(); rit != rtls->end(); rit++) {
		RTL *rtl = *rit;
		for (RTL::iterator it = rtl->getList().begin(); it != rtl->getList().end(); it++) {
			if (*it == this)
				return previous;
			previous = *it;
		}
	}
	return NULL;
}

Statement *Statement::getNextStatementInBB()
{
	assert(pbb);
	std::list<RTL*> *rtls = pbb->getRTLs();
	assert(rtls);
	bool wantNext = false;
	for (std::list<RTL*>::iterator rit = rtls->begin(); rit != rtls->end(); rit++) {
		RTL *rtl = *rit;
		for (RTL::iterator it = rtl->getList().begin(); it != rtl->getList().end(); it++) {
			if (wantNext)
				return *it;
			if (*it == this)
				wantNext = true;
		}
	}
	return NULL;
}

/*==============================================================================
 * FUNCTION:		operator<<
 * OVERVIEW:		Output operator for Statement*
 *					Just makes it easier to use e.g. std::cerr << myStmtStar
 * PARAMETERS:		os: output stream to send to
 *					p: ptr to Statement to print to the stream
 * RETURNS:			copy of os (for concatenation)
 *============================================================================*/
std::ostream& operator<<(std::ostream& os, Statement* s) {
	if (s == NULL) {os << "NULL "; return os;}
	s->print(os);
	return os;
}

bool Statement::isFlagAssgn() {
	if (kind != STMT_ASSIGN)
		return false;
	OPER op = ((Assign*)this)->getRight()->getOper();
	return (op == opFlagCall);
}

char* Statement::prints() {
	std::ostringstream ost;
	print(ost);
	strncpy(debug_buffer, ost.str().c_str(), DEBUG_BUFSIZE-1);
	debug_buffer[DEBUG_BUFSIZE-1] = '\0';
	return debug_buffer;
}

// This version prints much better in gdb
void Statement::dump() {
	print(std::cerr);
	std::cerr << "\n";
}


/* This function is designed to find basic flag calls, plus in addition two variations seen with Pentium FP code.
	These variations involve ANDing and/or XORing with constants. So it should return true for these values of e:
	ADDFLAGS(...)
	SETFFLAGS(...) & 0x45
	(SETFFLAGS(...) & 0x45) ^ 0x40
	FIXME: this may not be needed any more...
*/
bool hasSetFlags(Exp* e) {
	if (e->isFlagCall()) return true;
	OPER op = e->getOper();
	if (op != opBitAnd && op != opBitXor) return false;
	Exp* left  = ((Binary*)e)->getSubExp1();
	Exp* right = ((Binary*)e)->getSubExp2();
	if (!right->isIntConst()) return false;
	if (left->isFlagCall()) {
std::cerr << "hasSetFlags returns true with " << e << "\n";
		return true;
	}
	op = left->getOper();
	if (op != opBitAnd && op != opBitXor) return false;
	right = ((Binary*)left)->getSubExp2();
	left  = ((Binary*)left)->getSubExp1();
	if (!right->isIntConst()) return false;
	bool ret = left->isFlagCall();
if (ret)
 std::cerr << "hasSetFlags returns true with " << e << "\n";
	return ret;
}

// Return true if can propagate to Exp* e (must be a RefExp to return true)
// Note: does not consider whether e is able to be renamed (from a memory Primitive point of view), only if the
// definition can be propagated to this stmt
// Note: static member function
bool Statement::canPropagateToExp(Exp*e) {
	if (!e->isSubscript()) return false;
	if (((RefExp*)e)->isImplicitDef())
		// Can't propagate statement "-" or "0" (implicit assignments)
		return false;
	Statement* def = ((RefExp*)e)->getDef();
//	if (def == this)
		// Don't propagate to self! Can happen with %pc's (?!)
//		return false;
	if (def->isNullStatement())
		// Don't propagate a null statement! Can happen with %pc's (would have no effect, and would infinitely loop)
		return false;
	if (!def->isAssign()) return false;		// Only propagate ordinary assignments (so far)
#if 0
	if (def->isPhi())
		// Don't propagate phi statements!
		return false;
	if (def->isCall())
		return false;
	if (def->isBool())
		return false;
#endif
	Assign* adef = (Assign*)def;
	Exp* lhs = adef->getLeft();

	if (lhs->getType() && lhs->getType()->isArray()) {
		// Assigning to an array, don't propagate (Could be alias problems?)
		return false;
	}
	return true;
}

// Return true if any change; set convert if an indirect call statement is converted to direct (else unchanged)
bool Statement::propagateTo(bool& convert, std::map<Exp*, int, lessExpStar>* destCounts /* = NULL */ ) {
	bool change;
	int changes = 0;
	// int sp = proc->getSignature()->getStackRegister(proc->getProg());
	// Exp* regSp = Location::regOf(sp);
	int propMaxDepth = Boomerang::get()->propMaxDepth;
	// Repeat substituting into this statement while there is a reference component in it
	if (EXPERIMENTAL) {
		do {
			LocationSet exps;
			addUsedLocs(exps, true, true);	// First true to also add uses from collectors. For example, want to
											// propagate into the reaching definitions of calls. Second true is to only
											// look inside m[...]. These must be propagated into, regardless of -l
			LocationSet::iterator ll;
			change = false;
			for (ll = exps.begin(); ll != exps.end(); ll++) {
				Exp* e = *ll;
				if (!canPropagateToExp(e))
					continue;
				Assign* def = (Assign*)((RefExp*)e)->getDef();
				Exp* rhs = def->getRight();
				if (rhs->containsBareMemof())
					// Must never propagate unsubscripted memofs. You could be propagating past a definition, thereby
					// invalidating the IR
					continue;
				change |= doPropagateTo(e, def, convert);
			}
		} while (change && ++changes < 10);
	}

	// Now propagate the rest, but this time subject to -l propagation limiting
	do {
		LocationSet exps;
		addUsedLocs(exps, true);		// True to also add uses from collectors. For example, want to propagate into
										// the reaching definitions of calls. Third parameter defaults to false, to
										// find all locations, not just those inside m[...]
		LocationSet::iterator ll;
		change = false;
		// Example: m[r24{10}] := r25{20} + m[r26{30}]
		// exps has r24{10}, r25{30}, m[r26{30}], r26{30}
		for (ll = exps.begin(); ll != exps.end(); ll++) {
			Exp* e = *ll;
			if (!canPropagateToExp(e))
				continue;
			Assign* def = (Assign*)((RefExp*)e)->getDef();
			Exp* rhs = def->getRight();
			if (rhs->containsBareMemof())
				// Must never propagate unsubscripted memofs. You could be propagating past a definition, thereby
				// invalidating the IR
				continue;
			Exp* lhs = def->getLeft();

			if (EXPERIMENTAL) {
				// This is the old "don't propagate x=f(x)" heuristic. Hopefully it will work better now that we always
				// propagate into memofs etc. However, it might need a "and we're inside the right kind of loop"
				// condition
				LocationSet used;
				def->addUsedLocs(used);
				RefExp left(def->getLeft(), (Statement*)-1);
				RefExp *right = dynamic_cast<RefExp*>(def->getRight());
				// Beware of x := x{something else} (do want to do copy propagation)
				if (used.exists(&left) && !(right && *right->getSubExp1() == *left.getSubExp1()))
					// We have something like eax = eax + 1
					continue;
			}

			// Check if the -l flag (propMaxDepth) prevents this propagation
			if (destCounts && !lhs->isFlags()) {			// Always propagate to %flags
				std::map<Exp*, int, lessExpStar>::iterator ff = destCounts->find(e);
				if (ff != destCounts->end() && ff->second > 1 && rhs->getComplexityDepth(proc) >= propMaxDepth) {
					if (!def->getRight()->containsFlags()) {
						// This propagation is prevented by the -l limit
						continue;
					}
				}
			}
			change |= doPropagateTo(e, def, convert);
		}
	} while (change && ++changes < 10);
	// Simplify is very costly, especially for calls. I hope that doing one simplify at the end will not affect any
	// result...
	simplify();
	return changes != 0;
}


// Parameter convert is set true if an indirect call is converted to direct
// Return true if a change made
// Note: this procedure does not control what part of this statement is propagated to
// Propagate to e from definition statement def.
// Set convert to true if convert a call from indirect to direct.
bool Statement::doPropagateTo(Exp* e, Assign* def, bool& convert) {
	// Respect the -p N switch
	if (Boomerang::get()->numToPropagate >= 0) {
		if (Boomerang::get()->numToPropagate == 0) return false;
			Boomerang::get()->numToPropagate--;
	}

	if (VERBOSE)
		LOG << "propagating " << def << "\n" << "	   into " << this << "\n";

	bool change = replaceRef(e, def, convert);
	
	if (VERBOSE) {
		LOG << "	 result " << this << "\n\n";
	}
	return change;
}

// replace a use of def->getLeft() by def->getRight() in this statement
// return true if change
bool Statement::replaceRef(Exp* e, Assign *def, bool& convert) {
	Exp* rhs = def->getRight();
	assert(rhs);

	Exp* base = ((RefExp*)e)->getSubExp1();
	// Could be propagating %flags into %CF
	Exp* lhs = def->getLeft();
	if (base->getOper() == opCF && lhs->isFlags()) {
		assert(rhs->isFlagCall());
		char* str = ((Const*)((Binary*)rhs)->getSubExp1())->getStr();
		if (strncmp("SUBFLAGS", str, 8) == 0) {
			/* When the carry flag is used bare, and was defined in a subtract of the form lhs - rhs, then CF has
			   the value (lhs <u rhs).  lhs and rhs are the first and second parameters of the flagcall.
			   Note: the flagcall is a binary, with a Const (the name) and a list of expressions:
				 defRhs
				 /	  \
			Const	   opList
			"SUBFLAGS"	/	\
					   P1	opList
							 /	 \
							P2	opList
								 /	 \
								P3	 opNil
			*/
			Exp* relExp = new Binary(opLessUns,
				((Binary*)rhs)->getSubExp2()->getSubExp1(),
				((Binary*)rhs)->getSubExp2()->getSubExp2()->getSubExp1());
			searchAndReplace(new RefExp(new Terminal(opCF), def), relExp, true);
			return true;
		}
	}

	// do the replacement
	//bool convert = doReplaceRef(re, rhs);
	bool ret = searchAndReplace(e, rhs, true);		// Last parameter true to change collectors
	// assert(ret);

	if (ret && isCall()) {
		convert |= ((CallStatement*)this)->convertToDirect();
	}
	return ret;
}

bool Statement::isNullStatement() {
	if (kind != STMT_ASSIGN) return false;
	Exp* right = ((Assign*)this)->getRight();
	if (right->isSubscript()) {
		// Must refer to self to be null
		return this == ((RefExp*)right)->getDef();
	}
	else
		// Null if left == right
		return *((Assign*)this)->getLeft() == *right;
}

bool Statement::isFpush() {
	if (kind != STMT_ASSIGN) return false;
	return ((Assign*)this)->getRight()->getOper() == opFpush;
}
bool Statement::isFpop() {
	if (kind != STMT_ASSIGN) return false;
	return ((Assign*)this)->getRight()->getOper() == opFpop;
}


/*
 * This code was in hrtl.cpp
 * Classes derived from Statement
 */


#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

/******************************************************************************
 * GotoStatement methods
 *****************************************************************************/

/*==============================================================================
 * FUNCTION:		GotoStatement::GotoStatement
 * OVERVIEW:		Constructor.
 * PARAMETERS:		listStmt: a list of Statements (not the same as an RTL)
 *					  to serve as the initial list of statements
 * RETURNS:			N/a
 *============================================================================*/
GotoStatement::GotoStatement()
  : pDest(NULL), m_isComputed(false) {
	kind = STMT_GOTO;
}

/*==============================================================================
 * FUNCTION:		GotoStatement::GotoStatement
 * OVERVIEW:		Construct a jump to a fixed address
 * PARAMETERS:		uDest: native address of destination
 * RETURNS:			N/a
 *============================================================================*/
GotoStatement::GotoStatement(ADDRESS uDest) : m_isComputed(false) {
	kind = STMT_GOTO;
	pDest = new Const(uDest);
}

/*==============================================================================
 * FUNCTION:		GotoStatement::~GotoStatement
 * OVERVIEW:		Destructor
 * PARAMETERS:		None
 * RETURNS:			N/a
 *============================================================================*/
GotoStatement::~GotoStatement() {
	if (pDest) ;//delete pDest;
}

/*==============================================================================
 * FUNCTION:		GotoStatement::getFixedDest
 * OVERVIEW:		Get the fixed destination of this CTI. Assumes destination
 *					simplication has already been done so that a fixed dest will
 *					be of the Exp form:
 *					   opIntConst dest
 * PARAMETERS:		<none>
 * RETURNS:			Fixed dest or NO_ADDRESS if there isn't one
 *============================================================================*/
ADDRESS GotoStatement::getFixedDest() {
	if (pDest->getOper() != opIntConst) return NO_ADDRESS;
	return ((Const*)pDest)->getAddr();
}

/*==============================================================================
 * FUNCTION:		GotoStatement::setDest
 * OVERVIEW:		Set the destination of this jump to be a given expression.
 * PARAMETERS:		addr - the new fixed address
 * RETURNS:			Nothing
 *============================================================================*/
void GotoStatement::setDest(Exp* pd) {
	pDest = pd;
}

/*==============================================================================
 * FUNCTION:		GotoStatement::setDest
 * OVERVIEW:		Set the destination of this jump to be a given fixed address.
 * PARAMETERS:		addr - the new fixed address
 * RETURNS:			<nothing>
 *============================================================================*/
void GotoStatement::setDest(ADDRESS addr) {
// This fails in FrontSparcTest, do you really want it to Mike? -trent
//	assert(addr >= prog.limitTextLow && addr < prog.limitTextHigh);
	// Delete the old destination if there is one
	if (pDest != NULL)
		;//delete pDest;

	pDest = new Const(addr);
}

/*==============================================================================
 * FUNCTION:		GotoStatement::getDest
 * OVERVIEW:		Returns the destination of this CTI.
 * PARAMETERS:		None
 * RETURNS:			Pointer to the SS representing the dest of this jump
 *============================================================================*/
Exp* GotoStatement::getDest() {
	return pDest;
}

/*==============================================================================
 * FUNCTION:		GotoStatement::adjustFixedDest
 * OVERVIEW:		Adjust the destination of this CTI by a given amount. Causes
 *					an error is this destination is not a fixed destination
 *					(i.e. a constant offset).
 * PARAMETERS:		delta - the amount to add to the destination (can be
 *					negative)
 * RETURNS:			<nothing>
 *============================================================================*/
void GotoStatement::adjustFixedDest(int delta) {
	// Ensure that the destination is fixed.
	if (pDest == 0 || pDest->getOper() != opIntConst)
		LOG << "Can't adjust destination of non-static CTI\n";

	ADDRESS dest = ((Const*)pDest)->getAddr();
	((Const*)pDest)->setAddr(dest + delta);
}

bool GotoStatement::search(Exp* search, Exp*& result) {
	result = NULL;
	if (pDest)
		return pDest->search(search, result);
	return false;
}

/*==============================================================================
 * FUNCTION:		GotoStatement::searchAndReplace
 * OVERVIEW:		Replace all instances of search with replace.
 * PARAMETERS:		search - a location to search for
 *					replace - the expression with which to replace it
 *					cc - ignored
 * RETURNS:			True if any change
 *============================================================================*/
bool GotoStatement::searchAndReplace(Exp* search, Exp* replace, bool cc) {
	bool change = false;
	if (pDest) {
		pDest = pDest->searchReplaceAll(search, replace, change);
	}
	return change;
}

/*==============================================================================
 * FUNCTION:		GotoStatement::searchAll
 * OVERVIEW:		Find all instances of the search expression
 * PARAMETERS:		search - a location to search for
 *					result - a list which will have any matching exprs
 *							 appended to it
 * RETURNS:			true if there were any matches
 *============================================================================*/
bool GotoStatement::searchAll(Exp* search, std::list<Exp*> &result) {
	if (pDest)	return pDest->searchAll(search, result);
	return false;
}

/*==============================================================================
 * FUNCTION:		GotoStatement::print
 * OVERVIEW:		Display a text reprentation of this RTL to the given stream
 * NOTE:			Usually called from RTL::print, in which case the first 9
 *					  chars of the print have already been output to os
 * PARAMETERS:		os: stream to write to
 * RETURNS:			Nothing
 *============================================================================*/
void GotoStatement::print(std::ostream& os) {
	os << std::setw(4) << std::dec << number << " ";
	os << "GOTO ";
	if (pDest == NULL)
		os << "*no dest*";
	else if (pDest->getOper() != opIntConst)
		 pDest->print(os);
	else
		os << "0x" << std::hex << getFixedDest();
}

/*==============================================================================
 * FUNCTION:	  GotoStatement::setIsComputed
 * OVERVIEW:	  Sets the fact that this call is computed.
 * NOTE:		  This should really be removed, once CaseStatement and
 *					HLNwayCall are implemented properly
 * PARAMETERS:	  <none>
 * RETURNS:		  <nothing>
 *============================================================================*/
void GotoStatement::setIsComputed(bool b) {
	m_isComputed = b;
}

/*==============================================================================
 * FUNCTION:	  GotoStatement::isComputed
 * OVERVIEW:	  Returns whether or not this call is computed.
 * NOTE:		  This should really be removed, once CaseStatement and HLNwayCall
 *					are implemented properly
 * PARAMETERS:	  <none>
 * RETURNS:		  this call is computed
 *============================================================================*/
bool GotoStatement::isComputed() {
	return m_isComputed;
}

/*==============================================================================
 * FUNCTION:		GotoStatement::clone
 * OVERVIEW:		Deep copy clone
 * PARAMETERS:		<none>
 * RETURNS:			Pointer to a new Statement, a clone of this GotoStatement
 *============================================================================*/
Statement* GotoStatement::clone() {
	GotoStatement* ret = new GotoStatement();
	ret->pDest = pDest->clone();
	ret->m_isComputed = m_isComputed;
	// Statement members
	ret->pbb = pbb;
	ret->proc = proc;
	ret->number = number;
	return ret;
}

// visit this Statement in the RTL
bool GotoStatement::accept(StmtVisitor* visitor) {
	return visitor->visit(this);
}

void GotoStatement::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
	// dont generate any code for jumps, they will be handled by the BB
}

void GotoStatement::simplify() {
	if (isComputed()) {
		pDest = pDest->simplifyArith();
		pDest = pDest->simplify();
	}
}

/**********************************
 * BranchStatement methods
 **********************************/

/*==============================================================================
 * FUNCTION:		BranchStatement::BranchStatement
 * OVERVIEW:		Constructor.
 * PARAMETERS:		None
 * RETURNS:			N/a
 *============================================================================*/
BranchStatement::BranchStatement() : jtCond((BRANCH_TYPE)0), pCond(NULL), bFloat(false), size(0) {
	kind = STMT_BRANCH;
}

/*==============================================================================
 * FUNCTION:		BranchStatement::~BranchStatement
 * OVERVIEW:		Destructor
 * PARAMETERS:		None
 * RETURNS:			N/a
 *============================================================================*/
BranchStatement::~BranchStatement() {
	if (pCond)
		;//delete pCond;
}

/*==============================================================================
 * FUNCTION:		BranchStatement::setCondType
 * OVERVIEW:		Sets the BRANCH_TYPE of this jcond as well as the flag
 *					indicating whether or not the floating point condition codes
 *					are used.
 * PARAMETERS:		cond - the BRANCH_TYPE
 *					usesFloat - this condional jump checks the floating point
 *					  condition codes
 * RETURNS:			a semantic string
 *============================================================================*/
void BranchStatement::setCondType(BRANCH_TYPE cond, bool usesFloat /*= false*/) {
	jtCond = cond;
	bFloat = usesFloat;

	// set pCond to a high level representation of this type
	Exp* p = NULL;
	switch(cond) {
		case BRANCH_JE:
			p = new Binary(opEquals, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JNE:
			p = new Binary(opNotEqual, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JSL:
			p = new Binary(opLess, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JSLE:
			p = new Binary(opLessEq, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JSGE:
			p = new Binary(opGtrEq, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JSG:
			p = new Binary(opGtr, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JUL:
			p = new Binary(opLessUns, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JULE:
			p = new Binary(opLessEqUns, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JUGE:
			p = new Binary(opGtrEqUns, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JUG:
			p = new Binary(opGtrUns, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JMI:
			p = new Binary(opLess, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JPOS:
			p = new Binary(opGtr, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JOF:
			p = new Binary(opLessUns, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JNOF:
			p = new Binary(opGtrUns, new Terminal(opFlags), new Const(0));
			break;
		case BRANCH_JPAR:
			// Can't handle (could happen as a result of a failure of Pentium
			// floating point analysis)
			assert(false);
			break;
	}
	// this is such a hack.. preferably we should actually recognise 
	// SUBFLAGS32(..,..,..) > 0 instead of just SUBFLAGS32(..,..,..)
	// but I'll leave this in here for the moment as it actually works.
	if (!Boomerang::get()->noDecompile)
		p = new Terminal(usesFloat ? opFflags : opFlags);
	assert(p);
	setCondExpr(p);
}

/*==============================================================================
 * FUNCTION:		BranchStatement::makeSigned
 * OVERVIEW:		Change this from an unsigned to a signed branch
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
void BranchStatement::makeSigned() {
	// Make this into a signed branch
	switch (jtCond)
	{
		case BRANCH_JUL : jtCond = BRANCH_JSL;	break;
		case BRANCH_JULE: jtCond = BRANCH_JSLE; break;
		case BRANCH_JUGE: jtCond = BRANCH_JSGE; break;
		case BRANCH_JUG : jtCond = BRANCH_JSG;	break;
		default:
			// Do nothing for other cases
			break;
	}
}

/*==============================================================================
 * FUNCTION:		BranchStatement::getCondExpr
 * OVERVIEW:		Return the SemStr expression containing the HL condition.
 * PARAMETERS:		<none>
 * RETURNS:			ptr to an expression
 *============================================================================*/
Exp* BranchStatement::getCondExpr() {
	return pCond;
}

/*==============================================================================
 * FUNCTION:		BranchStatement::setCondExpr
 * OVERVIEW:		Set the SemStr expression containing the HL condition.
 * PARAMETERS:		Pointer to Exp to set
 * RETURNS:			<nothing>
 *============================================================================*/
void BranchStatement::setCondExpr(Exp* e) {
	if (pCond) ;//delete pCond;
	pCond = e;
}

bool BranchStatement::search(Exp* search, Exp*& result) {
	if (pCond) return pCond->search(search, result);
	result = NULL;
	return false;
}

/*==============================================================================
 * FUNCTION:		BranchStatement::searchAndReplace
 * OVERVIEW:		Replace all instances of search with replace.
 * PARAMETERS:		search - a location to search for
 *					replace - the expression with which to replace it
 *					cc - ignored
 * RETURNS:			True if any change
 *============================================================================*/
bool BranchStatement::searchAndReplace(Exp* search, Exp* replace, bool cc) {
	GotoStatement::searchAndReplace(search, replace, cc);
	bool change = false;
	if (pCond)
		pCond = pCond->searchReplaceAll(search, replace, change);
	return change;
}

// Convert from SSA form
void BranchStatement::fromSSAform(igraph& ig) {
	if (pCond)
		pCond = pCond->fromSSA(ig); 
}

/*==============================================================================
 * FUNCTION:		BranchStatement::searchAll
 * OVERVIEW:		Find all instances of the search expression
 * PARAMETERS:		search - a location to search for
 *					result - a list which will have any matching exprs
 *							 appended to it
 * RETURNS:			true if there were any matches
 *============================================================================*/
bool BranchStatement::searchAll(Exp* search, std::list<Exp*> &result) {
	if (pCond) return pCond->searchAll(search, result);
	return false;
}


/*==============================================================================
 * FUNCTION:		BranchStatement::print
 * OVERVIEW:		Write a text representation to the given stream
 * PARAMETERS:		os: stream
 * RETURNS:			Nothing
 *============================================================================*/
void BranchStatement::print(std::ostream& os) {
	os << std::setw(4) << std::dec << number << " ";
	os << "BRANCH ";
	if (pDest == NULL)
		os << "*no dest*";
	else if (!pDest->isIntConst())
		os << pDest;
	else {
		// Really we'd like to display the destination label here...
		os << "0x" << std::hex << getFixedDest();
	}
	os << ", condition ";
	switch (jtCond) {
		case BRANCH_JE:	   os << "equals"; break;
		case BRANCH_JNE:   os << "not equals"; break;
		case BRANCH_JSL:   os << "signed less"; break;
		case BRANCH_JSLE:  os << "signed less or equals"; break;
		case BRANCH_JSGE:  os << "signed greater or equals"; break;
		case BRANCH_JSG:   os << "signed greater"; break;
		case BRANCH_JUL:   os << "unsigned less"; break;
		case BRANCH_JULE:  os << "unsigned less or equals"; break;
		case BRANCH_JUGE:  os << "unsigned greater or equals"; break;
		case BRANCH_JUG:   os << "unsigned greater"; break;
		case BRANCH_JMI:   os << "minus"; break;
		case BRANCH_JPOS:  os << "plus"; break;
		case BRANCH_JOF:   os << "overflow"; break;
		case BRANCH_JNOF:  os << "no overflow"; break;
		case BRANCH_JPAR:  os << "parity"; break;
	}
	if (bFloat) os << " float";
	os << std::endl;
	if (pCond) {
		os << "High level: " << pCond;
	}
}

/*==============================================================================
 * FUNCTION:		BranchStatement::clone
 * OVERVIEW:		Deep copy clone
 * PARAMETERS:		<none>
 * RETURNS:			Pointer to a new Statement, a clone of this BranchStatement
 *============================================================================*/
Statement* BranchStatement::clone() {
	BranchStatement* ret = new BranchStatement();
	ret->pDest = pDest->clone();
	ret->m_isComputed = m_isComputed;
	ret->jtCond = jtCond;
	if (pCond) ret->pCond = pCond->clone();
	else ret->pCond = NULL;
	ret->bFloat = bFloat;
	// Statement members
	ret->pbb = pbb;
	ret->proc = proc;
	ret->number = number;
	return ret;
}

// visit this stmt
bool BranchStatement::accept(StmtVisitor* visitor) {
	return visitor->visit(this);
}

void BranchStatement::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
	// dont generate any code for jconds, they will be handled by the bb
}

bool BranchStatement::usesExp(Exp *e) {
	Exp *tmp;
	return pCond && pCond->search(e, tmp);
}


// Common to BranchStatement and BoolAssign
// Return true if this is now a floating point Branch
bool condToRelational(Exp*& pCond, BRANCH_TYPE jtCond) {
	pCond = pCond->simplifyArith()->simplify();

	std::stringstream os;
	pCond->print(os);
	std::string s = os.str();

	OPER condOp = pCond->getOper();
	if (condOp == opFlagCall && !strncmp(((Const*)pCond->getSubExp1())->getStr(), "SUBFLAGS", 8)) {
		OPER op = opWild;
		// Special for PPC unsigned compares; may be other cases in the future
	    bool makeUns = strncmp(((Const*)pCond->getSubExp1())->getStr(), "SUBFLAGSNL", 10) == 0;
		switch (jtCond) {
			case BRANCH_JE:	   op = opEquals; break;
			case BRANCH_JNE:   op = opNotEqual; break;
			case BRANCH_JSL:   if (makeUns) op = opLessUns; else op = opLess; break;
			case BRANCH_JSLE:  if (makeUns) op = opLessEqUns; else op = opLessEq; break;
			case BRANCH_JSGE:  if (makeUns) op = opGtrEqUns; else op = opGtrEq; break;
			case BRANCH_JSG:   if (makeUns) op = opGtrUns; else op = opGtr; break;
			case BRANCH_JUL:   op = opLessUns; break;
			case BRANCH_JULE:  op = opLessEqUns; break;
			case BRANCH_JUGE:  op = opGtrEqUns; break;
			case BRANCH_JUG:   op = opGtrUns; break;
			case BRANCH_JMI:
				/*	 pCond
					 /	  \
				Const	   opList
				"SUBFLAGS"	/	\
						   P1	opList
								 /	 \
								P2	opList
									 /	 \
									P3	 opNil */
				pCond = new Binary(opLess,			// P3 < 0
					pCond->getSubExp2()->getSubExp2()->getSubExp2()->getSubExp1()->clone(),
					new Const(0));
				break;
			case BRANCH_JPOS:
				pCond = new Binary(opGtrEq,			// P3 >= 0
					pCond->getSubExp2()->getSubExp2()->getSubExp2()->getSubExp1()->clone(),
					new Const(0));
				break;
			case BRANCH_JOF:
			case BRANCH_JNOF:
			case BRANCH_JPAR:
				break;
		}
		if (op != opWild) {
			pCond = new Binary(op,
				pCond->getSubExp2()->getSubExp1()->clone(),					// P1
				pCond->getSubExp2()->getSubExp2()->getSubExp1()->clone());	// P2
		}
	}
	else if (condOp == opFlagCall && !strncmp(((Const*)pCond->getSubExp1())->getStr(), "LOGICALFLAGS", 12)) {
		// Exp *e = pCond;
		OPER op = opWild;
		switch (jtCond) {
			case BRANCH_JE:	  op = opEquals; break;
			case BRANCH_JNE:  op = opNotEqual; break;
			case BRANCH_JMI:  op = opLess; break;
			case BRANCH_JPOS: op = opGtrEq; break;
			// FIXME: This next set is quite shakey. Really, we should put all the individual flag definitions out of
			// the flag definitions, and substitute these into the equivalent conditions for the branches (a big, ugly
			// job).
			case BRANCH_JSL:  op = opLess; break;
			case BRANCH_JSLE: op = opLessEq; break;
			case BRANCH_JSGE: op = opGtrEq; break;
			case BRANCH_JSG:  op = opGtr; break;
			// These next few seem to fluke working fine on architectures like X86, SPARC, and 68K which clear the
			// carry on all logical operations.
			case BRANCH_JUL:  op = opLessUns; break;	// NOTE: this is equivalent to never branching, since nothing
														// can be unsigned less than zero
			case BRANCH_JULE: op = opLessEqUns; break;
			case BRANCH_JUGE: op = opGtrEqUns; break;	// Similarly, this is equivalent to always branching
			case BRANCH_JUG:  op = opGtrUns; break;
			default:
				break;
		}
		if (op != opWild) {
			pCond = new Binary(op,
				pCond->getSubExp2()->getSubExp1()->clone(),
				new Const(0));
		}
	}
	else if (condOp == opFlagCall && !strncmp(((Const*)pCond->getSubExp1())->getStr(), "SETFFLAGS", 9)) {
		// Exp *e = pCond;
		OPER op = opWild;
		switch (jtCond) {
			case BRANCH_JE:	  op = opEquals; break;
			case BRANCH_JNE:  op = opNotEqual; break;
			case BRANCH_JMI:  op = opLess; break;
			case BRANCH_JPOS: op = opGtrEq; break;
			case BRANCH_JSL:  op = opLess; break;
			case BRANCH_JSLE: op = opLessEq; break;
			case BRANCH_JSGE: op = opGtrEq; break;
			case BRANCH_JSG:  op = opGtr; break;
			default:
				break;
		}
		if (op != opWild) {
			pCond = new Binary(op,
				pCond->getSubExp2()->getSubExp1()->clone(),
				pCond->getSubExp2()->getSubExp2()->getSubExp1()
					->clone());
		}
	}
	// ICK! This is all PENTIUM SPECIFIC... needs to go somewhere else.
	// Might be of the form (SETFFLAGS(...) & MASK) RELOP INTCONST where MASK could be a combination of 1, 4, and 40,
	// and relop could be == or ~=.  There could also be an XOR 40h after the AND
	// %fflags = 0..0.0 00 >
	// %fflags = 0..0.1 01 <
	// %fflags = 1..0.0 40 =
	// %fflags = 1..1.1 45 not comparable
	// Example: (SETTFLAGS(...) & 1) ~= 0
	// left = SETFFLAGS(...) & 1
	// left1 = SETFFLAGS(...) left2 = int 1, k = 0, mask = 1
	else if (condOp == opEquals || condOp == opNotEqual) {
		Exp* left =	 ((Binary*)pCond)->getSubExp1();
		Exp* right = ((Binary*)pCond)->getSubExp2();
		bool hasXor40 = false;
		if (left->getOper() == opBitXor && right->isIntConst()) {
			Exp* r2 = ((Binary*)left)->getSubExp2();
			if (r2->isIntConst()) {
				int k2 = ((Const*)r2)->getInt();
				if (k2 == 0x40) {
					hasXor40 = true;
					left = ((Binary*)left)->getSubExp1();
				}
			}
		}
		if (left->getOper() == opBitAnd && right->isIntConst()) {
			Exp* left1 = ((Binary*)left)->getSubExp1();
			Exp* left2 = ((Binary*)left)->getSubExp2();
			int k = ((Const*)right)->getInt();
			// Only interested in 40, 1
			k &= 0x41;
			if (left1->getOper() == opFlagCall && left2->isIntConst()) {
				int mask = ((Const*)left2)->getInt();
				// Only interested in 1, 40
				mask &= 0x41;
				OPER op = opWild;
				if (hasXor40) {
					assert(k == 0);
					op = condOp;
				} else {
					switch (mask) {
						case 1:
							if (condOp == opEquals && k == 0 ||
								condOp == opNotEqual && k == 1)
									op = opGtrEq;
							else
									op = opLess;
							break;
						case 0x40:
							if (condOp == opEquals && k == 0 ||
								condOp == opNotEqual && k == 0x40)
									op = opNotEqual;
							else
									op = opEquals;
							break;
						case 0x41:
							switch (k) {
								case 0:
									if (condOp == opEquals) op = opGtr;
									else op = opLessEq;
									break;
								case 1:
									if (condOp == opEquals) op = opLess;
									else op = opGtrEq;
									break;
								case 0x40:
									if (condOp == opEquals) op = opEquals;
									else op = opNotEqual;
									break;
								default:
									std::cerr << "BranchStatement::simplify: k is " << std::hex << k << "\n";
									assert(0);
							}
							break;
						default:
							std::cerr << "BranchStatement::simplify: Mask is " << std::hex << mask << "\n";
							assert(0);
					}
				}
				if (op != opWild) {
					pCond = new Binary(op,
						left1->getSubExp2()->getSubExp1(),
						left1->getSubExp2()->getSubExp2()->getSubExp1());
					return true;	  // This is now a float comparison
				}
			}
		}
	}
	return false;
}


void BranchStatement::simplify() {
	if (pCond) {
		if (condToRelational(pCond, jtCond))
			bFloat = true;
	}
}

/**********************************
 * CaseStatement methods
 **********************************/
/*==============================================================================
 * FUNCTION:		CaseStatement::CaseStatement
 * OVERVIEW:		Constructor.
 * PARAMETERS:		None
 * RETURNS:			N/a
 *============================================================================*/
CaseStatement::CaseStatement() :
	pSwitchInfo(NULL) {
	kind = STMT_CASE;
}

/*==============================================================================
 * FUNCTION:		CaseStatement::~CaseStatement
 * OVERVIEW:		Destructor
 * NOTE:			Don't delete the pSwitchVar; it's always a copy of something else (so don't delete twice)
 * PARAMETERS:		None
 * RETURNS:			N/a
 *============================================================================*/
CaseStatement::~CaseStatement() {
	if (pSwitchInfo)
		;//delete pSwitchInfo;
}

/*==============================================================================
 * FUNCTION:		CaseStatement::getSwitchInfo
 * OVERVIEW:		Return a pointer to a struct with switch information in it
 * PARAMETERS:		<none>
 * RETURNS:			a semantic string
 *============================================================================*/
SWITCH_INFO* CaseStatement::getSwitchInfo() {
	return pSwitchInfo;
}

/*==============================================================================
 * FUNCTION:		CaseStatement::setSwitchInfo
 * OVERVIEW:		Set a pointer to a SWITCH_INFO struct
 * PARAMETERS:		Pointer to SWITCH_INFO struct
 * RETURNS:			<nothing>
 *============================================================================*/
void CaseStatement::setSwitchInfo(SWITCH_INFO* psi) {
	pSwitchInfo = psi;
}

/*==============================================================================
 * FUNCTION:		CaseStatement::searchAndReplace
 * OVERVIEW:		Replace all instances of search with replace.
 * PARAMETERS:		search - a location to search for
 *					replace - the expression with which to replace it
 *					cc - ignored
 * RETURNS:			True if any change
 *============================================================================*/
bool CaseStatement::searchAndReplace(Exp* search, Exp* replace, bool cc) {
	bool ch = GotoStatement::searchAndReplace(search, replace, cc);
	bool ch2 = false;
	if (pSwitchInfo && pSwitchInfo->pSwitchVar)
		pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->searchReplaceAll(search, replace, ch2);
	return ch | ch2;
}

/*==============================================================================
 * FUNCTION:		CaseStatement::searchAll
 * OVERVIEW:		Find all instances of the search expression
 * PARAMETERS:		search - a location to search for
 *					result - a list which will have any matching exprs appended to it
 * NOTES:			search can't easily be made const
 * RETURNS:			true if there were any matches
 *============================================================================*/
bool CaseStatement::searchAll(Exp* search, std::list<Exp*> &result) {
	return GotoStatement::searchAll(search, result) ||
		( pSwitchInfo && pSwitchInfo->pSwitchVar && pSwitchInfo->pSwitchVar->searchAll(search, result) );
}

/*==============================================================================
 * FUNCTION:		CaseStatement::print
 * OVERVIEW:		Write a text representation to the given stream
 * PARAMETERS:		os: stream
 *					indent: number of columns to skip
 * RETURNS:			Nothing
 *============================================================================*/
void CaseStatement::print(std::ostream& os) {
	os << std::setw(4) << std::dec << number << " ";
	if (pSwitchInfo == NULL) {
		os << "CASE [";
		if (pDest == NULL)
			os << "*no dest*";
		else os << pDest;
		os << "]";
	} else
		os << "SWITCH(" << pSwitchInfo->pSwitchVar << ")\n";
}


/*==============================================================================
 * FUNCTION:		CaseStatement::clone
 * OVERVIEW:		Deep copy clone
 * PARAMETERS:		<none>
 * RETURNS:			Pointer to a new Statement that is a clone of this one
 *============================================================================*/
Statement* CaseStatement::clone() {
	CaseStatement* ret = new CaseStatement();
	ret->pDest = pDest->clone();
	ret->m_isComputed = m_isComputed;
	ret->pSwitchInfo = new SWITCH_INFO;
	*ret->pSwitchInfo = *pSwitchInfo;
	ret->pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->clone();
	// Statement members
	ret->pbb = pbb;
	ret->proc = proc;
	ret->number = number;
	return ret;
}

// visit this stmt
bool CaseStatement::accept(StmtVisitor* visitor) {
	return visitor->visit(this);
}

void CaseStatement::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
	// dont generate any code for switches, they will be handled by the bb
}

bool CaseStatement::usesExp(Exp *e) {
	// Before a switch statement is recognised, pDest is non null
	if (pDest)
		return *pDest == *e;
	// After a switch statement is recognised, pDest is null, and pSwitchInfo->pSwitchVar takes over
	if (pSwitchInfo->pSwitchVar)
		return *pSwitchInfo->pSwitchVar == *e;
	return false;
}


// Convert from SSA form
void CaseStatement::fromSSAform(igraph& ig) {
	if (pDest) {
		pDest = pDest->fromSSA(ig);
		return;
	}
	if (pSwitchInfo && pSwitchInfo->pSwitchVar)
		pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->fromSSA(ig); 
}

void CaseStatement::simplify() {
	if (pDest)
		pDest = pDest->simplify();
	else if (pSwitchInfo && pSwitchInfo->pSwitchVar)
		pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->simplify();
}

/**********************************
 *		CallStatement methods
 **********************************/

/*==============================================================================
 * FUNCTION:		 CallStatement::CallStatement
 * OVERVIEW:		 Constructor for a call
 * PARAMETERS:		 None
 * RETURNS:			 <nothing>
 *============================================================================*/
CallStatement::CallStatement(): returnAfterCall(false), calleeReturn(NULL) {
	kind = STMT_CALL;
	procDest = NULL;
	signature = NULL;
}

/*==============================================================================
 * FUNCTION:	  CallStatement::~CallStatement
 * OVERVIEW:	  Destructor
 * PARAMETERS:	  BB - the enclosing basic block of this call
 * RETURNS:		  <nothing>
 *============================================================================*/
CallStatement::~CallStatement() {
}

#if 0
Exp *CallStatement::getReturnExp(int i) {
	if (i >= (int)defines.size()) return NULL;
	return returns[i].e;
}
#endif

// Temporarily needed for ad-hoc type analysis
int CallStatement::findDefine(Exp *e) {
	StatementList::iterator rr;
	int i = 0;
	for (rr = defines.begin(); rr != defines.end(); ++rr, ++i) {
		Exp* ret = ((Assignment*)*rr)->getLeft();
		if (*ret == *e)
			return i;
	}
	return -1;
}

Exp *CallStatement::getProven(Exp *e) {
	if (procDest)
		return procDest->getProven(e);
	return NULL;
}

// Localise only components of e, i.e. xxx if e is m[xxx]
void CallStatement::localiseComp(Exp* e, int depth /* = -1 */) {
	if (e->isMemOf()) {
        ((Location*)e)->setSubExp1(localiseExp(((Location*)e)->getSubExp1(), depth));
	}
}
// Substitute the various components of expression e with the appropriate reaching definitions.
// Used in e.g. fixCallBypass (via the CallBypasser). Locations defined in this call are replaced with their proven
// values, which are in terms of the initial values at the start of the call (reaching definitions at the call)
Exp* CallStatement::localiseExp(Exp* e, int depth /* = -1 */) {
	if (!defCol.isInitialised()) return e;				// Don't attempt to subscript if the data flow not started yet
	Localiser l(this, depth);
	e = e->clone()->accept(&l);

	return e;
}

// Find the definition for the given expression, using the embedded Collector object
// Was called findArgument(), and used implicit arguments and signature parameters
// Note: must only operator on unsubscripted locations, otherwise it is invalid
Exp* CallStatement::findDefFor(Exp *e) {
	return defCol.findDefFor(e);
#if 0
	int n = -1;
	if (!m_isComputed && procDest) {			// ? What if we find a destination for a computed call?
		n = procDest->getSignature()->findParam(e);
		if (n != -1)
			return arguments[n];
		n = procDest->getSignature()->findImplicitParam(e);
		if (n != -1)
			return implicitArguments[n];
	} else {
		std::vector<Exp*> &params = proc->getProg()->getDefaultParams();
		if (params.size() != implicitArguments.size()) {
			LOG << "eep. " << implicitArguments.size() << " args ";
			if (procDest) {
				LOG << procDest->getName() << " ";
				LOG << "(" << procDest->getSignature()->getNumParams() << " params) ";
			} else
				LOG << "(no dest) ";
			for (int i = 0; i < (int)implicitArguments.size(); i++)
				LOG << implicitArguments[i] << " ";
			LOG << "\n";
		}
		assert(params.size() == implicitArguments.size());
		for (unsigned i = 0; i < params.size(); i++)
			if (*params[i] == *e) {
				n = i;
				break;
			}
		if (n != -1)
			return implicitArguments[n];
	}
	assert(n == -1);
	return NULL;
#endif
}

#if 0
void CallStatement::addArgument(Exp *e, UserProc* proc) {
	// Process the argument. For example, given m[esp+4], it might be needed to localise as m[esp{17}+4], then get
	// substituted to m[esp{-}-16], then localised again to m[esp{-}-16]{16}, then substituted again into r24{14}
	// (examples from test/pentium/fibo)
	e = e->clone();							// Don't modify parameter
	int depth = e->getMemDepth();
	arguments.push_back(e);
	Exp*& er = arguments.back();
	StatementSet empty;
	for (int d=0; d <= depth; d++) {
		er = localiseExp(er, d);		// Localise for this call, depth d
		propagateTo(-1, empty, d);		// Propagate into all args etc, depth d
	}
	// These expressions can end up becoming locals (?)
	Location *l = dynamic_cast<Location*>(arguments.back());
	if (l)
		l->setProc(proc);
}
#endif

Type *CallStatement::getArgumentType(int i) {
	assert(i < (int)arguments.size());
	if (signature == NULL) return NULL;
	return signature->getParamType(i);
}

/*==============================================================================
 * FUNCTION:	  CallStatement::setArguments
 * OVERVIEW:	  Set the arguments of this call.
 * PARAMETERS:	  arguments - the list of locations to set the arguments to (for testing)
 * RETURNS:		  <nothing>
 *============================================================================*/
void CallStatement::setArguments(StatementList& args) {
	arguments.clear();
	arguments.append(args);
	StatementList::iterator ll;
	for (ll = arguments.begin(); ll != arguments.end(); ++ll) {
		((Assign*)*ll)->setProc(proc);
		((Assign*)*ll)->setBB(pbb);
#if 0		// For ad-hoc TA: (not even correct now)
		Location *l = dynamic_cast<Location*>(*ll);
		if (l) {
			l->setProc(proc);
		}
#endif
	}
}

/*==============================================================================
 * FUNCTION:	  CallStatement::setSigArguments
 * OVERVIEW:	  Set the arguments of this call based on signature info
 * NOTE:		  Should only be called for calls to library functions
 * PARAMETERS:	  None
 * RETURNS:		  <nothing>
 *============================================================================*/
void CallStatement::setSigArguments() {
	if (signature) return;				// Already done
	if (procDest == NULL)
		// FIXME: Need to check this
		return;
	// Clone here because each call to procDest could have a different signature, modified by ellipsisProcessing
	signature = procDest->getSignature()->clone();
	procDest->addCaller(this);

	if (!procDest->isLib())
		return;				// Using dataflow analysis now
	int n = signature->getNumParams();
	int i;
	arguments.clear();
	for (i = 0; i < n; i++) {
		Exp *e = signature->getArgumentExp(i);
		assert(e);
		Location *l = dynamic_cast<Location*>(e);
		if (l) {
			l->setProc(proc);		// Needed?
		}
		Assign* as = new Assign(signature->getParamType(i), e->clone(), e->clone());
		as->setProc(proc);
		as->setBB(pbb);
		as->setNumber(number);		// So fromSSAform will work later
		arguments.append(as);
	}
#if 0
	if (signature->hasEllipsis()) {
		// Just guess 4 parameters for now
		for (int i = 0; i < 4; i++) {
			Exp* e = signature->getArgumentExp(arguments.size())->clone();
			Assign* as = new Assign(new VoidType, e->clone(), e->clone());
			arguments.append(as);
			as->setProc(proc);
			as->setBB(pbb);
		}
	}
#endif

#if 0
	n = signature->getNumImplicitParams();
	implicitArguments.resize(n, NULL);
	for (i = 0; i < n; i++) {
		Exp *e = signature->getImplicitParamExp(i);
		assert(e);
		implicitArguments[i] = e->clone();
		implicitArguments[i]->fixLocationProc(proc);
	}
#endif
 
	// initialize returns
	// FIXME: anything needed here?
}

bool CallStatement::search(Exp* search, Exp*& result) {
	bool found = GotoStatement::search(search, result);
	if (found) return true;
	StatementList::iterator ss;
	for (ss = defines.begin(); ss != defines.end(); ++ss) {
		if ((*ss)->search(search, result))
			return true;
	}
	for (ss = arguments.begin(); ss != arguments.end(); ++ss) {
		if ((*ss)->search(search, result))
			return true;
	}
	return false;
}

/*==============================================================================
 * FUNCTION:		CallStatement::searchAndReplace
 * OVERVIEW:		Replace all instances of search with replace.
 * PARAMETERS:		search - a location to search for
 *					replace - the expression with which to replace it
 *					cc - true to replace in collectors
 * RETURNS:			True if any change
 *============================================================================*/
bool CallStatement::searchAndReplace(Exp* search, Exp* replace, bool cc) {
	bool change = GotoStatement::searchAndReplace(search, replace, cc);
	StatementList::iterator ss;
	// FIXME: MVE: Check if we ever want to change the LHS of arguments or defines...
	for (ss = defines.begin(); ss != defines.end(); ++ss)
		change |= (*ss)->searchAndReplace(search, replace, cc);
	for (ss = arguments.begin(); ss != arguments.end(); ++ss)
		change |= (*ss)->searchAndReplace(search, replace, cc);
	if (cc) {
		DefCollector::iterator dd;
		for (dd = defCol.begin(); dd != defCol.end(); ++dd)
			change |= (*dd)->searchAndReplace(search, replace, cc);
	}
	return change;
}

/*==============================================================================
 * FUNCTION:		CallStatement::searchAll
 * OVERVIEW:		Find all instances of the search expression
 * PARAMETERS:		search - a location to search for
 *					result - a list which will have any matching exprs appended to it
 * RETURNS:			true if there were any matches
 *============================================================================*/
bool CallStatement::searchAll(Exp* search, std::list<Exp *>& result) {
	bool found = GotoStatement::searchAll(search, result);
	StatementList::iterator ss;
	for (ss = defines.begin(); ss != defines.end(); ++ss) {
		if ((*ss)->searchAll(search, result))
			found = true;
	}
	for (ss = arguments.begin(); ss != arguments.end(); ++ss) {
		if ((*ss)->searchAll(search, result))
			found = true;
	}
	return found;
}

/*==============================================================================
 * FUNCTION:		CallStatement::print
 * OVERVIEW:		Write a text representation of this RTL to the given stream
 * PARAMETERS:		os: stream to write to
 * RETURNS:			Nothing
 *============================================================================*/
void CallStatement::print(std::ostream& os) {
	os << std::setw(4) << std::dec << number << " ";
 
	// Define(s), if any
	if (defines.size()) {
		if (defines.size() > 1) os << "{";
		StatementList::iterator rr;
		bool first = true;
		for (rr = defines.begin(); rr != defines.end(); ++rr) {
			if (first)
				first = false;
			else
				os << ", ";
			os << "*" << ((Assignment*)*rr)->getType() << "* " << ((Assignment*)*rr)->getLeft();
		}
		if (defines.size() > 1) os << "}";
		os << " := ";
	} else if (isChildless())
		os << "<all> := ";

	os << "CALL ";
	if (procDest)
		os << procDest->getName();
	else if (pDest == NULL)
			os << "*no dest*";
	else {
		if (pDest->isIntConst())
			os << "0x" << std::hex << ((Const*)pDest)->getInt();
		else
			pDest->print(os);		// Could still be an expression
	}

	// Print the actual arguments of the call
	if (isChildless())
		os << "(<all>)";
	else {
		os << "(\n";
		StatementList::iterator aa;
		for (aa = arguments.begin(); aa != arguments.end(); ++aa) {
			os << "                ";
			((Assignment*)*aa)->printCompact(os);
			os << "\n";
		}
		os << "              )";
	}

#if 1
	// Collected reaching definitions
	os << "\n              Reaching definitions: ";
	defCol.print(os);
	os << "\n              Live variables: ";
	useCol.print(os);
#endif
}

/*==============================================================================
 * FUNCTION:		 CallStatement::setReturnAfterCall
 * OVERVIEW:		 Sets a bit that says that this call is effectively followed by a return. This happens e.g. on
 *						Sparc when there is a restore in the delay slot of the call
 * PARAMETERS:		 b: true if this is to be set; false to clear the bit
 * RETURNS:			 <nothing>
 *============================================================================*/
void CallStatement::setReturnAfterCall(bool b) {
	returnAfterCall = b;
}

/*==============================================================================
 * FUNCTION:		 CallStatement::isReturnAfterCall
 * OVERVIEW:		 Tests a bit that says that this call is effectively followed by a return. This happens e.g. on
 *						Sparc when there is a restore in the delay slot of the call
 * PARAMETERS:		 none
 * RETURNS:			 True if this call is effectively followed by a return
 *============================================================================*/
bool CallStatement::isReturnAfterCall() {
	return returnAfterCall;
}

/*==============================================================================
 * FUNCTION:		CallStatement::clone
 * OVERVIEW:		Deep copy clone
 * PARAMETERS:		<none>
 * RETURNS:			Pointer to a new Statement, a clone of this CallStatement
 *============================================================================*/
Statement* CallStatement::clone() {
	CallStatement* ret = new CallStatement();
	ret->pDest = pDest->clone();
	ret->m_isComputed = m_isComputed;
	StatementList::iterator ss;
	for (ss = arguments.begin(); ss != arguments.end(); ++ss)
		ret->arguments.append((*ss)->clone());
	for (ss = defines.begin(); ss != defines.end(); ++ss)
		ret->defines.append((*ss)->clone());
	// Statement members
	ret->pbb = pbb;
	ret->proc = proc;
	ret->number = number;
	return ret;
}

// visit this stmt
bool CallStatement::accept(StmtVisitor* visitor) {
	return visitor->visit(this);
}

Proc* CallStatement::getDestProc() {
	return procDest; 
}

void CallStatement::setDestProc(Proc* dest) { 
	assert(dest);
	// assert(procDest == NULL);		// No: not convenient for unit testing
	procDest = dest;
}

void CallStatement::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {

	Proc *p = getDestProc();

	if (p == NULL && isComputed()) {
		hll->AddIndCallStatement(indLevel, pDest, arguments, calcResults());
		return;
	}
	StatementList* results = calcResults();

#if 0
	LOG << "call: " << this;
	LOG << " in proc " << proc->getName() << "\n";
#endif
	assert(p);
	if (Boomerang::get()->noDecompile) {
		if (procDest->getSignature()->getNumReturns() > 0) {
			Assign* as = new Assign(new IntegerType(), new Unary(opRegOf, new Const(24)), new Unary(opRegOf, new Const(24)));
			as->setProc(proc);
			as->setBB(pbb);
			results->append(as);
		}
		
		// some hacks
		if (std::string(p->getName()) == "printf" ||
			std::string(p->getName()) == "scanf") {
			for (int i = 1; i < 3; i++) {
				Exp *e = signature->getArgumentExp(i);
				assert(e);
				Location *l = dynamic_cast<Location*>(e);
				if (l) {
					l->setProc(proc);		// Needed?
				}
				Assign* as = new Assign(signature->getParamType(i), e->clone(), e->clone());
				as->setProc(proc);
				as->setBB(pbb);
				as->setNumber(number);		// So fromSSAform will work later
				arguments.append(as);
			}
		}
	}
	if (p->isLib() && *p->getSignature()->getPreferedName()) {
		// How did this ever work? Surely you need the actual, substituted-into arguments!
#if 0
		std::vector<Exp*> args;
		for (unsigned int i = 0; i < p->getSignature()->getNumPreferedParams(); i++)
			args.push_back(arguments[p->getSignature()->getPreferedParam(i)]);
		hll->AddCallStatement(indLevel, p,	p->getSignature()->getPreferedName(), args, getReturns());
#else
		hll->AddCallStatement(indLevel, p,	p->getSignature()->getPreferedName(), arguments, results);
#endif
	} else
		hll->AddCallStatement(indLevel, p, p->getName(), arguments, results);
}

void CallStatement::simplify() {
	GotoStatement::simplify();
	StatementList::iterator ss;
	for (ss = arguments.begin(); ss != arguments.end(); ++ss)
		(*ss)->simplify();
	for (ss = defines.begin(); ss != defines.end(); ++ss)
		(*ss)->simplify();
}

bool GotoStatement::usesExp(Exp* e) {
	Exp* where;
	return (pDest->search(e, where));
}

bool CallStatement::usesExp(Exp *e) {
	if (GotoStatement::usesExp(e)) return true;
	StatementList::iterator ss;
	for (ss = arguments.begin(); ss != arguments.end(); ++ss)
		if ((*ss)->usesExp(e)) return true;
	for (ss = defines.begin(); ss != defines.end(); ++ss)
		if ((*ss)->usesExp(e)) return true;
	return false;
}

void CallStatement::getDefinitions(LocationSet &defs) {
	StatementList::iterator dd;
	for (dd = defines.begin(); dd != defines.end(); ++dd)
		defs.insert(((Assignment*)*dd)->getLeft());
	// Childless calls are supposed to define everything. In practice they don't really define things like %pc, so we
	// need some extra logic in getTypeFor()
	if (isChildless() && !Boomerang::get()->assumeABI)
		defs.insert(new Terminal(opDefineAll));
}

// Attempt to convert this call, if indirect, to a direct call.
// NOTE: at present, we igore the possibility that some other statement will modify the global. This is a serious
// limitation!!
bool CallStatement::convertToDirect() {
	if (!m_isComputed)
		return false;
	bool convertIndirect = false;
	Exp *e = pDest;
	if (pDest->isSubscript()) {
		Statement* def = ((RefExp*)e)->getDef();
		if (def && !def->isImplicit())
			return false;						// If an already defined global, don't convert
		e = ((RefExp*)e)->getSubExp1();
	}
	if (e->getOper() == opArrayIndex && 
			((Binary*)e)->getSubExp2()->isIntConst() &&
			((Const*)(((Binary*)e)->getSubExp2()))->getInt() == 0)
		e = ((Binary*)e)->getSubExp1();
	// Can actually have name{0}[0]{0} !!
	if (e->isSubscript())
		e = ((RefExp*)e)->getSubExp1();
	if (e->isIntConst()) {
		// ADDRESS u = (ADDRESS)((Const*)e)->getInt();
		// Just convert it to a direct call!
		// FIXME: to be completed
	} else if (e->isMemOf()) {
		// It might be a global that has not been processed yet
		Exp* sub = ((Unary*)e)->getSubExp1();
		if (sub->isIntConst()) {
			// m[K]: convert it to a global right here
			ADDRESS u = (ADDRESS)((Const*)sub)->getInt();
			proc->getProg()->globalUsed(u);
			const char *nam = proc->getProg()->getGlobalName(u);
			e = Location::global(nam, proc);
			pDest = new RefExp(e, NULL);
		}
	}
	if (!e->isGlobal()) {
		return false;
	}
	char *nam = ((Const*)e->getSubExp1())->getStr();
	Prog* prog = proc->getProg();
	ADDRESS gloAddr = prog->getGlobalAddr(nam);
	ADDRESS dest = prog->readNative4(gloAddr);
	// We'd better do some limit checking on the value. This does not guarantee that it's a valid proc pointer, but it
	// may help
	if (dest < prog->getLimitTextLow() || dest > prog->getLimitTextHigh())
		return false;		// Not a valid proc pointer
	Proc *p = prog->findProc(nam);
	bool bNewProc = p == NULL;
	if (bNewProc)
		p = prog->setNewProc(dest);
	if (VERBOSE)
		LOG << (bNewProc ? "new" : "existing") << " procedure for call to global '" << nam << " is " << p->getName() <<
			"\n";
	// we need to:
	// 1) replace the current return set with the return set of the new procDest
	// 2) call fixCallBypass (now fixCallAndPhiRefs) on the enclosing procedure
	// 3) fix the arguments (this will only affect the implicit arguments, the regular arguments should
	//    be empty at this point)
	// 3a replace current arguments with those of the new proc
	// 3b copy the signature from the new proc
	// 4) change this to a non-indirect call
	procDest = p;
	Signature *sig = p->getSignature();
	// pDest is currently still global5{-}, but we may as well make it a constant now, since that's how it will be
	// treated now
	pDest = new Const(dest);

	// 1
#if 0	// Don't have to do this now!
	if (procDest->isLib()) {
		int i;
		returns = new ReturnStatement;
    	for (i = 0; i < sig->getNumReturns(); i++) {
			ImplicitAssign* ia = new ImplicitAssign(sig->getReturnExp(i)->clone());
    		returns->addReturn(ia);
		}
	}
	else
		if (procDest)
			returns = (RetStatement*)((UserProc*)procDest)->getTheReturnStatement()->clone();
		// else it remains as a DefineAll
#endif

	// 2
	proc->fixCallAndPhiRefs();

	// 3
#if 0
	std::vector<Exp*> &params = proc->getProg()->getDefaultParams();
	std::vector<Exp*> oldargs = implicitArguments;
	std::vector<Exp*> newimpargs;
	newimpargs.resize(sig->getNumImplicitParams(), NULL);
	for (i = 0; i < sig->getNumImplicitParams(); i++) {
		bool gotsub = false;
		for (unsigned j = 0; j < params.size(); j++)
			if (*params[j] == *sig->getImplicitParamExp(i)) {
				newimpargs[i] = oldargs[j];
				gotsub = true;
				break;
			}
		if (!gotsub) {
			newimpargs[i] =
				sig->getImplicitParamExp(i)->clone();
			if (newimpargs[i]->getOper() == opMemOf) {
				newimpargs[i]->setSubExp1(localiseExp(newimpargs[i]->getSubExp1()));
			}
		}
	}
#endif
	// 3a Do the same with the regular arguments
#if 0
	assert(arguments.size() == 0);
	std::vector<Exp*> newargs;
	newargs.resize(sig->getNumParams(), NULL);
	for (i = 0; i < sig->getNumParams(); i++) {
		bool gotsub = false;
		for (unsigned j = 0; j < params.size(); j++)
			if (*params[j] == *sig->getParamExp(i)) {
				newargs[i] = oldargs[j];
				// Got something to substitute
				gotsub = true;
				break;
			}
		if (!gotsub) {
			Exp* parami = sig->getParamExp(i);
			newargs[i] = parami->clone();
			if (newargs[i]->getOper() == opMemOf) {
				newargs[i]->setSubExp1(localiseExp(newargs[i]->getSubExp1()));
			}
		}
	}
	// change em
	arguments = newargs;
	assert((int)arguments.size() == sig->getNumParams());
#else
	arguments.clear();
	for (unsigned i = 0; i < sig->getNumParams(); i++) {
		Exp* a = sig->getParamExp(i);
		Assign* as = new Assign(new VoidType(), a->clone(), a->clone());
		as->setProc(proc);
		as->setBB(pbb);
		arguments.append(as);
	}
	// std::cerr << "Step 3a: arguments now: ";
	// StatementList::iterator xx; for (xx = arguments.begin(); xx != arguments.end(); ++xx) {
	//		((Assignment*)*xx)->printCompact(std::cerr); std::cerr << ", ";
	// } std::cerr << "\n";
#endif
	// implicitArguments = newimpargs;
	// assert((int)implicitArguments.size() == sig->getNumImplicitParams());

	// 3b
	signature = p->getSignature()->clone();

	// 4
	m_isComputed = false;
	proc->undoComputedBB(this);
	proc->addCallee(procDest);
	procDest->printDetailsXML();
	convertIndirect = true;

	if (VERBOSE)
		LOG << "Result of convertToDirect: " << this << "\n";
	return convertIndirect;
}

void CallStatement::updateArgumentWithType(int n)
{
	if (!ADHOC_TYPE_ANALYSIS) return;
	// let's set the type of arguments to the signature of the called proc
	// both locations and constants can have a type set
	if (procDest) {
		Type *ty = procDest->getSignature()->getParamType(n);
		if (ty) {
			StatementList::iterator aa = arguments.begin();
			my_advance(aa, n);
			Exp *e = ((Assign*)*aa)->getRight();
			if (e->isSubscript())
				e = e->getSubExp1();
			if (e->isLocation())
				((Location*)e)->setType(ty->clone());
			else {
				Const *c = dynamic_cast<Const*>(e);
				if (c)
					c->setType(ty->clone());
			}					
		}
	}
}

#if 0
bool CallStatement::doReplaceRef(Exp* from, Exp* to) {
	bool change = false;
	bool convertIndirect = false;
	if (procDest == NULL && pDest) {
		pDest = pDest->searchReplaceAll(from, to, change);
		if (change) {
			if (VERBOSE)
				LOG << "propagated into call dest " << pDest << "\n";
			convertIndirect = convertToDirect();
		}
	}
	unsigned int i;

	StatementList::iterator ss;
	for (ss = arguments.begin(), i=0; ss != arguments.end(); ++ss, ++i) {
		// Propagate into the RIGHT hand side of arguments
		Exp*& a = ((Assign*)*ss)->getRightRef();
		a = a->searchReplaceAll(from, to, change);
		if (change) {
			a = a->simplifyArith()->simplify();
			if (VERBOSE)
				LOG << "call doReplaceRef: updated argument " << i << " with " << a << "\n";
			updateArgumentWithType(i);
		}
		// Also substitute into m[...] on the LEFT hand side, but don't change the LHS itself.
		Exp* al = ((Assign*)*ss)->getLeft();
		if (al->isMemOf()) {
			al->searchReplaceAll(from, to, change);
			if (change) al->simplifyArith()->simplify();
		}
	}

	defCol.searchReplaceAll(from, to, change);
	return convertIndirect;
}
#endif

Exp* CallStatement::getArgumentExp(int i)
{
	assert(i < (int)arguments.size());
	StatementList::iterator aa = arguments.begin();
	my_advance(aa, i);
	return ((Assign*)*aa)->getRight();
}

void CallStatement::setArgumentExp(int i, Exp *e)
{
	assert(i < (int)arguments.size());
	StatementList::iterator aa = arguments.begin();
	my_advance(aa, i);
	Exp*& a = ((Assign*)*aa)->getRightRef();
	a = e->clone();
	updateArgumentWithType(i);
}

int CallStatement::getNumArguments()
{
	return arguments.size();
}

void CallStatement::setNumArguments(int n) {
	int oldSize = arguments.size();
	StatementList::iterator aa = arguments.begin();
	my_advance(aa, n);
	arguments.erase(aa, arguments.end());
	// MVE: check if these need extra propagation
	for (int i = oldSize; i < n; i++) {
		Exp* a = procDest->getSignature()->getArgumentExp(i);
		Assign* as = new Assign(new VoidType(), a->clone(), a->clone());
		as->setProc(proc);
		as->setBB(pbb);
		arguments.append(as);
	}
}

void CallStatement::removeArgument(int i)
{
	StatementList::iterator aa = arguments.begin();
	my_advance(aa, i);
	arguments.erase(aa);
}

// Convert from SSA form
void CallStatement::fromSSAform(igraph& ig) {
	if (pDest)
		pDest = pDest->fromSSA(ig);
	StatementList::iterator ss;
	// FIXME: do the arguments need this treatment too? Surely they can't get tangled in interferences though
	for (ss = arguments.begin(); ss != arguments.end(); ++ss)
		(*ss)->fromSSAform(ig);
	// Note that defines have statements (assignments) within a statement (this call). The fromSSA logic, which needs
	// to subscript definitions on the left with the statement pointer, won't work if we just call the assignment's
	// fromSSA() function
	for (ss = defines.begin(); ss != defines.end(); ++ss) {
		Assignment* as = ((Assignment*)*ss);
		as->setLeft(as->getLeft()->fromSSAleft(ig, this));
	}
	// Don't think we'll need this anyway:
	// defCol.fromSSAform(ig);

	// However, need modifications of the use collector; needed when say eax is renamed to local5, otherwise
	// local5 is removed from the results of the call
	useCol.fromSSAform(ig, this);
}


// Processes each argument of a CallStatement, and the RHS of an Assign. Ad-hoc type analysis only.
Exp *processConstant(Exp *e, Type *t, Prog *prog, UserProc* proc, ADDRESS stmt) {
	if (t == NULL) return e;
	NamedType *nt = NULL;
	if (t->isNamed()) {
		nt = (NamedType*)t;
		t = ((NamedType*)t)->resolvesTo();
	}
	if (t == NULL) return e;
	// char* and a constant
	if (e->isIntConst()) {
		if (nt && nt->getName() == "LPCWSTR") {
			ADDRESS u = ((Const*)e)->getAddr();
			// TODO
			LOG << "possible wide char string at " << u << "\n";
		}
		if (t->resolvesToPointer()) {
			PointerType *pt = t->asPointer();
			Type *points_to = pt->getPointsTo();
			if (t->isCString()) {
				ADDRESS u = ((Const*)e)->getAddr();
				char *str = prog->getStringConstant(u, true);
				if (str) {
					e = new Const(str);
					// Check if we may have guessed this global incorrectly (usually as an array of char)
					const char* nam = prog->getGlobalName(u);
					if (nam) prog->setGlobalType(nam,
						new PointerType(new CharType()));
				} else {
					proc->getProg()->globalUsed(u);
					const char *nam = proc->getProg()->getGlobalName(u);
					if (nam)
						e = Location::global(nam, proc);
				}
			}
			if (points_to->resolvesToFunc()) {
				ADDRESS a = ((Const*)e)->getAddr();
				if (VERBOSE || 1)
					LOG << "found function pointer with constant value " << "of type " << pt->getCtype() 
						<< " in statement at addr " << stmt << ".  Decoding address " << a << "\n";
				// the address can be zero, i.e., NULL, if so, ignore it.
				if (a != 0) {
					if (!Boomerang::get()->noDecodeChildren)
						prog->decodeEntryPoint(a);
					Proc *p = prog->findProc(a);
					if (p) {
						Signature *sig = points_to->asFunc()->getSignature()->clone();
						if (sig->getName() == NULL ||
							strlen(sig->getName()) == 0 || 
							!strcmp(sig->getName(), "<ANON>") ||
							prog->findProc(sig->getName()) != NULL)
							sig->setName(p->getName());
						else
							p->setName(sig->getName());
						p->setSignature(sig);
						e = Location::global(p->getName(), proc);
					}
				}
			}
		} else if (t->resolvesToFloat()) {
			e = new Ternary(opItof, new Const(32), new Const(t->getSize()), e);
		} 
	}
#if 0
	if (t->isPointer() && e->getOper() != opAddrOf) {
		e = new Unary(opAddrOf, Location::memOf(e));
	}
#endif
	
	return e;
}

Type* Assignment::getTypeFor(Exp* e) {
	// assert(*lhs == *e);			// No: local vs base expression
	return type;
}

void Assignment::setTypeFor(Exp* e, Type* ty) {
	// assert(*lhs == *e);
	type = ty;
	if (DEBUG_TA)
		LOG << "    Changed type of " << this << "\n";
}

// Scan the returns for e. If found, return the type associated with that return
Type* CallStatement::getTypeFor(Exp* e) {
	// The defines "cache" what the destination proc is defining
	Assignment* as = defines.findOnLeft(e);
	if (as != NULL)
		return as->getType();
	if (e->isPC())
		// Special case: just return void*
		return new PointerType(new VoidType);
#if 0	// Else, we should leave it to the bypassing propagator to reference around the call.
		// The code below assumes that the call does not modify e; if e happens to be the stack pointer, this is
		// not always true.
	// See if it is in our reaching definitions
	Exp* rdef = defCol.findDefFor(e);
	if (rdef == NULL !ref->isSubscript()) return NULL;
	Statement* def = ((RefExp*)ref)->getDef();
	if (def == NULL) return NULL;
	return def->getTypeFor(e);
#if 0
	if (procDest && procDest->isLib()) {
		Signature* sig = procDest->getSignature();
		return sig->getTypeFor(e);
	}
	if (calleeReturn == NULL) return NULL;
	return calleeReturn->getTypeFor(e);
#endif
#else
	// return NULL;
	return new VoidType;
#endif
}

void CallStatement::setTypeFor(Exp* e, Type* ty) {
	Assignment* as = defines.findOnLeft(e);
	if (as != NULL)
		return as->setType(ty);
	// See if it is in our reaching definitions
	Exp* ref = defCol.findDefFor(e);
	if (ref == NULL || !ref->isSubscript()) return;
	Statement* def = ((RefExp*)ref)->getDef();
	if (def == NULL) return;
	def->setTypeFor(e, ty);
}


// This is temporarily horrible till types are consistently stored in assignments (including implicit and BoolAssign),
// and CallStatement
Type *Statement::getTypeFor(Exp *e, Prog *prog)
{
	Type *ty = NULL;
	switch (e->getOper()) {
		case opGlobal: {
			const char *nam = ((Const*)e->getSubExp1())->getStr();
			ty = prog->getGlobalType((char*)nam);
			break;
		}
		case opLocal: {
			const char *nam = ((Const*)e->getSubExp1())->getStr();
			ty = proc->getLocalType((char*)nam);
			break;
		}
		case opMemberAccess: {
			Type *tsubexp1 = getTypeFor(e->getSubExp1(), prog);
			if (tsubexp1 && tsubexp1->resolvesToCompound()) {
				CompoundType *compound = tsubexp1->asCompound();
				const char *nam = ((Const*)e->getSubExp2())->getStr();
				ty = compound->getType((char*)nam);
			}
			break;
		}
		case opSubscript: {
			Statement* def = ((RefExp*)e)->getDef();
			ty = def->getTypeFor(e->getSubExp1(), prog);
			break;
		}
		default:
			break;
	}

	// Defined by this statement
	// Ick: this needs to be polymorphic - MVE
	switch(kind) {
		case STMT_ASSIGN:
		case STMT_PHIASSIGN:
		case STMT_IMPASSIGN:
			if (*e == *((Assignment*)this)->getLeft())
				return ((Assignment*)this)->getType();
			break;
		case STMT_CALL: {
			CallStatement* call = (CallStatement*)this;
			Proc* dest = call->getDestProc();
			if (dest && !dest->isLib()) {
				ReturnStatement* rs = ((UserProc*)dest)->getTheReturnStatement();
				if (rs) {
					ReturnStatement::iterator rr;
					for (rr = rs->begin(); rr != rs->end(); rr++) {
						Assignment* as = (Assignment*)*rr;
						if (*as->getLeft() == *e)
							// ? Why does gcc need this cast?
							return ((Statement*)as)->getTypeFor(e, prog);
					}
				}
			}
				
#if 0
			std::vector<Exp*>& rets = call->getReturns();
			std::vector<Exp*>::iterator it;
			int n = call->getNumReturns();
			for (int i=0; i < n; i++) {
				if (*e == *call->getReturnExp(i)) {
					// This call defines expression e.
					// How do we get the type?
				}
			}
#endif
					
		}
		case STMT_BOOLASSIGN:
			return new BooleanType;
			break;
		default:
			break;
	}

	// MVE: not sure if this is right
	if (ty == NULL) ty = e->getType();	// May be m[blah].member
	return ty;
}

// For ad-hoc TA only...
bool CallStatement::processConstants(Prog *prog) {
	StatementList::iterator aa;
	int i=0;
	for (aa = arguments.begin(); aa != arguments.end(); ++aa, ++i) {
		Type *t = getArgumentType(i);
		Exp *e = ((Assign*)*aa)->getRight();
	
        // check for a[m[constant]{?}], treat it like a constant
        if (e->isAddrOf() && e->getSubExp1()->isSubscript() && e->getSubExp1()->getSubExp1()->isMemOf() && 
            	e->getSubExp1()->getSubExp1()->getSubExp1()->isIntConst())
            e = e->getSubExp1()->getSubExp1()->getSubExp1();

		((Assign*)*aa)->setRight(processConstant(e, t, prog, proc, pbb->getRTLWithStatement(this)->getAddress()));
	}

	return ellipsisProcessing(prog);
}

// This function has two jobs. One is to truncate the list of arguments based on the format string.
// The second is to add parameter types to the signature.
// If -Td is used, type analysis will be rerun with these changes.
bool CallStatement::ellipsisProcessing(Prog* prog) {

	// if (getDestProc() == NULL || !getDestProc()->getSignature()->hasEllipsis())
	if (getDestProc() == NULL || !signature->hasEllipsis())
		return false;
	// functions like printf almost always have too many args
	std::string name(getDestProc()->getName());
	int format;
	if ((name == "printf" || name == "scanf")) format = 0;
	else if (name == "sprintf" || name == "fprintf" || name == "sscanf") format = 1;
	else return false;
	if (VERBOSE)
		LOG << "ellipsis processing for " << name << "\n";
	char* formatStr = NULL;
	Exp* formatExp = getArgumentExp(format);
	// We sometimes see a[m[blah{...}]]
	if (formatExp->isAddrOf()) {
		formatExp = ((Unary*)formatExp)->getSubExp1();
		if (formatExp->isSubscript())
			formatExp = ((RefExp*)formatExp)->getSubExp1();
		if (formatExp->isMemOf())
			formatExp = ((Unary*)formatExp)->getSubExp1();
	}
	if (formatExp->isSubscript()) {
		// Maybe it's defined to be a Const string
		Statement* def = ((RefExp*)formatExp)->getDef();
		if (def == NULL) return false;		// Not all NULL refs get converted to implicits
		if (def->isAssign()) {
			// This would be unusual; propagation would normally take care of this
			Exp* rhs = ((Assign*)def)->getRight();
			if (rhs == NULL || !rhs->isStrConst()) return false;
			formatStr = ((Const*)rhs)->getStr();
		} else if (def->isPhi()) {
			// More likely. Example: switch_gcc. Only need ONE candidate format string
			PhiAssign* pa = (PhiAssign*)def;
			int n = pa->getNumDefs();
			for (int i=0; i < n; i++) {
				def = pa->getStmtAt(i);
				if (def == NULL) continue;
				if (!def->isAssign()) continue;
				Exp* rhs = ((Assign*)def)->getRight();
				if (rhs == NULL || !rhs->isStrConst()) continue;
				formatStr = ((Const*)rhs)->getStr();
				break;
			}
			if (formatStr == NULL) return false;
		} else return false;
	} else if (formatExp->isStrConst()) {
		formatStr = ((Const*)formatExp)->getStr();
	} else return false;
	// actually have to parse it
	// Format string is: % [flags] [width] [.precision] [size] type
	int n = 1;		// Count the format string itself (may also be "format" more arguments)
	char ch;
	// Set a flag if the name of the function is scanf/sscanf/fscanf
	bool isScanf = name == "scanf" || name.substr(1, 5) == "scanf";
	char *p = formatStr;
	while ((p = strchr(p, '%'))) {
		p++;				// Point past the %
		bool veryLong = false;			// %lld or %L
		do {
			ch = *p++;		// Skip size and precisionA
			switch (ch) {
				case '*':
					// Example: printf("Val: %*.*f\n", width, precision, val);
					n++;		// There is an extra parameter for the width or precision
					// This extra parameter is of type integer, never int* (so pass false as last argument)
					addSigParam(new IntegerType(), false);
					continue;
				case '-': case '+': case '#': case ' ':
					// Flag. Ignore
					continue;
				case '.':
					// Separates width and precision. Ignore.
					continue;
				case 'h': case 'l':
					// size of half or long. Argument is usually still one word. Ignore.
					// Exception: %llx
					// TODO: handle architectures where l implies two words
					// TODO: at least h has implications for scanf
					if (*p == 'l') {
						// %llx
						p++;		// Skip second l
						veryLong = true;
					}
					continue;
				case 'L':
					// long. TODO: handle L for long doubles.
					// n++;		// At least chew up one more parameter so later types are correct
					veryLong = true;
					continue;
				default:
					if ('0' <= ch && ch <= '9') continue;	// width or precision
					break;									// Else must be format type, handled below
			}
			break;
		} while (1);
		if (ch != '%')		// Don't count %%
			n++;
		switch (ch) {
			case 'd': case 'i':							// Signed integer
				addSigParam(new IntegerType(veryLong ? 64 : 32), isScanf);
				break;
			case 'u': case 'x': case 'X': case 'o':		// Unsigned integer
				addSigParam(new IntegerType(32, -1), isScanf);
				break;
			case 'f': case 'g': case 'G': case 'e': case 'E':	// Various floating point formats
				addSigParam(new FloatType(veryLong ? 128 : 64), isScanf);	// Note: may not be 64 bits
																						// for some archs
				break;
			case 's':									// String
				addSigParam(new PointerType(new CharType), isScanf);
				break;
			case 'c':									// Char
				addSigParam(new CharType, isScanf);
				break;
			case '%':
				break;			// Ignore %% (emits 1 percent char)
			default:
				LOG << "Unhandled format character " << ch << " in format string for call " << this << "\n";
		}
	}
	setNumArguments(format + n);
	signature->killEllipsis();	// So we don't do this again
	return true;
}

// Make an assign suitable for use as an argument from a callee context expression
Assign* CallStatement::makeArgAssign(Type* ty, Exp* e) {
	Exp* lhs = e->clone();
	localiseComp(lhs);			// Localise the components of lhs (if needed)
	Exp* rhs = localiseExp(e->clone());
	if (ADHOC_TYPE_ANALYSIS) {
		if (lhs->isLocation()) ((Location*)lhs)->setType(ty);
		Exp* base = rhs;
		if (rhs->isSubscript())
			base = ((RefExp*)rhs)->getSubExp1();
		if (base->isLocation()) ((Location*)base)->setType(ty);
	}
	Assign* as = new Assign(ty, lhs, rhs);
	as->setProc(proc);
	as->setBB(pbb);
	// It may need implicit converting (e.g. sp{-} -> sp{0})
	Cfg* cfg = proc->getCFG();
	if (cfg->implicitsDone()) {
		ImplicitConverter ic(cfg);
		StmtImplicitConverter sm(&ic, cfg);
		as->accept(&sm);
	}
	return as;
}

// Helper function for the above
void CallStatement::addSigParam(Type* ty, bool isScanf) {
	if (isScanf) ty = new PointerType(ty);
	signature->addParameter(ty);
	Exp* paramExp = signature->getParamExp(signature->getNumParams()-1);
	if (VERBOSE)
		LOG << "  ellipsisProcessing: adding parameter " << paramExp << " of type " << ty->getCtype() << "\n";
	if (arguments.size() < (unsigned)signature->getNumParams()) {
		Assign* as = makeArgAssign(ty, paramExp);
		arguments.append(as);
	}
}

/**********************************
 * ReturnStatement methods
 **********************************/

/*==============================================================================
 * FUNCTION:		 ReturnStatement::ReturnStatement
 * OVERVIEW:		 Constructor.
 * PARAMETERS:		 None
 * RETURNS:			 <nothing>
 *============================================================================*/
ReturnStatement::ReturnStatement() : retAddr(NO_ADDRESS) {
	kind = STMT_RET;
}

/*==============================================================================
 * FUNCTION:		 ReturnStatement::~ReturnStatement
 * OVERVIEW:		 Destructor.
 * PARAMETERS:		 <none>
 * RETURNS:			 <nothing>
 *============================================================================*/
ReturnStatement::~ReturnStatement() {
}

/*==============================================================================
 * FUNCTION:		ReturnStatement::clone
 * OVERVIEW:		Deep copy clone
 * PARAMETERS:		<none>
 * RETURNS:			Pointer to a new Statement, a clone of this ReturnStatement
 *============================================================================*/
Statement* ReturnStatement::clone() {
	ReturnStatement* ret = new ReturnStatement();
	iterator rr;
	for (rr = modifieds.begin(); rr != modifieds.end(); ++rr)
		ret->modifieds.append((ImplicitAssign*)(*rr)->clone());
	for (rr = returns.begin(); rr != returns.end(); ++rr)
		ret->returns.append((Assignment*)(*rr)->clone());
	ret->retAddr = retAddr;
	ret->col.makeCloneOf(col);
	// Statement members
	ret->pbb = pbb;
	ret->proc = proc;
	ret->number = number;
	return ret;
}

// visit this stmt
bool ReturnStatement::accept(StmtVisitor* visitor) {
	return visitor->visit(this);
}

void ReturnStatement::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
	hll->AddReturnStatement(indLevel, &getReturns());
}

void ReturnStatement::simplify() {
	iterator it;
	for (it = modifieds.begin(); it != modifieds.end(); it++)
		(*it)->simplify();
	for (it = returns.begin(); it != returns.end(); it++)
		(*it)->simplify();
}

// Remove the return (if any) related to loc. Loc may or may not be subscripted
void ReturnStatement::removeReturn(Exp* loc) {
	if (loc->isSubscript())
		loc = ((Location*)loc)->getSubExp1();
	iterator rr;
	for (rr = returns.begin(); rr != returns.end(); ++rr) {
		if (*((Assignment*)*rr)->getLeft() == *loc) {
			returns.erase(rr);
			return;					// Assume only one definition
		}
	}
}

void ReturnStatement::addReturn(Assignment* a) {
	returns.append(a);
}


// Convert from SSA form
void ReturnStatement::fromSSAform(igraph& ig) {
	ReturnStatement::iterator rr;
	for (rr = begin(); rr != end(); ++rr)
		(*rr)->fromSSAform(ig);
}

bool ReturnStatement::search(Exp* search, Exp*& result) {
	result = NULL;
	ReturnStatement::iterator rr;
	for (rr = begin(); rr != end(); ++rr) {
		if ((*rr)->search(search, result))
			return true;
	}
	return false;
}

bool ReturnStatement::searchAndReplace(Exp* search, Exp* replace, bool cc) {
	bool change = false;
	ReturnStatement::iterator rr;
	for (rr = begin(); rr != end(); ++rr)
		change |= (*rr)->searchAndReplace(search, replace, cc);
	if (cc) {
		DefCollector::iterator dd;
		for (dd = col.begin(); dd != col.end(); ++dd)
			change |= (*dd)->searchAndReplace(search, replace);
	}
	return change;
}

bool ReturnStatement::searchAll(Exp* search, std::list<Exp *>& result) {
	bool found = false;
	ReturnStatement::iterator rr;
	for (rr = begin(); rr != end(); ++rr) {
		if ((*rr)->searchAll(search, result))
			found = true;
	}
	return found;
}

bool CallStatement::isDefinition() {
	LocationSet defs;
	getDefinitions(defs);
	return defs.size() != 0;
}

bool ReturnStatement::usesExp(Exp *e) {
	Exp *where;
	ReturnStatement::iterator rr;
	for (rr = begin(); rr != end(); ++rr) {
		if ((*rr)->search(e, where))
			return true;
	}
	return false;
}

/**********************************************************************
 * BoolAssign methods
 * These are for statements that set a destination (usually to 1 or 0)
 * depending in a condition code (e.g. Pentium)
 **********************************************************************/

/*==============================================================================
 * FUNCTION:		 BoolAssign::BoolAssign
 * OVERVIEW:		 Constructor.
 * PARAMETERS:		 sz: size of the assignment
 * RETURNS:			 <N/a>
 *============================================================================*/
BoolAssign::BoolAssign(int sz): Assignment(NULL), jtCond((BRANCH_TYPE)0),
  pCond(NULL), bFloat(false), size(sz) {
	kind = STMT_BOOLASSIGN;
}

/*==============================================================================
 * FUNCTION:		BoolAssign::~BoolAssign
 * OVERVIEW:		Destructor
 * PARAMETERS:		None
 * RETURNS:			N/a
 *============================================================================*/
BoolAssign::~BoolAssign() {
	if (pCond)
		;//delete pCond;
}

/*==============================================================================
 * FUNCTION:		BoolAssign::setCondType
 * OVERVIEW:		Sets the BRANCH_TYPE of this jcond as well as the flag
 *					indicating whether or not the floating point condition codes
 *					are used.
 * PARAMETERS:		cond - the BRANCH_TYPE
 *					usesFloat - this condional jump checks the floating point
 *					  condition codes
 * RETURNS:			a semantic string
 *============================================================================*/
void BoolAssign::setCondType(BRANCH_TYPE cond, bool usesFloat /*= false*/) {
	jtCond = cond;
	bFloat = usesFloat;
	setCondExpr(new Terminal(opFlags));
}

/*==============================================================================
 * FUNCTION:		BoolAssign::makeSigned
 * OVERVIEW:		Change this from an unsigned to a signed branch
 * NOTE:			Not sure if this is ever going to be used
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
void BoolAssign::makeSigned() {
	// Make this into a signed branch
	switch (jtCond)
	{
		case BRANCH_JUL : jtCond = BRANCH_JSL;	break;
		case BRANCH_JULE: jtCond = BRANCH_JSLE; break;
		case BRANCH_JUGE: jtCond = BRANCH_JSGE; break;
		case BRANCH_JUG : jtCond = BRANCH_JSG;	break;
		default:
			// Do nothing for other cases
			break;
	}
}

/*==============================================================================
 * FUNCTION:		BoolAssign::getCondExpr
 * OVERVIEW:		Return the Exp expression containing the HL condition.
 * PARAMETERS:		<none>
 * RETURNS:			a semantic string
 *============================================================================*/
Exp* BoolAssign::getCondExpr() {
	return pCond;
}

/*==============================================================================
 * FUNCTION:		BoolAssign::setCondExpr
 * OVERVIEW:		Set the Exp expression containing the HL condition.
 * PARAMETERS:		Pointer to semantic string to set
 * RETURNS:			<nothing>
 *============================================================================*/
void BoolAssign::setCondExpr(Exp* pss) {
	if (pCond) ;//delete pCond;
	pCond = pss;
}

/*==============================================================================
 * FUNCTION:		BoolAssign::print
 * OVERVIEW:		Write a text representation to the given stream
 * PARAMETERS:		os: stream
 * RETURNS:			<Nothing>
 *============================================================================*/
void BoolAssign::printCompact(std::ostream& os /*= cout*/) {
	os << "BOOL ";
	lhs->print(os);
	os << " := CC(";
	switch (jtCond) {
		case BRANCH_JE:	   os << "equals"; break;
		case BRANCH_JNE:   os << "not equals"; break;
		case BRANCH_JSL:   os << "signed less"; break;
		case BRANCH_JSLE:  os << "signed less or equals"; break;
		case BRANCH_JSGE:  os << "signed greater or equals"; break;
		case BRANCH_JSG:   os << "signed greater"; break;
		case BRANCH_JUL:   os << "unsigned less"; break;
		case BRANCH_JULE:  os << "unsigned less or equals"; break;
		case BRANCH_JUGE:  os << "unsigned greater or equals"; break;
		case BRANCH_JUG:   os << "unsigned greater"; break;
		case BRANCH_JMI:   os << "minus"; break;
		case BRANCH_JPOS:  os << "plus"; break;
		case BRANCH_JOF:   os << "overflow"; break;
		case BRANCH_JNOF:  os << "no overflow"; break;
		case BRANCH_JPAR:  os << "parity"; break;
	}
	os << ")";
	if (bFloat) os << ", float";
	os << std::endl;
	if (pCond)
		os << "High level: " << pCond << "\n";
}

/*==============================================================================
 * FUNCTION:		BoolAssign::clone
 * OVERVIEW:		Deep copy clone
 * PARAMETERS:		<none>
 * RETURNS:			Pointer to a new Statement, a clone of this BoolAssign
 *============================================================================*/
Statement* BoolAssign::clone() {
	BoolAssign* ret = new BoolAssign(size);
	ret->jtCond = jtCond;
	if (pCond) ret->pCond = pCond->clone();
	else ret->pCond = NULL;
	ret->bFloat = bFloat;
	ret->size = size;
	// Statement members
	ret->pbb = pbb;
	ret->proc = proc;
	ret->number = number;
	return ret;
}

// visit this Statement
bool BoolAssign::accept(StmtVisitor* visitor) {
	return visitor->visit(this);
}

void BoolAssign::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
	assert(lhs);
	assert(pCond);
	// lhs := (pCond) ? 1 : 0
	Assign as(lhs->clone(), new Ternary(opTern, pCond->clone(),
	  new Const(1), new Const(0)));
	hll->AddAssignmentStatement(indLevel, &as);
}

void BoolAssign::simplify() {
	if (pCond)
		condToRelational(pCond, jtCond);
}

void BoolAssign::getDefinitions(LocationSet &defs) 
{
	defs.insert(getLeft());
}

bool BoolAssign::usesExp(Exp *e)
{
	assert(lhs && pCond);
	Exp *where = 0;
	return (pCond->search(e, where) || (lhs->isMemOf() && 
		((Unary*)lhs)->getSubExp1()->search(e, where)));
}

bool BoolAssign::processConstants(Prog *prog) {
	return false;
}

bool BoolAssign::search(Exp *search, Exp *&result)
{
	assert(lhs);
	if (lhs->search(search, result)) return true;
	assert(pCond);
	return pCond->search(search, result);
}

bool BoolAssign::searchAll(Exp* search, std::list<Exp*>& result)
{
	bool ch = false;
	assert(lhs);
	if (lhs->searchAll(search, result)) ch = true;
	assert(pCond);
	return pCond->searchAll(search, result) || ch;
}

bool BoolAssign::searchAndReplace(Exp *search, Exp *replace, bool cc) {
	bool chl, chr;
	assert(pCond);
	assert(lhs);
	pCond = pCond->searchReplaceAll(search, replace, chl);
	 lhs  =	  lhs->searchReplaceAll(search, replace, chr);
	return chl | chr;
}

// Convert from SSA form
void BoolAssign::fromSSAform(igraph& ig) {
	pCond = pCond->fromSSA(ig); 
	lhs	  = lhs	 ->fromSSAleft(ig, this);
}

// This is for setting up SETcc instructions; see include/decoder.h macro SETS
void BoolAssign::setLeftFromList(std::list<Statement*>* stmts) {
	assert(stmts->size() == 1);
	Assign* first = (Assign*)stmts->front();
	assert(first->getKind() == STMT_ASSIGN);
	lhs = first->getLeft();
}

//	//	//	//
// Assign //
//	//	//	//

Assignment::Assignment(Exp* lhs) : TypingStatement(new VoidType), lhs(lhs) {}
Assignment::Assignment(Type* ty, Exp* lhs) : TypingStatement(ty), lhs(lhs) {
	if (ADHOC_TYPE_ANALYSIS) {
		Location* loc = dynamic_cast<Location*>(lhs);
		if (loc)					// For example: could be %CF! Ugh.
			loc->setType(ty);
	}
}
Assignment::~Assignment() {}

Assign::Assign(Exp* lhs, Exp* rhs, Exp* guard)
  : Assignment(lhs), rhs(rhs), guard(guard) {
	kind = STMT_ASSIGN;
}

Assign::Assign(Type* ty, Exp* lhs, Exp* rhs, Exp* guard)
  : Assignment(ty, lhs), rhs(rhs), guard(guard) 
{
	kind = STMT_ASSIGN;
}
Assign::Assign(Assign& o) : Assignment(lhs->clone()) {
	kind = STMT_ASSIGN;
	rhs = o.rhs->clone();
	if (o.type)	 type  = o.type->clone();  else type  = NULL;
	if (o.guard) guard = o.guard->clone(); else guard = NULL;
}
ImplicitAssign::ImplicitAssign(ImplicitAssign& o) : Assignment(lhs->clone()) {
	kind = STMT_IMPASSIGN;
}
// The first virtual function (here the destructor) can't be in statement.h file for gcc
ImplicitAssign::~ImplicitAssign() { }

Statement* Assign::clone() {
	Assign* a = new Assign(type == NULL ? NULL : type->clone(), lhs->clone(), rhs->clone(),
		guard == NULL ? NULL : guard->clone());
	// Statement members
	a->pbb = pbb;
	a->proc = proc;
	a->number = number;
	return a;
}

Statement* PhiAssign::clone() {
	PhiAssign* pa = new PhiAssign(type, lhs);
	Definitions::iterator dd;
	for (dd = defVec.begin(); dd != defVec.end(); dd++) {
		PhiInfo pi;
		pi.def = dd->def;			// Don't clone the Statement pointer (never moves)
		pi.e = dd->e->clone();		// Do clone the expression pointer
		pa->defVec.push_back(pi);
	}
	return pa;
}

Statement* ImplicitAssign::clone() {
	ImplicitAssign* ia = new ImplicitAssign(type, lhs);
	return ia;
}

// visit this Statement
bool Assign::accept(StmtVisitor* visitor) {
	return visitor->visit(this);
}
bool PhiAssign::accept(StmtVisitor* visitor) {
	return visitor->visit(this);
}
bool ImplicitAssign::accept(StmtVisitor* visitor) {
	return visitor->visit(this);
}

void Assign::simplify() {
	// simplify arithmetic of assignment
	OPER leftop = lhs->getOper();
	if (Boomerang::get()->noBranchSimplify) {
		if (leftop == opZF || leftop == opCF || leftop == opOF || leftop == opNF)
			return;
	}

	// this is a very complex pattern :)
	// replace:	 1 r31 = a			 where a is an array pointer
	//			 2 r31 = phi{1 4}
	//			 3 m[r31{2}] = x
	//			 4 r31 = r31{2} + b	 where b is the size of the base of the 
	//								 array pointed at by a
	// with:	 1 r31 = 0
	//			 2 r31 = phi{1 4}
	//			 3 m[a][r31{2}] = x
	//			 4 r31 = r31{2} + 1
	// I just assume this can only happen in a loop.. 
	if (leftop == opMemOf && lhs->getSubExp1()->getOper() == opSubscript &&
			((RefExp*)lhs->getSubExp1())->getDef() && ((RefExp*)lhs->getSubExp1())->getDef()->isPhi()) {
		Statement *phistmt = ((RefExp*)lhs->getSubExp1())->getDef();
		PhiAssign *phi = (PhiAssign*)phistmt;
		if (phi->getNumDefs() == 2 && phi->getStmtAt(0) && phi->getStmtAt(1) &&
				phi->getStmtAt(0)->isAssign() && phi->getStmtAt(1)->isAssign()) {
			Assign *a1 = (Assign*)phi->getStmtAt(0);
			Assign *a4 = (Assign*)phi->getStmtAt(1);
			if (a1->getRight()->getType() && a4->getRight()->getOper() == opPlus &&
					a4->getRight()->getSubExp1()->getOper() == opSubscript &&
					((RefExp*)a4->getRight()->getSubExp1())->getDef() == phistmt &&
					*a4->getRight()->getSubExp1()->getSubExp1() == *phi->getLeft() &&
					a4->getRight()->getSubExp2()->getOper() == opIntConst) {
				Type *ty = a1->getRight()->getType();
				unsigned b = ((Const*)a4->getRight()->getSubExp2())->getInt();
				if (ty->resolvesToPointer()) {
					ty = ty->asPointer()->getPointsTo();
					if (ty->resolvesToArray() && b*8 == ty->asArray()->getBaseType()->getSize()) {
						if (VERBOSE)
							LOG << "doing complex pattern on " << this << " using " << a1 << " and " << a4 << "\n";
						((Const*)a4->getRight()->getSubExp2())->setInt(1);
						lhs = new Binary(opArrayIndex, 
							Location::memOf(a1->getRight()->clone(), proc), 
							lhs->getSubExp1()->clone());
						a1->setRight(new Const(0));
						if (VERBOSE)
							LOG << "replaced with " << this << " using " << a1 << " and " << a4 << "\n";
					}
				}
			}
		}
	}

	lhs = lhs->simplifyArith();
	rhs = rhs->simplifyArith();
	if (guard) guard = guard->simplifyArith();
	// simplify the resultant expression
	lhs = lhs->simplify();
	rhs = rhs->simplify();
	if (guard) guard = guard->simplify();

	// Perhaps the guard can go away
	if (guard && (guard->isTrue() || guard->isIntConst() && ((Const*)guard)->getInt() == 1))
		guard = NULL;			// No longer a guarded assignment

	if (lhs->getOper() == opMemOf) {
		lhs->setSubExp1(lhs->getSubExp1()->simplifyArith());
	}

	// this hack finds address constants.. it should go away when Mike writes some decent type analysis.
	if (DFA_TYPE_ANALYSIS) return;
	if (lhs->getOper() == opMemOf && lhs->getSubExp1()->getOper() == opSubscript) {
		RefExp *ref = (RefExp*)lhs->getSubExp1();
		Statement *phist = ref->getDef();
		PhiAssign *phi = NULL;
		if (phist /* && phist->getRight() */)		// ?
			phi = dynamic_cast<PhiAssign*>(phist);
		for (int i = 0; phi && i < phi->getNumDefs(); i++) 
			if (phi->getStmtAt(i)) {
				Assign *def = dynamic_cast<Assign*>(phi->getStmtAt(i));
				// Look for rX{-} - K or K
				if (def && (def->rhs->isIntConst() ||
						(def->rhs->getOper() == opMinus &&
						def->rhs->getSubExp1()->isSubscript() &&
						((RefExp*)def->rhs->getSubExp1())->isImplicitDef() &&
						def->rhs->getSubExp1()->getSubExp1()->isRegOf() &&
						def->rhs->getSubExp2()->isIntConst()))) {
					Exp *ne = new Unary(opAddrOf, Location::memOf(def->rhs, proc)); 
					if (VERBOSE)
						LOG << "replacing " << def->rhs << " with " << ne << " in " << def << "\n";
					def->rhs = ne;
				}
				if (def && def->rhs->getOper() == opAddrOf &&
						def->rhs->getSubExp1()->getOper() == opSubscript &&
						def->rhs->getSubExp1()->getSubExp1()->getOper() == opGlobal &&
						// MVE: opPhi!!
						rhs->getOper() != opPhi && rhs->getOper() != opItof &&
						rhs->getOper() != opFltConst) {
					Type *ty = proc->getProg()->getGlobalType(
								 ((Const*)def->rhs->getSubExp1()->
													getSubExp1()->
													getSubExp1())->getStr());
					if (ty && ty->isArray()) {
						Type *bty = ((ArrayType*)ty)->getBaseType();
						if (bty->isFloat()) {
							if (VERBOSE)
								LOG << "replacing " << rhs << " with ";
							rhs = new Ternary(opItof, new Const(32), new Const(bty->getSize()), rhs);
							if (VERBOSE)
								LOG << rhs << " (assign indicates float type)\n";
						}
					}
				}
			}
	}

	if (!ADHOC_TYPE_ANALYSIS) return;
	// let's gather some more accurate type information
	if (lhs->isLocation() && rhs->getType()) {
		Location *llhs = dynamic_cast<Location*>(lhs);
		assert(llhs);
		Type *ty = rhs->getType();
		llhs->setType(ty);
		if (DEBUG_TA)
			LOG << "setting type of " << llhs << " to " << ty->getCtype() << "\n";
	}

	if (lhs->isLocation() && rhs->isIntConst() && (lhs->getType() == NULL || lhs->getType()->isVoid())) {
		Location *llhs = dynamic_cast<Location*>(lhs);
		assert(llhs);
		Type* ty;
		if (type)
			ty = new IntegerType(type->getSize());
		else
			ty = new IntegerType();
		llhs->setType(ty);
		if (DEBUG_TA)
			LOG << "setting type of " << llhs << " to " << ty->getCtype() << "\n";
	}

	if (lhs->getType() && lhs->getType()->isFloat() && rhs->getOper() == opIntConst) {
		if (lhs->getType()->getSize() == 32) {
			unsigned n = ((Const*)rhs)->getInt();
			rhs = new Const(*(float*)&n);
		}
	}

	if (lhs->getType() && lhs->getType()->isArray() && lhs->getType()->getSize() > 0) {
		lhs = new Binary(opArrayIndex, lhs, new Const(0));
	}
}

void Assign::simplifyAddr() {
	lhs = lhs->simplifyAddr();
	rhs = rhs->simplifyAddr();
}

void Assignment::simplifyAddr() {
	lhs = lhs->simplifyAddr();
}


void Assign::fixSuccessor() {
	lhs = lhs->fixSuccessor();
	rhs = rhs->fixSuccessor();
}

void Assignment::print(std::ostream& os) {
	os << std::setw(4) << std::dec << number << " ";
	printCompact(os);
}
void Assign::printCompact(std::ostream& os) {
	os << "*" << type << "* ";
	if (guard) 
		os << guard << " => ";
	if (lhs) lhs->print(os);
	os << " := ";
	if (rhs) rhs->print(os);
}
void PhiAssign::printCompact(std::ostream& os) {
	os << "*" << type << "* ";
	if (lhs) lhs->print(os);
	os << " := phi";
	// Print as lhs := phi{9 17} for the common case where the lhs is the same location as all the referenced
	// locations. When not, print as local4 := phi(r24{9} argc{17})
	bool simple = true;
	int i, n = defVec.size();
	if (n != 0) {
		for (i = 0; i < n; i++) {
			// If e is NULL assume it is meant to match lhs
			if (defVec[i].e == NULL) continue;
			if (! (*defVec[i].e == *lhs)) {
				// One of the phi parameters has a different base expression to lhs. Use non simple print.
				simple = false;
				break;
			}
		}
	}
	iterator it;
	if (simple) {
		os << "{" << std::dec;
		for (it = defVec.begin(); it != defVec.end(); /* no increment */) {
			if (it->def)
				os << it->def->getNumber();
			else
				os << "-";
			if (++it != defVec.end())
				os << " ";
		}
		os << "}";
	} else {
		os << "(";
		for (it = defVec.begin(); it != defVec.end(); /* no increment */) {
			Exp* e = it->e;
			if (e == NULL)
				os << "NULL{";
			else
				os << e << "{";
			if (it->def)
				os << std::dec << it->def->getNumber();
			else
				os << "-";
			os << "}";
			if (++it != defVec.end())
				os << " ";
		}
		os << ")";
	}
}
void ImplicitAssign::printCompact(std::ostream& os) {
	os << "*" << type << "* ";
	if (lhs) lhs->print(os);
	os << " := -";
}



// All the Assignment-derived classes have the same definitions: the lhs
void Assignment::getDefinitions(LocationSet &defs) {
	if (lhs->getOper() == opAt)							// foo@[m:n] really only defines foo
		defs.insert(((Ternary*)lhs)->getSubExp1());
	else
		defs.insert(lhs);
	// Special case: flag calls define %CF (and others)
	if (lhs->isFlags()) {
		defs.insert(new Terminal(opCF));
	}
#if 0		// No! Use -O instead
	Location *loc = dynamic_cast<Location*>(lhs);
	if (loc)
		// This is to call a hack to define ax when eax defined, etc
		loc->getDefinitions(defs);
#endif
}

bool Assign::search(Exp* search, Exp*& result) {
	if (lhs->search(search, result))
		return true;
	return rhs->search(search, result);
}
bool PhiAssign::search(Exp* search, Exp*& result) {
	return lhs->search(search, result);
}
bool ImplicitAssign::search(Exp* search, Exp*& result) {
	return lhs->search(search, result);
}

bool Assign::searchAll(Exp* search, std::list<Exp*>& result) {
	bool res;
	std::list<Exp*> leftResult;
	std::list<Exp*>::iterator it;
	res = lhs->searchAll(search, leftResult);
	// Ugh: searchAll clears the list!
	res |= rhs->searchAll(search, result);
	for (it = leftResult.begin(); it != leftResult.end(); it++)
		result.push_back(*it);
	return res;
}
// FIXME: is this the right semantics for searching a phi statement, disregarding the RHS?
bool PhiAssign::searchAll(Exp* search, std::list<Exp*>& result) {
	return lhs->searchAll(search, result);
}
bool ImplicitAssign::searchAll(Exp* search, std::list<Exp*>& result) {
	return lhs->searchAll(search, result);
}

bool Assign::searchAndReplace(Exp* search, Exp* replace, bool cc) {
	bool chl, chr, chg = false;
	lhs = lhs->searchReplaceAll(search, replace, chl);
	rhs = rhs->searchReplaceAll(search, replace, chr);
	if (guard)
		guard = guard->searchReplaceAll(search, replace, chg);
	return chl | chr | chg;
}
bool PhiAssign::searchAndReplace(Exp* search, Exp* replace, bool cc) {
	bool change;
	lhs = lhs->searchReplaceAll(search, replace, change);
	std::vector<PhiInfo>::iterator it;
	for (it = defVec.begin(); it != defVec.end(); it++) {
		if (it->e == NULL) continue;
		bool ch;
		// Assume that the definitions will also be replaced
		it->e = it->e->searchReplaceAll(search, replace, ch);
		change |= ch;
	}
	return change;
}
bool ImplicitAssign::searchAndReplace(Exp* search, Exp* replace, bool cc) {
	bool change;
	lhs = lhs->searchReplaceAll(search, replace, change);
	return change;
}

void Assign::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
	hll->AddAssignmentStatement(indLevel, this);
}


int Assign::getMemDepth() {
	int d1 = lhs->getMemDepth();
	int d2 = rhs->getMemDepth();
	if (d1 >= d2) return d1;
	return d2;
}

void Assign::fromSSAform(igraph& ig) {
	lhs = lhs->fromSSAleft(ig, this);
	rhs = rhs->fromSSA(ig);
}

void Assignment::fromSSAform(igraph& ig) {
	lhs = lhs->fromSSAleft(ig, this);
}

void PhiAssign::fromSSAform(igraph& ig) {
	lhs = lhs->fromSSAleft(ig, this);
	iterator it;
	igraph::iterator gg;
	for (it = defVec.begin(); it != defVec.end(); it++) {
		if (it->e == NULL) continue;
		RefExp* r = new RefExp(it->e, it->def);
		gg = ig.find(r);
		if (gg != ig.end()) {
			it->e = gg->second;
		}
	}
}

// PhiExp and ImplicitExp:
bool Assignment::usesExp(Exp* e) {
	Exp *where = 0;
	return (lhs->isMemOf() || lhs->isRegOf()) && ((Unary*)lhs)->getSubExp1()->search(e, where);
}

bool Assign::usesExp(Exp *e) {
	Exp *where = 0;
	return (rhs->search(e, where) || ((lhs->isMemOf() || lhs->isRegOf()) && 
		((Unary*)lhs)->getSubExp1()->search(e, where)));
}

#if 0
bool Assign::doReplaceRef(Exp* from, Exp* to) {
	bool changeright = false;
	rhs = rhs->searchReplaceAll(from, to, changeright);
	bool changeleft = false;
	// If LHS is a memof or regof, substitute its subexpression as well
	if (lhs->isMemOf() || lhs->isRegOf()) {
		Exp* subsub1 = ((Unary*)lhs)->getSubExp1();
		((Unary*)lhs)->setSubExp1ND(subsub1->searchReplaceAll(from, to, changeleft));
	}
	//assert(changeright || changeleft);	// Check this
	if (!changeright && !changeleft) {
		// Could be propagating %flags into %CF
		Exp* baseFrom = ((RefExp*)from)->getSubExp1();
		if (baseFrom->isFlags()) {
			Assign* def = (Assign*)((RefExp*)from)->getDef();
			assert(def->isAssign());
			Exp* defRhs = def->getRight();
			assert(defRhs->isFlagCall());
			char* str = ((Const*)((Binary*)defRhs)->getSubExp1())->getStr();
			if (strncmp("SUBFLAGS", str, 8) == 0) {
				/* When the carry flag is used bare, and was defined in a subtract of the form lhs - rhs, then CF has
				   the value (lhs <u rhs).  lhs and rhs are the first and second parameters of the flagcall.
				   Note: the flagcall is a binary, with a Const (the name) and a list of expressions:
					 defRhs
					 /	  \
				Const	   opList
				"SUBFLAGS"	/	\
						   P1	opList
								 /	 \
								P2	opList
									 /	 \
									P3	 opNil
				*/
				Exp* e = new Binary(opLessUns,
					((Binary*)defRhs)->getSubExp2()->getSubExp1(),
					((Binary*)defRhs)->getSubExp2()->getSubExp2()->getSubExp1());
				rhs = rhs->searchReplaceAll(new RefExp(new Terminal(opCF), def), e, changeright);
			}
		}
	}
#if 0
	if (!changeright && !changeleft) {
		if (VERBOSE) {
			// This can happen all too frequently now with CallStatements and ReturnStatements containing multiple
			// assignments
			LOG << "could not change " << from << " to " << to << " in " << this << " !!\n";
		}
	}
#endif
	if (changeright) {
		// simplify the expression
		rhs = rhs->simplifyArith()->simplify();
	}
	if (changeleft) {
		lhs = lhs->simplifyArith()->simplify();
	}
	return false;
}

bool Assignment::doReplaceRef(Exp* from, Exp* to) {
	bool change = false;
	// If LHS is a memof or regof, substitute its subexpression
	if (lhs->isMemOf() || lhs->isRegOf()) {
		Exp* subsub1 = ((Unary*)lhs)->getSubExp1();
		((Unary*)lhs)->setSubExp1ND(subsub1->searchReplaceAll(from, to, change));
	}
	if (change)
		lhs = lhs->simplifyArith()->simplify();
	return false;
}
#endif

// Not sure if anything needed here
// MVE: check if can be deleted
bool Assign::processConstants(Prog* prog) {
#if 0
	LOG << "processing constants in assign lhs: " << lhs << " type: ";
	Type *ty = getTypeFor(lhs, prog);
	if (ty)
		LOG << ty->getCtype() << "\n";
	else
		LOG << "none\n";
#endif
	rhs = processConstant(rhs, Statement::getTypeFor(lhs, prog), prog, proc, pbb->getRTLWithStatement(this)->getAddress());
	return false;
}

bool PhiAssign::processConstants(Prog* prog) {
	return false;
}
bool ImplicitAssign::processConstants(Prog* prog) {
	return false;
}

void Assignment::genConstraints(LocationSet& cons) {
	// Almost every assignment has at least a size from decoding
	// MVE: do/will PhiAssign's have a valid type? Why not?
	if (type)
		cons.insert(new Binary(opEquals,
			new Unary(opTypeOf,
				new RefExp(lhs, this)),
			new TypeVal(type)));
}

// generate constraints
void Assign::genConstraints(LocationSet& cons) {
	Assignment::genConstraints(cons);	// Gen constraint for the LHS
	Exp* con = rhs->genConstraints(
		new Unary(opTypeOf,
			new RefExp(lhs->clone(), this)));
	if (con) cons.insert(con);
}

void PhiAssign::genConstraints(LocationSet& cons) {
	// Generate a constraints st that all the phi's have to be the same type as
	// result
	Exp* result = new Unary(opTypeOf, new RefExp(lhs, this));
	Definitions::iterator uu;
	for (uu = defVec.begin(); uu != defVec.end(); uu++) {
		Exp* conjunct = new Binary(opEquals,
			result,
			new Unary(opTypeOf,
				new RefExp(uu->e, uu->def)));
		cons.insert(conjunct);
	}
}

void CallStatement::genConstraints(LocationSet& cons) {
	Proc* dest = getDestProc();
	if (dest == NULL) return;
	Signature* destSig = dest->getSignature();
	// Generate a constraint for the type of each actual argument to be equal to the type of each formal parameter
	// (hopefully, these are already calculated correctly; if not, we need repeat till no change)
	StatementList::iterator aa;
	int p = 0;
	for (aa = arguments.begin(); aa != arguments.end(); ++aa, ++p) {
		Exp* arg = ((Assign*)*aa)->getRight();
		// Handle a[m[x]]
		if (arg->isAddrOf()) {
			Exp* sub = arg->getSubExp1();
			if (sub->isSubscript())
				sub = ((RefExp*)sub)->getSubExp1();
			if (sub->isMemOf())
				arg = ((Location*)sub)->getSubExp1();
		}
		if (arg->isRegOf() || arg->isMemOf() || arg->isSubscript() || arg->isLocal() || arg->isGlobal()) {
			Exp* con = new Binary(opEquals,
				new Unary(opTypeOf, arg->clone()),
				new TypeVal(destSig->getParamType(p)->clone()));
			cons.insert(con);
		}
	}

	if (dest->isLib()) {
		// A library procedure... check for two special cases
		std::string name = dest->getName();
		// Note: might have to chase back via a phi statement to get a sample
		// string
		char* str;
		Exp* arg0 = ((Assign*)*arguments.begin())->getRight();
		if ((name == "printf" || name == "scanf") && (str = arg0->getAnyStrConst()) != NULL) {
			// actually have to parse it
			int n = 1;		// Number of %s plus 1 = number of args
			char* p = str;
			while ((p = strchr(p, '%'))) {
				p++;
				Type* t = NULL;
				int longness = 0;
				bool sign = true;
				bool cont;
				do {
					cont = false;
					switch(*p) {
						case 'u':
							sign = false;
							cont = true;
							break;
						case 'x':
							sign = false;
							// Fall through
						case 'i':
						case 'd': {
							int size = 32;
							// Note: the following only works for 32 bit code or where sizeof(long) == sizeof(int)
							if (longness == 2) size = 64;
							t = new IntegerType(size, sign);
							break;
						}
						case 'f':
						case 'g':
							t = new FloatType(64);
							break;
						case 's':
							t = new PointerType(new CharType());
							break;
						case 'l':
							longness++;
							cont = true;
							break;
						case '.':
							cont = true;
							break;
						case '*':
							assert(0);	// Star format not handled yet
						default:
							if (*p >= '0' && *p <= '9')
								cont = true;
							break;
					}
					p++;
				} while (cont);
				if (t) {
					// scanf takes addresses of these
					if (name == "scanf")
						t = new PointerType(t);
					// Generate a constraint for the parameter
					TypeVal* tv = new TypeVal(t);
					StatementList::iterator aa = arguments.begin();
					my_advance(aa, n);
					Exp* argn = ((Assign*)*aa)->getRight();
					Exp* con = argn->genConstraints(tv);
					cons.insert(con);
				}
				n++;
			}
		}
	}
}

#if 0	// Get rid of this: MVE
void Exp::genConditionConstraints(LocationSet& cons) {
	// cond should be of the form a relop b, where relop is opEquals, opLess etc
	assert(getArity() == 2);
	Exp* a = ((Binary*)this)->getSubExp1();
	Exp* b = ((Binary*)this)->getSubExp2();
	// Constraint typeof(a) == typeof(b)
	// Generate constraints for a and b separately (if any)
	Exp* Ta; Exp* Tb;
	// MVE: are there other times when this is needed?
	if (a->isSizeCast()) {
		Ta = new Unary(opTypeOf, ((Binary*)a)->getSubExp2());
		Exp* con = a->genConstraints(Ta);
		if (con && !con->isTrue()) cons.insert(con);
	} else
		Ta = new Unary(opTypeOf, a);
	if (b->isSizeCast()) {
		Tb = new Unary(opTypeOf, ((Binary*)b)->getSubExp2());
		Exp* con = b->genConstraints(Tb);
		if (con && !con->isTrue()) cons.insert(con);
	} else
		Tb = new Unary(opTypeOf, b);
	Exp* equ = new Binary(opEquals, Ta, Tb);
	cons.insert(equ);
}
#endif

void BranchStatement::genConstraints(LocationSet& cons) {
	if (pCond == NULL && VERBOSE) {
		LOG << "Warning: BranchStatment " << number << " has no condition expression!\n";
		return;
	}
	Type* opsType;
	if (bFloat)
		opsType = new FloatType(0);
	else
		opsType = new IntegerType(0);
	if (  jtCond == BRANCH_JUGE || jtCond == BRANCH_JULE ||
		  jtCond == BRANCH_JUG || jtCond == BRANCH_JUL) {
		assert(!bFloat);
		((IntegerType*)opsType)->bumpSigned(-1);
	} else if (jtCond == BRANCH_JSGE || jtCond == BRANCH_JSLE ||
			   jtCond == BRANCH_JSG	 || jtCond == BRANCH_JSL) {
		assert(!bFloat);
		((IntegerType*)opsType)->bumpSigned(+1);
	}

	// Constraints leading from the condition
	assert(pCond->getArity() == 2);
	Exp* a = ((Binary*)pCond)->getSubExp1();
	Exp* b = ((Binary*)pCond)->getSubExp2();
	// Generate constraints for a and b separately (if any).  Often only need a size, since we get basic type and
	// signedness from the branch condition (e.g. jump if unsigned less)
	Exp* Ta; Exp* Tb;
	if (a->isSizeCast()) {
		opsType->setSize(((Const*)((Binary*)a)->getSubExp1())->getInt());
		Ta = new Unary(opTypeOf, ((Binary*)a)->getSubExp2());
	} else
		Ta = new Unary(opTypeOf, a);
	if (b->isSizeCast()) {
		opsType->setSize(((Const*)((Binary*)b)->getSubExp1())->getInt());
		Tb = new Unary(opTypeOf, ((Binary*)b)->getSubExp2());
	} else
		Tb = new Unary(opTypeOf, b);
	// Constrain that Ta == opsType and Tb == opsType
	Exp* con = new Binary(opEquals, Ta, new TypeVal(opsType));
	cons.insert(con);
		 con = new Binary(opEquals, Tb, new TypeVal(opsType));
	cons.insert(con);
}

int Statement::setConscripts(int n) {
	StmtConscriptSetter scs(n, false);
	accept(&scs);
	return scs.getLast();
}

void Statement::clearConscripts() {
	StmtConscriptSetter scs(0, true);
	accept(&scs);
}

// Cast the constant num to be of type ty. Return true if a change made
bool Statement::castConst(int num, Type* ty) {
	ExpConstCaster ecc(num, ty);
	StmtModifier scc(&ecc);
	accept(&scc);
	return ecc.isChanged();
}

void Statement::stripSizes() {
	SizeStripper ss;
	StmtModifier sm(&ss);
	accept(&sm);
}

// Visiting from class StmtExpVisitor
// Visit all the various expressions in a statement
bool Assign::accept(StmtExpVisitor* v) {
	bool override;
	bool ret = v->visit(this, override);
	if (override)
		// The visitor has overridden this functionality.  This is needed for example in UsedLocFinder, where the lhs of
		// an assignment is not used (but if it's m[blah], then blah is used)
		return ret;
	if (ret && lhs) ret = lhs->accept(v->ev);
	if (ret && rhs) ret = rhs->accept(v->ev);
	return ret;
}

bool PhiAssign::accept(StmtExpVisitor* v) {
	bool override;
	bool ret = v->visit(this, override);
	if (override) return ret;
	if (ret && lhs) ret = lhs->accept(v->ev);
	return ret;
}

bool ImplicitAssign::accept(StmtExpVisitor* v) {
	bool override;
	bool ret = v->visit(this, override);
	if (override) return ret;
	if (ret && lhs) ret = lhs->accept(v->ev);
	return ret;
}


bool GotoStatement::accept(StmtExpVisitor* v) {
	bool override;
	bool ret = v->visit(this, override);
	if (override) return ret;
	if (ret && pDest)
		ret = pDest->accept(v->ev);
	return ret;
}

bool BranchStatement::accept(StmtExpVisitor* v) {
	bool override;
	bool ret = v->visit(this, override);
	if (override) return ret;
	// Destination will always be a const for X86, so the below will never be used in practice
	if (ret && pDest)
		ret = pDest->accept(v->ev);
	if (ret && pCond)
		ret = pCond->accept(v->ev);
	return ret;
}

bool CaseStatement::accept(StmtExpVisitor* v) {
	bool override;
	bool ret = v->visit(this, override);
	if (override) return ret;
	if (ret && pDest)
		ret = pDest->accept(v->ev);
	if (ret && pSwitchInfo && pSwitchInfo->pSwitchVar)
		ret = pSwitchInfo->pSwitchVar->accept(v->ev);
	return ret;
}

bool CallStatement::accept(StmtExpVisitor* v) {
	bool override;
	bool ret = v->visit(this, override);
	if (override) return ret;
	if (ret && pDest)
		ret = pDest->accept(v->ev);
	StatementList::iterator it;
	for (it = arguments.begin(); ret && it != arguments.end(); it++)
		ret = (*it)->accept(v);
	// FIXME: why aren't defines counted?
#if 0		// Do we want to accept visits to the defines? Not sure now...
	std::vector<ReturnInfo>::iterator rr;
	for (rr = defines.begin(); ret && rr != defines.end(); rr++)
		if (rr->e)			// Can be NULL now to line up with other returns
			ret = rr->e->accept(v->ev);
#endif
	// FIXME: should be count in the collectors?
	return ret;
}

bool ReturnStatement::accept(StmtExpVisitor* v) {
	bool override;
	ReturnStatement::iterator rr;
	if (!v->visit(this, override))
		return false;
	if (override) return true;
	if (!v->isIgnoreCol()) {
		DefCollector::iterator dd;
		for (dd = col.begin(); dd != col.end(); ++dd)
			if (!(*dd)->accept(v))
				return false;
		// EXPERIMENTAL: for now, count the modifieds as if they are a collector (so most, if not all of the time,
		// ignore them). This is so that we can detect better when a definition is used only once, and therefore
		// propagate anything to it
		for (rr = modifieds.begin(); rr != modifieds.end(); ++rr)
			if (!(*rr)->accept(v))
				return false;
	}
	for (rr = returns.begin(); rr != returns.end(); ++rr)
		if (!(*rr)->accept(v))
			return false;
	return true;
}

bool BoolAssign::accept(StmtExpVisitor* v) {
	bool override;
	bool ret = v->visit(this, override);
	if (override) return ret;
	if (ret && pCond)
		ret = pCond->accept(v->ev);
	return ret;
}

// Visiting from class StmtModifier
// Modify all the various expressions in a statement
bool Assign::accept(StmtModifier* v) {
	bool recur;
	v->visit(this, recur);
	v->mod->clearMod();
	if (recur) lhs = lhs->accept(v->mod);
	if (recur) rhs = rhs->accept(v->mod);
	if (VERBOSE && v->mod->isMod())
		LOG << "Assignment changed: now " << this << "\n";
	return true;
}
bool PhiAssign::accept(StmtModifier* v) {
	bool recur;
	v->visit(this, recur);
	v->mod->clearMod();
	if (recur) lhs = lhs->accept(v->mod);
	if (VERBOSE && v->mod->isMod())
		LOG << "PhiAssign changed: now " << this << "\n";
	return true;
}

bool ImplicitAssign::accept(StmtModifier* v) {
	bool recur;
	v->visit(this, recur);
	v->mod->clearMod();
	if (recur) lhs = lhs->accept(v->mod);
	if (VERBOSE && v->mod->isMod())
		LOG << "ImplicitAssign changed: now " << this << "\n";
	return true;
}

bool BoolAssign::accept(StmtModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (pCond && recur)
		pCond = pCond->accept(v->mod);
	if (recur && lhs->isMemOf()) {
        ((Location*)lhs)->setSubExp1(((Location*)lhs)->getSubExp1()->accept(v->mod));
	}
	return true;
}


bool GotoStatement::accept(StmtModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (pDest && recur)
		pDest = pDest->accept(v->mod);
	return true;
}

bool BranchStatement::accept(StmtModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (pDest && recur)
		pDest = pDest->accept(v->mod);
	if (pCond && recur)
		pCond = pCond->accept(v->mod);
	return true;
}

bool CaseStatement::accept(StmtModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (pDest && recur)
		pDest = pDest->accept(v->mod);
	if (pSwitchInfo && pSwitchInfo->pSwitchVar && recur)
		pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->accept(v->mod);
	return true;
}

bool CallStatement::accept(StmtModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (!recur) return true;
	if (pDest)
		pDest = pDest->accept(v->mod);
	StatementList::iterator it;
	for (it = arguments.begin(); recur && it != arguments.end(); it++)
		(*it)->accept(v);
	// For example: needed for CallBypasser so that a collected definition that happens to be another call gets
	// adjusted
	// I'm thinking no at present... let the bypass and propagate while possible logic take care of it, and leave the
	// collectors as the rename logic set it
#if 0
	Collector::iterator cc;
	for (cc = defCol.begin(); cc != defCol.end(); cc++)
		(*cc)->accept(v->mod);			// Always refexps, so never need to change Exp pointer (i.e. don't need *cc = )
#endif
	StatementList::iterator dd;
	for (dd = defines.begin(); recur && dd != defines.end(); ++dd)
		(*dd)->accept(v);
	return true;
}

bool ReturnStatement::accept(StmtModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (!recur) return true;
	if (!v->ignoreCollector()) {
		DefCollector::iterator dd;
		for (dd = col.begin(); dd != col.end(); ++dd)
			if (!(*dd)->accept(v))
				return false;
	}
	ReturnStatement::iterator rr;
	for (rr = modifieds.begin(); rr != modifieds.end(); ++rr)
		if (!(*rr)->accept(v))
			return false;
	for (rr = returns.begin(); rr != returns.end(); ++rr)
		if (!(*rr)->accept(v))
			return false;
	return true;
}

// Visiting from class StmtPartModifier
// Modify all the various expressions in a statement, except for the top level of the LHS of assignments
bool Assign::accept(StmtPartModifier* v) {
	bool recur;
	v->visit(this, recur);
	v->mod->clearMod();
	if (recur && lhs->isMemOf()) {
        ((Location*)lhs)->setSubExp1(((Location*)lhs)->getSubExp1()->accept(v->mod));
	}
	if (recur) rhs = rhs->accept(v->mod);
	if (VERBOSE && v->mod->isMod())
		LOG << "Assignment changed: now " << this << "\n";
	return true;
}
bool PhiAssign::accept(StmtPartModifier* v) {
	bool recur;
	v->visit(this, recur);
	v->mod->clearMod();
	if (recur && lhs->isMemOf()) {
        ((Location*)lhs)->setSubExp1(((Location*)lhs)->getSubExp1()->accept(v->mod));
	}
	if (VERBOSE && v->mod->isMod())
		LOG << "PhiAssign changed: now " << this << "\n";
	return true;
}

bool ImplicitAssign::accept(StmtPartModifier* v) {
	bool recur;
	v->visit(this, recur);
	v->mod->clearMod();
	if (recur && lhs->isMemOf()) {
        ((Location*)lhs)->setSubExp1(((Location*)lhs)->getSubExp1()->accept(v->mod));
	}
	if (VERBOSE && v->mod->isMod())
		LOG << "ImplicitAssign changed: now " << this << "\n";
	return true;
}


bool GotoStatement::accept(StmtPartModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (pDest && recur)
		pDest = pDest->accept(v->mod);
	return true;
}

bool BranchStatement::accept(StmtPartModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (pDest && recur)
		pDest = pDest->accept(v->mod);
	if (pCond && recur)
		pCond = pCond->accept(v->mod);
	return true;
}

bool CaseStatement::accept(StmtPartModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (pDest && recur)
		pDest = pDest->accept(v->mod);
	if (pSwitchInfo && pSwitchInfo->pSwitchVar && recur)
		pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->accept(v->mod);
	return true;
}

bool CallStatement::accept(StmtPartModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (pDest && recur)
		pDest = pDest->accept(v->mod);
	StatementList::iterator it;
	for (it = arguments.begin(); recur && it != arguments.end(); it++)
		(*it)->accept(v);
	// For example: needed for CallBypasser so that a collected definition that happens to be another call gets
	// adjusted
	// But now I'm thinking no, the bypass and propagate while possible logic should take care of it.
	// Then again, what about the use collectors in calls? Best to do it.
	if (!v->ignoreCollector()) {
		DefCollector::iterator dd;
		for (dd = defCol.begin(); dd != defCol.end(); dd++)
			(*dd)->accept(v);
		UseCollector::iterator uu;
		for (uu = useCol.begin(); uu != useCol.end(); ++uu)
			// I believe that these should never change at the top level, e.g. m[esp{30} + 4] -> m[esp{-} - 20]
			(*uu)->accept(v->mod);
	}
	StatementList::iterator dd;
	for (dd = defines.begin(); recur && dd != defines.end(); dd++)
		(*dd)->accept(v);
	return true;
}

bool ReturnStatement::accept(StmtPartModifier* v) {
	bool recur;
	v->visit(this, recur);
	ReturnStatement::iterator rr;
	for (rr = modifieds.begin(); rr != modifieds.end(); ++rr)
		if (!(*rr)->accept(v))
			return false;
	for (rr = returns.begin(); rr != returns.end(); ++rr)
		if (!(*rr)->accept(v))
			return false;
	return true;
}

bool BoolAssign::accept(StmtPartModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (pCond && recur)
		pCond = pCond->accept(v->mod);
	if (lhs && recur)
		lhs = lhs->accept(v->mod);
	return true;
}

// Fix references to the returns of call statements
void Statement::bypass() {
	CallBypasser cb(this);
	StmtPartModifier sm(&cb);			// Use the Part modifier so we don't change the top level of LHS of assigns etc
	accept(&sm);
	if (cb.isTopChanged())
		simplify();						// E.g. m[esp{20}] := blah -> m[esp{-}-20+4] := blah
}

// Find the locations used by expressions in this Statement.
// Use the StmtExpVisitor and UsedLocsFinder visitor classes
void Statement::addUsedLocs(LocationSet& used, bool cc, bool memOnly /*= false */) {
	UsedLocsFinder ulf(used, memOnly);
	UsedLocsVisitor ulv(&ulf, cc);
	accept(&ulv);
}

// For all expressions in this Statement, replace any e with e{def}
void Statement::subscriptVar(Exp* e, Statement* def /*, Cfg* cfg */) {
	ExpSubscripter es(e, def /*, cfg*/);
	StmtSubscripter ss(&es);
	accept(&ss);
}

// Find all constants in this Statement
void Statement::findConstants(std::list<Const*>& lc) {
	ConstFinder cf(lc);
	StmtConstFinder scf(&cf);
	accept(&scf);
}

// Convert this PhiAssignment to an ordinary Assignment.  Hopefully, this is the only place that Statements change from
// one class to another.  All throughout the code, we assume that the addresses of Statement objects do not change,
// so we need this slight hack to overwrite one object with another
void PhiAssign::convertToAssign(Exp* rhs) {
	// I believe we always want to propagate to these ex-phi's; check!:
	rhs = rhs->propagateAll();
	// Thanks to tamlin for this cleaner way of implementing this hack
	assert(sizeof(Assign) <= sizeof(PhiAssign)); 
	int n = number;									// These items disappear with the destructor below
	PBB bb = pbb;
	UserProc* p = proc;
	Exp* lhs_ = lhs;
	Exp* rhs_ = rhs;
	Type* type_ = type;
	this->~PhiAssign(); 							// Explicitly destroy this, but keep the memory allocated.
	Assign* a = new(this) Assign(type_, lhs_, rhs_);// construct in-place. Note that 'a' == 'this' 
	a->setNumber(n); 
	a->setProc(p); 
	a->setBB(bb); 
}



#if 0			// Doesn't seem to be called any more
// This is a hack.	If we have a phi which has one of its elements referencing a statement which is defined as a 
// function address, then we can use this information to resolve references to indirect calls more aggressively.
// Note that this is not technically correct and will give the wrong result if the callee of an indirect call
// actually modifies a function pointer in the caller. 
bool PhiAssign::hasGlobalFuncParam()
{
	unsigned n = defVec.size();
	for (unsigned i = 0; i < n; i++) {
		Statement* u = defVec[i].def;
		if (u == NULL) continue;
		Exp *right = u->getRight();
		if (right == NULL)
			continue;
		if (right->getOper() == opGlobal ||
			(right->getOper() == opSubscript && right->getSubExp1()->getOper() == opGlobal)) {
			Exp *e = right;
			if (right->getOper() == opSubscript)
				e = right->getSubExp1();
			char *nam = ((Const*)e->getSubExp1())->getStr();
			Proc *p = proc->getProg()->findProc(nam);
			if (p == NULL)
				p = proc->getProg()->getLibraryProc(nam);
			if (p) {
				if (VERBOSE)
					LOG << "statement " << i << " of " << this << " is a global func\n";
				return true;
			}
		}
#if 0
		// BAD: this can loop forever if we have a phi loop
		if (u->isPhi() && u->getRight() != this &&
			((PhiExp*)u->getRight())->hasGlobalFuncParam(prog))
			return true;
#endif
	}
	return false;
}
#endif

void PhiAssign::simplify() {
	lhs = lhs->simplify();

	if (defVec.begin() != defVec.end()) {
		Definitions::iterator uu;
		bool allSame = true;
		uu = defVec.begin();
		Statement* first;
		for (first = (uu++)->def; uu != defVec.end(); uu++) {
			if (uu->def != first) {
				allSame = false;
				break;
			}
		}

		if (allSame) {
			if (VERBOSE)
				LOG << "all the same in " << this << "\n";
			convertToAssign(new RefExp(lhs, first));
			return;
		}

		bool onlyOneNotThis = true;
		Statement *notthis = (Statement*)-1;
		for (uu = defVec.begin(); uu != defVec.end(); uu++) {
			if (uu->def == NULL || uu->def->isImplicit() || !uu->def->isPhi() || uu->def != this)
				if (notthis != (Statement*)-1) {
					onlyOneNotThis = false;
					break;
				} else notthis = uu->def;
		}

		if (onlyOneNotThis && notthis != (Statement*)-1) {
			if (VERBOSE)
				LOG << "all but one not this in " << this << "\n";
			convertToAssign(new RefExp(lhs, notthis));
			return;
		}
	}
}

#if 0			// This functionality is in UserProc::fixCallAndPhiRefs now
void PhiAssign::simplifyRefs() {
	Definitions::iterator uu;
	for (uu = defVec.begin(); uu != defVec.end(); ) {
		// Look for a phi chain: *uu is an assignment whose RHS is the same expression as our LHS
		// It is most likely a phi statement that was converted to an Assign
		if (uu->def && uu->def->isAssign() && 
				((Assign*)uu->def)->getRight()->getOper() == opSubscript &&
				*((Assign*)uu->def)->getRight()->getSubExp1() == *lhs) {
			Assign*& adef = (Assign*&)uu->def;
			// If the assignment is to this phi...
			if (((RefExp*)adef->getRight())->getDef() == this) {
				// ... then *uu can be removed
				if (VERBOSE)
					LOG << "removing statement " << uu->def << " from phi at " << number << "\n";
				uu = defVec.erase(uu);
				continue;
			}
			// Else follow the chain, to get closer to the real, utlimate definition
			if (VERBOSE)
				LOG << "replacing " << adef->getNumber() << " with ";
			uu->def = ((RefExp*)adef->getRight())->getDef();
			if (VERBOSE) {
				int n = 0;
				if (uu->def) n = adef->getNumber();
				LOG << n << " in phi at " << number << " result is: " << this << "\n";
			}
		}
		uu++;
	}
}
#endif

static Exp* regOfWild = Location::regOf(new Terminal(opWild));
static Exp* regOfWildRef = new RefExp(regOfWild, (Statement*)-1);

// FIXME: Check if this is used any more
void Assignment::regReplace(UserProc* proc) {
	if (! (*lhs == *regOfWild)) return;
	std::list<Exp**> li;
	Exp* tmp = new RefExp(lhs, this);
	// Make a temporary reference for the LHS
	li.push_front(&tmp);
	proc->regReplaceList(li);
	lhs = tmp;
}
void Assign::regReplace(UserProc* proc) {
	std::list<Exp**> li;
	Exp::doSearch(regOfWildRef, rhs, li, false);
	proc->regReplaceList(li);
	// Now process the LHS
	Assignment::regReplace(proc);
}
void GotoStatement::regReplace(UserProc* proc) {
	std::list<Exp**> li;
	Exp::doSearch(regOfWildRef, pDest, li, false);
	proc->regReplaceList(li);
}
void BranchStatement::regReplace(UserProc* proc) {
	std::list<Exp**> li;
	Exp::doSearch(regOfWildRef, pDest, li, false);
	Exp::doSearch(regOfWildRef, pCond, li, false);
	proc->regReplaceList(li);
}
void CaseStatement::regReplace(UserProc* proc) {
	std::list<Exp**> li;
	Exp::doSearch(regOfWildRef, pSwitchInfo->pSwitchVar, li, false);
	proc->regReplaceList(li);
}
void CallStatement::regReplace(UserProc* proc) {
	std::list<Exp**> li;
	Exp::doSearch(regOfWildRef, pDest, li, false);
	StatementList::iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++)
		(*it)->regReplace(proc);
#if 0
	for (it = implicitArguments.begin(); it != implicitArguments.end(); it++)
		Exp::doSearch(regOfWildRef, *it, li, false);
#endif
#if 0			// Returns are in the context of the callee
	// Note: returns are "on the left hand side", and hence are never subscripted. So wrap in a RefExp
	std::vector<ReturnInfo>::iterator rr;
	for (rr = returns->begin(); rr != returns->end(); rr++) {
		if (rr->e && *rr->e == *regOfWild) {
			std::list<Exp**> rli;
			Exp* tmp = new RefExp(rr->e, this);
			rli.push_front(&tmp);
			proc->regReplaceList(rli);
			rr->e = tmp;
		}
	}
#endif
}
void ReturnStatement::regReplace(UserProc* proc) {
	for (iterator it = modifieds.begin(); it != modifieds.end(); ++it)
		(*it)->regReplace(proc);
}

void PhiAssign::putAt(int i, Statement* def, Exp* e) {
	assert(e); // should be something surely
	if (i >= (int)defVec.size())
		defVec.resize(i+1);		// Note: possible to insert uninitialised elements
	defVec[i].def = def;
	defVec[i].e = e;
}

void CallStatement::setLeftFor(Exp* forExp, Exp* newExp) {
#if 0
	for (unsigned u = 0; u < defines.size(); u++) {
		if (*returns[u].e == *forExp) {
			returns[u].e = newExp;
			return;
		}
	}
#else
	std::cerr << "! Attempt to setLeftFor this call statement! forExp is " << forExp << ", newExp is " << newExp <<
		"\n";
	assert(0);
#endif
}

bool Assignment::definesLoc(Exp* loc) {
	if (lhs->getOper() == opAt)					// For foo@[x:y], match of foo==loc OR whole thing == loc
		if (*((Ternary*)lhs)->getSubExp1() == *loc) return true;
	return *lhs == *loc;
}

bool CallStatement::definesLoc(Exp* loc) {
	StatementList::iterator dd;
	for (dd = defines.begin(); dd != defines.end(); ++dd) {
		Exp* lhs = ((Assign*)*dd)->getLeft();
		if (*lhs == *loc)
			return true;
	}
	return false;
}

// Does a ReturnStatement define anything? Not really, the locations are already defined earlier in the procedure.
// However, nothing comes after the return statement, so it doesn't hurt to pretend it does, and this is a place to
// store the return type(s) for example.
// FIXME: seems it would be cleaner to say that Return Statements don't define anything.
bool ReturnStatement::definesLoc(Exp* loc) {
	iterator it;
	for (it = modifieds.begin(); it != modifieds.end(); it++) {
		if ((*it)->definesLoc(loc))
			return true;
	}
	return false;
}

// FIXME: see above
void ReturnStatement::getDefinitions(LocationSet& ls) {
	iterator rr;
	for (rr = modifieds.begin(); rr != modifieds.end(); ++rr)
		(*rr)->getDefinitions(ls);
}

Type* ReturnStatement::getTypeFor(Exp* e) {
	ReturnStatement::iterator rr;
	for (rr = modifieds.begin(); rr != modifieds.end(); rr++) {
		if (*((Assignment*)*rr)->getLeft() == *e)
			return ((Assignment*)*rr)->getType();
	}
	return NULL;
}

void ReturnStatement::setTypeFor(Exp*e, Type* ty) {
	ReturnStatement::iterator rr;
	for (rr = modifieds.begin(); rr != modifieds.end(); rr++) {
		if (*((Assignment*)*rr)->getLeft() == *e) {
			((Assignment*)*rr)->setType(ty);
			break;
		}
	}
	for (rr = returns.begin(); rr != returns.end(); rr++) {
		if (*((Assignment*)*rr)->getLeft() == *e) {
			((Assignment*)*rr)->setType(ty);
			return;
		}
	}
}

#define RETSTMT_COLS 120
void ReturnStatement::print(std::ostream& os) {
	os << std::setw(4) << std::dec << number << " ";
	os << "RET";
	iterator it;
	bool first = true;
	unsigned column = 19;
	for (it = returns.begin(); it != returns.end(); ++it) {
		std::ostringstream ost;
		((Assignment*)*it)->printCompact(ost);
		unsigned len = ost.str().length();
		if (first) {
			first = false;
			os << " ";
		} else if (column + 4 + len > RETSTMT_COLS) {	// 4 for command 3 spaces
			if (column != RETSTMT_COLS-1) os << ",";	// Comma at end of line
			os << "\n                ";
			column = 16;
		} else {
			os << ",   ";
			column += 4;
		}
		os << ost.str().c_str();
		column += len;
	}
	os << "\n              Modifieds: ";
	first = true;
	column = 25;
	for (it = modifieds.begin(); it != modifieds.end(); ++it) {
		std::ostringstream ost;
		Assign* as = (Assign*)*it;
		Type* ty = as->getType();
		if (ty)
			ost << "*" << ty << "* ";
		ost << as->getLeft();
		unsigned len = ost.str().length();
		if (first)
			first = false;
		else if (column + 3 + len > RETSTMT_COLS) {		// 3 for comma and 2 spaces
			if (column != RETSTMT_COLS-1) os << ",";	// Comma at end of line
			os << "\n                ";
			column = 16;
		} else {
			os << ",  ";
			column += 3;
		}
		os << ost.str().c_str();
		column += len;
	}
#if 1
	// Collected reaching definitions
	os << "\n              Reaching definitions: ";
	col.print(os);
#endif
}

// A helper class for comparing Assignment*'s sensibly
bool lessAssignment::operator()(const Assignment* x, const Assignment* y) const {
	Assignment* xx = const_cast<Assignment*>(x);
	Assignment* yy = const_cast<Assignment*>(y);
	return (*xx->getLeft() < *yy->getLeft());		// Compare the LHS expressions
}

// Repeat the above for Assign's; sometimes the compiler doesn't (yet) understand that Assign's are Assignment's
bool lessAssign::operator()(const Assign* x, const Assign* y) const {
	Assign* xx = const_cast<Assign*>(x);
	Assign* yy = const_cast<Assign*>(y);
	return (*xx->getLeft() < *yy->getLeft());		// Compare the LHS expressions
}

// Update the modifieds, in case the signature and hence ordering and filtering has changed, or the locations in the
// collector have changed. Does NOT remove preserveds (deferred until updating returns).
void ReturnStatement::updateModifieds() {
	Signature* sig = proc->getSignature();
	StatementList oldMods(modifieds);					// Copy the old modifieds
	modifieds.clear();
	// For each location in the collector, make sure that there is an assignment in the old modifieds, which will
	// be filtered and sorted to become the new modifieds
	// Ick... O(N*M) (N existing modifeds, M collected locations)
	DefCollector::iterator ll;
	StatementList::iterator it;
	for (ll = col.begin(); ll != col.end(); ++ll) {
		bool found = false;
		Assign* as = (Assign*)*ll;
		Exp* colLhs = as->getLeft();
		if (proc->filterReturns(colLhs))
			continue;									// Filtered out
		for (it = oldMods.begin(); it != oldMods.end(); it++) {
			Exp* lhs = ((Assign*)*it)->getLeft();
			if (*lhs == *colLhs) {
				found = true;
				break;
			}
		}
		if (!found) {
			as->setProc(proc);							// Comes from the Collector
			as->setBB(pbb);
			oldMods.append(as->clone());
		}
	}

	// Mostly the old modifications will be in the correct order, and inserting will be fastest near the start of the
	// new list. So read the old modifications in reverse order
	for (it = oldMods.end(); it != oldMods.begin(); ) {
		--it;										// Becuase we are using a forwards iterator backwards
		// Make sure the LHS is still in the collector
		Assign* as = (Assign*)*it;
		Exp* lhs = as->getLeft();
		if (!col.existsOnLeft(lhs))
			continue;						// Not in collector: delete it (don't copy it)
		if (proc->filterReturns(lhs))
			continue;						// Filtered out: delete it
	
		// Insert as, in order, into the existing set of modifications
		StatementList::iterator nn;
		bool inserted = false;
		for (nn = modifieds.begin(); nn != modifieds.end(); ++nn) {
			if (sig->returnCompare(*as, *(Assign*)*nn)) {		// If the new assignment is less than the current one
				nn = modifieds.insert(nn, as);					// then insert before this position
				inserted = true;
				break;
			}
		}
		if (!inserted)
			modifieds.insert(modifieds.end(), as);	// In case larger than all existing elements
	}
}

// Update the returns, in case the signature and hence ordering and filtering has changed, or the locations in the
// modifieds list
void ReturnStatement::updateReturns() {
	Signature* sig = proc->getSignature();
	int sp = sig->getStackRegister();
	StatementList oldRets(returns);					// Copy the old returns
	returns.clear();
	// For each location in the modifieds, make sure that there is an assignment in the old returns, which will
	// be filtered and sorted to become the new returns
	// Ick... O(N*M) (N existing returns, M modifieds locations)
	StatementList::iterator dd, it;
	for (dd = modifieds.begin(); dd != modifieds.end(); ++dd) {
		bool found = false;
		Exp* loc = ((Assignment*)*dd)->getLeft();
		if (proc->filterReturns(loc))
			continue;									// Filtered out
		// Special case for the stack pointer: it has to be a modified (otherwise, the changes will bypass the calls),
		// but it is not wanted as a return
		if (loc->isRegN(sp)) continue;
		for (it = oldRets.begin(); it != oldRets.end(); it++) {
			Exp* lhs = ((Assign*)*it)->getLeft();
			if (*lhs == *loc) {
				found = true;
				break;
			}
		}
		if (!found) {
			Assign* as = new Assign(loc->clone(), loc->clone());
			as->setProc(proc);
			as->setBB(pbb);
			oldRets.append(as);
		}
	}

	// Mostly the old returns will be in the correct order, and inserting will be fastest near the start of the
	// new list. So read the old returns in reverse order
	for (it = oldRets.end(); it != oldRets.begin(); ) {
		--it;										// Becuase we are using a forwards iterator backwards
		// Make sure the LHS is still in the modifieds
		Assign* as = (Assign*)*it;
		Exp* lhs = as->getLeft();
		if (!modifieds.existsOnLeft(lhs))
			continue;						// Not in modifieds: delete it (don't copy it)
		if (proc->filterReturns(lhs))
			continue;						// Filtered out: delete it
#if 1
		// Preserveds are NOT returns (nothing changes, so what are we returning?)
		// Check if it is a preserved location, e.g. r29 := r29{-}
		Exp* rhs = as->getRight();
		if (rhs->isSubscript() && ((RefExp*)rhs)->isImplicitDef() && *((RefExp*)rhs)->getSubExp1() == *lhs)
			continue;						// Filter out the preserveds
#endif
			
		// Insert as, in order, into the existing set of returns
		StatementList::iterator nn;
		bool inserted = false;
		for (nn = returns.begin(); nn != returns.end(); ++nn) {
			if (sig->returnCompare(*as, *(Assign*)*nn)) {		// If the new assignment is less than the current one
				nn = returns.insert(nn, as);					// then insert before this position
				inserted = true;
				break;
			}
		}
		if (!inserted)
			returns.insert(returns.end(), as);	// In case larger than all existing elements
	}
}

// Set the defines to the set of locations modified by the callee, or if no callee, to all variables live at this call
void CallStatement::updateDefines() {
	Signature* sig;
	if (procDest)
		// The signature knows how to order the returns
		sig = procDest->getSignature();
	else
		// Else just use the enclosing proc's signature
		sig = proc->getSignature();

	if (procDest && procDest->isLib()) {
		sig->setLibraryDefines(&defines);				// Set the locations defined
		return;
	} else if (Boomerang::get()->assumeABI) {
		// Risky: just assume the ABI caller save registers are defined
		Signature::setABIdefines(proc->getProg(), &defines);
		return;
	}

	// Move the defines to a temporary list
	StatementList oldDefines(defines);					// Copy the old defines
	StatementList::iterator it;
	defines.clear();

	if (procDest && calleeReturn) {
		StatementList::iterator mm;
		StatementList& modifieds = ((UserProc*)procDest)->getModifieds();
		for (mm = modifieds.begin(); mm != modifieds.end(); ++mm) {
			ImplicitAssign* ias = (ImplicitAssign*)*mm;
			Exp* loc = ias->getLeft();
			if (proc->filterReturns(loc))
				continue;
			if (!oldDefines.existsOnLeft(loc))
				oldDefines.append(ias->clone());
		}
	} else {
		// Ensure that everything in the UseCollector has an entry in oldDefines
		LocationSet::iterator ll;
		for (ll = useCol.begin(); ll != useCol.end(); ++ll) {
			Exp* loc = *ll;
			if (proc->filterReturns(loc))
				continue;									// Filtered out
			if (!oldDefines.existsOnLeft(loc)) {
				ImplicitAssign* as = new ImplicitAssign(loc->clone());
				as->setProc(proc);
				as->setBB(pbb);
				oldDefines.append(as);
			}
		}
	}

	for (it = oldDefines.end(); it != oldDefines.begin(); ) {
		--it;										// Becuase we are using a forwards iterator backwards
		// Make sure the LHS is still in the return or collector
		Assign* as = (Assign*)*it;
		Exp* lhs = as->getLeft();
		if (calleeReturn) {
			if (!calleeReturn->definesLoc(lhs))
				continue;						// Not in callee returns
		} else {
			if (!useCol.existsNS(lhs))
				continue;						// Not in collector: delete it (don't copy it)
		}
		if (proc->filterReturns(lhs))
			continue;						// Filtered out: delete it

		// Insert as, in order, into the existing set of definitions
		StatementList::iterator nn;
		bool inserted = false;
		for (nn = defines.begin(); nn != defines.end(); ++nn) {
			if (sig->returnCompare(*as, *(Assign*)*nn)) {	// If the new assignment is less than the current one
				nn = defines.insert(nn, as);				// then insert before this position
				inserted = true;
				break;
			}
		}
		if (!inserted)
			defines.insert(defines.end(), as);	// In case larger than all existing elements
	}
}

// A helper class for updateArguments. It just dishes out a new argument from one of the three sources: the signature,
// the callee parameters, or the defCollector in the call
class ArgSourceProvider {
enum Src {SRC_LIB, SRC_CALLEE, SRC_COL};
		Src			src;
		CallStatement* call;
		int			i, n;				// For SRC_LIB
		Signature*	callSig;
		StatementList::iterator pp;		// For SRC_CALLEE
		StatementList* calleeParams;
		DefCollector::iterator cc;		// For SRC_COL
		DefCollector* defCol;
public:
					ArgSourceProvider(CallStatement* call);
		Exp*		nextArgLoc();		// Get the next location (not subscripted)
		Type*		curType(Exp* e);	// Get the current location's type
		bool		exists(Exp* loc);	// True if the given location (not subscripted) exists as a source
		Exp*		localise(Exp* e);	// Localise to this call if necessary
};

ArgSourceProvider::ArgSourceProvider(CallStatement* call) : call(call) {
	Proc* procDest = call->getDestProc();
	if (procDest && procDest->isLib()) {
		src = SRC_LIB;
		callSig = call->getSignature();
		n = callSig->getNumParams();
		i = 0;
	} else if (call->getCalleeReturn() != NULL) {
		src = SRC_CALLEE;
		calleeParams = &((UserProc*)procDest)->getParameters();
		pp = calleeParams->begin();
	} else {
		Signature* destSig = NULL;
		if (procDest)
			destSig = procDest->getSignature();
		if (destSig && destSig->isForced()) {
			src = SRC_LIB;
			callSig = destSig;
			n = callSig->getNumParams();
			i = 0;
		} else {
			src = SRC_COL;
			defCol = call->getDefCollector();
			cc = defCol->begin();
		}
	}
}

Exp* ArgSourceProvider::nextArgLoc() {
	Exp* s;
	bool allZero;
	switch(src) {
		case SRC_LIB:
			if (i == n) return NULL;
			s = callSig->getParamExp(i++)->clone();
			s->removeSubscripts(allZero);		// e.g. m[sp{-} + 4] -> m[sp + 4]
			call->localiseComp(s);
			return s;
		case SRC_CALLEE:
			if (pp == calleeParams->end()) return NULL;
			s = ((Assignment*)*pp++)->getLeft()->clone();
			s->removeSubscripts(allZero);
			call->localiseComp(s);					// Localise the components. Has the effect of translating into
													// the contect of this caller 
			return s;
		case SRC_COL:
			if (cc == defCol->end()) return NULL;
			// Give the location, i.e. the left hand side of the assignment
			return ((Assign*)*cc++)->getLeft();
	}
	return NULL;		// Suppress warning
}

Exp* ArgSourceProvider::localise(Exp* e) {
	if (src == SRC_COL) {
		// Provide the RHS of the current assignment
		Exp* ret = ((Assign*)*--cc)->getRight();
		++cc;
		return ret;
	}
	// Else just use the call to localise
	return call->localiseExp(e);
}

Type* ArgSourceProvider::curType(Exp* e) {
	switch(src) {
		case SRC_LIB:
			return callSig->getParamType(i-1);
		case SRC_CALLEE: {
			Type* ty = ((Assignment*)*--pp)->getType();
			pp++;
			return ty;
		}
		case SRC_COL: {
			// Mostly, there won't be a type here, I would think...
			Type* ty = (*--cc)->getType();
			++cc;
			return ty;
		}
	}
	return NULL;		// Suppress warning
}

bool ArgSourceProvider::exists(Exp* e) {
	bool allZero;
	switch (src) {
		case SRC_LIB:
			if (callSig->hasEllipsis())
				// FIXME: for now, just don't check
				return true;
			for (i=0; i < n; i++) {
				Exp* sigParam = callSig->getParamExp(i)->clone();
				sigParam->removeSubscripts(allZero);
				call->localiseComp(sigParam);
				if (*sigParam == *e)
					return true;
			}
			return false;
		case SRC_CALLEE:
			for (pp = calleeParams->begin(); pp != calleeParams->end(); ++pp) {
				Exp* par = ((Assignment*)*pp)->getLeft()->clone();
				par->removeSubscripts(allZero);
				call->localiseComp(par);
				if (*par == *e)
					return true;
			}
			return false;
		case SRC_COL:
			return defCol->existsOnLeft(e);
	}
	return false;			// Suppress warning
}

void CallStatement::updateArguments() {
	/* If this is a library call, source = signature
		else if there is a callee return, source = callee parameters
		else
		  if a forced callee signature, source = signature
		  else source is def collector in this call.
		oldArguments = arguments
		clear arguments
		for each arg lhs in source
			if exists in oldArguments, leave alone
			else if not filtered append assignment lhs=lhs to oldarguments
		for each argument as in oldArguments in reverse order
			lhs = as->getLeft
			if (lhs does not exist in source) continue
			if filterParams(lhs) continue
			insert as into arguments, considering sig->argumentCompare
	*/
	// Note that if propagations are limited, arguments and collected reaching definitions can be in terms of phi
	// statements that have since been translated to assignments. So propagate through them now
	// FIXME: reconsider! There are problems (e.g. with test/pentium/fromSSA2, test/pentium/fbranch) if you propagate
	// to the expressions in the arguments (e.g. m[esp{phi1}-20]) but don't propagate into ordinary statements that
	// define the actual argument. For example, you might have m[esp{-}-56] in the call, but the actual definition of
	// the printf argument is still m[esp{phi1} -20] = "%d".
	if (EXPERIMENTAL) {
		bool convert;
		propagateTo(convert);
	}
	StatementList oldArguments(arguments);
	arguments.clear();
	if (EXPERIMENTAL) {
	// I don't really know why this is needed, but I was seeing r28 := ((((((r28{-} - 4) - 4) - 4) - 8) - 4) - 4) - 4:
	DefCollector::iterator dd;
	for (dd = defCol.begin(); dd != defCol.end(); ++dd)
		(*dd)->simplify();
	}

	Signature* sig = proc->getSignature();
	// Ensure everything in the callee's signature (if this is a library call), or the callee parameters (if available),
	// or the def collector if not,  exists in oldArguments
	ArgSourceProvider asp(this);
	Exp* loc;
	while ((loc = asp.nextArgLoc()) != NULL) {
		if (proc->filterParams(loc))
			continue;
		if (!oldArguments.existsOnLeft(loc)) {
			Exp* rhs = asp.localise(loc->clone());
			Assign* as = new Assign(asp.curType(loc), loc->clone(), rhs);
			as->setProc(proc);
			as->setBB(pbb);
			oldArguments.append(as);
		}
	}

	StatementList::iterator it;
	for (it = oldArguments.end(); it != oldArguments.begin(); ) {
		--it;										// Becuase we are using a forwards iterator backwards
		// Make sure the LHS is still in the callee signature / callee parameters / use collector
		Assign* as = (Assign*)*it;
		Exp* lhs = as->getLeft();
		if (!asp.exists(lhs)) continue;
		if (proc->filterParams(lhs))
			continue;						// Filtered out: delete it

		// Insert as, in order, into the existing set of definitions
		StatementList::iterator nn;
		bool inserted = false;
		for (nn = arguments.begin(); nn != arguments.end(); ++nn) {
			if (sig->argumentCompare(*as, *(Assign*)*nn)) {		// If the new assignment is less than the current one
				nn = arguments.insert(nn, as);					// then insert before this position
				inserted = true;
				break;
			}
		}
		if (!inserted)
			arguments.insert(arguments.end(), as);				// In case larger than all existing elements
	}
}

#if 0		// localiseExp() is the same thing
// Convert an expression like m[sp+4] in the callee context to m[sp{-} - 32] (in the context of the call)
Exp* CallStatement::fromCalleeContext(Exp* e) {
	Exp* sp = Location::regOf(signature->getStackRegister());
	Exp* refSp = defCol.findNS(sp);
	if (refSp == NULL)
		return e;				// No stack pointer definition reaches here, so no change needed
	Statement* def = ((RefExp*)refSp)->getDef();
	if (!def->isAssign())
		return e;				// ? Definition for sp is not an assignment
	Exp* rhs = ((Assign*)def)->getRight();	// RHS of assign should be in terms of sp{-}
	bool change;
	return e->clone()->searchReplaceAll(sp, rhs, change);
}
#endif

// Calculate results(this) = defines(this) isect live(this)
// Note: could use a LocationList for this, but then there is nowhere to store the types (for DFA based TA)
// So the RHS is just ignored
StatementList* CallStatement::calcResults() {
	StatementList* ret = new StatementList;
	if (procDest) {
		Signature* sig = procDest->getSignature();
		if (procDest && procDest->isLib()) {
			int n = sig->getNumReturns();
			for (int i=1; i < n; i++) {						// Ignore first (stack pointer) return
				Exp* sigReturn = sig->getReturnExp(i);
				// But we have translated out of SSA form, so some registers have had to have been replaced with locals
				// So wrap the return register in a ref to this and check the locals
				RefExp* wrappedRet = new RefExp(sigReturn, this);
				char* locName = proc->findLocal(wrappedRet);	// E.g. r24{16}
				if (locName)
					sigReturn = Location::local(locName, proc);	// Replace e.g. r24 with local19
				if (useCol.exists(sigReturn)) {
					ImplicitAssign* as = new ImplicitAssign(sig->getReturnType(i), sigReturn);
					ret->append(as);
				}
			}
		} else {
			Exp* rsp = Location::regOf(proc->getSignature()->getStackRegister(proc->getProg()));
			StatementList::iterator dd;
			for (dd = defines.begin(); dd != defines.end(); ++dd) {
				Exp* lhs = ((Assign*)*dd)->getLeft();
				// The stack pointer is allowed as a define, so remove it here as a special case non result
				if (*lhs == *rsp) continue;
				if (useCol.exists(lhs))
					ret->append(*dd);
			}
		}
	} else {
		// For a call with no destination at this late stage, use everything live at the call except for the stack
		// pointer register. Needs to be sorted
		UseCollector::iterator rr;								// Iterates through reaching definitions
		StatementList::iterator nn;								// Iterates through new results
		Signature* sig = proc->getSignature();
		int sp = sig->getStackRegister();
		for (rr = useCol.begin(); rr != useCol.end(); ++rr) {
			Exp* loc = *rr;
			if (proc->filterReturns(loc)) continue;				// Ignore filtered locations
			if (loc->isRegN(sp)) continue;						// Ignore the stack pointer
			ImplicitAssign* as = new ImplicitAssign(loc);		// Create an implicit assignment
			bool inserted = false;
			for (nn = ret->begin(); nn != ret->end(); ++nn) {
				// If the new assignment is less than the current one,
				if (sig->returnCompare(*as, *(Assignment*)*nn)) {
					nn = ret->insert(nn, as);					// then insert before this position
					inserted = true;
					break;
				}
			}
			if (!inserted)
				ret->insert(ret->end(), as);					// In case larger than all existing elements
		}
	}
	return ret;
}

Type* Assignment::getType() {
	if (ADHOC_TYPE_ANALYSIS)
		return lhs->getType();
	else
		return type;
}

void Assignment::setType(Type* ty) {
	type = ty;
	if (ADHOC_TYPE_ANALYSIS) {
		Location* loc = dynamic_cast<Location*>(lhs);
		if (loc)					// For example: could be %CF! Ugh.
			loc->setType(ty);
	}
}

#if 0
// A temporary HACK for getting rid of the %CF in returns
void ReturnStatement::specialProcessing() {
	iterator it;
	for (it = returns.begin(); it != returns.end(); it++) {
		Exp* lhs = ((Assign*)*it)->getLeft();
		if (lhs->getOper() == opCF) {
			Exp* rhs = ((Assign*)*it)->getRight();
			if (rhs->isSubscript() &&
					!((RefExp*)rhs)->isImplicitDef() &&
					((RefExp*)rhs)->getSubExp1()->getOper() == opCF) {
				// We have a non SUBFLAGS definition reaching the exit; just delete the return of %CF
				returns.erase(it);
				return;
			}
		}
	}
}
#endif

void CallStatement::removeDefine(Exp* e) {
	StatementList::iterator ss;
	for (ss = defines.begin(); ss != defines.end(); ++ss) {
		Assign* as = ((Assign*)*ss);
		if (*as->getLeft() == *e) {
			defines.erase(ss);
			return;
		}
	}
	LOG << "WARNING: could not remove define " << e << " from call " << this << "\n";
}

bool CallStatement::isChildless() {
	if (procDest == NULL) return true;
	if (procDest->isLib()) return false;
	// Early in the decompile process, recursive calls are treated as childless, so they use and define all
	if (((UserProc*)procDest)->isEarlyRecursive())
		return true;
	return calleeReturn == NULL;
}

Exp* CallStatement::bypassRef(RefExp* r, bool& ch) {
	Exp* base = r->getSubExp1();
	Exp* proven;
	ch = false;
	if (procDest && procDest->isLib()) {
		Signature* sig = procDest->getSignature();
		proven = sig->getProven(base);	
		if (proven == NULL) {			// Not (known to be) preserved
			if (sig->findReturn(base) != -1)
				return r;				// Definately defined, it's the return
			// Otherwise, not all that sure. Assume that library calls pass things like local variables
		}
	} else {
		// Was using the defines to decide if something is preserved, but consider sp+4 for stack based machines
		// Have to use the proven information for the callee (if any)
		if (procDest == NULL)
			return r;				// Childless callees transmit nothing
		//if (procDest->isLocal(base))					// ICK! Need to prove locals and parameters through calls...
		// FIXME: temporary HACK! Ignores alias issues.
		if (!procDest->isLib() && ((UserProc*)procDest)->isLocalOrParam(base)) {
			Exp* ret = localiseExp(base->clone());	// Assume that it is proved as preserved
			ch = true;
			if (VERBOSE)
				LOG << base << " allowed to bypass call statement " << number << " ignoring aliasing; result " << ret <<
					"\n";
			return ret;
		}
			
		proven = procDest->getProven(base);			// e.g. r28+4
	}
	if (proven == NULL)
		return r;										// Can't bypass, since nothing proven
	Exp* to = localiseExp(base);						// e.g. r28{17}
	assert(to);
	proven = proven->clone();							// Don't modify the expressions in destProc->proven!
	proven = proven->searchReplaceAll(base, to, ch);	// e.g. r28{17} + 4
	if (ch && VERBOSE)
		LOG << "bypassRef() replacing " << r << " with " << proven << "\n";
	return proven;
}

void ReturnStatement::removeModified(Exp* loc) {
	modifieds.removeDefOf(loc);
	returns.removeDefOf(loc);
}

void CallStatement::addDefine(ImplicitAssign* as) {
	defines.append(as);
}

TypingStatement::TypingStatement(Type* ty) : type(ty) {
}

// NOTE: ImpRefStatement not yet used
void ImpRefStatement::print(std::ostream& os) {
	os << "     *";				// No statement number
	os << type << "* IMP REF " << addressExp;
}

void ImpRefStatement::meetWith(Type* ty, bool& ch) {
	type = type->meetWith(ty, ch);
}

Statement* ImpRefStatement::clone() {
	return new ImpRefStatement(type->clone(), addressExp->clone());
}
bool ImpRefStatement::accept(StmtVisitor* visitor) {
	return visitor->visit(this);
}
bool	ImpRefStatement::accept(StmtExpVisitor* v) {
	bool override;
	bool ret = v->visit(this, override);
	if (override)
		return ret;
	if (ret) ret = addressExp->accept(v->ev);
	return ret;
}
bool	ImpRefStatement::accept(StmtModifier* v) {
	bool recur;
	v->visit(this, recur);
	v->mod->clearMod();
	if (recur) addressExp = addressExp->accept(v->mod);
	if (VERBOSE && v->mod->isMod())
		LOG << "ImplicitRef changed: now " << this << "\n";
	return true;
}
bool	ImpRefStatement::accept(StmtPartModifier* v) {
	bool recur;
	v->visit(this, recur);
	v->mod->clearMod();
	if (recur) addressExp = addressExp->accept(v->mod);
	if (VERBOSE && v->mod->isMod())
		LOG << "ImplicitRef changed: now " << this << "\n";
	return true;
}
bool	ImpRefStatement::processConstants(Prog* prog) { return false;}
bool	ImpRefStatement::search(Exp* search, Exp*& result) {
	result = NULL;
	return addressExp->search(search, result);
}
bool	ImpRefStatement::searchAll(Exp* search, std::list<Exp*, std::allocator<Exp*> >& result) {
	return addressExp->searchAll(search, result);
}
bool	ImpRefStatement::searchAndReplace(Exp* search, Exp* replace, bool cc) {
	bool change;
	addressExp = addressExp->searchReplaceAll(search, replace, change);
	return change;
}
void	ImpRefStatement::simplify() {addressExp = addressExp->simplify();}
void	ImpRefStatement::regReplace(UserProc* proc) {
	std::list<Exp**> li;
	Exp::doSearch(regOfWildRef, addressExp, li, false);
	proc->regReplaceList(li);
}

void CallStatement::eliminateDuplicateArgs() {
	StatementList::iterator it;
	LocationSet ls;
	for (it = arguments.begin(); it != arguments.end(); ) {
		Exp* lhs = ((Assignment*)*it)->getLeft();
		if (ls.exists(lhs)) {
			// This is a duplicate
			it = arguments.erase(it);
			continue;
		}
		ls.insert(lhs);
		++it;
	}
}

void PhiAssign::enumerateParams(std::list<Exp*>& le) {
	iterator it;
	for (it = begin(); it != end(); ++it) {
		if (it->e == NULL) continue;
		RefExp* r = new RefExp(it->e, it->def);
		le.push_back(r);
	}
}

// For debugging
void dumpDestCounts(std::map<Exp*, int, lessExpStar>* destCounts) {
	std::map<Exp*, int, lessExpStar>::iterator it;
	for (it = destCounts->begin(); it != destCounts->end(); ++it) {
		std::cerr << std::setw(4) << std::dec << it->second << " " << it->first << "\n";
	}
}

bool JunctionStatement::accept(StmtVisitor* visitor)
{
	return true;
}

bool JunctionStatement::accept(StmtExpVisitor* visitor)
{
	return true;
}

bool JunctionStatement::accept(StmtModifier* visitor)
{
	return true;
}

bool JunctionStatement::accept(StmtPartModifier* visitor)
{
	return true;
}

void JunctionStatement::print(std::ostream &os)
{
	os << std::setw(4) << std::dec << number << " ";
	os << "JUNCTION ";
	for (int i = 0; i < pbb->getNumInEdges(); i++) {
		os << std::hex << pbb->getInEdges()[i]->getHiAddr() << std::dec;
		if (pbb->isBackEdge(i))
			os << "*";
		os << " ";
	}
	if (isLoopJunction())
		os << "LOOP";
	os << "\n\t\t\tranges: ";
	ranges.print(os);
}


