/*
 * Copyright (C) 2004-2005, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   dfa.cpp
 * OVERVIEW:   Implementation of class Type functions related to solving type analysis in an iterative, data-flow-based
 *				manner
 *============================================================================*/

/*
 * $Revision$	// 1.30.2.11
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
#include "log.h"
#include "proc.h"
#include <sstream>

static int nextUnionNumber = 0;

int max(int a, int b) {		// Faster to write than to find the #include for
	return a>b ? a : b;
}

#define DFA_ITER_LIMIT 20

// m[idx*K1 + K2]; leave idx wild
static Exp* scaledArrayPat = Location::memOf(
	new Binary(opPlus,
		new Binary(opMult,
			new Terminal(opWild),
			new Terminal(opWildIntConst)),
		new Terminal(opWildIntConst)));
// idx + K; leave idx wild
static Exp* unscaledArrayPat = new Binary(opPlus,
		new Terminal(opWild),
		new Terminal(opWildIntConst));

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
					LOG << " caused change: " << *it << "\n";
			}
		}
		if (!ch)
			// No more changes: round robin algorithm terminates
			break;
	}
	if (ch)
		LOG << "### WARNING: iteration limit exceeded for dfaTypeAnalysis of procedure " << getName() << " ###\n";

	if (DEBUG_TA) {
		LOG << "\n ### results for data flow based type analysis for " << getName() << " ###\n";
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
				CallStatement* call = (CallStatement*)s;
				ReturnStatement* rs = call->getCalleeReturn();
				if (rs == NULL) continue;
				UseCollector* uc = call->getUseCollector();
				ReturnStatement::iterator rr;
				bool first = true;
				for (rr = rs->begin(); rr != rs->end(); ++rr) {
					// Intersect the callee's returns with the live locations at the call, i.e. make sure that they
					// exist in *uc
					Exp* lhs = ((Assignment*)*rr)->getLeft();
					if (!uc->exists(lhs))
						continue;				// Intersection fails
					if (first)
						LOG << "       returns: ";
					else
						LOG << ", ";
					LOG << ((Assignment*)*rr)->getType()->getCtype() << " " << ((Assignment*)*rr)->getLeft();
				}
				LOG << "\n";
			}
		}
		LOG << "\n ### end results for Data flow based Type Analysis for " << getName() << " ###\n\n";
	}

	// Now use the type information gathered
	Prog* prog = getProg();
	if (DEBUG_TA)
		LOG << " ### mapping expressions to local variables for " << getName() << " ###\n";
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		s->dfaMapLocals();
	}
	if (DEBUG_TA)
		LOG << " ### end mapping expressions to local variables for " << getName() << " ###\n";

	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		// 1) constants
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
						con->setStr(str);
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
								ne = new Binary(opArrayIndex, g, new Const(0));
							else 
								ne = g;
						}
						Exp* memof = Location::memOf(con);
						s->searchAndReplace(memof->clone(), ne);
					}
				} else if (baseType->resolvesToArray()) {
					// We have found a constant in s which has type pointer to array of alpha. We can't get the parent
					// of con, but we can find it with the pattern unscaledArrayPat.
					std::list<Exp*> result;
					s->searchAll(unscaledArrayPat, result);
					for (std::list<Exp*>::iterator rr = result.begin(); rr != result.end(); rr++) {
						// idx + K
						Const* constK = (Const*)((Binary*)*rr)->getSubExp2();
						// Note: keep searching till we find the pattern with this constant, since other constants may
						// not be used as pointer to array type.
						if (constK != con) continue;
						ADDRESS K = (ADDRESS)constK->getInt();
						Exp* idx = ((Binary*)*rr)->getSubExp1();
						Exp* arr = new Unary(opAddrOf,
							new Binary(opArrayIndex,
								Location::global(prog->getGlobalName(K), this),
								idx));
						// Beware of changing expressions in implicit assignments... map can become invalid
						bool isImplicit = s->isImplicit();
						if (isImplicit)
							cfg->removeImplicitAssign(((ImplicitAssign*)s)->getLeft());
						s->searchAndReplace(unscaledArrayPat, arr);
						// s will likely have an m[a[array]], so simplify
						s->simplifyAddr();
						if (isImplicit)
							// Replace the implicit assignment entry. Note that s' lhs has changed
							cfg->findImplicitAssign(((ImplicitAssign*)s)->getLeft());
						// Ensure that the global is declared
						// Ugh... I think that arrays and pointers to arrays are muddled!
						prog->globalUsed(K, baseType);
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
			} else /* if (t->isArray()) */ {
				prog->globalUsed(val, t);
			}
		}

		// 2) Search for the scaled array pattern and replace it with an array use
		// m[idx*K1 + K2]
		std::list<Exp*> result;
		s->searchAll(scaledArrayPat, result);
		for (std::list<Exp*>::iterator rr = result.begin(); rr != result.end(); rr++) {
			//Type* ty = s->getTypeFor(*rr);
			// FIXME: should check that we use with array type...
			// Find idx and K2
			Exp* t = ((Unary*)(*rr)->getSubExp1());		// idx*K1 + K2
			Exp* l = ((Binary*)t)->getSubExp1();		// idx*K1
			Exp* r = ((Binary*)t)->getSubExp2();		// K2
			ADDRESS K2 = (ADDRESS)((Const*)r)->getInt();
			Exp* idx = ((Binary*)l)->getSubExp1();
			// Replace with the array expression
			Exp* arr = new Binary(opArrayIndex,
				Location::global(prog->getGlobalName(K2), this),
				idx);
			s->searchAndReplace(scaledArrayPat, arr);
		}

		// 3) Change the type of any parameters. The types for these will be stored in an ImplicitAssign
		Exp* lhs;
		if (s->isImplicit() && (lhs = ((ImplicitAssign*)s)->getLeft(), lhs->isParam())) {
			setParamType(((Const*)((Location*)lhs)->getSubExp1())->getStr(), ((ImplicitAssign*)s)->getType());
		}

	}

	if (VERBOSE) {
		LOG << "### After application of DFA Type Analysis for " << getName() << " ###\n";
		printToLog();
		LOG << "### End application of DFA Type Analysis for " << getName() << " ###\n";
	}
}

// This is the core of the data-flow-based type analysis algorithm: implementing the meet operator.
// In classic lattice-based terms, the TOP type is void; there is no BOTTOM type since we handle overconstraints with
// unions.
// Consider various pieces of knowledge about the types. There could be:
// a) void: no information. Void meet x = x.
// b) size only: find a size large enough to contain the two types.
// c) broad type only, e.g. floating point
// d) signedness, no size
// e) size, no signedness
// f) broad type, size, and (for integer broad type), signedness

// ch set true if any change

Type* VoidType::meetWith(Type* other, bool& ch) {
	// void meet x = x
	ch |= !other->isVoid();
	return other->clone();
}

Type* FuncType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (*this == *other) return this;		// NOTE: at present, compares names as well as types and num parameters
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
		LOG << "integer size " << size << " meet with SizeType size " << ((SizeType*)other)->getSize() << "!\n";
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
		return other->clone();
	}
	if (other->isSize() && ((SizeType*)other)->getSize() == 8)
		return this;
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
			Type* thisBase = points_to;
			Type* otherBase = otherPtr->points_to;
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
					if (fType->isVoid()) return other->clone();
					Type* ofType = otherPtr->getFinalPointsTo();
					if (ofType->isVoid()) return this;
					if (*fType == *ofType) return this;
				}
			}
			if (thisBase->isCompatibleWith(otherBase)) {
				points_to = points_to->meetWith(otherBase, ch);
				return this;
			}
			// The bases did not meet successfully. Union the pointers.
			return createUnion(other, ch);
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
			// base_type = newBase;		// No: call setBaseType to adjust length
			setBaseType(newBase);
		}
		return this;
	}
	if (*base_type == *other)
		return this;
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
	std::list<UnionElement>::iterator it;
	if (other->isUnion()) {
		if (this == other)				// Note: pointer comparison
			return this;				// Avoid infinite recursion
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
	if (other->isPointer() && other->asPointer()->getPointsTo() == this) {
		LOG << "WARNING! attempt to union " << getCtype() << " with pointer to self!\n";
		return this;
	}
	for (it = li.begin(); it != li.end(); it++) {
		Type* curr = it->type->clone();
		if (curr->isCompatibleWith(other)) {
			it->type = curr->meetWith(other, ch);
			return this;
		}
	}

	// Other is not compatible with any of my component types. Add a new type
	char name[20];
	sprintf(name, "x%d", ++nextUnionNumber);
	addType(other->clone(), name);
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
			other->setSize(size);
			return other->clone();
		}
		if (other->getSize() == size)
			return other->clone();
LOG << "WARNING: size " << size << " meet with " << other->getCtype() << "; allowing temporarily\n";
return other->clone();
	}
	return createUnion(other, ch);
}

Type* UpperType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isUpper()) {
		UpperType* otherUpp = other->asUpper();
		Type* newBase = base_type->clone()->meetWith(otherUpp->base_type, ch);
		if (*newBase != *base_type) {
			ch = true;
			base_type = newBase;
		}
		return this;
	}
	// Needs work?
	return createUnion(other, ch);
}

Type* LowerType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isUpper()) {
		LowerType* otherLow = other->asLower();
		Type* newBase = base_type->clone()->meetWith(otherLow->base_type, ch);
		if (*newBase != *base_type) {
			ch = true;
			base_type = newBase;
		}
		return this;
	}
	// Needs work?
	return createUnion(other, ch);
}

Type* Type::createUnion(Type* other, bool& ch) {
	// Note: this should not be a UnionType
	if (other->isUnion())
		return other->meetWith(this, ch)->clone();		// Put all the hard union logic in one place

	char name[20];
	sprintf(name, "x%d", ++nextUnionNumber);
	UnionType* u = new UnionType;
	u->addType(this->clone(), name);
	sprintf(name, "x%d", ++nextUnionNumber);
	u->addType(other->clone(), name);
	ch = true;
	return u;
}


void CallStatement::dfaTypeAnalysis(bool& ch) {
	// Iterate through the arguments
	StatementList::iterator aa;
	for (aa = arguments.begin(); aa != arguments.end(); ++aa) {
		// The below will ascend type, meet type with that of arg, and descend type. Note that the type of the assign
		// will already be that of the signature, if this is a library call, from updateArguments()
		((Assign*)*aa)->dfaTypeAnalysis(ch);
	}
	// The destination is a pointer to a function with this function's signature (if any)
	if (pDest)
		pDest->descendType(new FuncType(signature), ch, proc);
}

void ReturnStatement::dfaTypeAnalysis(bool& ch) {
	StatementList::iterator mm, rr;
	for (mm = modifieds.begin(); mm != modifieds.end(); ++mm) {
		((Assign*)*mm)->dfaTypeAnalysis(ch);
	}
	for (rr = returns.begin(); rr != returns.end(); ++rr) {
		((Assign*)*rr)->dfaTypeAnalysis(ch);
	}
}

// For x0 := phi(x1, x2, ...) want
// Ex0 := Ex0 meet (Ex1 meet Ex2 meet ...)
// Ex1 := Ex1 meet Ex0
// Ex2 := Ex1 meet Ex0
// ...
// The others are correct.
void PhiAssign::dfaTypeAnalysis(bool& ch) {
	iterator it;
	Type* meetOfArgs = defVec[0].def->getTypeFor(lhs);
	for (it = ++defVec.begin(); it != defVec.end(); it++) {
		assert(it->def);
		Type* typeOfDef = it->def->getTypeFor(it->e);
		meetOfArgs = meetOfArgs->meetWith(typeOfDef, ch);
	}
	type = type->meetWith(meetOfArgs, ch);
	for (it = defVec.begin(); it != defVec.end(); it++)
		it->def->meetWithFor(type, it->e, ch);
	Assignment::dfaTypeAnalysis(ch);			// Handle the LHS
}

void Assign::dfaTypeAnalysis(bool& ch) {
	Type* tr = rhs->ascendType();
	type = type->meetWith(tr, ch);
	rhs->descendType(type, ch, proc);
	Assignment::dfaTypeAnalysis(ch);		// Handle the LHS
}

void Assignment::dfaTypeAnalysis(bool& ch) {
	if (lhs->isMemOf())
		// Push down the fact that the memof is a pointer to the assignment type
		lhs->descendType(type, ch, proc);
}

void BranchStatement::dfaTypeAnalysis(bool& ch) {
	pCond->descendType(new BooleanType(), ch, proc);
	// Not fully implemented yet?
}

void BoolAssign::dfaTypeAnalysis(bool& ch) {
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
		return ta->clone();
	}
	if (ta->isInteger()) {
		if (tb->isPointer())
			return tb->clone();
		return tb->clone();
	}
	if (tb->isPointer())
		return tb->clone();
	return ta->clone();
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
			return tc->clone();
		return to->clone();
	}
	if (tc->isInteger()) {
		if (to->isPointer())
			return tc->createUnion(to, ch);
		return to->clone();
	}
	if (to->isPointer())
		return new IntegerType;
	return tc->clone();
}

//					tc=
//  tb=		alpha*	int		pi
// alpha*	bottom	alpha*	alpha*
// int		alpha*	int		pi
// pi		alpha*	int		pi
Type* deltaMinuend(Type* tc, Type* tb) {
	bool ch;
	if (tc->isPointer()) {
		if (tb->isPointer())
			return tc->createUnion(tb, ch);
		return tc->clone();
	}
	if (tc->isInteger()) {
		if (tb->isPointer())
			return tb->clone();
		return tc->clone();
	}
	if (tb->isPointer())
		return tb->clone();
	return tc->clone();
}

//					tc=
//  ta=		alpha*	int		pi
// alpha*	int		alpha*	pi
// int		bottom	int		int
// pi		alpha*	int		pi
Type* deltaSubtrahend(Type* tc, Type* ta) {
	bool ch;
	if (tc->isPointer()) {
		if (ta->isPointer())
			return new IntegerType;
		if (ta->isInteger())
			return tc->createUnion(ta, ch);
		return new IntegerType;
	}
	if (tc->isInteger())
		return ta->clone();
	if (ta->isPointer())
		return tc->clone();
	return ta->clone();
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
			return ta->clone();
		return tb->clone();
	}
	if (ta->isInteger()) {
		if (tb->isPointer())
			return ta->createUnion(tb, ch);
		if (tb->isInteger())
			return tb->clone();
		return new IntegerType;
	}
	if (tb->isPointer())
		return new IntegerType;
	return ta->clone();
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
			// Do I need to check here for Array* promotion? I think checking in descendType is enough
		case opMinus:
			return deltaDifference(ta, tb);
		case opMult: case opDiv:
			return new IntegerType(ta->getSize(), -1);
		case opMults: case opDivs: case opShiftRA:
			return new IntegerType(ta->getSize(), +1);
		case opBitAnd: case opBitOr: case opBitXor: case opShiftR: case opShiftL:
			return new IntegerType(ta->getSize(), 0);
		case opLess:	case opGtr:		case opLessEq:		case opGtrEq:
		case opLessUns:	case opGtrUns:	case opLessEqUns:	case opGtrEqUns:
			return new BooleanType();
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
		case opZfill: case opSgnEx: {
			int toSize = ((Const*)subExp2)->getInt();
			return Type::newIntegerLikeType(toSize, op==opZfill ? -1 : 1);
		}

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

void Binary::descendType(Type* parentType, bool& ch, UserProc* proc) {
	if (op == opFlagCall) return;
	Type* ta = subExp1->ascendType();
	Type* tb = subExp2->ascendType();
	Signature* sig = proc->getSignature();
	Prog* prog = proc->getProg();
	if (parentType->isPointer() && sig->isAddrOfStackLocal(prog, this)) {
		// this is the address of some local
		Exp* localExp = Location::memOf(this);
		// FIXME: Note there is a theory problem here; there is currently no reliable way to find the definition for
		// localExp. For now, ignore all but implicit assignments...
		Cfg* cfg = proc->getCFG();
		Statement* impDef = cfg->findImplicitAssign(localExp);
		if (impDef)
			impDef->meetWithFor(parentType->asPointer()->getPointsTo(), localExp, ch);
			return;
	}
	switch (op) {
		case opPlus:
			if (parentType->isPointer()) {
				if (ta->isInteger() && !subExp1->isIntConst()) {
std::cerr << "ARRAY HACK: parentType is " << parentType << ", tb is " << tb->getCtype() << ", ta is " << ta << ", this is " << this << "\n";
LOG << "ARRAY HACK for " << this << "\n";
					assert(subExp2->isIntConst());
					int val = ((Const*)subExp2)->getInt();
					tb = new PointerType(
						prog->makeArrayType(val, ((PointerType*)parentType)->getPointsTo()->clone()));
				}
				else if (tb->isInteger() && !subExp2->isIntConst()) {
std::cerr << "ARRAY HACK: parentType is " << parentType << ", ta is " << ta << ", this is " << this << "\n";
LOG << "ARRAY HACK for " << this << "\n";
					assert(subExp1->isIntConst());
					int val = ((Const*)subExp1)->getInt();
					ta = new PointerType(
						prog->makeArrayType(val, ((PointerType*)parentType)->getPointsTo()->clone()));
                }
			}
			ta = ta->meetWith(sigmaAddend(parentType, tb), ch);
			subExp1->descendType(ta, ch, proc);
			tb = tb->meetWith(sigmaAddend(parentType, ta), ch);
			subExp2->descendType(tb, ch, proc);
			break;
		case opMinus:
			ta = ta->meetWith(deltaMinuend(parentType, tb), ch);
			subExp1->descendType(ta, ch, proc);
			tb = tb->meetWith(deltaSubtrahend(parentType, ta), ch);
			subExp2->descendType(tb, ch, proc);
			break;
		case opGtrUns:	case opLessUns:
		case opGtrEqUns:case opLessEqUns: {
			ta = ta->meetWith(tb, ch);									// Meet operand types with each other
			ta = ta->meetWith(new IntegerType(ta->getSize(), -1), ch);	// Must be unsigned
			subExp1->descendType(ta, ch, proc);
			subExp2->descendType(ta, ch, proc);
			break;
		}
		case opGtr:	case opLess:
		case opGtrEq:case opLessEq: {
			ta = ta->meetWith(tb, ch);									// Meet operand types with each other
			ta = ta->meetWith(new IntegerType(ta->getSize(), +1), ch);	// Must be signed
			subExp1->descendType(ta, ch, proc);
			subExp2->descendType(ta, ch, proc);
			break;
		}
		case opBitAnd: case opBitOr: case opBitXor: case opShiftR: case opShiftL:
		case opMults: case opDivs: case opShiftRA:
		case opMult: case opDiv: {
			int signedness;
			switch (op) {
				case opBitAnd: case opBitOr: case opBitXor: case opShiftR: case opShiftL:
					signedness = 0; break;
				case opMults: case opDivs: case opShiftRA:
					signedness = -1; break;
				case opMult: case opDiv:
					signedness = -1; break;
				default:
					break;
			}

			int parentSize = parentType->getSize();
			ta = ta->meetWith(new IntegerType(parentSize, signedness), ch);
			subExp1->descendType(ta, ch, proc);
			tb = tb->meetWith(new IntegerType(parentSize, signedness), ch);
			subExp2->descendType(tb, ch, proc);
			break;
		}
		default:
			// Many more cases to implement
			break;
	}
}

void RefExp::descendType(Type* parentType, bool& ch, UserProc* proc) {
	Type* newType = def->meetWithFor(parentType, subExp1, ch);
	// In case subExp1 is a m[...]
	subExp1->descendType(newType, ch, proc);
}

void Const::descendType(Type* parentType, bool& ch, UserProc* proc) {
	type = type->meetWith(parentType, ch);
}

void Unary::descendType(Type* parentType, bool& ch, UserProc* proc) {
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
					LOG << "type WARNING: apparent array reference at " << this << " has stride " << stride*8 <<
						" bits, but parent type " << parentType->getCtype() << " has size " <<
						parentType->getSize() << "\n";
				// The index is integer type
				Exp* x = ((Binary*)leftOfPlus)->getSubExp1();
				x->descendType(new IntegerType(parentType->getSize(), 0), ch, proc);
				// K2 is of type <array of parentType>
				Const* constK2 = (Const*)((Binary*)subExp1)->getSubExp2();
				ADDRESS intK2 = (ADDRESS)constK2->getInt();
				Prog* prog = proc->getProg();
				constK2->descendType(prog->makeArrayType(intK2, parentType), ch, proc);
			}
			// Other cases, e.g. struct reference m[x + K1] or m[x + p] where p is a pointer
			else
				subExp1->descendType(new PointerType(parentType), ch, proc);
			break;
		default:
			break;
	}
}

void Ternary::descendType(Type* parentType, bool& ch, UserProc* proc) {
	switch (op) {
		case opFsize:
			subExp3->descendType(new FloatType(((Const*)subExp1)->getInt()), ch, proc);
			break;
		case opZfill: case opSgnEx: {
			int fromSize = ((Const*)subExp1)->getInt();
			Type* fromType;
			fromType = Type::newIntegerLikeType(fromSize, op == opZfill ? -1 : 1);
			subExp3->descendType(fromType, ch, proc);
			break;
		}

		default:
			break;
	}
}

void TypedExp::descendType(Type* parentType, bool& ch, UserProc* proc) {
}

void Terminal::descendType(Type* parentType, bool& ch, UserProc* proc) {
}

// Map expressions to locals, using the (so far DFA based) type analysis information
// Basically, descend types, and when you get to m[...] compare with the local high level pattern;
// when at a sum or difference, check for the address of locals high level pattern that is a pointer

void Statement::dfaMapLocals() {
	DfaLocalMapper dlc(proc);
	StmtDfaLocalMapper sdlc(&dlc, true);		// True to ignore def collector in return statement
	accept(&sdlc);
	if (VERBOSE && dlc.change)
		LOG << "statement mapped with new local(s): " << number << "\n";
}

void StmtDfaLocalMapper::visit(Assign* s, bool& recur) {
	((DfaLocalMapper*)mod)->setType(s->getType());
	recur = true;
}
void StmtDfaLocalMapper::visit(PhiAssign* s, bool& recur) {
	((DfaLocalMapper*)mod)->setType(s->getType());
	recur = true;
}
void StmtDfaLocalMapper::visit(ImplicitAssign* s, bool& recur) {
	((DfaLocalMapper*)mod)->setType(s->getType());
	recur = true;
}
void StmtDfaLocalMapper::visit(BoolAssign* s, bool& recur) {
	((DfaLocalMapper*)mod)->setType(s->getType());
	recur = true;
}
void StmtDfaLocalMapper::visit(BranchStatement* s, bool& recur) {
	((DfaLocalMapper*)mod)->setType(new BooleanType);
	recur = true;
}
void StmtDfaLocalMapper::visit(ReturnStatement* s, bool& recur) {
	ReturnStatement::iterator rr;
	for (rr = s->begin(); rr != s->end(); ++rr)
		(*rr)->accept(this);
	recur = false;
}
void StmtDfaLocalMapper::visit(CallStatement* s, bool& recur) {
	// First the destination. The type of this expression will be a pointer to a function with s' dest's signature
	Exp* pDest = s->getDest();
	Signature* sig = s->getSignature();
	if (pDest) {
		FuncType* ft = new FuncType;
		if (sig)
			ft->setSignature(sig);
		((DfaLocalMapper*)mod)->setType(ft);
		s->setDest(pDest->accept(mod));
	}
	StatementList::iterator it;
	StatementList& arguments = s->getArguments();
	// Should we get argument types from the signature, or ascend from the argument expression?
	// Ideally, it should come to the same thing, but consider if the argument is sp-K... sp essentially
	// always becomes void*, and so the type is lost
	unsigned u = 0;
	for (it = arguments.begin(); it != arguments.end(); ++it, ++u) {
		Type* pt = NULL;
		if (sig) 
			pt = sig->getParamType(u); 	// Could be NULL if we are involved in recursion
		if (sig && pt)
			((DfaLocalMapper*)mod)->setType(pt);
		else
			((DfaLocalMapper*)mod)->setType(((Assignment*)*it)->getLeft()->ascendType());
		(*it)->accept(this);
	}
#if 0
	std::vector<Exp*>& implicitArguments = s->getImplicitArguments();
	for (it = implicitArguments.begin(); recur && it != implicitArguments.end(); it++) {
		((DfaLocalMapper*)mod)->setType((*it)->ascendType());
		*it = (*it)->accept(mod);
	}
#endif
#if 0
	std::vector<ReturnInfo>::iterator rr;
	std::vector<ReturnInfo>& returns = s->getReturns();
	for (rr = returns.begin(); recur && rr != returns.end(); rr++) {
		if (rr->e == NULL) continue;			// Can be NULL now; just ignore
		((DfaLocalMapper*)mod)->setType(rr->type);
		rr->e = rr->e->accept(mod);
	}
#endif
	recur = false;
}


// Map expressions to locals
DfaLocalMapper::DfaLocalMapper(UserProc* proc) : parentType(NULL), proc(proc) {
	sig = proc->getSignature();
	prog = proc->getProg();
	change = false;
}

Exp* DfaLocalMapper::preVisit(Location* e, bool& recur) {
	// Check if this is an appropriate pattern for local variables	
	recur = true;
	if (e->isMemOf()) {
		if (sig->isStackLocal(proc->getProg(), e)) {
			change = true;			// We've made a mapping
			Exp* ret = proc->getSymbolExp(e, parentType, true);
			// ret is now *usually* a local so postVisit won't expect parentType changed
			// Note: at least one of Trent's hacks can cause m[a[...]] to be returned
			if (ret->isMemOf())
				parentType = new PointerType(parentType);
			recur = false;			// Don't dig inside m[x] to make m[a[m[x]]] !
			// Map, don't modify, so don't set e to ret. We want to fall through here to set the parent type, so that
			// we are consistent and fix the pointer up always in the postVisit function.
		}
		// When we recurse into the m[...], the type will be changed
		parentType = new PointerType(parentType);
	}
	return e;
}
Exp* DfaLocalMapper::postVisit(Location* e) {
	if (e->isMemOf()) {
		// We should have set the type to be a pointer in preVisit; undo that change now
		PointerType* pt = parentType->asPointer();
		assert(pt);
		parentType = pt->getPointsTo();
	}
	return e;
}

Exp* DfaLocalMapper::preVisit(Binary* e, bool& recur) {
	// Check for sp -/+ K, but only if TA indicates this is a pointer
	if (parentType->isPointer() && sig->isAddrOfStackLocal(prog, e)) {
		//mod = true;
		// We have something like sp-K; wrap it in a[ m[ ]] to get the correct exp for the existing local (if any)
		Exp* memOf_e = Location::memOf(e);
		proc->getSymbolExp(memOf_e, parentType->asPointer()->getPointsTo(), true);
		return new Unary(opAddrOf, memOf_e);
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
					LOG << "  sig caused change: " << (*it)->getType()->getCtype() << " " << (*it)->getName() << "\n";
			}
		}
	}
	return ch;
}


bool VoidType::isCompatibleWith(Type* other) {
	return true;		// Void is compatible with any type
}

bool SizeType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	int otherSize = other->getSize();
	if (otherSize == size || otherSize == 0) return true;
	if (other->isUnion()) return other->isCompatibleWith(this);
	if (other->isArray()) return isCompatibleWith(((ArrayType*)other)->getBaseType());
	//return false;
	// For now, size32 and double will be considered compatible (helps test/pentium/global2)
return true;
}

bool IntegerType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	if (other->isInteger()) return true;
	if (other->isChar()) return true;
	if (other->isUnion()) return other->isCompatibleWith(this);
	if (other->isSize() && ((SizeType*)other)->getSize() == size) return true;
	// I am compatible with an array of myself:
	if (other->isArray()) return isCompatibleWith(((ArrayType*)other)->getBaseType());
	return false;
}

bool FloatType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	if (other->isFloat()) return true;
	if (other->isUnion()) return other->isCompatibleWith(this);
	if (other->isArray()) return isCompatibleWith(((ArrayType*)other)->getBaseType());
	if (other->isSize() && ((SizeType*)other)->getSize() == size) return true;
	return false;
}

bool CharType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	if (other->isChar()) return true;
	if (other->isInteger()) return true;
	if (other->isSize() && ((SizeType*)other)->getSize() == 8) return true;
	if (other->isUnion()) return other->isCompatibleWith(this);
	if (other->isArray()) return isCompatibleWith(((ArrayType*)other)->getBaseType());
	return false;
}

bool BooleanType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	if (other->isBoolean()) return true;
	if (other->isUnion()) return other->isCompatibleWith(this);
	if (other->isSize() && ((SizeType*)other)->getSize() == 1) return true;
	return false;
}

bool FuncType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	if (*this == *other) return true;		// MVE: should not compare names!
	if (other->isUnion()) return other->isCompatibleWith(this);
	if (other->isSize() && ((SizeType*)other)->getSize() == STD_SIZE) return true;
	return false;
}

bool PointerType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	if (other->isUnion()) return other->isCompatibleWith(this);
	if (other->isSize() && ((SizeType*)other)->getSize() == STD_SIZE) return true;
	if (!other->isPointer()) return false;
	return points_to->isCompatibleWith(other->asPointer()->points_to);
}

bool NamedType::isCompatibleWith(Type* other) {
	Type* resTo = resolvesTo();
	if (resTo)
		return resolvesTo()->isCompatibleWith(other);
	if (other->isVoid()) return true;
	return (*this == *other);
}

bool ArrayType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	if (other->isArray() && base_type->isCompatibleWith(other->asArray()->base_type)) return true;
	if (base_type->isCompatibleWith(other)) return true;		// An array of x is compatible with x
	if (other->isUnion()) return other->isCompatibleWith(this);
	return false;
}

bool UnionType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	std::list<UnionElement>::iterator it;
	if (other->isUnion()) {
		if (this == other)				// Note: pointer comparison
			return true;				// Avoid infinite recursion
		UnionType* otherUnion = (UnionType*)other;
		for (it = otherUnion->li.begin(); it != otherUnion->li.end(); it++)
			if (isCompatibleWith(it->type)) return true;
		return false;
	}
	// Other is not a UnionType
	for (it = li.begin(); it != li.end(); it++)
		if (other->isCompatibleWith(it->type)) return true;
	return false;
}

bool CompoundType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	if (other->isUnion()) return other->isCompatibleWith(this);
	if (!other->isCompound()) return false;
	CompoundType* otherComp = (CompoundType*)other;
	int n = otherComp->getNumTypes();
	if (n != (int)types.size()) return false;		// Is a subcompound compatible with its supercompound?
	for (int i=0; i < n; i++)
		if (!types[i]->isCompatibleWith(otherComp->types[i])) return false;
	return true;
}

bool UpperType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	if (other->isUpper() && base_type->isCompatibleWith(other->asUpper()->base_type)) return true;
	if (other->isUnion()) return other->isCompatibleWith(this);
	return false;
}

bool LowerType::isCompatibleWith(Type* other) {
	if (other->isVoid()) return true;
	if (other->isLower() && base_type->isCompatibleWith(other->asLower()->base_type)) return true;
	if (other->isUnion()) return other->isCompatibleWith(this);
	return false;
}

