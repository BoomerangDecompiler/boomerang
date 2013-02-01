/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************//**
 * \file       managed.cpp
 * \brief   Implementation of "managed" classes such as StatementSet, which feature makeUnion etc
 ******************************************************************************/

/*
 * $Revision$    // 1.15.2.13
 *
 * 26 Aug 03 - Mike: Split off from statement.cpp
 * 21 Jun 05 - Mike: Added AssignSet
 */

#include <sstream>
#include <cstring>

#include "types.h"
#include "managed.h"
#include "statement.h"
#include "exp.h"
#include "log.h"
#include "boomerang.h"
#include "proc.h"

extern char debug_buffer[];        // For prints functions


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
    for (it = other.begin(); it != other.end(); it++) {
        insert(*it);
    }
}

// Make this set the difference of itself and other
void StatementSet::makeDiff(StatementSet& other) {
    std::set<Statement*>::iterator it;
    for (it = other.begin(); it != other.end(); it++) {
        erase(*it);
    }
}


// Make this set the intersection of itself and other
void StatementSet::makeIsect(StatementSet& other) {
    std::set<Statement*>::iterator it, ff;
    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);
        if (ff == other.end())
            // Not in both sets
            erase(it);
    }
}

// Check for the subset relation, i.e. are all my elements also in the set
// other. Effectively (this intersect other) == this
bool StatementSet::isSubSetOf(StatementSet& other) {
    std::set<Statement*>::iterator it, ff;
    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);
        if (ff == other.end())
            return false;
    }
    return true;
}


// Remove this Statement. Return false if it was not found
bool StatementSet::remove(Statement* s) {
    if (find(s) != end()) {
        erase(s);
        return true;
    }
    return false;
}

// Search for s in this Statement set. Return true if found
bool StatementSet::exists(Statement* s) {
    iterator it = find(s);
    return (it != end());
}

// Find a definition for loc in this Statement set. Return true if found
bool StatementSet::definesLoc(Exp* loc) {
    for (iterator it = begin(); it != end(); it++) {
        if ((*it)->definesLoc(loc))
            return true;
    }
    return false;
}

// Print to a string, for debugging
char* StatementSet::prints() {
    std::ostringstream ost;
    std::set<Statement*>::iterator it;
    for (it = begin(); it != end(); it++) {
        if (it != begin()) ost << ",\t";
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
    for (it = begin(); it != end(); it++) {
        if (it != begin()) os << ",\t";
        os << *it;
    }
    os << "\n";
}

// Print just the numbers to stream os
void StatementSet::printNums(std::ostream& os) {
    os << std::dec;
    for (iterator it = begin(); it != end(); ) {
        if (*it)
            (*it)->printNum(os);
        else
            os << "-";                // Special case for nullptr definition
        if (++it != end())
            os << " ";
    }
}

bool StatementSet::operator<(const StatementSet& o) const {
    if (size() < o.size()) return true;
    if (size() > o.size()) return false;
    const_iterator it1, it2;
    for (it1 = begin(), it2 = o.begin(); it1 != end();
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
    for (it = other.begin(); it != other.end(); it++) {
        insert(*it);
    }
}

// Make this set the difference of itself and other
void AssignSet::makeDiff(AssignSet& other) {
    iterator it;
    for (it = other.begin(); it != other.end(); it++) {
        erase(*it);
    }
}


// Make this set the intersection of itself and other
void AssignSet::makeIsect(AssignSet& other) {
    iterator it, ff;
    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);
        if (ff == other.end())
            // Not in both sets
            erase(it);
    }
}

// Check for the subset relation, i.e. are all my elements also in the set
// other. Effectively (this intersect other) == this
bool AssignSet::isSubSetOf(AssignSet& other) {
    iterator it, ff;
    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);
        if (ff == other.end())
            return false;
    }
    return true;
}


// Remove this Assign. Return false if it was not found
bool AssignSet::remove(Assign* a) {
    if (find(a) != end()) {
        erase(a);
        return true;
    }
    return false;
}

// Search for a in this Assign set. Return true if found
bool AssignSet::exists(Assign* a) {
    iterator it = find(a);
    return (it != end());
}

// Find a definition for loc in this Assign set. Return true if found
bool AssignSet::definesLoc(Exp* loc) {
    Assign* as = new Assign(loc, new Terminal(opWild));
    iterator ff = find(as);
    return ff != end();
}

// Find a definition for loc on the LHS in this Assign set. If found, return pointer to the Assign with that LHS
Assign* AssignSet::lookupLoc(Exp* loc) {
    Assign* as = new Assign(loc, new Terminal(opWild));
    iterator ff = find(as);
    if (ff == end()) return nullptr;
    return *ff;
}

// Print to a string, for debugging
char* AssignSet::prints() {
    std::ostringstream ost;
    iterator it;
    for (it = begin(); it != end(); it++) {
        if (it != begin()) ost << ",\t";
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
    for (it = begin(); it != end(); it++) {
        if (it != begin())
            os << ",\t";
        os << *it;
    }
    os << "\n";
}

// Print just the numbers to stream os
void AssignSet::printNums(std::ostream& os) {
    os << std::dec;
    for (iterator it = begin(); it != end(); ) {
        if (*it)
            (*it)->printNum(os);
        else
            os << "-";                // Special case for nullptr definition
        if (++it != end())
            os << " ";
    }
}

bool AssignSet::operator<(const AssignSet& o) const {
    if (size() < o.size()) return true;
    if (size() > o.size()) return false;
    const_iterator it1, it2;
    for (it1 = begin(), it2 = o.begin(); it1 != end();
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
    //delete *it;          // These expressions were cloned when created
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

bool LocationSet::operator==(const LocationSet& o) const {
    // We want to compare the locations, not the pointers
    if (size() != o.size()) return false;
    std::set<Exp*, lessExpStar>::const_iterator it1, it2;
    for (it1 = lset.begin(), it2 = o.lset.begin(); it1 != lset.end(); it1++, it2++) {
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
    // Note: can't search with a wildcard, since it doesn't have the weak ordering required (I think)
    RefExp r(e, nullptr);
    // Note: the below assumes that nullptr is less than any other pointer
    iterator it = lset.lower_bound(&r);
    if (it == lset.end())
        return nullptr;
    if ((*((RefExp*) *it)->getSubExp1() == *e))
        return *it;
    else
        return nullptr;
}

// Given an unsubscripted location e, return true if e{-} or e{0} exists in the set
bool LocationSet::existsImplicit(Exp* e) {
    RefExp r(e, nullptr);
    iterator it = lset.lower_bound(&r);        // First element >= r
    // Note: the below relies on the fact that nullptr is less than any other pointer. Try later entries in the set:
    while (it != lset.end()) {
        if (!(*it)->isSubscript()) return false;        // Looking for e{something} (could be e.g. %pc)
        if (!(*((RefExp*) *it)->getSubExp1() == *e))    // Gone past e{anything}?
            return false;                                // Yes, then e{-} or e{0} cannot exist
        if (((RefExp*) *it)->isImplicitDef())            // Check for e{-} or e{0}
            return true;                                // Found
        ++it;                                            // Else check next entry
    }
    return false;
}

// Find a location with a different def, but same expression. For example, pass r28{10},
// return true if r28{20} in the set. If return true, dr points to the first different ref
bool LocationSet::findDifferentRef(RefExp* e, Exp *&dr) {
    assert(e);
    RefExp search(e->getSubExp1()->clone(), (Statement*)-1);
    std::set<Exp*, lessExpStar>::iterator pos = lset.find(&search);
    if (pos == lset.end())
        return false;
    while (pos != lset.end()) {
        assert(*pos);
        // Exit if we've gone to a new base expression
        // E.g. searching for r13{10} and **pos is r14{0}
        // Note: we want a ref-sensitive compare, but with the outer refs stripped off
        // For example: m[r29{10} - 16]{any} is different from m[r29{20} - 16]{any}
        if (!(*(*pos)->getSubExp1() == *e->getSubExp1()))
            break;
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
    lset = newSet;            // Replace the old set!
    // Note: don't delete the old exps; they are copied in the new set
}

// Substitute s into all members of the set
void LocationSet::substitute(Assign& a) {
    Exp* lhs = a.getLeft();
    if (lhs == nullptr) return;
    Exp* rhs = a.getRight();
    if (rhs == nullptr) return;        // ? Will this ever happen?
    std::set<Exp*, lessExpStar>::iterator it;
    // Note: it's important not to change the pointer in the set of pointers to expressions, without removing and
    // inserting again. Otherwise, the set becomes out of order, and operations such as set comparison fail!
    // To avoid any funny behaviour when iterating the loop, we use the following two sets
    LocationSet removeSet;            // These will be removed after the loop
    LocationSet removeAndDelete;    // These will be removed then deleted
    LocationSet insertSet;            // These will be inserted after the loop
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
                removeSet.insert(*it);        // Note: remove the unmodified ptr
                insertSet.insert(loc);
            }
        }
    }
    makeDiff(removeSet);       // Remove the items to be removed
    makeDiff(removeAndDelete); // These are to be removed as well
    makeUnion(insertSet);       // Insert the items to be added
    // Now delete the expressions that are no longer needed
    std::set<Exp*, lessExpStar>::iterator dd;
    for (dd = removeAndDelete.lset.begin(); dd != removeAndDelete.lset.end();
         dd++)
        delete *dd;                // Plug that memory leak
}

//
// StatementList methods
//

bool StatementList::remove(Statement* s) {
    iterator it;
    for (it = begin(); it != end(); it++) {
        if (*it == s) {
            erase(it);
            return true;
        }
    }
    return false;
}

void StatementList::append(StatementList& sl) {
    insert(end(),sl.begin(),sl.end());
}

void StatementList::append(StatementSet& ss) {
    insert(end(),ss.begin(),ss.end());
}

char* StatementList::prints() {
    std::ostringstream ost;
    for (iterator it = begin(); it != end(); it++) {
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
        svec.resize(idx+1, nullptr);
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
            os << "-";                // Special case for no definition
        if (++it != svec.end())
            os << " ";
    }
}


// Special intersection method: this := a intersect b
void StatementList::makeIsect(StatementList& a, LocationSet& b) {
    clear();
    for (iterator it = a.begin(); it != a.end(); ++it) {
        Assignment* as = (Assignment*)*it;
        if (b.exists(as->getLeft()))
            push_back(as);
    }
}

void StatementList::makeCloneOf(StatementList& o) {
    clear();
    for (iterator it = o.begin(); it != o.end(); it++)
        push_back((*it)->clone());
}

// Return true if loc appears on the left of any statements in this list
// Note: statements in this list are assumed to be assignments
bool StatementList::existsOnLeft(Exp* loc) {
    for (iterator it = begin(); it != end(); it++) {
        if (*((Assignment*)*it)->getLeft() == *loc)
            return true;
    }
    return false;
}

// Remove the first definition where loc appears on the left
// Note: statements in this list are assumed to be assignments
void StatementList::removeDefOf(Exp* loc) {
    for (iterator it = begin(); it != end(); it++) {
        if (*((Assignment*)*it)->getLeft() == *loc) {
            erase(it);
            return;
        }
    }
}

// Find the first Assignment with loc on the LHS
Assignment* StatementList::findOnLeft(Exp* loc) {
    if (empty())
        return nullptr;
    for (iterator it = begin(); it != end(); it++) {
        Exp *left = ((Assignment*)*it)->getLeft();
        if (*left == *loc)
            return (Assignment*)*it;
        if (left->isLocal()) {
            Location *l = (Location*)left;
            Exp *e = l->getProc()->expFromSymbol(((Const*)l->getSubExp1())->getStr());
            if (e && ((*e == *loc) || (e->isSubscript() && *e->getSubExp1() == *loc))) {
                return (Assignment*)*it;
            }
        }
    }
    return nullptr;
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
Range::Range() : stride(1), lowerBound(MIN), upperBound(MAX) {
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
        if (base == nullptr)
            //NOTE: was "base = new Const(0);"
            this->base = new Const(0);
        if (lowerBound > upperBound)
            this->upperBound = lowerBound;
        if (upperBound < lowerBound)
            this->lowerBound = upperBound;
    }
}

void Range::print(std::ostream &os) {
    assert(lowerBound <= upperBound);
    if (base->isIntConst() && ((Const*)base)->getInt() == 0 &&
            lowerBound == MIN && upperBound == MAX) {
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
        if (lowerBound == MIN)
            os << "-inf";
        else
            os << lowerBound;
        os << ", ";
        if (upperBound == MAX)
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

void Range::unionWith(Range &r) {
    if (VERBOSE && DEBUG_RANGE_ANALYSIS)
        LOG << "unioning " << this << " with " << r << " got ";
    assert(base && r.base);
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
                if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                    LOG << this << "\n";
                return;
            }
        }
    }
    if (!(*base == *r.base)) {
        stride = 1; lowerBound = MIN; upperBound = MAX; base = new Const(0);
        if (VERBOSE && DEBUG_RANGE_ANALYSIS)
            LOG << this << "\n";
        return;
    }
    if (stride != r.stride)
        stride = std::min(stride, r.stride);
    if (lowerBound != r.lowerBound)
        lowerBound = std::min(lowerBound, r.lowerBound);
    if (upperBound != r.upperBound)
        upperBound = std::max(upperBound, r.upperBound);
    if (VERBOSE && DEBUG_RANGE_ANALYSIS)
        LOG << this << "\n";
}

void Range::widenWith(Range &r) {
    if (VERBOSE && DEBUG_RANGE_ANALYSIS)
        LOG << "widening " << this << " with " << r << " got ";
    if (!(*base == *r.base)) {
        stride = 1; lowerBound = MIN; upperBound = MAX; base = new Const(0);
        if (VERBOSE && DEBUG_RANGE_ANALYSIS)
            LOG << this << "\n";
        return;
    }
    // ignore stride for now
    if (r.getLowerBound() < lowerBound)
        lowerBound = MIN;
    if (r.getUpperBound() > upperBound)
        upperBound = MAX;
    if (VERBOSE && DEBUG_RANGE_ANALYSIS)
        LOG << this << "\n";
}
Range &RangeMap::getRange(Exp *loc) {
    if (ranges.find(loc) == ranges.end()) {
        return *(new Range(1, Range::MIN, Range::MAX, new Const(0)));
    }
    return ranges[loc];
}

void RangeMap::unionwith(RangeMap &other) {
    for (std::map<Exp*, Range, lessExpStar>::iterator it = other.ranges.begin(); it != other.ranges.end(); it++) {
        if (ranges.find((*it).first) == ranges.end()) {
            ranges[(*it).first] = (*it).second;
        } else {
            ranges[(*it).first].unionWith((*it).second);
        }
    }
}

void RangeMap::widenwith(RangeMap &other) {
    for (auto it = other.ranges.begin(); it != other.ranges.end(); it++) {
        if (ranges.find((*it).first) == ranges.end()) {
            ranges[(*it).first] = (*it).second;
        } else {
            ranges[(*it).first].widenWith((*it).second);
        }
    }
}


void RangeMap::print(std::ostream &os) {
    for (auto it = ranges.begin(); it != ranges.end(); it++) {
        if (it != ranges.begin())
            os << ", ";
        (*it).first->print(os);
        os << " -> ";
        (*it).second.print(os);
    }
}

Exp *RangeMap::substInto(Exp *e, std::set<Exp*, lessExpStar> *only) {
    bool changes;
    int count = 0;
    do {
        changes = false;
        for (std::map<Exp*, Range, lessExpStar>::iterator it = ranges.begin(); it != ranges.end(); it++) {
            if (only && only->find((*it).first) == only->end())
                continue;
            bool change = false;
            Exp *eold = nullptr;
            if(DEBUG_RANGE_ANALYSIS)
                eold=e->clone();
            if ((*it).second.getLowerBound() == (*it).second.getUpperBound()) {
                e = e->searchReplaceAll((*it).first, (new Binary(opPlus, (*it).second.getBase(), new Const((*it).second.getLowerBound())))->simplify(), change);
            }
            if (change) {
                e = e->simplify()->simplifyArith();
                if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                    LOG << "applied " << (*it).first << " to " << eold << " to get " << e << "\n";
                changes = true;
            }
        }
        count++;
        assert(count < 5);
    } while(changes);
    return e;
}

void RangeMap::killAllMemOfs() {
    for (std::map<Exp*, Range, lessExpStar>::iterator it = ranges.begin(); it != ranges.end(); it++) {
        if ((*it).first->isMemOf()) {
            Range empty;
            (*it).second.unionWith(empty);
        }
    }
}

bool Range::operator==(Range &other) {
    return stride == other.stride && lowerBound == other.lowerBound && upperBound == other.upperBound && *base == *other.base;
}

// return true if this range map is a subset of the other range map
bool RangeMap::isSubset(RangeMap &other) {
    for (std::pair<Exp*, Range> it : ranges) {
        if (other.ranges.find(it.first) == other.ranges.end()) {
            if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                LOG << "did not find " << it.first << " in other, not a subset\n";
            return false;
        }
        Range &r = other.ranges[it.first];
        if (!(it.second == r)) {
            if (VERBOSE && DEBUG_RANGE_ANALYSIS)
                LOG << "range for " << it.first << " in other " << r << " is not equal to range in this " << it.second << ", not a subset\n";
            return false;
        }
    }
    return true;
}


//    class ConnectionGraph

void ConnectionGraph::add(Exp* a, Exp* b) {
    iterator ff = emap.find(a);
    while (ff != emap.end() && *ff->first == *a) {
        if (*ff->second == *b) return;        // Don't add a second entry
        ++ff;
    }
    std::pair<Exp*, Exp*> pr;
    pr.first = a; pr.second = b;
    emap.insert(pr);
}

void ConnectionGraph::connect(Exp* a, Exp* b) {
    add(a, b);
    add(b, a);
}
//! Return a count of locations connected to \a e
int ConnectionGraph::count(Exp* e) const {
    const_iterator ff = emap.find(e);
    int n = 0;
    while (ff != emap.end() && *ff->first == *e) {
        ++n;
        ++ff;
    }
    return n;
}
//! Return true if a is connected to b
bool ConnectionGraph::isConnected(Exp* a, const Exp& b) const {
    const_iterator ff = emap.find(a);
    while (ff != emap.end() && *ff->first == *a) {
        if (*ff->second == b)
            return true;                    // Found the connection
        ++ff;
    }
    return false;
}


// Modify the map so that a <-> b becomes a <-> c
//! Update the map that used to be a <-> b, now it is a <-> c
void ConnectionGraph::update(Exp* a, Exp* b, Exp* c) {
    // find a->b
    iterator ff = emap.find(a);
    while (ff != emap.end() && *ff->first == *a) {
        if (*ff->second == *b) {
            ff->second = c;            // Now a->c
            break;
        }
        ++ff;
    }
    // find b -> a
    ff = emap.find(b);
    while (ff != emap.end() && *ff->first == *b) {
        if (*ff->second == *a) {
            emap.erase(ff);
            add(c, a);                // Now c->a
            break;
        }
        ++ff;
    }
}

// Remove the mapping at *aa, and return a valid iterator for looping
ConnectionGraph::iterator ConnectionGraph::remove(iterator aa) {
    assert (aa != emap.end());
    Exp* b = aa->second;
    emap.erase(aa++);
    iterator bb = emap.find(b);
    assert(bb != emap.end());
    if (bb == aa)
        ++aa;
    emap.erase(bb);
    return aa;
}

// For debugging
void dumpConnectionGraph(const ConnectionGraph* cg) {
    ConnectionGraph::const_iterator cc;
    for (cc = cg->begin(); cc != cg->end(); ++cc)
        std::cerr << cc->first << " <-> " << cc->second << "\n";
}

void ConnectionGraph::dump() const {
    for (auto iter : *this)
        std::cerr << iter.first << " <-> " << iter.second << "\n";
}
