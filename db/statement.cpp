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
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

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

// replace a use of def->getLeft() by def->getRight() in this statement
bool Statement::replaceRef(Assign *def) {
	Exp* lhs = def->getLeft();
	Exp* rhs = def->getRight();
	assert(lhs);
	assert(rhs);
	// "Wrap" the LHS in a RefExp.  This is so that it matches with the thing it is replacing.
	// Example: 42: r28 := r28{14}-4 into m[r28-24] := m[r28{42}] + ...
	// The r28 needs to be subscripted with {42} to match the thing on the RHS that is being substituted into.
	// (It also makes sure it never matches the other r28, which should really be r28{-}).
	Unary* re;
	re = new RefExp(lhs, def);

	// do the replacement
	bool convert = doReplaceRef(re, rhs);

	// Careful: don't delete re while lhs is still a part of it!
	// Else, will delete lhs, which is still a part of def!
	re->setSubExp1ND(NULL);
	//delete re;
	return convert;
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

// exclude: a set of statements not to propagate from
// memDepth: the max level of expressions to propagate FROM
// toDepth: the exact level that we will propagate TO (unless toDepth == -1)
// limit: if true, use a heuristic to try to limit the amount of propagation (e.g. fromSSA)

// Reasoning for toDepth is supposed to be as follows: you can't safely propagate any memory expression
// until all its subexpressions (expressions at lower memory depths) are
// propagated. Examples:
// 1: r28 = r28-4
// 2: m[m[r28]+4] = 10
// 3: r28 = r28-4
// 4: r25 = m[m[r28]+4]	  // Appears defined in 2 if don't consider FROM depths
// 5: r24 = r28
// 6: m[r24] = r[28]
// 6: m[r25] = 4
// 7: m[m[m[r24]]+m[r25]] = 99
// Ordinarily would subst m[r24] first, at level 2... ugh, can't think of an
// example where it would matter.

// Return true if an indirect call statement is converted to direct
bool Statement::propagateTo(int memDepth, int toDepth, bool limit /* = true */) {
#if 0		// If don't propagate into flag assigns, some converting to locals doesn't happen, and errors occur
	// don't propagate to flag assigns
	if (isFlagAssgn())
		return;
#endif
	// don't propagate into temp definitions (? why? Can this ever happen?)
#if 0		// Don't want to propagate a temp from one RTL to another, but DO want to propagate withing one RTL
			// Example: test/OSX/hello stmw instruction
	if (isAssign() && getLeft()->isTemp())
		return false;
#endif
	bool change;
	bool convert = false;
	int changes = 0;
	int sp = proc->getSignature()->getStackRegister(proc->getProg());
	Exp* regSp = Location::regOf(sp);
	// Repeat substituting into this statement while there is a single reference component in it
	// But all RefExps will have just one component. Maybe calls (later) will have more than one ref
	// Example: y := a{2,3} + b{4} + c{0}
	// can substitute b{4} into this, but not a. Can't do c either, since there is no definition (it's a parameter).
	do {
		LocationSet exps;
		addUsedLocs(exps, true);		// True to also add uses from collectors
		LocationSet::iterator ll;
		change = false;
		// Example: m[r24{10}] := r25{20} + m[r26{30}]
		// exps has r24{10}, r25{30}, m[r26{30}], r26{30}
		for (ll = exps.begin(); ll != exps.end(); ll++) {
			Exp* e = *ll;
			if (toDepth != -1 && e->getMemDepth() != toDepth)
				continue;
			if (!e->isSubscript()) continue;
			// Can propagate TO this (if memory depths are suitable)
			if (((RefExp*)e)->isImplicitDef())
				// Can't propagate statement "0" (implicit assignments)
				continue;
			Statement* def = ((RefExp*)e)->getDef();
			if (def == this)
				// Don't propagate to self! Can happen with %pc's
				continue;
			if (def->isNullStatement())
				// Don't propagate a null statement! Can happen with %pc's
				// (this would have no effect, and would infinitely loop)
				continue;
			if (!def->isAssign()) continue;		// Only propagate ordinary assignments (MVE: Check!)
#if 0
			if (def->isPhi())
				// Don't propagate phi statements!
				continue;
			if (def->isCall())
				continue;
			if (def->isBool())
				continue;
#endif
#if 1	// Sorry, I don't believe prop into branches is wrong... MVE.  By not propagating into branches, we get memory
		// locations not converted to locals, for example (e.g. test/source/csp.c)
		 
			Assign* adef = (Assign*)def;
			if (isBranch()) {
				Exp* defRight = adef->getRight();
				// Propagating to a branch often doesn't give good results, unless propagating flags
				// Special case for Pentium: allow prop of
				// r12 := SETFFLAGS(...) (fflags stored via register AH)
				if (!hasSetFlags(defRight))
					// Allow propagation of constants
					if (defRight->isIntConst() || defRight->isFltConst())
						if (VERBOSE) LOG << "Allowing prop. into branch (1) of " << def << "\n";
						else
							;
					// ?? Also allow any assignments to temps or assignment of anything to anything subscripted.
					// Trent: was the latter meant to be anything NOT subscripted?
					else if (defRight->getOper() != opSubscript && !adef->getLeft()->isTemp())
						continue;
					else
						if (VERBOSE) LOG << "Allowing prop. into branch (2) of " << def << "\n";
			}
#endif
			if (adef->getLeft()->getType() && adef->getLeft()->getType()->isArray()) {
				// Assigning to an array, don't propagate
				continue;
			}
			if (limit && ! (*adef->getLeft() == *regSp)) {
				// Try to prevent too much propagation, e.g. fromSSA, sumarray
				LocationSet used;
				adef->addUsedLocs(used);
				RefExp left(adef->getLeft(), (Statement*)-1);
				RefExp *right = dynamic_cast<RefExp*>(adef->getRight());
				if (used.exists(&left) && !(right && *right->getSubExp1() == *left.getSubExp1()))
					// We have something like eax = eax + 1
					continue;
			}
			change = doPropagateTo(memDepth, adef, convert);
		}
	} while (change && ++changes < 20);
	// Simplify is very costly, especially for calls. I hope that doing one simplify at the end will not affect any
	// result...
	simplify();
	return convert;
}


// Parameter convert is set true if an indirect call is converted to direct
// Return true if a change made
bool Statement::doPropagateTo(int memDepth, Assign* def, bool& convert) {
	// Check the depth of the definition (an assignment)
	// This checks the depth for the left and right sides, and gives the max for both. Example: can't propagate
	// tmp := m[x] to foo := tmp if memDepth == 0
	int depth = def->getMemDepth();
	// MVE: check if we want the test for memDepth == -1
	if (depth > memDepth && memDepth != -1)
		return false;

	// Respect the -p N switch
	if (Boomerang::get()->numToPropagate >= 0) {
		if (Boomerang::get()->numToPropagate == 0) return false;
			Boomerang::get()->numToPropagate--;
	}

	if (VERBOSE)
		LOG << "Propagating " << def << "\n" << "	   into " << this << "\n";
	convert |= replaceRef(def);
	// simplify is costly... done once above
	// simplify();
	if (VERBOSE) {
		LOG << "	 result " << this << "\n\n";
	}
	return true;		// FIXME! Should be true if changed. Need a visitor
}

#if 0
bool Statement::operator==(Statement& other) {
	// When do we need this?
	assert(0);
	Assign* ae1 = dynamic_cast<Assign*>(this);
	Assign* ae2 = dynamic_cast<Assign*>(&other);
	assert(ae1);
	assert(ae2);
	return *ae1 == *ae2;
	return false;
}
#endif

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
 * RETURNS:			True if any change
 *============================================================================*/
bool GotoStatement::searchAndReplace(Exp* search, Exp* replace) {
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
void GotoStatement::print(std::ostream& os /*= cout*/) {
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
#if 0
	switch(cond) {
		case BRANCH_JE:
			p = new Terminal(opZF);
			break;
		case BRANCH_JNE:
			p = new Unary(opNot, new Terminal(opZF));
			break;
		case BRANCH_JSL:
			// N xor V
			p = new Binary(opNotEqual, new Terminal(opNF), new Terminal(opOF));
			break;
		case BRANCH_JSLE:
			// Z or (N xor V)
			p = new Binary(opOr,
				new Terminal(opZF),
				new Binary(opNotEqual, new Terminal(opNF), new Terminal(opOF)));
			break;
		case BRANCH_JSGE:
			// not (N xor V) same as (N == V)
			p = new Binary(opEquals, new Terminal(opNF), new Terminal(opOF));
			break;
		case BRANCH_JSG:
			// not (Z or (N xor V))
			p = new Unary(opNot,
				new Binary(opOr,
					new Terminal(opZF),
					new Binary(opNotEqual,
						new Terminal(opNF), new Terminal(opOF))));
			break;
		case BRANCH_JUL:
			// C
			p = new Terminal(opCF);
			break;
		case BRANCH_JULE:
			// C or Z
			p = new Binary(opOr, new Terminal(opCF), 
								 new Terminal(opZF));
			break;
		case BRANCH_JUGE:
			// not C
			p = new Unary(opNot, new Terminal(opCF));
			break;
		case BRANCH_JUG:
			// not (C or Z)
			p = new Unary(opNot,
				new Binary(opOr,
					new Terminal(opCF),
					new Terminal(opZF)));
			break;
		case BRANCH_JMI:
			// N
			p = new Terminal(opNF);
			break;
		case BRANCH_JPOS:
			// not N
			p = new Unary(opNot, new Terminal(opNF));
			break;
		case BRANCH_JOF:
			// V
			p = new Terminal(opOF);
			break;
		case BRANCH_JNOF:
			// not V
			p = new Unary(opNot, new Terminal(opOF));
			break;
		case BRANCH_JPAR:
			// Can't handle (could happen as a result of a failure of Pentium
			// floating point analysis)
			assert(false);
			break;
	}
#else
	p = new Terminal(usesFloat ? opFflags : opFlags);
#endif
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
 * RETURNS:			True if any change
 *============================================================================*/
bool BranchStatement::searchAndReplace(Exp* search, Exp* replace) {
	GotoStatement::searchAndReplace(search, replace);
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
void BranchStatement::print(std::ostream& os /*= cout*/) {
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

bool BranchStatement::doReplaceRef(Exp* from, Exp* to) {
	bool change;
	assert(pCond);
	pCond = pCond->searchReplaceAll(from, to, change);
	simplify();
	return false;
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
 * RETURNS:			True if any change
 *============================================================================*/
bool CaseStatement::searchAndReplace(Exp* search, Exp* replace) {
	GotoStatement::searchAndReplace(search, replace);
	bool ch = false;
	if (pSwitchInfo && pSwitchInfo->pSwitchVar)
		pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->searchReplaceAll(search, replace, ch);
	return ch;
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
void CaseStatement::print(std::ostream& os /*= cout*/) {
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

bool CaseStatement::doReplaceRef(Exp* from, Exp* to) {
	bool change;
	if (pDest) {
		pDest = pDest->searchReplaceAll(from, to, change);
		pDest = pDest->simplify();
		return false;
	}
	assert(pSwitchInfo && pSwitchInfo->pSwitchVar);
	pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->searchReplaceAll(from, to, change);
	pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->simplify();
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
		Exp*& sub1 = ((Location*)e)->refSubExp1();
		sub1 = localiseExp(sub1, depth);
	}
}
// Substitute the various components of expression e with the appropriate reaching definitions.
// Used in e.g. fixCallBypass (via the CallBypasser). Locations defined in this call are replaced with their proven
// values, which are in terms of the initial values at the start of the call (reaching definitions at the call)
Exp* CallStatement::localiseExp(Exp* e, int depth /* = -1 */) {
	if (!defCol.isInitialised()) return e;				// Don't attempt to subscript if the data flow not started yet
	Localiser l(this, depth);
	e = e->clone()->accept(&l);

#if 0		// Use the BypassingPropagator to do this now
	// Now check if e happens to have components that are references to other call statements
	// Example: test/pentium/fib: r29 needs to get through 2 call statements to the original def
	LocationSet locs;
	LocationSet::iterator xx;
	locs.clear();
	e->addUsedLocs(locs);
	for (xx = locs.begin(); xx != locs.end(); xx++) {		// For each used location in e
		if (!(*xx)->isSubscript())
			continue;										// Could be e.g. m[x{y} + K]
		Statement* def = ((RefExp*)*xx)->getDef();
		Exp* base = ((RefExp*)*xx)->getSubExp1();
		if (def && def->isCall()) {
			Exp* r = ((CallStatement*)def)->localiseExp(base);
			bool ch;
			e = e->searchReplaceAll(*xx, r, ch);
		}
	}
	//e = e->simplifyArith()->simplify();
	// Now propagate to this expression
	//return e->propagateToExp();
#else
	return e;
#endif
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
 * RETURNS:			True if any change
 *============================================================================*/
bool CallStatement::searchAndReplace(Exp* search, Exp* replace) {
	bool change = GotoStatement::searchAndReplace(search, replace);
	StatementList::iterator ss;
	// FIXME: MVE: Check if we ever want to change the LHS of arguments or defines...
	for (ss = defines.begin(); ss != defines.end(); ++ss)
		change |= (*ss)->searchAndReplace(search, replace);
	for (ss = arguments.begin(); ss != arguments.end(); ++ss)
		change |= (*ss)->searchAndReplace(search, replace);
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
void CallStatement::print(std::ostream& os /*= cout*/) {
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
	}

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
	os << "(\n";
	StatementList::iterator aa;
	for (aa = arguments.begin(); aa != arguments.end(); ++aa) {
		os << "                ";
		((Assignment*)*aa)->printCompact(os);
		os << "\n";
	}
	os << "              )";

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
	assert(procDest == NULL);
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

bool CallStatement::isDefinition() {
	LocationSet defs;
	getDefinitions(defs);
	return defs.size() != 0;
}

void CallStatement::getDefinitions(LocationSet &defs) {
	StatementList::iterator dd;
	for (dd = defines.begin(); dd != defines.end(); ++dd)
		defs.insert(((Assignment*)*dd)->getLeft());
	// Childless calls are supposed to define everything. In practice they don't really define things like %pc, so we
	// need some extra logic in getTypeFor()
	if (isChildless())
		defs.insert(new Terminal(opDefineAll));
}

bool CallStatement::convertToDirect() {
	if (!m_isComputed)
		return false;
	bool convertIndirect = false;
	Exp *e = pDest;
	if (pDest->isSubscript())
		e = ((RefExp*)e)->getSubExp1();
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
	Proc *p = proc->getProg()->findProc(nam);
	if (p == NULL)
		p = proc->getProg()->getLibraryProc(nam);
	if (VERBOSE)
		LOG << "this is a global '" << nam << "'\n";
	if (p) {
		if (VERBOSE) {
			LOG << "this is a proc " << p->getName() << "\n";
			std::ostringstream st;
			print(st);
			LOG << st.str().c_str();			
		}
		// we need to:
		// 1) replace the current return set with the return set of the new procDest
		// 2) call fixCallBypass on the enclosing procedure
		// 3) fix the arguments (this will only affect the implicit arguments, the regular arguments should
		//    be empty at this point)
		// 3a replace current arguments with those of the new proc
		// 3b copy the signature from the new proc
		// 4) change this to a non-indirect call
		procDest = p;
		Signature *sig = p->getSignature();

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
					newimpargs[i]->refSubExp1() = localiseExp(newimpargs[i]-> getSubExp1());
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
					newargs[i]->refSubExp1() = localiseExp(newargs[i]->getSubExp1());
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
	}
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
			advance(aa, n);
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

Exp* CallStatement::getArgumentExp(int i)
{
	assert(i < (int)arguments.size());
	StatementList::iterator aa = arguments.begin();
	advance(aa, i);
	return ((Assign*)*aa)->getRight();
}

void CallStatement::setArgumentExp(int i, Exp *e)
{
	assert(i < (int)arguments.size());
	StatementList::iterator aa = arguments.begin();
	advance(aa, i);
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
	advance(aa, n);
	arguments.erase(aa, arguments.end());
	// MVE: check if these need extra propagation
	for (int i = oldSize; i < n; i++) {
		Exp* a = procDest->getSignature()->getArgumentExp(i);
		Assign* as = new Assign(new VoidType(), a->clone(), a->clone());
		as->setProc(proc);
		arguments.append(as);
	}
}

void CallStatement::removeArgument(int i)
{
	StatementList::iterator aa = arguments.begin();
	advance(aa, i);
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
Exp *processConstant(Exp *e, Type *t, Prog *prog, UserProc* proc) {
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
				if (VERBOSE)
					LOG << "found function pointer with constant value " << "of type " << pt->getCtype() 
						<< ".  Decoding address " << a << "\n";
				if (!Boomerang::get()->noDecodeChildren)
					prog->decodeExtraEntrypoint(a);
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
	// See if it is in our reaching definitions
	Exp* ref = defCol.findDefFor(e);
	if (ref == NULL || !ref->isSubscript()) return NULL;
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

		((Assign*)*aa)->setRight(processConstant(e, t, prog, proc));
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
		LOG << "Ellipsis processing for " << name << "\n";
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

bool ReturnStatement::searchAndReplace(Exp* search, Exp* replace) {
	bool change = false;
	ReturnStatement::iterator rr;
	for (rr = begin(); rr != end(); ++rr)
		change |= (*rr)->searchAndReplace(search, replace);
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

bool ReturnStatement::usesExp(Exp *e) {
	Exp *where;
	ReturnStatement::iterator rr;
	for (rr = begin(); rr != end(); ++rr) {
		if ((*rr)->search(e, where))
			return true;
	}
	return false;
}

bool ReturnStatement::doReplaceRef(Exp* from, Exp* to) {
	bool change = false;
	ReturnStatement::iterator rr;
	for (rr = begin(); rr != end(); ++rr)
		change |= (*rr)->doReplaceRef(from, to);
	// Ugh - even though we can lookup assignments based on the LHS, it is quite possible to have assignments such
	// as a := b+c, which need changing if from happened to be b or c
	col.searchReplaceAll(from, to, change);
	return change;
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

bool BoolAssign::searchAndReplace(Exp *search, Exp *replace) {
	bool change = false;
	assert(pCond);
	assert(lhs);
	pCond = pCond->searchReplaceAll(search, replace, change);
	 lhs  =	  lhs->searchReplaceAll(search, replace, change);
	return change;
}

// Convert from SSA form
void BoolAssign::fromSSAform(igraph& ig) {
	pCond = pCond->fromSSA(ig); 
	lhs	  = lhs	 ->fromSSAleft(ig, this);
}

bool BoolAssign::doReplaceRef(Exp* from, Exp* to) {
	searchAndReplace(from, to);
	simplify();
	return false;
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

Assignment::Assignment(Exp* lhs) : type(new VoidType), lhs(lhs) {}
Assignment::Assignment(Type* ty, Exp* lhs) : type(ty), lhs(lhs) {}
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
				int b = ((Const*)a4->getRight()->getSubExp2())->getInt();
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
		lhs->refSubExp1() = lhs->getSubExp1()->simplifyArith();
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
		if (VERBOSE)
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
		if (VERBOSE)
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
			// HACK, if e is NULL assume it is ment to match lhs
			if (defVec[i].e == NULL)
				defVec[i].e = lhs;
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
			os << it->e << "{";
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
bool PhiAssign::searchAll(Exp* search, std::list<Exp*>& result) {
	return lhs->searchAll(search, result);
}
bool ImplicitAssign::searchAll(Exp* search, std::list<Exp*>& result) {
	return lhs->searchAll(search, result);
}

bool Assign::searchAndReplace(Exp* search, Exp* replace) {
	bool change = false;
	lhs = lhs->searchReplaceAll(search, replace, change);
	rhs = rhs->searchReplaceAll(search, replace, change);
	if (guard)
		guard = guard->searchReplaceAll(search, replace, change);
	return change;
}
bool PhiAssign::searchAndReplace(Exp* search, Exp* replace) {
	bool change = false;
	lhs = lhs->searchReplaceAll(search, replace, change);
	std::vector<PhiInfo>::iterator it;
	for (it = defVec.begin(); it != defVec.end(); it++)
		// Assume that the definitions will also be replaced
		it->e = it->e->searchReplaceAll(search, replace, change);
	return change;
}
bool ImplicitAssign::searchAndReplace(Exp* search, Exp* replace) {
	bool change = false;
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
	rhs = processConstant(rhs, Statement::getTypeFor(lhs, prog), prog, proc);
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
					advance(aa, n);
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
#if 0
	for (it = implicitArguments.begin(); ret && it != implicitArguments.end(); it++)
		ret = (*it)->accept(v->ev);
#endif
#if 0		// Do we want to accept changes to the returns? Not sure now...
	std::vector<ReturnInfo>::iterator rr;
	for (rr = defines.begin(); ret && rr != defines.end(); rr++)
		if (rr->e)			// Can be NULL now to line up with other returns
			ret = rr->e->accept(v->ev);
#endif
	return ret;
}

bool ReturnStatement::accept(StmtExpVisitor* v) {
	bool override;
	if (!v->visit(this, override))
		return false;
	if (override) return true;
	DefCollector::iterator dd;
	for (dd = col.begin(); dd != col.end(); ++dd)
		if (!(*dd)->accept(v))
			return false;
	ReturnStatement::iterator rr;
	for (rr = modifieds.begin(); rr != modifieds.end(); ++rr)
		if (!(*rr)->accept(v))
			return false;
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
	if (pDest && recur)
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

bool BoolAssign::accept(StmtModifier* v) {
	bool recur;
	v->visit(this, recur);
	if (pCond && recur)
		pCond = pCond->accept(v->mod);
	if (recur && lhs->isMemOf()) {
		Exp*& sub1 = ((Location*)lhs)->refSubExp1();
		sub1 = sub1->accept(v->mod);
	}
	return true;
}

// Visiting from class StmtPartModifier
// Modify all the various expressions in a statement, except for the top level of the LHS of assignments
bool Assign::accept(StmtPartModifier* v) {
	bool recur;
	v->visit(this, recur);
	v->mod->clearMod();
	if (recur && lhs->isMemOf()) {
		Exp*& sub1 = ((Location*)lhs)->refSubExp1();
		sub1 = sub1->accept(v->mod);
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
		Exp*& sub1 = ((Location*)lhs)->refSubExp1();
		sub1 = sub1->accept(v->mod);
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
		Exp*& sub1 = ((Location*)lhs)->refSubExp1();
		sub1 = sub1->accept(v->mod);
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
#if 0
	Collector::iterator cc;
	for (cc = defCol.begin(); cc != defCol.end(); cc++)
		(*cc)->accept(v->mod);			// Always refexps, so never need to change Exp pointer (i.e. don't need *cc = )
#endif
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
void Statement::bypassAndPropagate() {
	BypassingPropagator bp(this);
	StmtPartModifier sm(&bp);			// Use the Part modifier so we don't change the top level of LHS of assigns etc
	accept(&sm);
	if (bp.isTopChanged())
		simplify();						// E.g. m[esp{20}] := blah -> m[esp{-}-20+4] := blah
}

// Find the locations used by expressions in this Statement.
// Use the StmtExpVisitor and UsedLocsFinder visitor classes
void Statement::addUsedLocs(LocationSet& used, bool cc) {
	UsedLocsFinder ulf(used);
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

// Convert this PhiAssignment to an ordinary Assignment
// Hopefully, this is the only place that Statements change from one form
// to another.
// All throughout the code, we assume that the addresses of Statement objects
// do not change, so we need this slight hack to overwrite one object with another
void PhiAssign::convertToAssign(Exp* rhs) {
	Assign* a = new Assign(type, lhs, rhs);
	a->setNumber(number);
	a->setProc(proc);
	a->setBB(pbb);
	assert(sizeof(Assign) <= sizeof(PhiAssign));
	memcpy(this, a, sizeof(Assign));
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

Type* Statement::meetWithFor(Type* ty, Exp* e, bool& ch) {
	bool thisCh = false;
	Type* newType = getTypeFor(e)->meetWith(ty, thisCh);
	if (thisCh) {
		ch = true;
		setTypeFor(e, newType);
	}
	return newType;
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
// collector have changed. Does NOT remove preserveds.
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
#if 0
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

	// Move the defines to a temporary list
	StatementList oldDefines(defines);					// Copy the old defines
	StatementList::iterator it;
	defines.clear();

	if (procDest && procDest->isLib()) {
		sig->setLibraryDefines(&defines);				// Set the locations defined
		return;
	} else if (procDest && calleeReturn) {
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
		src = SRC_COL;
		defCol = call->getDefCollector();
		cc = defCol->begin();
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
	return call->localiseExp(e->clone());
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
		else if there is a callee, source = callee parameters
		else no callee, source is def collector in this call.
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
	StatementList oldArguments(arguments);
	arguments.clear();
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
				if (useCol.exists(sigReturn)) {
					ImplicitAssign* as = new ImplicitAssign(sig->getReturnType(i), sigReturn);
					ret->append(as);
				}
			}
		} else {
			Exp* rsp = Location::regOf(sig->getStackRegister());
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
		assert("not implemented yet" - "not implemented yet");
	}
	return ret;
}

Type* Assignment::getType() {
	if (ADHOC_TYPE_ANALYSIS)
		return lhs->getType();
	else
		return type;
}

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
		if (!procDest->isLib() &&
				((UserProc*)procDest)->isLocalOrParam(base)) {
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
