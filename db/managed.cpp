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
 * OVERVIEW:   Implementation of "managed" classes such as StatementSet, which
 *				feature makeUnion etc
 *============================================================================*/

/*
 * $Revision$
 * 26 Aug 03 - Mike: Split off from statement.cpp
 */

#include <sstream>

#include "types.h"
#include "managed.h"
#include "statement.h"
#include "exp.h"

std::ostream& operator<<(std::ostream& os, StatementSet* ss) {
	ss->print(os);
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

// Find s in this Statement set. Return true if found
bool StatementSet::exists(Statement* s) {
	std::set<Statement*>::iterator it = sset.find(s);
	return (it != sset.end());
}

// Find a definition for loc in this Statement set. Return true if found
bool StatementSet::defines(Exp* loc) {
	for (iterator it = sset.begin(); it != sset.end(); it++) {
		Exp* lhs = (*it)->getLeft();
		if (lhs && (*lhs == *loc))
			return true;
	}
	return false;
}

// Remove if defines the given expression
bool StatementSet::removeIfDefines(Exp* given) {
	bool found = false;
	for (iterator it = sset.begin(); it != sset.end(); it++) {
		Exp* left = (*it)->getLeft();
		if (left && *left == *given) {
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

extern char debug_buffer[];		 // For prints functions
// Print to a string, for debugging
char* StatementSet::prints() {
	std::ostringstream ost;
	std::set<Statement*>::iterator it;
	for (it = sset.begin(); it != sset.end(); it++) {
		if (it != sset.begin()) ost << ",\t";
		ost << *it;
	}
	ost << "\n";
	strncpy(debug_buffer, ost.str().c_str(), 199);
	debug_buffer[199] = '\0';
	return debug_buffer;
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
// LocationSet methods
//

// Assignment operator
LocationSet& LocationSet::operator=(const LocationSet& o) {
	sset.clear();
	std::set<Exp*, lessExpStar>::const_iterator it;
	for (it = o.sset.begin(); it != o.sset.end(); it++) {
		sset.insert((*it)->clone());
	}
	return *this;
}

// Copy constructor
LocationSet::LocationSet(const LocationSet& o) {
	std::set<Exp*, lessExpStar>::const_iterator it;
	for (it = o.sset.begin(); it != o.sset.end(); it++)
		sset.insert((*it)->clone());
}

char* LocationSet::prints() {
	std::ostringstream ost;
	std::set<Exp*, lessExpStar>::iterator it;
	for (it = sset.begin(); it != sset.end(); it++) {
		if (it != sset.begin()) ost << ",\t";
		ost << *it;
	}
	ost << "\n";
	strncpy(debug_buffer, ost.str().c_str(), 199);
	debug_buffer[199] = '\0';
	return debug_buffer;
}

void LocationSet::print(std::ostream& os) {
	std::set<Exp*, lessExpStar>::iterator it;
	for (it = sset.begin(); it != sset.end(); it++) {
		if (it != sset.begin()) os << ",\t";
		os << *it;
	}
	os << "\n";
}

void LocationSet::remove(Exp* given) {
	std::set<Exp*, lessExpStar>::iterator it = sset.find(given);
	if (it == sset.end()) return;
//std::cerr << "LocationSet::remove at " << std::hex << (unsigned)this << " of " << *it << "\n";
//std::cerr << "before: "; print();
	// NOTE: if the below uncommented, things go crazy. Valgrind says that
	// the deleted value gets used next in LocationSet::operator== ?!
	//delete *it;		  // These expressions were cloned when created
	sset.erase(it);
//std::cerr << "after : "; print();
}

#if 0
void LocationSet::remove(LocSetIter ll) {
	//delete *ll;		// Don't trust this either
	sset.erase(ll);
}
#endif

// Remove locations defined by any of the given set of statements
// Used for killing in liveness sets
void LocationSet::removeIfDefines(StatementSet& given) {
	StatementSet::iterator it;
	for (it = given.begin(); it != given.end(); it++) {
		Statement* s = (Statement*)*it;
		Exp* givenLeft = s->getLeft();
		if (givenLeft)
			sset.erase(givenLeft);
	}
}

// Make this set the union of itself and other
void LocationSet::makeUnion(LocationSet& other) {
	iterator it;
	for (it = other.sset.begin(); it != other.sset.end(); it++) {
		sset.insert(*it);
	}
}

// Make this set the set difference of itself and other
void LocationSet::makeDiff(LocationSet& other) {
	std::set<Exp*, lessExpStar>::iterator it;
	for (it = other.sset.begin(); it != other.sset.end(); it++) {
		sset.erase(*it);
	}
}

#if 0
Exp* LocationSet::getFirst(LocSetIter& it) {
	it = sset.begin();
	if (it == sset.end())
		// No elements
		return NULL;
	return *it;			// Else return the first element
}

Exp* LocationSet::getNext(LocSetIter& it) {
	if (++it == sset.end())
		// No more elements
		return NULL;
	return *it;			// Else return the next element
}
#endif

bool LocationSet::operator==(const LocationSet& o) const {
	// We want to compare the strings, not the pointers
	if (size() != o.size()) return false;
	std::set<Exp*, lessExpStar>::const_iterator it1, it2;
	for (it1 = sset.begin(), it2 = o.sset.begin(); it1 != sset.end();
	  it1++, it2++) {
		if (!(**it1 == **it2)) return false;
	}
	return true;
}

bool LocationSet::find(Exp* e) {
	return sset.find(e) != sset.end();
}

bool LocationSet::findDifferentRef(RefExp* e, Exp *&dr) {
	RefExp search(e->getSubExp1()->clone(), (Statement*)-1);
	std::set<Exp*, lessExpStar>::iterator pos = sset.find(&search);
	if (pos == sset.end()) return false;
	while (pos != sset.end()) {
		// Exit if we've gone to a new base expression
		// E.g. searching for r13{10} and **pos is r14{0}
		if (!(**pos *= *e)) break;		// *= is ref-insensitive compare
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
	for (it = sset.begin(); it != sset.end(); it++)
		newSet.insert((*it)->expSubscriptVar(*it, d /* , cfg */));
	sset = newSet;			// Replace the old set!
	// Note: don't delete the old exps; they are copied in the new set
}

// Substitute s into all members of the set
void LocationSet::substitute(Statement& s) {
	Exp* lhs = s.getLeft();
	if (lhs == NULL) return;
	Exp* rhs = s.getRight();
	if (rhs == NULL) return;		// ? Will this ever happen?
	std::set<Exp*, lessExpStar>::iterator it;
	// Note: it's important not to change the pointer in the set of pointers
	// to expressions, without removing and inserting again. Otherwise, the
	// set becomes out of order, and operations such as set comparison fail!
	// To avoid any funny behaviour when iterating the loop, we use the follow-
	// ing two sets
	LocationSet removeSet;			// These will be removed after the loop
	LocationSet removeAndDelete;	// These will be removed then deleted
	LocationSet insertSet;			// These will be inserted after the loop
	bool change;
	for (it = sset.begin(); it != sset.end(); it++) {
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
	for (dd = removeAndDelete.sset.begin(); dd != removeAndDelete.sset.end();
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
	strncpy(debug_buffer, ost.str().c_str(), 199);
	debug_buffer[199] = '\0';
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

// Print only the left hand sides to stream os
void StatementVec::printLefts(std::ostream& os) {
	for (iterator it = svec.begin(); it != svec.end(); ) {
		if (*it) {
			Exp* left = (*it)->getLeft();
			if (left) {
				left->print(os);
				os << "{" << std::dec << (*it)->getNumber() << "}";
			} else 
				os << "-";
		}
		else
			os << "-";
		if (++it != svec.end())
			os << " ";
	}
}

char* StatementVec::prints() {
	std::ostringstream ost;
	iterator it;
	for (it = svec.begin(); it != svec.end(); it++) {
		ost << *it << ",\t";
	}
	strncpy(debug_buffer, ost.str().c_str(), 199);
	debug_buffer[199] = '\0';
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



