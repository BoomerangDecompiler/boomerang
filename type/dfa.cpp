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
	// First use the type information from the signature. Sometimes needed to split variables (e.g. argc as a
	// int and char* in sparc/switch_gcc)
	bool ch = signature->dfaTypeAnalysis(cfg);
	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator it;
	int iter;
	for (iter = 1; iter <= DFA_ITER_LIMIT; iter++) {
		ch = false;
		for (it = stmts.begin(); it != stmts.end(); it++) {
			bool thisCh = false;
			(*it)->dfaTypeAnalysis(thisCh);
			if (thisCh) {
				ch = true;
				if (DEBUG_TA)
					LOG << " Caused change: " << *it << "\n";
			}
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
			// If s is a call, also display its return types
			if (s->isCall()) {
				std::vector<ReturnInfo>& returns = ((CallStatement*)s)->getReturns();
				int n = returns.size();
				if (n) {
					LOG << "       Returns: ";
					for (int i=0; i < n; i++)
						if (returns[i].e) LOG << returns[i].type->getCtype() << " " << returns[i].e << "  ";
					LOG << "\n";
				}
			}
		}
		LOG << "\n *** End results for Data flow based Type Analysis for " << getName() << " ***\n\n";
	}

	// Now use the type information gathered
	Prog* prog = getProg();
	if (DEBUG_TA)
		LOG << " *** Converting expressions to local variables for " << getName() << " ***\n";
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		s->dfaConvertLocals();
	}
	if (DEBUG_TA)
		LOG << " *** End converting expressions to local variables for " << getName() << " ***\n";
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		//Type* t = s->getType();
		// Locations
		// ...
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
			} else if (t->isFloat()) {
				if (t->getSize() == 32) {
					// Reinterpret as a float (and convert to double)
					//con->setFlt(reinterpret_cast<float>(con->getInt()));
					int tmp = con->getInt();
					con->setFlt(*(float*)&tmp);		// Reinterpret to float, then cast to double
				}
				// MVE: more work if double?
				con->setOper(opFltConst);
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
		ch |= (signedness > 0 != oldSignedness > 0);		// Changed from signed to not necessarily signed
		ch |= (signedness < 0 != oldSignedness < 0);		// Changed from unsigned to not necessarily unsigned
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
		int oldSize = size;
		size = max(size, ((SizeType*)other)->getSize());
		ch = size != oldSize;
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
			if (otherBase->isPointer()) {
if (thisBase->isPointer() && thisBase->asPointer()->getPointsTo() == thisBase)
  std::cerr << "HACK! BAD POINTER 1\n";
if (otherBase->isPointer() && otherBase->asPointer()->getPointsTo() == otherBase)
  std::cerr << "HACK! BAD POINTER 2\n";
if (thisBase == otherBase)	// Note: compare pointers
  return this;				// Crude attempt to prevent stack overflow
				if (*thisBase == *otherBase)
					return this;
				if (pointerDepth() == otherPtr->pointerDepth()) {
					Type* fType = getFinalPointsTo();
					if (fType->isVoid()) return other;
					Type* ofType = otherPtr->getFinalPointsTo();
					if (ofType->isVoid()) return this;
					if (*fType == *ofType) return this;
				}
				return createUnion(other, ch);
			}
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
	if (other->isArray()) {
		ArrayType* otherArr = other->asArray();
		Type* newBase = base_type->clone()->meetWith(otherArr->base_type, ch);
		if (*newBase != *base_type) {
			ch = true;
			base_type = newBase;
		}
		return this;
	}
	// Needs work?
	return createUnion(other, ch);
}

Type* NamedType::meetWith(Type* other, bool& ch) {
	Type * rt = resolvesTo();
	if (rt)
		return rt->meetWith(other, ch);
	if (other->isVoid()) return this;
	if (*this == *other) return this;
	return createUnion(other, ch);
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
	std::list<UnionElement>::iterator it;
	if (other->isUnion()) {
		ch = true;
		UnionType* otherUnion = (UnionType*)other;
		// Always return this, never other, (even if other is larger than this) because otherwise iterators can become
		// invalid below
		for (it = otherUnion->li.begin(); it != otherUnion->li.end(); it++) {
			meetWith(it->type, ch);
			return this;
		}
	}

	// Other is a non union type
	for (it = li.begin(); it != li.end(); it++) {
		Type* curr = it->type->clone();
		bool thisCh = false;
		curr = curr->meetWith(other, thisCh);
		if (!curr->isUnion()) {
			// These types met successfully. Replace the current union type with this one
			it->type = curr;
			ch = thisCh;
			return this;
		}
	}

	// Other did not meet with any of my component types. Add a new one
	char name[20];
	sprintf(name, "x%d", ++nextUnionNumber);
	addType(other, name);
	ch = true;
	return this;
}

Type* SizeType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isSize()) {
		if (((SizeType*)other)->size != size) {
			LOG << "size " << size << " meet with size " << ((SizeType*)other)->size << "!\n";
			int oldSize = size;
			size = max(size, ((SizeType*)other)->size);
			ch = size != oldSize;
		}
		return this;
	}
	ch = true;
	if (other->isInteger() || other->isFloat() || other->isPointer()) {
		if (other->getSize() == 0) {
			other->setSize(max(size, other->getSize()));
			return other;
		}
		if (other->getSize() == size)
			return other;
	}
	return createUnion(other, ch);
}

Type* Type::createUnion(Type* other, bool& ch) {
	// Note: this should not be a UnionType
	if (other->isUnion())
		return other->meetWith(this, ch);		// Put all the hard union logic in one place

	char name[20];
	sprintf(name, "x%d", ++nextUnionNumber);
	UnionType* u = new UnionType;
	u->addType(this, name);
	sprintf(name, "x%d", ++nextUnionNumber);
	u->addType(other, name);
	ch = true;
	return u;
}


void CallStatement::dfaTypeAnalysis(bool& ch) {
	Signature* sig = procDest->getSignature();
	//Prog* prog = procDest->getProg();
	// Iterate through the arguments
	int n = sig->getNumParams();
	for (int i=0; i < n; i++) {
		Exp* e = getArgumentExp(i);
		Type* t = sig->getParamType(i);
		e->descendType(t, ch);
#if 0
		Const* c;
		if (e->isSubscript()) {
			// A subscripted location. Find the definition
			RefExp* r = (RefExp*)e;
			Statement* def = r->getRef();
			assert(def);
			Type* tParam = def->getTypeFor(r->getSubExp1());
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
#endif
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
	Type* meetOfArgs = stmtVec[0]->getTypeFor(lhs);
	for (i=1; i < n; i++)
		if (stmtVec[i] && stmtVec[i]->getTypeFor(lhs))
			meetOfArgs = meetOfArgs->meetWith(stmtVec[i]->getTypeFor(lhs), ch);
	type = type->meetWith(meetOfArgs, ch);
	for (i=0; i < n; i++) {
		if (stmtVec[i]) stmtVec[i]->meetWithFor(type, lhs, ch);
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
		case opMult:
		case opDiv:
			return new IntegerType(ta->getSize(), -1);
		case opMults:
		case opDivs:
			return new IntegerType(ta->getSize(), +1);
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
	return def->getTypeFor(subExp1);
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
	switch (op) {
		case opFsize:
			return new FloatType(((Const*)subExp2)->getInt());
		default:
			break;
	}
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
	Type* newType = def->meetWithFor(parentType, subExp1, ch);
	// In case subExp1 is a m[...]
	subExp1->descendType(newType, ch);
}

void Const::descendType(Type* parentType, bool& ch) {
	type = type->meetWith(parentType, ch);
}

void Unary::descendType(Type* parentType, bool& ch) {
	switch (op) {
		case opMemOf:
			// Check for m[x*K1 + K2]: array with base K2 and stride K1
			if (subExp1->getOper() == opPlus &&
					((Binary*)subExp1)->getSubExp1()->getOper() == opMult &&
					((Binary*)subExp1)->getSubExp2()->isIntConst() &&
					((Binary*)((Binary*)subExp1)->getSubExp1())->getSubExp2()->isIntConst()) {
				Exp* leftOfPlus = ((Binary*)subExp1)->getSubExp1();
				// We would expect the stride to be the same size as the base type
				int stride =  ((Const*)((Binary*)leftOfPlus)->getSubExp2())->getInt();
				if (DEBUG_TA && stride*8 != parentType->getSize())
					LOG << "Type WARNING: apparent array reference at " << this << " has stride " << stride*8 <<
						" bits, but parent type " << parentType->getCtype() << " has size " <<
						parentType->getSize() << "\n";
				// The index is integer type
				Exp* x = ((Binary*)leftOfPlus)->getSubExp1();
				x->descendType(new IntegerType(parentType->getSize(), 0), ch);
				// K2 is of type <array of parentType>
				Exp* K2 = ((Binary*)subExp1)->getSubExp2();
				K2->descendType(new ArrayType(parentType), ch);
			}
			// Other cases, e.g. struct reference m[x + K1] or m[x + p] where p is a pointer
			else
				subExp1->descendType(new PointerType(parentType), ch);
			break;
		default:
			break;
	}
}

void Ternary::descendType(Type* parentType, bool& ch) {
	switch (op) {
		case opFsize:
			subExp3->descendType(new FloatType(((Const*)subExp1)->getInt()), ch);
			break;
		default:
			break;
	}
}

void TypedExp::descendType(Type* parentType, bool& ch) {
}

void Terminal::descendType(Type* parentType, bool& ch) {
}

// Convert expressions to locals, using the (so far DFA based) type analysis information
// Basically, descend types, and when you get to m[...] compare with the local high level pattern;
// when at a sum or difference, check for the address of locals high level pattern that is a pointer

void Statement::dfaConvertLocals() {
	DfaLocalConverter dlc(proc);
	StmtDfaLocalConverter sdlc(&dlc);
	accept(&sdlc);
}

void StmtDfaLocalConverter::visit(Assign* s, bool& recur) {
	((DfaLocalConverter*)mod)->setType(s->getType());
	recur = true;
}
void StmtDfaLocalConverter::visit(PhiAssign* s, bool& recur) {
	((DfaLocalConverter*)mod)->setType(s->getType());
	recur = true;
}
void StmtDfaLocalConverter::visit(ImplicitAssign* s, bool& recur) {
	((DfaLocalConverter*)mod)->setType(s->getType());
	recur = true;
}
void StmtDfaLocalConverter::visit(BoolAssign* s, bool& recur) {
	((DfaLocalConverter*)mod)->setType(s->getType());
	recur = true;
}
void StmtDfaLocalConverter::visit(BranchStatement* s, bool& recur) {
	((DfaLocalConverter*)mod)->setType(new BooleanType);
	Exp* pCond = s->getCondExpr();
	s->setCondExpr(pCond->accept(mod));
	recur = false;
}
void StmtDfaLocalConverter::visit(ReturnStatement* s, bool& recur) {
	int n = s->getNumReturns();
	for (int i=0; i < n; i++) {
		Exp* ret = s->getReturnExp(i);
		((DfaLocalConverter*)mod)->setType(ret->ascendType());
		s->setReturnExp(i, ret->accept(mod));
	}
	recur = false;
}
void StmtDfaLocalConverter::visit(CallStatement* s, bool& recur) {
	// First the destination. The type of this expression will be a pointer to a function with s' dest's signature
	Exp* pDest = s->getDest();
	if (pDest) {
		FuncType* ft = new FuncType;
		Proc* destProc = s->getDestProc();
		if (destProc && destProc->getSignature())
			ft->setSignature(destProc->getSignature());
		((DfaLocalConverter*)mod)->setType(ft);
		s->setDest(pDest->accept(mod));
	}
	std::vector<Exp*>::iterator it;
	std::vector<Exp*>& arguments = s->getArguments();
	for (it = arguments.begin(); recur && it != arguments.end(); it++) {
		// MVE: Should we get argument types from the signature, or ascend from the argument expression?
		// Should come to the same thing, and the signature is presumably more efficient
		// But is it always available?
		((DfaLocalConverter*)mod)->setType((*it)->ascendType());
		*it = (*it)->accept(mod);
	}
	std::vector<Exp*>& implicitArguments = s->getImplicitArguments();
	for (it = implicitArguments.begin(); recur && it != implicitArguments.end(); it++) {
		((DfaLocalConverter*)mod)->setType((*it)->ascendType());
		*it = (*it)->accept(mod);
	}
	std::vector<ReturnInfo>::iterator rr;
	std::vector<ReturnInfo>& returns = s->getReturns();
	for (rr = returns.begin(); recur && rr != returns.end(); rr++) {
		if (rr->e == NULL) continue;			// Can be NULL now; just ignore
		((DfaLocalConverter*)mod)->setType(rr->type);
		rr->e = rr->e->accept(mod);
	}
	recur = false;
}


// Convert expressions to locals
DfaLocalConverter::DfaLocalConverter(UserProc* proc) : parentType(NULL), proc(proc) {
	sig = proc->getSignature();
	prog = proc->getProg();
}

Exp* DfaLocalConverter::preVisit(Location* e, bool& recur) {
	// Check if this is an appropriate pattern for local variables	
	if (e->isMemOf()) {
		if (sig->isStackLocal(proc->getProg(), e)) {
			recur = false;
			//mod = true;			// We've made a modification
			Exp* ret = proc->getLocalExp(e, parentType, true);
			// ret is now *usually* a local so postVisit won't expect parentType changed
			// Note: at least one of Trent's hacks can cause m[a[...]] to be returned
			if (ret->isMemOf())
				parentType = new PointerType(parentType);
			return ret;
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
		//mod = true;
		// We have something like sp-K; wrap it in a m[] to get the correct exp for the existing local (if any)
		Exp* memOf_e = Location::memOf(e);
		return new Unary(opAddrOf,
			proc->getLocalExp(memOf_e, parentType->asPointer()->getPointsTo(), true));
	}
	recur = true;
	return e;
}

bool Signature::dfaTypeAnalysis(Cfg* cfg) {
	bool ch = false;
	std::vector<Parameter*>::iterator it;
	for (it = params.begin(); it != params.end(); it++) {
		// Parameters should be defined in an implicit assignment
		Statement* def = cfg->findImplicitParamAssign(*it);
		if (def) { 			// But sometimes they are not used, and hence have no implicit definition
			bool thisCh = false;
			def->meetWithFor((*it)->getType(), (*it)->getExp(), thisCh);
			if (thisCh) {
				ch = true;
				if (DEBUG_TA)
					LOG << "  Sig caused change: " << (*it)->getType()->getCtype() << " " << (*it)->getName() << "\n";
			}
		}
	}
	return ch;
}
