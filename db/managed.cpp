/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   managed.cpp
 * OVERVIEW:   Implementation of "managed" classes such as StatementSet, which feature makeUnion etc
 *============================================================================*/

/*
 * $Revision$	// 1.15.2.13
 *
 * 26 Aug 03 - Mike: Split off from statement.cpp
 * 21 Jun 05 - Mike: Added AssignSet
 */

#include <sstream>

#include "types.h"
#include "managed.h"
#include "statement.h"
#include "exp.h"
#include "log.h"
#include "boomerang.h"

extern char debug_buffer[];		// For prints functions


std::ostream& operator<<(std::ostream& os, StatementSet* ss) {
	ss->print(os);
	return os;
}

std::ostream& operator<<(std::ostream& os, AssignSet* as) {
	as->print(os);
	return os;
}

std::ostream& operator<<(std::ostream& os, LocationSet* ls) {
	ls->print(os);
	return os;
}


//
// StatementSet methods
//

// Make this set the union of itself and other
void StatementSet::makeUnion(StatementSet& other) {
	std::set<Statement*>::iterator it;
	for (it = other.sset.begin(); it != other.sset.end(); it++) {
		sset.insert(*it);
	}
}

// Make this set the difference of itself and other
void StatementSet::makeDiff(StatementSet& other) {
	std::set<Statement*>::iterator it;
	for (it = other.sset.begin(); it != other.sset.end(); it++) {
		sset.erase(*it);
	}
}


// Make this set the intersection of itself and other
void StatementSet::makeIsect(StatementSet& other) {
	std::set<Statement*>::iterator it, ff;
	for (it = sset.begin(); it != sset.end(); it++) {
		ff = other.sset.find(*it);
		if (ff == other.sset.end())
			// Not in both sets
			sset.erase(it);
	}
}

// Check for the subset relation, i.e. are all my elements also in the set
// other. Effectively (this intersect other) == this
bool StatementSet::isSubSetOf(StatementSet& other) {
	std::set<Statement*>::iterator it, ff;
	for (it = sset.begin(); it != sset.end(); it++) {
		ff = other.sset.find(*it);
		if (ff == other.sset.end())
			return false;
	}
	return true;
}


// Remove this Statement. Return false if it was not found
bool StatementSet::remove(Statement* s) {
	if (sset.find(s) != sset.end()) {
		sset.erase(s);
		return true;
	}
	return false;
}

// Search for s in this Statement set. Return true if found
bool StatementSet::exists(Statement* s) {
	std::set<Statement*>::iterator it = sset.find(s);
	return (it != sset.end());
}

// Find a definition for loc in this Statement set. Return true if found
bool StatementSet::definesLoc(Exp* loc) {
	for (iterator it = sset.begin(); it != sset.end(); it++) {
		if ((*it)->definesLoc(loc))
			return true;
	}
	return false;
}

#if 0
// Remove if defines the given expression
bool StatementSet::removeIfDefines(Exp* given) {
	bool found = false;
	for (iterator it = sset.begin(); it != sset.end(); it++) {
		if ((*it)->defines(given)) {
			// Erase this Statement
			sset.erase(it);
			found = true;
		}
	}
	return found;
}

// As above, but given a whole statement set
bool StatementSet::removeIfDefines(StatementSet& given) {
	bool found = false;
	for (iterator it = given.sset.begin(); it != given.sset.end(); it++) {
		Exp* givenLeft = (*it)->getLeft();
		if (givenLeft)
			found |= removeIfDefines(givenLeft);
	}
	return found;
}
#endif

// Print to a string, for debugging
char* StatementSet::prints() {
	std::ostringstream ost;
	std::set<Statement*>::iterator it;
	for (it = sset.begin(); it != sset.end(); it++) {
		if (it != sset.begin()) ost << ",\t";
		ost << *it;
	}
	ost << "\n";
	strncpy(debug_buffer, ost.str().c_str(), DEBUG_BUFSIZE-1);
	debug_buffer[DEBUG_BUFSIZE-1] = '\0';
	return debug_buffer;
}

void StatementSet::dump() {
	print(std::cerr);
}

void StatementSet::print(std::ostream& os) {
	std::set<Statement*>::iterator it;
	for (it = sset.begin(); it != sset.end(); it++) {
		if (it != sset.begin()) os << ",\t";
		os << *it;
	}
	os << "\n";
}

// Print just the numbers to stream os
void StatementSet::printNums(std::ostream& os) {
	os << std::dec;
	for (iterator it = sset.begin(); it != sset.end(); ) {
		if (*it)
			(*it)->printNum(os);
		else
			os << "-";				// Special case for NULL definition
		if (++it != sset.end())
			os << " ";
	}
}

bool StatementSet::operator<(const StatementSet& o) const {
	if (sset.size() < o.sset.size()) return true;
	if (sset.size() > o.sset.size()) return false;
	std::set<Statement*>::const_iterator it1, it2;
	for (it1 = sset.begin(), it2 = o.sset.begin(); it1 != sset.end();
	  it1++, it2++) {
		if (*it1 < *it2) return true;
		if (*it1 > *it2) return false;
	}
	return false;
}

//
// AssignSet methods
//

// Make this set the union of itself and other
void AssignSet::makeUnion(AssignSet& other) {
	iterator it;
	for (it = other.aset.begin(); it != other.aset.end(); it++) {
		aset.insert(*it);
	}
}

// Make this set the difference of itself and other
void AssignSet::makeDiff(AssignSet& other) {
	iterator it;
	for (it = other.aset.begin(); it != other.aset.end(); it++) {
		aset.erase(*it);
	}
}


// Make this set the intersection of itself and other
void AssignSet::makeIsect(AssignSet& other) {
	iterator it, ff;
	for (it = aset.begin(); it != aset.end(); it++) {
		ff = other.aset.find(*it);
		if (ff == other.aset.end())
			// Not in both sets
			aset.erase(it);
	}
}

// Check for the subset relation, i.e. are all my elements also in the set
// other. Effectively (this intersect other) == this
bool AssignSet::isSubSetOf(AssignSet& other) {
	iterator it, ff;
	for (it = aset.begin(); it != aset.end(); it++) {
		ff = other.aset.find(*it);
		if (ff == other.aset.end())
			return false;
	}
	return true;
}


// Remove this Assign. Return false if it was not found
bool AssignSet::remove(Assign* a) {
	if (aset.find(a) != aset.end()) {
		aset.erase(a);
		return true;
	}
	return false;
}

// Search for a in this Assign set. Return true if found
bool AssignSet::exists(Assign* a) {
	iterator it = aset.find(a);
	return (it != aset.end());
}

// Find a definition for loc in this Assign set. Return true if found
bool AssignSet::definesLoc(Exp* loc) {
	Assign* as = new Assign(loc, new Terminal(opWild));
	iterator ff = aset.find(as);
	return ff != aset.end();
}

// Find a definition for loc on the LHS in this Assign set. If found, return pointer to the Assign with that LHS
Assign* AssignSet::lookupLoc(Exp* loc) {
	Assign* as = new Assign(loc, new Terminal(opWild));
	iterator ff = aset.find(as);
	if (ff == aset.end()) return NULL;
	return *ff;
}

#if 0
// Remove if defines the given expression
bool AssignSet::removeIfDefines(Exp* given) {
	bool found = false;
	for (iterator it = aset.begin(); it != aset.end(); it++) {
		if ((*it)->defines(given)) {
			// Erase this Statement
			aset.erase(it);
			found = true;
		}
	}
	return found;
}

// As above, but given a whole statement set
bool AssignSet::removeIfDefines(AssignSet& given) {
	bool found = false;
	for (iterator it = given.aset.begin(); it != given.aset.end(); it++) {
		Exp* givenLeft = (*it)->getLeft();
		if (givenLeft)
			found |= removeIfDefines(givenLeft);
	}
	return found;
}
#endif

// Print to a string, for debugging
char* AssignSet::prints() {
	std::ostringstream ost;
	iterator it;
	for (it = aset.begin(); it != aset.end(); it++) {
		if (it != aset.begin()) ost << ",\t";
		ost << *it;
	}
	ost << "\n";
	strncpy(debug_buffer, ost.str().c_str(), DEBUG_BUFSIZE-1);
	debug_buffer[DEBUG_BUFSIZE-1] = '\0';
	return debug_buffer;
}

void AssignSet::dump() {
	print(std::cerr);
}

void AssignSet::print(std::ostream& os) {
	iterator it;
	for (it = aset.begin(); it != aset.end(); it++) {
		if (it != aset.begin()) os << ",\t";
		os << *it;
	}
	os << "\n";
}

// Print just the numbers to stream os
void AssignSet::printNums(std::ostream& os) {
	os << std::dec;
	for (iterator it = aset.begin(); it != aset.end(); ) {
		if (*it)
			(*it)->printNum(os);
		else
			os << "-";				// Special case for NULL definition
		if (++it != aset.end())
			os << " ";
	}
}

bool AssignSet::operator<(const AssignSet& o) const {
	if (aset.size() < o.aset.size()) return true;
	if (aset.size() > o.aset.size()) return false;
	const_iterator it1, it2;
	for (it1 = aset.begin(), it2 = o.aset.begin(); it1 != aset.end();
	  it1++, it2++) {
		if (*it1 < *it2) return true;
		if (*it1 > *it2) return false;
	}
	return false;
}

//
// LocationSet methods
//

// Assignment operator
LocationSet& LocationSet::operator=(const LocationSet& o) {
	lset.clear();
	std::set<Exp*, lessExpStar>::const_iterator it;
	for (it = o.lset.begin(); it != o.lset.end(); it++) {
		lset.insert((*it)->clone());
	}
	return *this;
}

// Copy constructor
LocationSet::LocationSet(const LocationSet& o) {
	std::set<Exp*, lessExpStar>::const_iterator it;
	for (it = o.lset.begin(); it != o.lset.end(); it++)
		lset.insert((*it)->clone());
}

char* LocationSet::prints() {
	std::ostringstream ost;
	std::set<Exp*, lessExpStar>::iterator it;
	for (it = lset.begin(); it != lset.end(); it++) {
		if (it != lset.begin()) ost << ",\t";
		ost << *it;
	}
	strncpy(debug_buffer, ost.str().c_str(), DEBUG_BUFSIZE-1);
	debug_buffer[DEBUG_BUFSIZE-1] = '\0';
	return debug_buffer;
}

void LocationSet::dump() {
	print(std::cerr);
}

void LocationSet::print(std::ostream& os) {
	std::set<Exp*, lessExpStar>::iterator it;
	for (it = lset.begin(); it != lset.end(); it++) {
		if (it != lset.begin()) os << ",\t";
		os << *it;
	}
}

void LocationSet::remove(Exp* given) {
	std::set<Exp*, lessExpStar>::iterator it = lset.find(given);
	if (it == lset.end()) return;
//std::cerr << "LocationSet::remove at " << std::hex << (unsigned)this << " of " << *it << "\n";
//std::cerr << "before: "; print();
	// NOTE: if the below uncommented, things go crazy. Valgrind says that
	// the deleted value gets used next in LocationSet::operator== ?!
	//delete *it;		  // These expressions were cloned when created
	lset.erase(it);
//std::cerr << "after : "; print();
}

// Remove locations defined by any of the given set of statements
// Used for killing in liveness sets
void LocationSet::removeIfDefines(StatementSet& given) {
	StatementSet::iterator it;
	for (it = given.begin(); it != given.end(); ++it) {
		Statement* s = (Statement*)*it;
		LocationSet defs;
		s->getDefinitions(defs);
		LocationSet::iterator dd;
		for (dd = defs.begin(); dd != defs.end(); ++dd) 
			lset.erase(*dd);
	}
}

// Make this set the union of itself and other
void LocationSet::makeUnion(LocationSet& other) {
	iterator it;
	for (it = other.lset.begin(); it != other.lset.end(); it++) {
		lset.insert(*it);
	}
}

// Make this set the set difference of itself and other
void LocationSet::makeDiff(LocationSet& other) {
	std::set<Exp*, lessExpStar>::iterator it;
	for (it = other.lset.begin(); it != other.lset.end(); it++) {
		lset.erase(*it);
	}
}

#if 0
Exp* LocationSet::getFirst(LocSetIter& it) {
	it = lset.begin();
	if (it == lset.end())
		// No elements
		return NULL;
	return *it;			// Else return the first element
}

Exp* LocationSet::getNext(LocSetIter& it) {
	if (++it == lset.end())
		// No more elements
		return NULL;
	return *it;			// Else return the next element
}
#endif

bool LocationSet::operator==(const LocationSet& o) const {
	// We want to compare the strings, not the pointers
	if (size() != o.size()) return false;
	std::set<Exp*, lessExpStar>::const_iterator it1, it2;
	for (it1 = lset.begin(), it2 = o.lset.begin(); it1 != lset.end();
	  it1++, it2++) {
		if (!(**it1 == **it2)) return false;
	}
	return true;
}

bool LocationSet::exists(Exp* e) {
	return lset.find(e) != lset.end();
}

// This set is assumed to be of subscripted locations (e.g. a Collector), and we want to find the unsubscripted
// location e in the set
Exp* LocationSet::findNS(Exp* e) {
	// Note: can't do this efficiently with a wildcard, since you can't order wildcards sensibly (I think)
	// RefExp r(e, (Statement*)-1);
	// return lset.find(&r) != lset.end();
	iterator it;
	for (it = lset.begin(); it != lset.end(); ++it) {
		if (**it *= *e)				// Ignore subscripts
			return *it;
	}
	return NULL;
}


// Find a location with a different def, but same expression. For example, pass r28{10},
// return true if r28{20} in the set. If return true, dr points to the first different ref
bool LocationSet::findDifferentRef(RefExp* e, Exp *&dr) {
	RefExp search(e->getSubExp1()->clone(), (Statement*)-1);
	std::set<Exp*, lessExpStar>::iterator pos = lset.find(&search);
	if (pos == lset.end()) return false;
	while (pos != lset.end()) {
		// Exit if we've gone to a new base expression
		// E.g. searching for r13{10} and **pos is r14{0}
		// Note: we want a ref-sensitive compare, but with the outer refs stripped off
		// For example: m[r29{10} - 16]{any} is different from m[r29{20} - 16]{any}
		if (!(*(*pos)->getSubExp1() == *e->getSubExp1())) break;
		// Bases are the same; return true if only different ref
		if (!(**pos == *e)) {
			dr = *pos;
			return true; 
		} 
		++pos;
	}
	return false;
}

// Add a subscript (to definition d) to each element
void LocationSet::addSubscript(Statement* d /* , Cfg* cfg */) {
	std::set<Exp*, lessExpStar>::iterator it;
	std::set<Exp*, lessExpStar> newSet;
	for (it = lset.begin(); it != lset.end(); it++)
		newSet.insert((*it)->expSubscriptVar(*it, d /* , cfg */));
	lset = newSet;			// Replace the old set!
	// Note: don't delete the old exps; they are copied in the new set
}

// Substitute s into all members of the set
void LocationSet::substitute(Assign& a) {
	Exp* lhs = a.getLeft();
	if (lhs == NULL) return;
	Exp* rhs = a.getRight();
	if (rhs == NULL) return;		// ? Will this ever happen?
	std::set<Exp*, lessExpStar>::iterator it;
	// Note: it's important not to change the pointer in the set of pointers to expressions, without removing and
	// inserting again. Otherwise, the set becomes out of order, and operations such as set comparison fail!
	// To avoid any funny behaviour when iterating the loop, we use the following two sets
	LocationSet removeSet;			// These will be removed after the loop
	LocationSet removeAndDelete;	// These will be removed then deleted
	LocationSet insertSet;			// These will be inserted after the loop
	bool change;
	for (it = lset.begin(); it != lset.end(); it++) {
		Exp* loc = *it;
		Exp* replace;
		if (loc->search(lhs, replace)) {
			if (rhs->isTerminal()) {
				// This is no longer a location of interest (e.g. %pc)
				removeSet.insert(loc);
				continue;
			}
			loc = loc->clone()->searchReplaceAll(lhs, rhs, change);
			if (change) {
				loc = loc->simplifyArith();
				loc = loc->simplify();
				// If the result is no longer a register or memory (e.g.
				// r[28]-4), then delete this expression and insert any
				// components it uses (in the example, just r[28])
				if (!loc->isRegOf() && !loc->isMemOf()) {
					// Note: can't delete the expression yet, because the
					// act of insertion into the remove set requires silent
					// calls to the compare function
					removeAndDelete.insert(*it);
					loc->addUsedLocs(insertSet);
					continue;
				}
				// Else we just want to replace it
				// Regardless of whether the top level expression pointer has
				// changed, remove and insert it from the set of pointers
				removeSet.insert(*it);		// Note: remove the unmodified ptr
				insertSet.insert(loc);
			}
		}
	}
	makeDiff(removeSet);	   // Remove the items to be removed
	makeDiff(removeAndDelete); // These are to be removed as well
	makeUnion(insertSet);	   // Insert the items to be added
	// Now delete the expressions that are no longer needed
	std::set<Exp*, lessExpStar>::iterator dd;
	for (dd = removeAndDelete.lset.begin(); dd != removeAndDelete.lset.end();
	  dd++)
		delete *dd;				// Plug that memory leak
}

//
// StatementList methods
//

bool StatementList::remove(Statement* s) {
	std::list<Statement*>::iterator it;
	for (it = slist.begin(); it != slist.end(); it++) {
		if (*it == s) {
			slist.erase(it);
			return true;
		}
	}
	return false;
}

void StatementList::append(StatementList& sl) {
	for (iterator it = sl.slist.begin(); it != sl.slist.end(); it++) {
		slist.push_back(*it);
	}
}

void StatementList::append(StatementSet& ss) {
	for (StatementSet::iterator it = ss.begin(); it != ss.end(); it++) {
		slist.push_back(*it);
	}
}

#if 0
Statement* StatementList::getFirst(StmtListIter& it) {
	it = slist.begin();
	if (it == slist.end())
		// No elements
		return NULL;
	return *it;			// Else return the first element
}

Statement* StatementList::getNext(StmtListIter& it) {
	if (++it == slist.end())
		// No more elements
		return NULL;
	return *it;			// Else return the next element
}

Statement* StatementList::getLast(StmtListRevIter& it) {
	it = slist.rbegin();
	if (it == slist.rend())
		// No elements
		return NULL;
	return *it;			// Else return the last element
}

Statement* StatementList::getPrev(StmtListRevIter& it) {
	if (++it == slist.rend())
		// No more elements
		return NULL;
	return *it;			// Else return the previous element
}
#endif

char* StatementList::prints() {
	std::ostringstream ost;
	for (iterator it = slist.begin(); it != slist.end(); it++) {
		ost << *it << ",\t";
	}
	strncpy(debug_buffer, ost.str().c_str(), DEBUG_BUFSIZE-1);
	debug_buffer[DEBUG_BUFSIZE-1] = '\0';
	return debug_buffer;
}

//
// StatementVec methods
//

void StatementVec::putAt(int idx, Statement* s) {
	if (idx >= (int)svec.size())
		svec.resize(idx+1, NULL);
	svec[idx] = s;
}

StatementVec::iterator StatementVec::remove(iterator it) {
/*
	iterator oldoldit = it;
	iterator oldit = it;
	for (it++; it != svec.end(); it++, oldit++) 
		*oldit = *it;
	svec.resize(svec.size()-1);
	return oldoldit;
*/
	return svec.erase(it);
}

char* StatementVec::prints() {
	std::ostringstream ost;
	iterator it;
	for (it = svec.begin(); it != svec.end(); it++) {
		ost << *it << ",\t";
	}
	strncpy(debug_buffer, ost.str().c_str(), DEBUG_BUFSIZE-1);
	debug_buffer[DEBUG_BUFSIZE-1] = '\0';
	return debug_buffer;
}

// Print just the numbers to stream os
void StatementVec::printNums(std::ostream& os) {
	iterator it;
	os << std::dec;
	for (it = svec.begin(); it != svec.end(); ) {
		if (*it)
			(*it)->printNum(os);
		else
			os << "-";				// Special case for no definition
		if (++it != svec.end())
			os << " ";
	}
}


// Special intersection method: this := a intersect b
void StatementList::makeIsect(StatementList& a, LocationSet& b) {
	slist.clear();
	for (iterator it = a.slist.begin(); it != a.slist.end(); ++it) {
		Assignment* as = (Assignment*)*it;
		if (b.exists(as->getLeft()))
			slist.push_back(as);
	}
}

void StatementList::makeCloneOf(StatementList& o) {
	slist.clear();
	for (iterator it = o.slist.begin(); it != o.slist.end(); it++)
		slist.push_back((*it)->clone());
}

// Return true if loc appears on the left of any statements in this list
// Note: statements in this list are assumed to be assignments
bool StatementList::existsOnLeft(Exp* loc) {
	for (iterator it = slist.begin(); it != slist.end(); it++) {
		if (*((Assignment*)*it)->getLeft() == *loc)
			return true;
	}
	return false;
}

// Remove the first definition where loc appears on the left
// Note: statements in this list are assumed to be assignments
void StatementList::removeDefOf(Exp* loc) {
	for (iterator it = slist.begin(); it != slist.end(); it++) {
		if (*((Assignment*)*it)->getLeft() == *loc) {
			erase(it);
			return;
		}
	}
}

// Find the first Assignment with loc on the LHS
Assignment* StatementList::findOnLeft(Exp* loc) {
    for (iterator it = slist.begin(); it != slist.end(); it++)
        if (*((Assignment*)*it)->getLeft() == *loc)
            return (Assignment*)*it;
    return NULL;
}

void LocationSet::diff(LocationSet* o) {
	std::set<Exp*, lessExpStar>::iterator it;
	bool printed2not1 = false;
	for (it = o->lset.begin(); it != o->lset.end(); it++) {
		Exp* oe = *it;
		if (lset.find(oe) == lset.end()) {
			if (!printed2not1) {
				printed2not1 = true;
				std::cerr << "In set 2 but not set 1:\n";
			}
			std::cerr << oe << "\t";
		}
	}
	if (printed2not1)
		std::cerr << "\n";
	bool printed1not2 = false;
	for (it = lset.begin(); it != lset.end(); it++) {
		Exp* e = *it;
		if (o->lset.find(e) == o->lset.end()) {
			if (!printed1not2) {
				printed1not2 = true;
				std::cerr << "In set 1 but not set 2:\n";
			}
			std::cerr << e << "\t";
		}
	}
	if (printed1not2)
		std::cerr << "\n";
}
Range::Range() : stride(1), lowerBound(NEGINFINITY), upperBound(INFINITY)
{
	base = new Const(0);
}

Range::Range(int stride, int lowerBound, int upperBound, Exp *base) : 
		stride(stride), lowerBound(lowerBound), upperBound(upperBound), base(base) {
	if (lowerBound == upperBound && lowerBound == 0 && (base->getOper() == opMinus || base->getOper() == opPlus) &&
		base->getSubExp2()->isIntConst()) {
		this->lowerBound = ((Const*)base->getSubExp2())->getInt();
		if (base->getOper() == opMinus)
			this->lowerBound = -this->lowerBound;
		this->upperBound = this->lowerBound;
		this->base = base->getSubExp1();
	} else {
		if (base == NULL)
			base = new Const(0);
		if (lowerBound > upperBound)
			this->upperBound = lowerBound;
		if (upperBound < lowerBound)
			this->lowerBound = upperBound;
	}
}

void Range::print(std::ostream &os)
{
	assert(lowerBound <= upperBound);
	if (base->isIntConst() && ((Const*)base)->getInt() == 0 &&
		lowerBound == NEGINFINITY && upperBound == INFINITY) {
		os << "T";
		return;
	}
	bool needPlus = false;
	if (lowerBound == upperBound) {
		if (!base->isIntConst() || ((Const*)base)->getInt() != 0) {
			if (lowerBound != 0) {
				os << lowerBound;
				needPlus = true;
			}
		} else {
			needPlus = true;
			os << lowerBound;
		}
	} else {
		if (stride != 1)
			os << stride;
		os << "[";
		if (lowerBound == NEGINFINITY)
			os << "-inf";
		else
			os << lowerBound;
		os << ", ";
		if (upperBound == INFINITY)
			os << "inf";
		else
			os << upperBound;
		os << "]";
		needPlus = true;
	}
	if (!base->isIntConst() || ((Const*)base)->getInt() != 0) {
		if (needPlus)
			os << " + ";
		base->print(os);
	}
}

void Range::unionWith(Range &r)
{
	if (VERBOSE)
		LOG << "unioning " << this << " with " << r << " got ";
	if (base->getOper() == opMinus && r.base->getOper() == opMinus &&
		*base->getSubExp1() == *r.base->getSubExp1() &&
		base->getSubExp2()->isIntConst() && r.base->getSubExp2()->isIntConst()) {
		int c1 = ((Const*)base->getSubExp2())->getInt();
		int c2 = ((Const*)r.base->getSubExp2())->getInt();
		if (c1 != c2) {
			if (lowerBound == r.lowerBound && upperBound == r.upperBound &&
				lowerBound == 0) {
				lowerBound = std::min(-c1, -c2);
				upperBound = std::max(-c1, -c2);
				base = base->getSubExp1();
				if (VERBOSE)
					LOG << this << "\n";
				return;
			}
		}
	}
	if (!(*base == *r.base)) {
		stride = 1; lowerBound = NEGINFINITY; upperBound = INFINITY; base = new Const(0);
		if (VERBOSE)
			LOG << this << "\n";
		return;
	}
	if (stride != r.stride)
		stride = std::min(stride, r.stride);
	if (lowerBound != r.lowerBound)
		lowerBound = std::min(lowerBound, r.lowerBound);
	if (upperBound != r.upperBound)
		upperBound = std::max(upperBound, r.upperBound);
	if (VERBOSE)
		LOG << this << "\n";
}

void Range::widenWith(Range &r)
{
	if (VERBOSE)
		LOG << "widening " << this << " with " << r << " got ";
	if (!(*base == *r.base)) {
		stride = 1; lowerBound = NEGINFINITY; upperBound = INFINITY; base = new Const(0);
		if (VERBOSE)
			LOG << this << "\n";
		return;
	}
	// ignore stride for now
	if (r.getLowerBound() < lowerBound) 
		lowerBound = NEGINFINITY;
	if (r.getUpperBound() > upperBound)
		upperBound = INFINITY;
	if (VERBOSE)
		LOG << this << "\n";
}
Range &RangeMap::getRange(Exp *loc) {
	if (ranges.find(loc) == ranges.end()) {
		return *(new Range(1, NEGINFINITY, INFINITY, new Const(0)));
	}
	return ranges[loc];
}

void RangeMap::unionwith(RangeMap &other)
{
	for (std::map<Exp*, Range, lessExpStar>::iterator it = other.ranges.begin(); it != other.ranges.end(); it++) {
		if (ranges.find((*it).first) == ranges.end()) {
			ranges[(*it).first] = (*it).second;
		} else {
			ranges[(*it).first].unionWith((*it).second);
		}
	}
}

void RangeMap::widenwith(RangeMap &other)
{
	for (std::map<Exp*, Range, lessExpStar>::iterator it = other.ranges.begin(); it != other.ranges.end(); it++) {
		if (ranges.find((*it).first) == ranges.end()) {
			ranges[(*it).first] = (*it).second;
		} else {
			ranges[(*it).first].widenWith((*it).second);
		}
	}
}


void RangeMap::print(std::ostream &os)
{
	for (std::map<Exp*, Range, lessExpStar>::iterator it = ranges.begin(); it != ranges.end(); it++) {
		if (it != ranges.begin())
			os << ", ";
		(*it).first->print(os);
		os << " -> ";
		(*it).second.print(os);
	}
}

Exp *RangeMap::substInto(Exp *e)
{
	bool changes;
	int count = 0;
	do {
		changes = false;
		for (std::map<Exp*, Range, lessExpStar>::iterator it = ranges.begin(); it != ranges.end(); it++) {
			bool change = false;
			Exp *eold = e->clone();
			if ((*it).second.getLowerBound() == (*it).second.getUpperBound()) {
				e = e->searchReplaceAll((*it).first, (new Binary(opPlus, (*it).second.getBase(), new Const((*it).second.getLowerBound())))->simplify(), change);
			}
			if (change) {
				e = e->simplify()->simplifyArith();
				if (VERBOSE)
					LOG << "applied " << (*it).first << " to " << eold << " to get " << e << "\n";
				changes = true;
			}
		}
		count++;
		assert(count < 5);
	} while(changes);
	return e;
}

void RangeMap::killAllMemOfs()
{
	for (std::map<Exp*, Range, lessExpStar>::iterator it = ranges.begin(); it != ranges.end(); it++) {
		if ((*it).first->isMemOf()) {
			Range empty;
			(*it).second.unionWith(empty);
		}
	}
}

bool Range::operator==(Range &other)
{
	return stride == other.stride && lowerBound == other.lowerBound && upperBound == other.upperBound && *base == *other.base;
}

// return true if this range map is a subset of the other range map
bool RangeMap::isSubset(RangeMap &other)
{
	for (std::map<Exp*, Range, lessExpStar>::iterator it = ranges.begin(); it != ranges.end(); it++) {
		if (other.ranges.find((*it).first) == other.ranges.end())
			return false;
		Range &r = other.ranges[(*it).first];
		if (!((*it).second == r))
			return false;
	}
	return true;
}



