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
#include <sstream>

static int nextUnionNumber = 0;

int max(int a, int b) {		// Faster to write than to find the #include for
	return a>b ? a : b;
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
	if (this == other) return this;		// NOTE: at present, compares names as well as types and number of parameters
	ch = true;
	return createUnion(other);
}

Type* IntegerType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isInteger()) {
		IntegerType* otherInt = other->asInteger();
		// Signedness
		int oldSignedness = signedness;
		if (otherInt->isSigned())
			signedness++;
		else
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
	ch = true;
	return createUnion(other);
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
	ch = true;
	return createUnion(other);
}

Type* BooleanType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isBoolean())
		return this;
	ch = true;
	return createUnion(other);
}

Type* CharType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isChar()) return this;
	// Also allow char to merge with integer
	ch = true;
	if (other->isInteger()) return other;
	return createUnion(other);
}

Type* PointerType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (other->isSize() && ((SizeType*)other)->getSize() == STD_SIZE) return this;
	if (other->isPointer()) {
		PointerType* otherPtr = other->asPointer();
		if (pointsToAlpha() && !otherPtr->pointsToAlpha()) {
			setPointsTo(otherPtr->getPointsTo());
			ch = true;
		}
		return this;
	}
	// Would be good to understand class hierarchys, so we know if a* is the same as b* when b is a subclass of a
	ch = true;
	return createUnion(other);
}

Type* ArrayType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	// Needs work
	ch = true;
	return createUnion(other);
}

Type* NamedType::meetWith(Type* other, bool& ch) {
	return resolvesTo()->meetWith(other, ch);
}

Type* CompoundType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (!other->isCompound()) { ch = true; return createUnion(other);}
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
	if (this == other) return this;
	// Not compatible structs. Create a union of both complete structs.
	// NOTE: may be possible to take advantage of some overlaps of the two structures some day.
	ch = true;
	return createUnion(other);
}

Type* UnionType::meetWith(Type* other, bool& ch) {
	if (other->isVoid()) return this;
	if (this == other) return this;
	ch = true;
	return createUnion(other);
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
		if (other->getSize() != size) {
			other->setSize(size);
		}
	}
	ch = true;
	return other;
}

Type* Type::createUnion(Type* other) {
	char name[20];
	sprintf(name, "x%d", ++nextUnionNumber);
	if (isUnion()) {
		((UnionType*)this)->addType(other, name);
		return this;
	}
	if (other->isUnion()) {
		((UnionType*)other)->addType(this, name);
		return other;
	}
	UnionType* u = new UnionType;
	u->addType(this, name);
	sprintf(name, "x%d", ++nextUnionNumber);
	u->addType(other, name);
	return u;
}


void CallStatement::dfaTypeAnalysis(bool& ch) {
	Signature* sig = procDest->getSignature();
	Prog* prog = procDest->getProg();
	// Iterate through the parameters
	int n = sig->getNumParams();
	for (int i=0; i < n; i++) {
		Exp* e = getArgumentExp(i);
		Type* t = sig->getParamType(i);
		Const* c;
		if (e->isSubscript()) {
			// A subscripted location. Find the definition
			RefExp* r = (RefExp*)e;
			Statement* def = r->getRef();
			// assert(def);			// Soon!
if (def == NULL) continue;
			Type* tParam = def->getType();
			assert(tParam);
			Type* oldTparam = tParam;
			tParam = tParam->meetWith(t, ch);
			def->setType(tParam);
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
	for (i=0; i < n; i++) 
		if (stmtVec[i] && stmtVec[i]->getType()) {
			bool thisCh = false;
			Type* res = stmtVec[i]->getType()->meetWith(type, thisCh);
			if (thisCh) {
				stmtVec[i]->setType(res);
				ch = true;
			}
		}
}

void Assign::dfaTypeAnalysis(bool& ch) {
if (number == 6)
 std::cerr << "HACK!\n";
	Type* tr = rhs->ascendType();
	type = type->meetWith(tr, ch);
	rhs->descendType(type, ch);
}

void BranchStatement::dfaTypeAnalysis(bool& ch) {
	// Not implemented yet
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
	if (ta->isPointer()) {
		if (tb->isPointer())
			return ta->createUnion(tb);
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
	if (tc->isPointer()) {
		if (to->isPointer())
			return new IntegerType;
		if (to->isInteger())
			return tc;
		return to;
	}
	if (tc->isInteger()) {
		if (to->isPointer())
			return tc->createUnion(to);
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
	if (tc->isPointer()) {
		if (tb->isPointer())
			return tc->createUnion(tb);
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
	if (tc->isPointer()) {
		if (ta->isPointer())
			return new IntegerType;
		if (ta->isInteger())
			return tc->createUnion(ta);
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
	if (ta->isPointer()) {
		if (tb->isPointer())
			return new IntegerType;
		if (tb->isInteger())
			return ta;
		return tb;
	}
	if (ta->isInteger()) {
		if (tb->isInteger())
			return ta->createUnion(tb);
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
		default:
			// Many more cases to implement
			break;
	}
}

void RefExp::descendType(Type* parentType, bool& ch) {
if (def == NULL) return;
	Type* oldType = def->getType();
	def->setType(parentType);
	ch |= oldType != parentType;
}

void Const::descendType(Type* parentType, bool& ch) {
	ch |= *type != *parentType;
	type = parentType;
}

void Unary::descendType(Type* parentType, bool& ch) {
}

void Ternary::descendType(Type* parentType, bool& ch) {
}

void TypedExp::descendType(Type* parentType, bool& ch) {
}

void Terminal::descendType(Type* parentType, bool& ch) {
}

