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
	if (this == other) return this;		// NOTE: at present, compares names as well as types and number of parameters
	ch = true;
	return createUnion(other);
}

Type* IntegerType::meetWith(Type* other, bool& ch) {
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
	ch = true;
	return createUnion(other);
}

Type* FloatType::meetWith(Type* other, bool& ch) {
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
	if (other->isBoolean())
		return this;
	ch = true;
	return createUnion(other);
}

Type* CharType::meetWith(Type* other, bool& ch) {
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
	// Needs work
	ch = true;
	return createUnion(other);
}

Type* NamedType::meetWith(Type* other, bool& ch) {
	return resolvesTo()->meetWith(other, ch);
}

Type* CompoundType::meetWith(Type* other, bool& ch) {
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
	if (this == other) return this;
	ch = true;
	return createUnion(other);
}

Type* SizeType::meetWith(Type* other, bool& ch) {
	if (other->isInteger() || other->isFloat() || other->isPointer() || other->isSize()) {
		if (other->getSize() != size) {
			ch = true;
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
