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
Type* VoidType::meetWith(Type* other) {
	// void meet x = x
	return other;
}

Type* FuncType::meetWith(Type* other) {
	return createUnion(other);
}

Type* IntegerType::meetWith(Type* other) {
	return createUnion(other);
}

Type* FloatType::meetWith(Type* other) {
	return createUnion(other);
}

Type* BooleanType::meetWith(Type* other) {
	return createUnion(other);
}

Type* CharType::meetWith(Type* other) {
	return createUnion(other);
}

Type* PointerType::meetWith(Type* other) {
	return createUnion(other);
}

Type* ArrayType::meetWith(Type* other) {
	return createUnion(other);
}

Type* NamedType::meetWith(Type* other) {
	return createUnion(other);
}

Type* CompoundType::meetWith(Type* other) {
	return createUnion(other);
}

Type* UnionType::meetWith(Type* other) {
	return createUnion(other);
}

Type* SizeType::meetWith(Type* other) {
	return createUnion(other);
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
