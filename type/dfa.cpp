/*
 * Copyright (C) 2004, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   dfa.cpp
 * OVERVIEW:   Implementation of class Type functions related to solving
 *             type analysis in an iterative, data-flow-based manner
 *============================================================================*/

/*
 * $Revision$
 *
 * 24/Sep/04 - Mike: Created
 */

#include "type.h"
#include "boomerang.h"
#include "signature.h"
#include "exp.h"
#include "prog.h"
#include "util.h"
#include "visitor.h"
#include <sstream>

static int nextUnionNumber = 0;

int max(int a, int b) {		// Faster to write than to find the #include for
	return a>b ? a : b;
}

#define DFA_ITER_LIMIT 20

void UserProc::dfaTypeAnalysis() {
	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator it;
	bool ch;
	int iter;
	for (iter = 1; iter <= DFA_ITER_LIMIT; iter++) {
		ch = false;
		for (it = stmts.begin(); it != stmts.end(); it++) {
			(*it)->dfaTypeAnalysis(ch);	  
		}
		if (!ch)
			// No more changes: round robin algorithm terminates
			break;
	}
	if (ch)
		LOG << "**** Iteration limit exceeded for dfaTypeAnalysis of procedure " << getName() << " ****\n";

	if (DEBUG_TA) {
		LOG << "\n *** Results for Data flow based Type Analysis for " << getName() << " ***\n";
		LOG << iter << " iterations\n";
		for (it = stmts.begin(); it != stmts.end(); it++) {
			Statement* s = *it;
			LOG << s << "\n";			// Print the statement; has dest type
			// Now print type for each constant in this Statement
			std::list<Const*> lc;
			std::list<Const*>::iterator cc;
			s->findConstants(lc);
			if (lc.size()) {
				LOG << "       ";
				for (cc = lc.begin(); cc != lc.end(); cc++)
					LOG << (*cc)->getType()->getCtype() << " " << *cc << "  ";
				LOG << "\n";
			}
		}
		LOG << "\n *** End results for Data flow based Type Analysis for " << getName() << " ***\n\n";
	}

	// Now use the type information gathered
	Prog* prog = getProg();
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		//Type* t = s->getType();
		// Locations
		// ...
		// Convert expressions to locals
		s->dfaConvertLocals();
		// Constants
		std::list<Const*>lc;
		s->findConstants(lc);
		std::list<Const*>::iterator cc;
		for (cc = lc.begin(); cc != lc.end(); cc++) {
			Const* con = (Const*)*cc;
			Type* t = con->getType();
			int val = con->getInt();
			if (t && t->isPointer()) {
				PointerType* pt = t->asPointer();
				Type* baseType = pt->getPointsTo();
				if (baseType->resolvesToChar()) {
					// Convert to a string	MVE: check for read-only?
					// Also, distinguish between pointer to one char, and ptr to many?
					char* str = prog->getStringConstant(val, true);
					if (str) {
						// Make a string
						con->setStr(escapeStr(str));
						con->setOper(opStrConst);
					}
				} else if (baseType->resolvesToInteger() || baseType->resolvesToFloat()) {
					ADDRESS addr = (ADDRESS) con->getInt();
					prog->globalUsed(addr, baseType);
					const char *gloName = prog->getGlobalName(addr);
					if (gloName) {
						ADDRESS r = addr - prog->getGlobalAddr((char*)gloName);
						Exp *ne;
						if (r) {
							Location *g = Location::global(strdup(gloName), this);
							ne = Location::memOf(
								new Binary(opPlus,
									new Unary(opAddrOf, g),
									new Const(r)), this);
						} else {
							Type *ty = prog->getGlobalType((char*)gloName);
							if (s->isAssign() && ((Assign*)s)->getType()) {
								int bits = ((Assign*)s)->getType()->getSize();
								if (ty == NULL || ty->getSize() == 0)
									prog->setGlobalType((char*)gloName, new IntegerType(bits));
							}
							Location *g = Location::global(strdup(gloName), this);
							if (ty && ty->isArray()) 
								ne = new Binary(opArraySubscript, g, new Const(0));
							else 
								ne = g;
						}
						Exp* memof = Location::memOf(con);
						s->searchAndReplace(memof->clone(), ne);
					}
				}
			}
		}
	}

	if (VERBOSE) {
		LOG << "*** After application of DFA Type Analysis for " << getName() << " ***\n";
		printToLog();
		LOG << "*** End application of DFA Type Analysis for " << getName() << " ***\n";
	}
}

// This is the core of the data-flow-based type analysis algorithm: implementing the meet operator.
// In classic lattice-based terms, the TOP type is void; there is no BOTTOM type since we handle
// overconstraints with unions.
// Consider various pieces of knowledge about the types. There could be:
// a) void: no information. Void meet x = x.
// b) size only: find a size large enough to contain the two types.
// c) broad type only, e.g. floating point
// d) signedness, no size
// e) size, no signedness
// f) broad type, size, and (for integer broad type), 

// ch set true if any change

Type* VoidType::meetWith(Type* other, bool& ch) {
	// void meet x = x
	ch |= !other->isVoid();
	return other;
}

Type* FuncType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (*this == *other) return this;		// NOTE: at present, compares names as well as types and number of parameters
	return createUnion(other, ch);
}

Type* IntegerType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isInteger()) {
		IntegerType* otherInt = other->asInteger();
		// Signedness
		int oldSignedness = signedness;
		if (otherInt->signedness > 0)
			signedness++;
		else if (otherInt->signedness < 0)
			signedness--;
		ch |= (signedness >= 0 != oldSignedness >= 0);
		// Size. Assume 0 indicates unknown size
		int oldSize = size;
		size = max(size, otherInt->size);
		ch |= (size != oldSize);
		return this;
	}
	if (other->isSize()) {
		if (size == 0) {		// Doubt this will ever happen
			size = ((SizeType*)other)->getSize();
			return this;
		}
		if (size == ((SizeType*)other)->getSize()) return this;
		LOG << "Integer size " << size << " meet with SizeType size " << ((SizeType*)other)->getSize() << "!\n";
		size = max(size, ((SizeType*)other)->getSize());
		return this;
	}
	return createUnion(other, ch);
}

Type* FloatType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isFloat()) {
		FloatType* otherFlt = other->asFloat();
		int oldSize = size;
		size = max(size, otherFlt->size);
		ch |= size != oldSize;
		return this;
	}
	if (other->isSize()) {
		int otherSize = other->getSize();
		ch |= size != otherSize;
		size = max(size, otherSize);
		return this;
	}
	return createUnion(other, ch);
}

Type* BooleanType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isBoolean())
		return this;
	return createUnion(other, ch);
}

Type* CharType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isChar()) return this;
	// Also allow char to merge with integer
	if (other->isInteger()) {
		ch = true;
		return other;
	}
	return createUnion(other, ch);
}

Type* PointerType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isSize() && ((SizeType*)other)->getSize() == STD_SIZE) return this;
	if (other->isPointer()) {
		PointerType* otherPtr = other->asPointer();
		if (pointsToAlpha() && !otherPtr->pointsToAlpha()) {
			setPointsTo(otherPtr->getPointsTo());
			ch = true;
		} else {
			// We have a meeting of two pointers. First, see if the base types will meet
			bool baseCh = false;
			Type* thisBase = getPointsTo();
			Type* otherBase = otherPtr->getPointsTo();
			if (otherBase->isPointer())
				// Don't recurse infinately. Just union the pointers
				return createUnion(other, ch);
			thisBase = thisBase->meetWith(otherBase, baseCh);
			if (thisBase->isUnion()) {
				// The bases did not meet successfully. Union the pointers.
				return createUnion(other, ch);
			} else {
				// The bases did meet successfully. Return a pointer to this possibly changed type.
				if (baseCh) {
					ch = true;
					setPointsTo(thisBase);
				}
			}
		}
		return this;
	}
	// Would be good to understand class hierarchys, so we know if a* is the same as b* when b is a subclass of a
	return createUnion(other, ch);
}

Type* ArrayType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	// Needs work
	return createUnion(other, ch);
}

Type* NamedType::meetWith(Type* other, bool& ch) {
	return resolvesTo()->meetWith(other, ch);
}

Type* CompoundType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (!other->isCompound()) return createUnion(other, ch);
	CompoundType* otherCmp = other->asCompound();
	if (otherCmp->isSuperStructOf(this)) {
		// The other structure has a superset of my struct's offsets. Preserve the names etc of the bigger struct.
		ch = true;
		return this;
	}
	if (isSubStructOf(otherCmp)) {
		// This is a superstruct of other
		ch = true;
		return this;
	}
	if (*this == *other) return this;
	// Not compatible structs. Create a union of both complete structs.
	// NOTE: may be possible to take advantage of some overlaps of the two structures some day.
	return createUnion(other, ch);
}

Type* UnionType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (*this == *other) return this;
	return createUnion(other, ch);
}

Type* SizeType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isSize()) {
		if (((SizeType*)other)->size != size) {
			LOG << "size " << size << " meet with size " << ((SizeType*)other)->size << "!\n";
			size = max(size, ((SizeType*)other)->size);
			ch = true;
		}
		return this;
	}
	if (other->isInteger() || other->isFloat() || other->isPointer()) {
		other->setSize(max(size, other->getSize()));
	}
	ch = true;
	return other;
}

Type* Type::createUnion(Type* other, bool& ch) {
	char name[20];
	sprintf(name, "x%d", ++nextUnionNumber);
	if (isUnion()) {
		if (((UnionType*)this)->findType(other))
			// The type already exists; no change
			return this;
		ch = true;
		((UnionType*)this)->addType(other, name);
		return this;
	}
	if (other->isUnion()) {
		if (((UnionType*)other)->findType(this))
			// The type already exists in the other union
			return other;
		ch = true;
		((UnionType*)other)->addType(this, name);
		return other;
	}
	UnionType* u = new UnionType;
	u->addType(this, name);
	sprintf(name, "x%d", ++nextUnionNumber);
	u->addType(other, name);
	ch = true;
	return u;
}


void CallStatement::dfaTypeAnalysis(bool& ch) {
	Signature* sig = procDest->getSignature();
	Prog* prog = procDest->getProg();
	// Iterate through the arguments
	int n = sig->getNumParams();
	for (int i=0; i < n; i++) {
		Exp* e = getArgumentExp(i);
		Type* t = sig->getParamType(i);
		Const* c;
		if (e->isSubscript()) {
			// A subscripted location. Find the definition
			RefExp* r = (RefExp*)e;
			Statement* def = r->getRef();
			assert(def);
			Type* tParam = def->getType();
			assert(tParam);
			Type* oldTparam = tParam;
			tParam = tParam->meetWith(t, ch);
			// Set the type of def, and if r is a memof, handle the memof operand
			r->descendType(tParam, ch);
			if (DEBUG_TA && tParam != oldTparam)
				LOG << "Type of " << r << " changed from " << oldTparam->getCtype() << " to " <<
					tParam->getCtype() << "\n";
		} else if ((c = dynamic_cast<Const*>(e)) != NULL) {
			// A constant.
			Type* oldConType = c->getType();
			c->setType(t);
			ch |= (t != oldConType);
		} else if (t->isPointer() && sig->isAddrOfStackLocal(prog, e)) {
			// e is probably the address of some local
			Exp* localExp = Location::memOf(e);
			char* name = proc->getSymbolName(localExp);
			if (name) {
				Exp* old = arguments[i]->clone();
				arguments[i] = new Unary(opAddrOf, Location::local(name, proc));
				if (DEBUG_TA)
					LOG << "Changed argument " << i << " was " << old << ", result is " << this << "\n";
			}
		}
	}
	// The destination is a pointer to a function with this function's signature (if any)
	if (pDest) {
		Signature* sig;
		if (procDest)
			sig = procDest->getSignature();
		else
			sig = NULL;
		pDest->descendType(new FuncType(sig), ch);
	}
}

// For x0 := phi(x1, x2, ...) want
// Ex0 := Ex0 meet (Ex1 meet Ex2 meet ...)
// Ex1 := Ex1 meet Ex0
// Ex2 := Ex1 meet Ex0
// ...
void PhiAssign::dfaTypeAnalysis(bool& ch) {
	unsigned i, n = stmtVec.size();
	Type* meetOfPred = stmtVec[0]->getType();
	for (i=1; i < n; i++)
		if (stmtVec[i] && stmtVec[i]->getType())
			meetOfPred = meetOfPred->meetWith(stmtVec[i]->getType(), ch);
	type = type->meetWith(meetOfPred, ch);
	for (i=0; i < n; i++) {
		if (stmtVec[i] && stmtVec[i]->getType()) {
			bool thisCh = false;
			Type* res = stmtVec[i]->getType()->meetWith(type, thisCh);
			if (thisCh) {
				stmtVec[i]->setType(res);
				ch = true;
			}
		}
	}
	Assignment::dfaTypeAnalysis(ch);		// Handle the LHS
}

void Assign::dfaTypeAnalysis(bool& ch) {
	Type* tr = rhs->ascendType();
	type = type->meetWith(tr, ch);
	rhs->descendType(type, ch);
	Assignment::dfaTypeAnalysis(ch);		// Handle the LHS
}

void Assignment::dfaTypeAnalysis(bool& ch) {
	if (lhs->isMemOf())
		// Push down the fact that the memof is a pointer to the assignment type
		lhs->descendType(type, ch);
}

void BranchStatement::dfaTypeAnalysis(bool& ch) {
	pCond->descendType(new BooleanType(), ch);
	// Not fully implemented yet?
}

void BoolAssign::dfaTypeAnalysis(bool& ch) {
	// Not implemented yet
}

void ReturnStatement::dfaTypeAnalysis(bool& ch) {
	// Not implemented yet
}

// Special operators for handling addition and subtraction in a data flow based type analysis
//					ta=
//  tb=		alpha*	int		pi
// alpha*	bottom	alpha*	alpha*
// int		alpha*	int		pi
// pi		alpha*	pi		pi
Type* sigmaSum(Type* ta, Type* tb) {
	bool ch;
	if (ta->isPointer()) {
		if (tb->isPointer())
			return ta->createUnion(tb, ch);
		return ta;
	}
	if (ta->isInteger()) {
		if (tb->isPointer())
			return tb;
		return tb;
	}
	if (tb->isPointer())
		return tb;
	return ta;
}


//					tc=
//  to=		alpha*	int		pi
// alpha*	int		bottom	int
// int		alpha*	int		pi
// pi		pi		pi		pi
Type* sigmaAddend(Type* tc, Type* to) {
	bool ch;
	if (tc->isPointer()) {
		if (to->isPointer())
			return new IntegerType;
		if (to->isInteger())
			return tc;
		return to;
	}
	if (tc->isInteger()) {
		if (to->isPointer())
			return tc->createUnion(to, ch);
		return to;
	}
	if (to->isPointer())
		return new IntegerType;
	return tc;
}

//					tc=
//  tb=		alpha*	int		pi
// alpha*	bottom	alpha*	alpha*
// int		alpha*	int		pi
// pi		alpha*	int		pi
Type* deltaSubtrahend(Type* tc, Type* tb) {
	bool ch;
	if (tc->isPointer()) {
		if (tb->isPointer())
			return tc->createUnion(tb, ch);
		return tc;
	}
	if (tc->isInteger()) {
		if (tb->isPointer())
			return tb;
		return tc;
	}
	if (tb->isPointer())
		return tb;
	return tc;
}

//					tc=
//  ta=		alpha*	int		pi
// alpha*	int		alpha*	pi
// int		bottom	int		int
// pi		alpha*	int		pi
Type* deltaSubtractor(Type* tc, Type* ta) {
	bool ch;
	if (tc->isPointer()) {
		if (ta->isPointer())
			return new IntegerType;
		if (ta->isInteger())
			return tc->createUnion(ta, ch);
		return new IntegerType;
	}
	if (tc->isInteger())
		return ta;
	if (ta->isPointer())
		return tc;
	return ta;
}

//					ta=
//  tb=		alpha*	int		pi
// alpha*	int		bottom	int
// int		alpha*	int		pi
// pi		pi		int		pi
Type* deltaDifference(Type* ta, Type* tb) {
	bool ch;
	if (ta->isPointer()) {
		if (tb->isPointer())
			return new IntegerType;
		if (tb->isInteger())
			return ta;
		return tb;
	}
	if (ta->isInteger()) {
		if (tb->isPointer())
			return ta->createUnion(tb, ch);
		if (tb->isInteger())
			return tb;
		return new IntegerType;
	}
	if (tb->isPointer())
		return new IntegerType;
	return ta;
}

//	//	//	//	//	//	//	//	//	//	//
//										//
//	ascendType: draw type information	//
//		up the expression tree			//
//										//
//	//	//	//	//	//	//	//	//	//	//

Type* Binary::ascendType() {
	if (op == opFlagCall) return new VoidType;
	Type* ta = subExp1->ascendType();
	Type* tb = subExp2->ascendType();
	switch (op) {
		case opPlus:
			return sigmaSum(ta, tb);
		case opMinus:
			return deltaDifference(ta, tb);
		default:
			// Many more cases to implement
			return new VoidType;
	}
}

// Constants and subscripted locations are at the leaves of the expression tree. Just return their stored types.
Type* RefExp::ascendType() {
if (def == NULL) {
	std::cerr << "Warning! Null reference in " << this << "\n";
	return new VoidType;
}
	return def->getType();
}
Type* Const::ascendType() {
	if (type->isVoid()) {
		switch (op) {
			case opIntConst:
				if (u.i != 0 && (u.i < 0x1000 && u.i > -0x100))
					// Assume that small nonzero integer constants are of integer type (can't be pointers)
					type = new IntegerType(STD_SIZE, u.i < 0);
				break;
			case opLongConst:
				type = new IntegerType(STD_SIZE*2, u.i < 0);
				break;
			case opFltConst:
				type = new FloatType(64);
				break;
			case opStrConst:
				type = new PointerType(new CharType);
				break;
			case opFuncConst:
				type = new FuncType;		// More needed here?
				break;
			default:
				assert(0);					// Bad Const
		}
	}
	return type;
}
// Can also find various terminals at the leaves of an expression tree
Type* Terminal::ascendType() {
	switch (op) {
		case opPC:
			return new IntegerType(STD_SIZE, -1);
		case opCF: case opZF:
			return new BooleanType;
		default:
			std::cerr << "Type for terminal " << this << " not implemented!\n";
			return new VoidType;
	}
}

Type* Unary::ascendType() {
	Type* ta = subExp1->ascendType();
	switch (op) {
		case opMemOf:
			if (ta->isPointer())
				return ta->asPointer()->getPointsTo();
			else
				return new VoidType();		// NOT SURE! Really should be bottom
			break;
		default:
			break;
	}
	return new VoidType;
}

Type* Ternary::ascendType() {
	return new VoidType;
}

Type* TypedExp::ascendType() {
	return type;
}


//	//	//	//	//	//	//	//	//	//	//
//										//
//	descendType: push type information	//
//		down the expression tree		//
//										//
//	//	//	//	//	//	//	//	//	//	//

void Binary::descendType(Type* parentType, bool& ch) {
	if (op == opFlagCall) return;
	Type* ta = subExp1->ascendType();
	Type* tb = subExp2->ascendType();
	switch (op) {
		case opPlus:
			ta = ta->meetWith(sigmaAddend(parentType, tb), ch);
			subExp1->descendType(ta, ch);
			tb = tb->meetWith(sigmaAddend(parentType, ta), ch);
			subExp2->descendType(tb, ch);
			break;
		case opMinus:
			ta = ta->meetWith(deltaSubtrahend(parentType, tb), ch);
			subExp1->descendType(ta, ch);
			tb = tb->meetWith(deltaSubtractor(parentType, ta), ch);
			subExp2->descendType(tb, ch);
			break;
		case opGtrUns:	case opLessUns:
		case opGtrEqUns:case opLessEqUns:
			ta = ta->meetWith(new IntegerType(32, -1), ch);
			subExp1->descendType(ta, ch);
			tb = tb->meetWith(new IntegerType(32, -1), ch);
			subExp2->descendType(tb, ch);
		default:
			// Many more cases to implement
			break;
	}
}

void RefExp::descendType(Type* parentType, bool& ch) {
	Type* oldType = def->getType();
	def->setType(parentType);
	ch |= oldType != parentType;
	subExp1->descendType(parentType, ch);
}

void Const::descendType(Type* parentType, bool& ch) {
	type = type->meetWith(parentType, ch);
}

void Unary::descendType(Type* parentType, bool& ch) {
	switch (op) {
		case opMemOf:
			subExp1->descendType(new PointerType(parentType), ch);
			break;
		default:
			break;
	}
}

void Ternary::descendType(Type* parentType, bool& ch) {
}

void TypedExp::descendType(Type* parentType, bool& ch) {
}

void Terminal::descendType(Type* parentType, bool& ch) {
}

// Convert expressions to locals, using the (so far DFA based) type analysis information
// Basically, descend types, and when you get to m[...] compare with the local high level pattern;
// when at a sum or difference, check for the address of locals high level pattern that is a pointer
void Statement::dfaConvertLocals() {
	DfaLocalConverter dlc(getType(), proc);
	StmtModifier sm(&dlc);
	accept(&sm);
}

// Convert expressions to locals
DfaLocalConverter::DfaLocalConverter(Type* ty, UserProc* proc) : parentType(ty), proc(proc) {
// Example: BranchStatement... does the condition have a top level type?
if (parentType == NULL) parentType = new VoidType();	// MVE: Hack for now
	sig = proc->getSignature();
	prog = proc->getProg();
	sp = sig->getStackRegister();
}

Exp* DfaLocalConverter::preVisit(Location* e, bool& recur) {
	// Check if this is an appropriate pattern for local variables	
	if (e->isMemOf()) {
		if (sig->isStackLocal(proc->getProg(), e)) {
			recur = false;
			mod = true;			// We've made a modification
			// Don't change parentType; e is a Location now so postVisit won't expect parentTypt changed
			return proc->getLocalExp(e, parentType, true);
		}
		// When we recurse into the m[...], the type will be changed
		parentType = new PointerType(parentType);
	}
	recur = true;
	return e;
}
Exp* DfaLocalConverter::postVisit(Location* e) {
	if (e->isMemOf()) {
		PointerType* pt = parentType->asPointer();
		assert(pt);
		parentType = pt->getPointsTo();
	}
	return e;
}

Exp* DfaLocalConverter::preVisit(Binary* e, bool& recur) {
	// Check for sp -/+ K, but only if TA indicates this is a pointer
	if (parentType->isPointer() && sig->isAddrOfStackLocal(prog, e)) {
		recur = false;
		mod = true;
		return proc->getLocalExp(e, parentType->asPointer()->getPointsTo(), true);	// MVE: Check this!
	}
	recur = true;
	return e;
}

