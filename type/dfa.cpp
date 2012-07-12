/*
 * Copyright (C) 2004-2006, Mike Van Emmerik
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
 * 24 Sep 04 - Mike: Created
 * 25 Aug 05 - Mike: Switch from Mycroft style "pointer to alpha plus integer equals pointer to another alpha" to
 *						Van Emmerik style "result is void*" for sigma and delta functions
 */

#include "gc.h"
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
#include <cstring>
#if defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(disable:4996)		// Warnings about e.g. _strdup deprecated in VS 2005
#endif

static int nextUnionNumber = 0;

#ifndef max
int max(int a, int b) {		// Faster to write than to find the #include for
	return a>b ? a : b;
}
#endif

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

// The purpose of this funciton and others like it is to establish safe static roots for garbage collection purposes
// This is particularly important for OS X where it is known that the collector can't see global variables, but it is
// suspected that this is actually important for other architectures as well
void init_dfa() {
#ifndef NO_GARBAGE_COLLECTOR
	static Exp** gc_pointers = (Exp**) GC_MALLOC_UNCOLLECTABLE(2*sizeof(Exp*));
	gc_pointers[0] = scaledArrayPat;
	gc_pointers[1] = unscaledArrayPat;
#endif
}


static int progress = 0;
void UserProc::dfaTypeAnalysis() {
	Boomerang::get()->alert_decompile_debug_point(this, "before dfa type analysis");

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
			if (++progress >= 2000) {
				progress = 0;
				std::cerr << "t" << std::flush;
			}
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
#if 0
	Boomerang::get()->alert_decompile_debug_point(this, "before mapping locals from dfa type analysis");
	if (DEBUG_TA)
		LOG << " ### mapping expressions to local variables for " << getName() << " ###\n";
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		s->dfaMapLocals();
	}
	if (DEBUG_TA)
		LOG << " ### end mapping expressions to local variables for " << getName() << " ###\n";
#endif

	Boomerang::get()->alert_decompile_debug_point(this, "before other uses of dfa type analysis");

	Prog* prog = getProg();
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
			if (t && t->resolvesToPointer()) {
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
				} else if (baseType->resolvesToInteger() || baseType->resolvesToFloat() || baseType->resolvesToSize()) {
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
							if (ty && ty->resolvesToArray()) 
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
			} else if (t->resolvesToFloat()) {
				if (con->isIntConst()) {
					// Reinterpret as a float (and convert to double)
					//con->setFlt(reinterpret_cast<float>(con->getInt()));
					int tmp = con->getInt();
					con->setFlt(*(float*)&tmp);		// Reinterpret to float, then cast to double
					con->setOper(opFltConst);
					con->setType(new FloatType(64));
				}
				// MVE: more work if double?
			} else /* if (t->resolvesToArray()) */ {
				prog->globalUsed(val, t);
			}
		}

		// 2) Search for the scaled array pattern and replace it with an array use m[idx*K1 + K2]
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
			const char* nam = prog->getGlobalName(K2);
			if (nam == NULL)
				nam = prog->newGlobalName(K2);
			Exp* arr = new Binary(opArrayIndex,			// 
				Location::global(nam, this),
				idx);
			if (s->searchAndReplace(scaledArrayPat, arr)) {
				if (s->isImplicit())
					// Register an array of appropriate type
					prog->globalUsed(K2, new ArrayType(((ImplicitAssign*)s)->getType()));
			}
		}

		// 3) Check implicit assigns for parameter and global types.
		if (s->isImplicit()) {
			Exp* lhs = ((ImplicitAssign*)s)->getLeft();
			Type* iType = ((ImplicitAssign*)s)->getType();
			// Note: parameters are not explicit any more
			//if (lhs->isParam()) {	// }
			bool allZero;
			Exp* slhs = lhs->clone()->removeSubscripts(allZero);
			int i = signature->findParam(slhs);
			if (i != -1)
				setParamType(i, iType);
			else if (lhs->isMemOf()) {
				Exp* sub = ((Location*)lhs)->getSubExp1();
				if (sub->isIntConst()) {
					// We have a m[K] := -
					int K = ((Const*)sub)->getInt();
					prog->globalUsed(K, iType);
				}
			} else if (lhs->isGlobal()) {
				char* gname = ((Const*)((Location*)lhs)->getSubExp1())->getStr();
				prog->setGlobalType(gname, iType);
			}
		}

		// 4) Add the locals (soon globals as well) to the localTable, to sort out the overlaps
		if (s->isTyping()) {
			Exp* addrExp = NULL;
			Type* typeExp = NULL;
			if (s->isAssignment()) {
				Exp* lhs = ((Assignment*)s)->getLeft();
				if (lhs->isMemOf()) {
					addrExp = ((Location*)lhs)->getSubExp1();
					typeExp = ((Assignment*)s)->getType();
				}
			}
			else {
				// Assume an implicit reference
				addrExp = ((ImpRefStatement*)s)->getAddressExp();
				if (addrExp->isTypedExp() && ((TypedExp*)addrExp)->getType()->resolvesToPointer())
					addrExp = ((Unary*)addrExp)->getSubExp1();
				typeExp = ((ImpRefStatement*)s)->getType();
				// typeExp should be a pointer expression, or a union of pointer types
				if (typeExp->resolvesToUnion())
					typeExp = typeExp->asUnion()->dereferenceUnion();
				else {
					assert(typeExp->resolvesToPointer());
					typeExp = typeExp->asPointer()->getPointsTo();
				}
			}
			if (addrExp && signature->isAddrOfStackLocal(prog, addrExp)) {
				int addr = 0;
				if (addrExp->getArity() == 2 && signature->isOpCompatStackLocal(addrExp->getOper())) {
					Const* K = (Const*) ((Binary*)addrExp)->getSubExp2();
					if (K->isConst()) {
						addr = K->getInt();
						if (addrExp->getOper() == opMinus)
							addr = -addr;
					}
				}
				LOG << "in proc " << getName() << " adding addrExp " << addrExp << " to local table\n";
				Type * ty = ((TypingStatement*)s)->getType();
				localTable.addItem(addr, lookupSym(Location::memOf(addrExp), ty), typeExp);
			}
		}
	}


	if (VERBOSE) {
		LOG << "### after application of dfa type analysis for " << getName() << " ###\n";
		printToLog();
		LOG << "### end application of dfa type analysis for " << getName() << " ###\n";
	}

	Boomerang::get()->alert_decompile_debug_point(this, "after dfa type analysis");
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

Type* VoidType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	// void meet x = x
	ch |= !other->resolvesToVoid();
	return other->clone();
}

Type* FuncType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	if (*this == *other) return this;		// NOTE: at present, compares names as well as types and num parameters
	return createUnion(other, ch, bHighestPtr);
}

Type* IntegerType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	if (other->resolvesToInteger()) {
		IntegerType* otherInt = other->asInteger();
		// Signedness
		int oldSignedness = signedness;
		if (otherInt->signedness > 0)
			signedness++;
		else if (otherInt->signedness < 0)
			signedness--;
		ch |= ((signedness > 0) != (oldSignedness > 0));	// Changed from signed to not necessarily signed
		ch |= ((signedness < 0) != (oldSignedness < 0));	// Changed from unsigned to not necessarily unsigned
		// Size. Assume 0 indicates unknown size
		unsigned oldSize = size;
		size = max(size, otherInt->size);
		ch |= (size != oldSize);
		return this;
	}
	if (other->resolvesToSize()) {
		if (size == 0) {		// Doubt this will ever happen
			size = ((SizeType*)other)->getSize();
			return this;
		}
		if (size == ((SizeType*)other)->getSize()) return this;
		LOG << "integer size " << size << " meet with SizeType size " << ((SizeType*)other)->getSize() << "!\n";
		unsigned oldSize = size;
		size = max(size, ((SizeType*)other)->getSize());
		ch = size != oldSize;
		return this;
	}
	return createUnion(other, ch, bHighestPtr);
}

Type* FloatType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	if (other->resolvesToFloat()) {
		FloatType* otherFlt = other->asFloat();
		unsigned oldSize = size;
		size = max(size, otherFlt->size);
		ch |= size != oldSize;
		return this;
	}
	if (other->resolvesToSize()) {
		unsigned otherSize = other->getSize();
		ch |= size != otherSize;
		size = max(size, otherSize);
		return this;
	}
	return createUnion(other, ch, bHighestPtr);
}

Type* BooleanType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	if (other->resolvesToBoolean())
		return this;
	return createUnion(other, ch, bHighestPtr);
}

Type* CharType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	if (other->resolvesToChar()) return this;
	// Also allow char to merge with integer
	if (other->resolvesToInteger()) {
		ch = true;
		return other->clone();
	}
	if (other->resolvesToSize() && ((SizeType*)other)->getSize() == 8)
		return this;
	return createUnion(other, ch, bHighestPtr);
}

Type* PointerType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	if (other->resolvesToSize() && ((SizeType*)other)->getSize() == STD_SIZE) return this;
	if (other->resolvesToPointer()) {
		PointerType* otherPtr = other->asPointer();
		if (pointsToAlpha() && !otherPtr->pointsToAlpha()) {
			setPointsTo(otherPtr->getPointsTo());
			ch = true;
		} else {
			// We have a meeting of two pointers.
			Type* thisBase = points_to;
			Type* otherBase = otherPtr->points_to;
			if (bHighestPtr) {
				// We want the greatest type of thisBase and otherBase
				if (thisBase->isSubTypeOrEqual(otherBase))
					return other->clone();
				if (otherBase->isSubTypeOrEqual(thisBase))
					return this;
				// There may be another type that is a superset of this and other; for now return void*
				return new PointerType(new VoidType);
			}
			// See if the base types will meet
			if (otherBase->resolvesToPointer()) {
if (thisBase->resolvesToPointer() && thisBase->asPointer()->getPointsTo() == thisBase)
  std::cerr << "HACK! BAD POINTER 1\n";
if (otherBase->resolvesToPointer() && otherBase->asPointer()->getPointsTo() == otherBase)
  std::cerr << "HACK! BAD POINTER 2\n";
if (thisBase == otherBase)	// Note: compare pointers
  return this;				// Crude attempt to prevent stack overflow
				if (*thisBase == *otherBase)
					return this;
				if (pointerDepth() == otherPtr->pointerDepth()) {
					Type* fType = getFinalPointsTo();
					if (fType->resolvesToVoid()) return other->clone();
					Type* ofType = otherPtr->getFinalPointsTo();
					if (ofType->resolvesToVoid()) return this;
					if (*fType == *ofType) return this;
				}
			}
			if (thisBase->isCompatibleWith(otherBase)) {
				points_to = points_to->meetWith(otherBase, ch, bHighestPtr);
				return this;
			}
			// The bases did not meet successfully. Union the pointers.
			return createUnion(other, ch, bHighestPtr);
		}
		return this;
	}
	// Would be good to understand class hierarchys, so we know if a* is the same as b* when b is a subclass of a
	return createUnion(other, ch, bHighestPtr);
}

Type* ArrayType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	if (other->resolvesToArray()) {
		ArrayType* otherArr = other->asArray();
		Type* newBase = base_type->clone()->meetWith(otherArr->base_type, ch, bHighestPtr);
		if (*newBase != *base_type) {
			ch = true;
			// base_type = newBase;		// No: call setBaseType to adjust length
			setBaseType(newBase);
		}
        if (other->asArray()->getLength() < getLength()) {
            this->setLength(other->asArray()->getLength());
        }
		return this;
	}
	if (*base_type == *other)
		return this;
	// Needs work?
	return createUnion(other, ch, bHighestPtr);
}

Type* NamedType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	Type * rt = resolvesTo();
	if (rt) {
		Type* ret = rt->meetWith(other, ch, bHighestPtr);
		if (ret == rt)
			return this;			// Retain the named type, much better than some compound type
		return ret;					// Otherwise, whatever the result is
	}
	if (other->resolvesToVoid()) return this;
	if (*this == *other) return this;
	return createUnion(other, ch, bHighestPtr);
}

Type* CompoundType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	if (!other->resolvesToCompound()) {
		if (types[0]->isCompatibleWith(other))
			// struct meet first element = struct
			return this;
		return createUnion(other, ch, bHighestPtr);
	}
	CompoundType* otherCmp = other->asCompound();
	if (otherCmp->isSuperStructOf(this)) {
		// The other structure has a superset of my struct's offsets. Preserve the names etc of the bigger struct.
		ch = true;
		return other;
	}
	if (isSubStructOf(otherCmp)) {
		// This is a superstruct of other
		ch = true;
		return this;
	}
	if (*this == *other) return this;
	// Not compatible structs. Create a union of both complete structs.
	// NOTE: may be possible to take advantage of some overlaps of the two structures some day.
	return createUnion(other, ch, bHighestPtr);
}

#define PRINT_UNION 0									// Set to 1 to debug unions to stderr
#ifdef PRINT_UNION
unsigned unionCount = 0;
#endif

Type* UnionType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	std::list<UnionElement>::iterator it;
	if (other->resolvesToUnion()) {
		if (this == other)				// Note: pointer comparison
			return this;				// Avoid infinite recursion
		ch = true;
		UnionType* otherUnion = (UnionType*)other;
		// Always return this, never other, (even if other is larger than this) because otherwise iterators can become
		// invalid below
		for (it = otherUnion->li.begin(); it != otherUnion->li.end(); it++) {
			meetWith(it->type, ch, bHighestPtr);
			return this;
		}
	}

	// Other is a non union type
	if (other->resolvesToPointer() && other->asPointer()->getPointsTo() == this) {
		LOG << "WARNING! attempt to union " << getCtype() << " with pointer to self!\n";
		return this;
	}
	for (it = li.begin(); it != li.end(); it++) {
		Type* curr = it->type->clone();
		if (curr->isCompatibleWith(other)) {
			it->type = curr->meetWith(other, ch, bHighestPtr);
			return this;
		}
	}

	// Other is not compatible with any of my component types. Add a new type
	char name[20];
#if PRINT_UNION											// Set above
	if (unionCount == 999)								// Adjust the count to catch the one you want
		std::cerr << "createUnion breakpokint\n";		// Note: you need two breakpoints (also in Type::createUnion)
	std::cerr << "  " << ++unionCount << " Created union from " << getCtype() << " and " << other->getCtype();
#endif
	sprintf(name, "x%d", ++nextUnionNumber);
	addType(other->clone(), name);
#if PRINT_UNION
	std::cerr << ", result is " << getCtype() << "\n";
#endif
	ch = true;
	return this;
}

Type* SizeType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	if (other->resolvesToSize()) {
		if (((SizeType*)other)->size != size) {
			LOG << "size " << size << " meet with size " << ((SizeType*)other)->size << "!\n";
			unsigned oldSize = size;
			size = max(size, ((SizeType*)other)->size);
			ch = size != oldSize;
		}
		return this;
	}
	ch = true;
	if (other->resolvesToInteger() || other->resolvesToFloat() || other->resolvesToPointer()) {
		if (other->getSize() == 0) {
			other->setSize(size);
			return other->clone();
		}
		if (other->getSize() == size)
			return other->clone();
LOG << "WARNING: size " << size << " meet with " << other->getCtype() << "; allowing temporarily\n";
return other->clone();
	}
	return createUnion(other, ch, bHighestPtr);
}

Type* UpperType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	if (other->resolvesToUpper()) {
		UpperType* otherUpp = other->asUpper();
		Type* newBase = base_type->clone()->meetWith(otherUpp->base_type, ch, bHighestPtr);
		if (*newBase != *base_type) {
			ch = true;
			base_type = newBase;
		}
		return this;
	}
	// Needs work?
	return createUnion(other, ch, bHighestPtr);
}

Type* LowerType::meetWith(Type* other, bool& ch, bool bHighestPtr) {
	if (other->resolvesToVoid()) return this;
	if (other->resolvesToUpper()) {
		LowerType* otherLow = other->asLower();
		Type* newBase = base_type->clone()->meetWith(otherLow->base_type, ch, bHighestPtr);
		if (*newBase != *base_type) {
			ch = true;
			base_type = newBase;
		}
		return this;
	}
	// Needs work?
	return createUnion(other, ch, bHighestPtr);
}

Type* Statement::meetWithFor(Type* ty, Exp* e, bool& ch) {
	bool thisCh = false;
	Type* newType = getTypeFor(e)->meetWith(ty, thisCh);
	if (thisCh) {
		ch = true;
		setTypeFor(e, newType->clone());
	}
	return newType;
}

Type* Type::createUnion(Type* other, bool& ch, bool bHighestPtr /* = false */) {

	assert(!resolvesToUnion());										// `this' should not be a UnionType
	if (other->resolvesToUnion())
		return other->meetWith(this, ch, bHighestPtr)->clone();		// Put all the hard union logic in one place
	// Check for anytype meet compound with anytype as first element
	if (other->resolvesToCompound()) {
		CompoundType* otherComp = other->asCompound();
		Type* firstType = otherComp->getType((unsigned)0);
		if (firstType->isCompatibleWith(this))
			// struct meet first element = struct
			return other->clone();
	}
	// Check for anytype meet array of anytype
	if (other->resolvesToArray()) {
		ArrayType* otherArr = other->asArray();
		Type* elemTy = otherArr->getBaseType();
		if (elemTy->isCompatibleWith(this))
			// array meet element = array
			return other->clone();
	}

	char name[20];
#if PRINT_UNION
	if (unionCount == 999)								// Adjust the count to catch the one you want
		std::cerr << "createUnion breakpokint\n";		// Note: you need two breakpoints (also in UnionType::meetWith)
#endif
	sprintf(name, "x%d", ++nextUnionNumber);
	UnionType* u = new UnionType;
	u->addType(this->clone(), name);
	sprintf(name, "x%d", ++nextUnionNumber);
	u->addType(other->clone(), name);
	ch = true;
#if PRINT_UNION
	std::cerr << "  " << ++unionCount << " Created union from " << getCtype() << " and " << other->getCtype() <<
		", result is " << u->getCtype() << "\n";
#endif
	return u;
}


void CallStatement::dfaTypeAnalysis(bool& ch) {
	// Iterate through the arguments
	StatementList::iterator aa;
    int n = 0;
	for (aa = arguments.begin(); aa != arguments.end(); ++aa, ++n) {
        if (procDest && procDest->getSignature()->getParamBoundMax(n) && ((Assign*)*aa)->getRight()->isIntConst()) {
            Assign *a = (Assign*)*aa;
            std::string boundmax = procDest->getSignature()->getParamBoundMax(n);
            assert(a->getType()->resolvesToInteger());
            StatementList::iterator aat;
            int nt = 0;
            for (aat = arguments.begin(); aat != arguments.end(); ++aat, ++nt)
                if (boundmax == procDest->getSignature()->getParamName(nt)) {
                    Type *tyt = ((Assign*)*aat)->getType();
                    if (tyt->resolvesToPointer() && tyt->asPointer()->getPointsTo()->resolvesToArray() && tyt->asPointer()->getPointsTo()->asArray()->isUnbounded())
                        tyt->asPointer()->getPointsTo()->asArray()->setLength(((Const*)a->getRight())->getInt());
                    break;
                }
        }
		// The below will ascend type, meet type with that of arg, and descend type. Note that the type of the assign
		// will already be that of the signature, if this is a library call, from updateArguments()
		((Assign*)*aa)->dfaTypeAnalysis(ch);
	}
	// The destination is a pointer to a function with this function's signature (if any)
	if (pDest) {
		if (signature)
			pDest->descendType(new FuncType(signature), ch, this);
		else if (procDest)
			pDest->descendType(new FuncType(procDest->getSignature()), ch, this);
	}
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
// Tx0 := Tx0 meet (Tx1 meet Tx2 meet ...)
// Tx1 := Tx1 meet Tx0
// Tx2 := Tx2 meet Tx0
// ...
void PhiAssign::dfaTypeAnalysis(bool& ch) {
	iterator it = defVec.begin();
	while (it->e == NULL && it != defVec.end())
		++it;
	assert(it != defVec.end());
	Type* meetOfArgs = it->def->getTypeFor(lhs);
	for (++it; it != defVec.end(); it++) {
		if (it->e == NULL) continue;
		assert(it->def);
		Type* typeOfDef = it->def->getTypeFor(it->e);
		meetOfArgs = meetOfArgs->meetWith(typeOfDef, ch);
	}
	type = type->meetWith(meetOfArgs, ch);
	for (it = defVec.begin(); it != defVec.end(); it++) {
		if (it->e == NULL) continue;
		it->def->meetWithFor(type, it->e, ch);
	}
	Assignment::dfaTypeAnalysis(ch);			// Handle the LHS
}

void Assign::dfaTypeAnalysis(bool& ch) {
	Type* tr = rhs->ascendType();
	type = type->meetWith(tr, ch, true);	// Note: bHighestPtr is set true, since the lhs could have a greater type
											// (more possibilities) than the rhs. Example: pEmployee = pManager.
	rhs->descendType(type, ch, this);		// This will effect rhs = rhs MEET lhs
	Assignment::dfaTypeAnalysis(ch);		// Handle the LHS wrt m[] operands
}

void Assignment::dfaTypeAnalysis(bool& ch) {
	Signature* sig = proc->getSignature();
	// Don't do this for the common case of an ordinary local, since it generates hundreds of implicit references,
	// without any new type information
	if (lhs->isMemOf() && !sig->isStackLocal(proc->getProg(), lhs)) {
		Exp* addr = ((Unary*)lhs)->getSubExp1();
		// Meet the assignment type with *(type of the address)
		Type* addrType = addr->ascendType();
		Type* memofType;
		if (addrType->resolvesToPointer())
			memofType = addrType->asPointer()->getPointsTo();
		else
			memofType = new VoidType;
		type = type->meetWith(memofType, ch);
		// Push down the fact that the memof operand is a pointer to the assignment type
		addrType = new PointerType(type);
		addr->descendType(addrType, ch, this);
	}
}

void BranchStatement::dfaTypeAnalysis(bool& ch) {
	if (pCond)
		pCond->descendType(new BooleanType(), ch, this);
	// Not fully implemented yet?
}

void ImplicitAssign::dfaTypeAnalysis(bool& ch) {
	Assignment::dfaTypeAnalysis(ch);
}

void BoolAssign::dfaTypeAnalysis(bool& ch) {
	// Not properly implemented yet
	Assignment::dfaTypeAnalysis(ch);
}

// Special operators for handling addition and subtraction in a data flow based type analysis
//					ta=
//  tb=		alpha*	int		pi
// beta*	bottom	void*	void*
// int		void*	int		pi
// pi		void*	pi		pi
Type* sigmaSum(Type* ta, Type* tb) {
	bool ch;
	if (ta->resolvesToPointer()) {
		if (tb->resolvesToPointer())
			return ta->createUnion(tb, ch);
		return new PointerType(new VoidType);
	}
	if (ta->resolvesToInteger()) {
		if (tb->resolvesToPointer())
			return new PointerType(new VoidType);
		return tb->clone();
	}
	if (tb->resolvesToPointer())
		return new PointerType(new VoidType);
	return ta->clone();
}


//					tc=
//  to=		beta*	int		pi
// alpha*	int		bottom	int
// int		void*	int		pi
// pi		pi		pi		pi
Type* sigmaAddend(Type* tc, Type* to) {
	bool ch;
	if (tc->resolvesToPointer()) {
		if (to->resolvesToPointer())
			return new IntegerType;
		if (to->resolvesToInteger())
			return new PointerType(new VoidType);
		return to->clone();
	}
	if (tc->resolvesToInteger()) {
		if (to->resolvesToPointer())
			return tc->createUnion(to, ch);
		return to->clone();
	}
	if (to->resolvesToPointer())
		return new IntegerType;
	return tc->clone();
}

//					tc=
//  tb=		beta*	int		pi
// alpha*	bottom	void*	void*
// int		void*	int		pi
// pi		void*	int		pi
Type* deltaMinuend(Type* tc, Type* tb) {
	bool ch;
	if (tc->resolvesToPointer()) {
		if (tb->resolvesToPointer())
			return tc->createUnion(tb, ch);
		return new PointerType(new VoidType);
	}
	if (tc->resolvesToInteger()) {
		if (tb->resolvesToPointer())
			return new PointerType(new VoidType);
		return tc->clone();
	}
	if (tb->resolvesToPointer())
		return new PointerType(new VoidType);
	return tc->clone();
}

//					tc=
//  ta=		beta*	int		pi
// alpha*	int		void*	pi
// int		bottom	int		int
// pi		int		pi		pi
Type* deltaSubtrahend(Type* tc, Type* ta) {
	bool ch;
	if (tc->resolvesToPointer()) {
		if (ta->resolvesToPointer())
			return new IntegerType;
		if (ta->resolvesToInteger())
			return tc->createUnion(ta, ch);
		return new IntegerType;
	}
	if (tc->resolvesToInteger())
		if (ta->resolvesToPointer())
			return new PointerType(new VoidType);
		return ta->clone();
	if (ta->resolvesToPointer())
		return tc->clone();
	return ta->clone();
}

//					ta=
//  tb=		alpha*	int		pi
// beta*	int		bottom	int
// int		void*	int		pi
// pi		pi		int		pi
Type* deltaDifference(Type* ta, Type* tb) {
	bool ch;
	if (ta->resolvesToPointer()) {
		if (tb->resolvesToPointer())
			return new IntegerType;
		if (tb->resolvesToInteger())
			return new PointerType(new VoidType);
		return tb->clone();
	}
	if (ta->resolvesToInteger()) {
		if (tb->resolvesToPointer())
			return ta->createUnion(tb, ch);
		return new IntegerType;
	}
	if (tb->resolvesToPointer())
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
	if (type->resolvesToVoid()) {
		switch (op) {
			case opIntConst:
// could be anything, Boolean, Character, we could be bit fiddling pointers for all we know - trentw
#if 0
				if (u.i != 0 && (u.i < 0x1000 && u.i > -0x100))
					// Assume that small nonzero integer constants are of integer type (can't be pointers)
					// But note that you can't say anything about sign; these are bit patterns, not HLL constants
					// (e.g. all ones could be signed -1 or unsigned 0xFFFFFFFF)
					type = new IntegerType(STD_SIZE, 0);
#endif
				break;
			case opLongConst:
				type = new IntegerType(STD_SIZE*2, 0);
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
		case opDefineAll:
			return new VoidType;
		case opFlags:
			return new IntegerType(STD_SIZE, -1);
		default:
			std::cerr << "ascendType() for terminal " << this << " not implemented!\n";
			return new VoidType;
	}
}

Type* Unary::ascendType() {
	Type* ta = subExp1->ascendType();
	switch (op) {
		case opMemOf:
			if (ta->resolvesToPointer())
				return ta->asPointer()->getPointsTo();
			else
				return new VoidType();		// NOT SURE! Really should be bottom
			break;
        case opAddrOf:
            return new PointerType(ta);
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

void Binary::descendType(Type* parentType, bool& ch, Statement* s) {
	if (op == opFlagCall) return;
	Type* ta = subExp1->ascendType();
	Type* tb = subExp2->ascendType();
	Type* nt;							// "New" type for certain operators
	// The following is an idea of Mike's that is not yet implemented well. It is designed to handle the situation
	// where the only reference to a local is where its address is taken. In the current implementation, it incorrectly
	// triggers with every ordinary local reference, causing esp to appear used in the final program
#if 0
	Signature* sig = s->getProc()->getSignature();
	Prog* prog = s->getProc()->getProg();
	if (parentType->resolvesToPointer() && !parentType->asPointer()->getPointsTo()->resolvesToVoid() &&
			sig->isAddrOfStackLocal(prog, this)) {
		// This is the address of some local. What I used to do is to make an implicit assignment for the local, and
		// try to meet with the real assignment later. But this had some problems. Now, make an implicit *reference*
		// to the specified address; this should eventually meet with the main assignment(s).
		s->getProc()->setImplicitRef(s, this, parentType);
	}
#endif
	switch (op) {
		case opPlus:
			ta = ta->meetWith(sigmaAddend(parentType, tb), ch);
			subExp1->descendType(ta, ch, s);
			tb = tb->meetWith(sigmaAddend(parentType, ta), ch);
			subExp2->descendType(tb, ch, s);
			break;
		case opMinus:
			ta = ta->meetWith(deltaMinuend(parentType, tb), ch);
			subExp1->descendType(ta, ch, s);
			tb = tb->meetWith(deltaSubtrahend(parentType, ta), ch);
			subExp2->descendType(tb, ch, s);
			break;
		case opGtrUns:	case opLessUns:
		case opGtrEqUns:case opLessEqUns: {
			nt = new IntegerType(ta->getSize(), -1);			// Used as unsigned
			ta = ta->meetWith(nt, ch);
			tb = tb->meetWith(nt, ch);
			subExp1->descendType(ta, ch, s);
			subExp2->descendType(tb, ch, s);
			break;
		}
		case opGtr:	case opLess:
		case opGtrEq:case opLessEq: {
			nt = new IntegerType(ta->getSize(), +1);			// Used as signed
			ta = ta->meetWith(nt, ch);
			tb = tb->meetWith(nt, ch);
			subExp1->descendType(ta, ch, s);
			subExp2->descendType(tb, ch, s);
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
					signedness = 1; break;
				case opMult: case opDiv:
					signedness = -1; break;
				default:
					signedness = 0; break;		// Unknown signedness
			}

			int parentSize = parentType->getSize();
			ta = ta->meetWith(new IntegerType(parentSize, signedness), ch);
			subExp1->descendType(ta, ch, s);
			if (op == opShiftL || op == opShiftR || op == opShiftRA)
				// These operators are not symmetric; doesn't force a signedness on the second operand
				// FIXME: should there be a gentle bias twowards unsigned? Generally, you can't shift by negative
				// amounts.
				signedness = 0;
			tb = tb->meetWith(new IntegerType(parentSize, signedness), ch);
			subExp2->descendType(tb, ch, s);
			break;
		}
		default:
			// Many more cases to implement
			break;
	}
}

void RefExp::descendType(Type* parentType, bool& ch, Statement* s) {
	Type* newType = def->meetWithFor(parentType, subExp1, ch);
	// In case subExp1 is a m[...]
	subExp1->descendType(newType, ch, s);
}

void Const::descendType(Type* parentType, bool& ch, Statement* s) {
	bool thisCh;
	type = type->meetWith(parentType, thisCh);
	ch |= thisCh;
	if (thisCh) {
		// May need to change the representation
		if (type->resolvesToFloat()) {
			if (op == opIntConst) {
				op = opFltConst;
				type = new FloatType(64);
				float f = *(float*)&u.i;
				u.d = (double)f;
			}
			else if (op == opLongConst) {
				op = opFltConst;
				type = new FloatType(64);
				double d = *(double*)&u.ll;
				u.d = d;
			}
		}
		// May be other cases
	}
}

void Unary::descendType(Type* parentType, bool& ch, Statement* s) {
	switch (op) {
		case opMemOf:
			// Check for m[x*K1 + K2]: array with base K2 and stride K1
			if (subExp1->getOper() == opPlus &&
					((Binary*)subExp1)->getSubExp1()->getOper() == opMult &&
					((Binary*)subExp1)->getSubExp2()->isIntConst() &&
					((Binary*)((Binary*)subExp1)->getSubExp1())->getSubExp2()->isIntConst()) {
				Exp* leftOfPlus = ((Binary*)subExp1)->getSubExp1();
				// We would expect the stride to be the same size as the base type
				unsigned stride =  ((Const*)((Binary*)leftOfPlus)->getSubExp2())->getInt();
				if (DEBUG_TA && stride*8 != parentType->getSize())
					LOG << "type WARNING: apparent array reference at " << this << " has stride " << stride*8 <<
						" bits, but parent type " << parentType->getCtype() << " has size " <<
						parentType->getSize() << "\n";
				// The index is integer type
				Exp* x = ((Binary*)leftOfPlus)->getSubExp1();
				x->descendType(new IntegerType(parentType->getSize(), 0), ch, s);
				// K2 is of type <array of parentType>
				Const* constK2 = (Const*)((Binary*)subExp1)->getSubExp2();
				ADDRESS intK2 = (ADDRESS)constK2->getInt();
				Prog* prog = s->getProc()->getProg();
				constK2->descendType(prog->makeArrayType(intK2, parentType), ch, s);
			}
			else if (subExp1->getOper() == opPlus &&
					((Binary*)subExp1)->getSubExp1()->isSubscript() &&
					((RefExp*)((Binary*)subExp1)->getSubExp1())->isLocation() &&
					((Binary*)subExp1)->getSubExp2()->isIntConst()) {
				// m[l1 + K]
				Location* l1 = (Location*)((RefExp*)((Binary*)subExp1)->getSubExp1());
				Type* l1Type = l1->ascendType();
				int K = ((Const*)((Binary*)subExp1)->getSubExp2())->getInt();
				if (l1Type->resolvesToPointer()) {
					// This is a struct reference m[ptr + K]; ptr points to the struct and K is an offset into it
					// First find out if we already have struct information
					if (l1Type->asPointer()->resolvesToCompound()) {
						CompoundType* ct = l1Type->asPointer()->asCompound();
						if (ct->isGeneric())
							ct->updateGenericMember(K, parentType, ch);
						else
							// would like to force a simplify here; I guess it will happen soon enough
							;
					} else {
						// Need to create a generic stuct with a least one member at offset K
						CompoundType* ct = new CompoundType(true);
						ct->updateGenericMember(K, parentType, ch);
					}
				}
				else {
					// K must be the pointer, so this is a global array
					// FIXME: finish this case
				}
			// FIXME: many other cases
			}
			else
				subExp1->descendType(new PointerType(parentType), ch, s);
			break;
        case opAddrOf:
            if (parentType->resolvesToPointer())
                subExp1->descendType(parentType->asPointer()->getPointsTo(), ch, s);
            break;
		case opGlobal: {
			Prog* prog = s->getProc()->getProg();
			char* name = ((Const*)subExp1)->getStr();
			Type* ty = prog->getGlobalType(name);
			if(ty) {
				ty = ty->meetWith(parentType, ch);
				prog->setGlobalType(name, ty);
			}
			break;
		}
		default:
			break;
	}
}

void Ternary::descendType(Type* parentType, bool& ch, Statement* s) {
	switch (op) {
		case opFsize:
			subExp3->descendType(new FloatType(((Const*)subExp1)->getInt()), ch, s);
			break;
		case opZfill: case opSgnEx: {
			int fromSize = ((Const*)subExp1)->getInt();
			Type* fromType;
			fromType = Type::newIntegerLikeType(fromSize, op == opZfill ? -1 : 1);
			subExp3->descendType(fromType, ch, s);
			break;
		}

		default:
			break;
	}
}

void TypedExp::descendType(Type* parentType, bool& ch, Statement* s) {
}

void Terminal::descendType(Type* parentType, bool& ch, Statement* s) {
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


// Note: to prevent infinite recursion, CompoundType, ArrayType, and UnionType implement this function as a delegation
// to isCompatible()
bool Type::isCompatibleWith(Type* other, bool all /* = false */) {
	if (other->resolvesToCompound() ||
		other->resolvesToArray() ||
		other->resolvesToUnion())
			return other->isCompatible(this, all);
	return isCompatible(other, all);
}

bool VoidType::isCompatible(Type* other, bool all) {
	return true;		// Void is compatible with any type
}

bool SizeType::isCompatible(Type* other, bool all) {
	if (other->resolvesToVoid()) return true;
	unsigned otherSize = other->getSize();
	if (other->resolvesToFunc()) return false;
	// FIXME: why is there a test for size 0 here?
	if (otherSize == size || otherSize == 0) return true;
	if (other->resolvesToUnion()) return other->isCompatibleWith(this);
	if (other->resolvesToArray()) return isCompatibleWith(((ArrayType*)other)->getBaseType());
	//return false;
	// For now, size32 and double will be considered compatible (helps test/pentium/global2)
return true;
}

bool IntegerType::isCompatible(Type* other, bool all) {
	if (other->resolvesToVoid()) return true;
	if (other->resolvesToInteger()) return true;
	if (other->resolvesToChar()) return true;
	if (other->resolvesToUnion()) return other->isCompatibleWith(this);
	if (other->resolvesToSize() && ((SizeType*)other)->getSize() == size) return true;
	return false;
}

bool FloatType::isCompatible(Type* other, bool all) {
	if (other->resolvesToVoid()) return true;
	if (other->resolvesToFloat()) return true;
	if (other->resolvesToUnion()) return other->isCompatibleWith(this);
	if (other->resolvesToArray()) return isCompatibleWith(((ArrayType*)other)->getBaseType());
	if (other->resolvesToSize() && ((SizeType*)other)->getSize() == size) return true;
	return false;
}

bool CharType::isCompatible(Type* other, bool all) {
	if (other->resolvesToVoid()) return true;
	if (other->resolvesToChar()) return true;
	if (other->resolvesToInteger()) return true;
	if (other->resolvesToSize() && ((SizeType*)other)->getSize() == 8) return true;
	if (other->resolvesToUnion()) return other->isCompatibleWith(this);
	if (other->resolvesToArray()) return isCompatibleWith(((ArrayType*)other)->getBaseType());
	return false;
}

bool BooleanType::isCompatible(Type* other, bool all) {
	if (other->resolvesToVoid()) return true;
	if (other->resolvesToBoolean()) return true;
	if (other->resolvesToUnion()) return other->isCompatibleWith(this);
	if (other->resolvesToSize() && ((SizeType*)other)->getSize() == 1) return true;
	return false;
}

bool FuncType::isCompatible(Type* other, bool all) {
	assert(signature);
	if (other->resolvesToVoid()) return true;
	if (*this == *other) return true;		// MVE: should not compare names!
	if (other->resolvesToUnion()) return other->isCompatibleWith(this);
	if (other->resolvesToSize() && ((SizeType*)other)->getSize() == STD_SIZE) return true;	
	if (other->resolvesToFunc()) {
		assert(other->asFunc()->signature);
		if (*other->asFunc()->signature == *signature) return true;
	}
	return false;
}

bool PointerType::isCompatible(Type* other, bool all) {
	if (other->resolvesToVoid()) return true;
	if (other->resolvesToUnion()) return other->isCompatibleWith(this);
	if (other->resolvesToSize() && ((SizeType*)other)->getSize() == STD_SIZE) return true;
	if (!other->resolvesToPointer()) return false;
	return points_to->isCompatibleWith(other->asPointer()->points_to);
}

bool NamedType::isCompatible(Type* other, bool all) {
	if (other->isNamed() && name == ((NamedType*)other)->getName())
		return true;
	Type* resTo = resolvesTo();
	if (resTo)
		return resolvesTo()->isCompatibleWith(other);
	if (other->resolvesToVoid()) return true;
	return (*this == *other);
}

bool ArrayType::isCompatible(Type* other, bool all) {
	if (other->resolvesToVoid()) return true;
	if (other->resolvesToArray() && base_type->isCompatibleWith(other->asArray()->base_type)) return true;
	if (other->resolvesToUnion()) return other->isCompatibleWith(this);
	if (!all && base_type->isCompatibleWith(other)) return true;		// An array of x is compatible with x
	return false;
}

bool UnionType::isCompatible(Type* other, bool all) {
	if (other->resolvesToVoid()) return true;
	std::list<UnionElement>::iterator it;
	if (other->resolvesToUnion()) {
		if (this == other)				// Note: pointer comparison
			return true;				// Avoid infinite recursion
		UnionType* otherUnion = (UnionType*)other;
		// Unions are compatible if one is a subset of the other
		if (li.size() < otherUnion->li.size()) {
			for (it = li.begin(); it != li.end(); it++)
				if (!otherUnion->isCompatible(it->type, all)) return false;
		}
		else {
			for (it = otherUnion->li.begin(); it != otherUnion->li.end(); it++)
				if (!isCompatible(it->type, all)) return false;
		}
		return true;
	}
	// Other is not a UnionType
	for (it = li.begin(); it != li.end(); it++)
		if (other->isCompatibleWith(it->type), all) return true;
	return false;
}

bool CompoundType::isCompatible(Type* other, bool all) {
	if (other->resolvesToVoid()) return true;
	if (other->resolvesToUnion()) return other->isCompatibleWith(this);
	if (!other->resolvesToCompound())
		// Used to always return false here. But in fact, a struct is compatible with its first member (if all is false)
		return !all && types[0]->isCompatibleWith(other);
	CompoundType* otherComp = other->asCompound();
	int n = otherComp->getNumTypes();
	if (n != (int)types.size()) return false;		// Is a subcompound compatible with a supercompound?
	for (int i=0; i < n; i++)
		if (!types[i]->isCompatibleWith(otherComp->types[i])) return false;
	return true;
}

bool UpperType::isCompatible(Type* other, bool all) {
	if (other->resolvesToVoid()) return true;
	if (other->resolvesToUpper() && base_type->isCompatibleWith(other->asUpper()->base_type)) return true;
	if (other->resolvesToUnion()) return other->isCompatibleWith(this);
	return false;
}

bool LowerType::isCompatible(Type* other, bool all) {
	if (other->resolvesToVoid()) return true;
	if (other->resolvesToLower() && base_type->isCompatibleWith(other->asLower()->base_type)) return true;
	if (other->resolvesToUnion()) return other->isCompatibleWith(this);
	return false;
}

bool Type::isSubTypeOrEqual(Type* other) {
	if (resolvesToVoid()) return true;
	if (*this == *other) return true;
	if (this->resolvesToCompound() && other->resolvesToCompound())
		return this->asCompound()->isSubStructOf(other);
	// Not really sure here
	return false;
}

Type* Type::dereference() {
	if (resolvesToPointer())
		return asPointer()->getPointsTo();
	if (resolvesToUnion())
		return asUnion()->dereferenceUnion();
	return new VoidType();			// Can't dereference this type. Note: should probably be bottom
}

// Dereference this union. If it is a union of pointers, return a union of the dereferenced items. Else return VoidType
// (note: should probably be bottom)
Type* UnionType::dereferenceUnion() {
	UnionType* ret = new UnionType;
	char name[20];
	std::list<UnionElement>::iterator it;
	for (it = li.begin(); it != li.end(); it++) {
		Type* elem = it->type->dereference();
		if (elem->resolvesToVoid())
			return elem;			// Return void for the whole thing
		sprintf(name, "x%d", ++nextUnionNumber);
		ret->addType(elem->clone(), name);
	}
	return ret;
}

