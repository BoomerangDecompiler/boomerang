/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       constraint.cpp
 * OVERVIEW:   Implementation of objects related to type constraints
 *============================================================================*/

/*
 * $Revision$
 *
 * 22 Aug 03 - Mike: Created
 */

#include "constraint.h"
#include "managed.h"
#include "exp.h"
#include "boomerang.h"
#include <sstream>

void ConstraintMap::print(std::ostream& os) {
    iterator kk;
    bool first = true;
    for (kk = cmap.begin(); kk != cmap.end(); kk++) {
        if (first) first = false;
        else os << ", ";
        os << kk->first << " = " << kk->second;
    }
    os << "\n";
}

extern char debug_buffer[];
char* ConstraintMap::prints() {
    std::ostringstream ost;
    print(ost);
    strncpy(debug_buffer, ost.str().c_str(), 999);
    debug_buffer[999] = '\0';
    return debug_buffer;
}

void ConstraintMap::insert(Exp* term) {
    assert(term->isEquality());
    Exp* lhs = ((Binary*)term)->getSubExp1();
    Exp* rhs = ((Binary*)term)->getSubExp2();
    cmap[lhs] = rhs;
}


void EquateMap::print(std::ostream& os) {
    iterator ee;
    for (ee = emap.begin(); ee != emap.end(); ee++) {
        os << "  " << ee->first << " = " << ee->second.prints();
    }
    os << "\n";
}

char* EquateMap::prints() {
    std::ostringstream ost;
    print(ost);
    strncpy(debug_buffer, ost.str().c_str(), 999);
    debug_buffer[999] = '\0';
    return debug_buffer;
}



Constraints::~Constraints() {
    LocationSet::iterator cc;
    for (cc = conSet.begin(); cc != conSet.end(); cc++) {
        delete *cc;
    }
}

// Get the next disjunct from this disjunction
// Assumes that the remainder is of the for a or (b or c), or (a or b) or c
// But NOT (a or b) or (c or d)
// Could also be just a (a conjunction, or a single constraint)
// Note: remainder is changed by this function
Exp* nextDisjunct(Exp*& remainder) {
    if (remainder == NULL) return NULL;
    if (remainder->isDisjunction()) {
        Exp* d1 = ((Binary*)remainder)->getSubExp1();
        Exp* d2 = ((Binary*)remainder)->getSubExp2();
        if (d1->isDisjunction()) {
            remainder = d1;
            return d2;
        }
        remainder = d2;
        return d1;
    }
    // Else, we have one disjunct. Return it
    Exp* ret = remainder;
    remainder = NULL;
    return ret;
}

Exp* nextConjunct(Exp*& remainder) {
    if (remainder == NULL) return NULL;
    if (remainder->isConjunction()) {
        Exp* c1 = ((Binary*)remainder)->getSubExp1();
        Exp* c2 = ((Binary*)remainder)->getSubExp2();
        if (c1->isConjunction()) {
            remainder = c1;
            return c2;
        }
        remainder = c2;
        return c1;
    }
    // Else, we have one conjunct. Return it
    Exp* ret = remainder;
    remainder = NULL;
    return ret;
}

bool Constraints::solve(std::list<ConstraintMap>& solns) {
std::cerr << conSet.size() << " constraints:";
conSet.print(std::cerr);
    // Replace Ta[loc] = ptr(alpha) with
    //         Tloc = alpha
    LocationSet::iterator cc;
    for (cc = conSet.begin(); cc != conSet.end(); cc++) {
        Exp* c = *cc;
        if (!c->isEquality()) continue;
        Exp* left  = ((Binary*)c)->getSubExp1();
        if (!left->isTypeOf()) continue;
        Exp* leftSub = ((Unary*)left)->getSubExp1();
        if (!leftSub->isAddrOf()) continue;
        Exp* right = ((Binary*)c)->getSubExp2();
        if (!right->isTypeVal()) continue;
        Type* t = ((TypeVal*)right)->getType();
        if (!t->isPointer()) continue;
        // Don't modify a key in a map
        Exp* clone = c->clone();
        // left is typeof(addressof(something)) -> typeof(something)
        left  = ((Binary*)clone)->getSubExp1();
        leftSub = ((Unary*)left)->getSubExp1();
        Exp* something = ((Unary*)leftSub)->getSubExp1();
        ((Unary*)left)->setSubExp1ND(something);
        ((Unary*)leftSub)->setSubExp1ND(NULL);
        delete leftSub;
        // right is <alpha*> -> <alpha>
        right = ((Binary*)clone)->getSubExp2();
        t = ((TypeVal*)right)->getType();
        ((TypeVal*)right)->setType(((PointerType*)t)->getPointsTo()->clone());
        delete t;
        conSet.remove(c);
        conSet.insert(clone);
        delete c;
    }

    // Sort constraints into a few categories. Disjunctions go to a special
    // list, always true is just ignored, and constraints of the form
    // typeof(x) = y (where y is a type value) go to a map called fixed.
    // Constraint terms of the form Tx = Ty go into a map of LocationSets
    // called equates for fast lookup
    for (cc = conSet.begin(); cc != conSet.end(); cc++) {
        Exp* c = *cc;
        if (c->isTrue()) continue;
        if (c->isFalse()) {
            if (VERBOSE || DEBUG_TA)
                std::cerr << "Constraint failure: always false constraint\n";
            return false;
        }
        if (c->isDisjunction()) {
            disjunctions.push_back(c);
            continue;
        }
        // Break up conjunctions into terms
        Exp* rem = c, *term;
        while ((term = nextConjunct(rem)) != NULL) {
            assert(term->isEquality());
            Exp* lhs = ((Binary*)term)->getSubExp1();
            Exp* rhs = ((Binary*)term)->getSubExp2();
            if (rhs->isTypeOf()) {
                // Of the form typeof(x) = typeof(z)
                // Insert into equates 
                equates.addEquate(lhs, rhs);
            } else {
                // Of the form typeof(x) = <typeval>
                // Insert into fixed
                assert(rhs->isTypeVal());
                fixed[lhs] = rhs;
            }
        }
    }

{std::cerr << "\n" << disjunctions.size() << " disjunctions: "; std::list<Exp*>::iterator dd; for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++) std::cerr << *dd << ", "; std::cerr << "\n";}
std::cerr << fixed.size() << " fixed: " << fixed.prints();
std::cerr << equates.size() << " equates: " << equates.prints();

    // Substitute the fixed types into the disjunctions
    ConstraintMap::iterator kk;
    for (kk = fixed.begin(); kk != fixed.end(); kk++) {
        Exp* from = kk->first;
        Exp* to = kk->second;
        bool ch;
        std::list<Exp*>::iterator dd;
        for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++)
            (*dd)->searchReplaceAll(from, to, ch);
    }

    // Substitute the fixed types into the equates. This may generate more
    // fixed types
    ConstraintMap extra;
    ConstraintMap in = fixed;
    while (in.size()) {
        extra.clear();
        for (kk = in.begin(); kk != in.end(); kk++) {
            Exp* lhs = kk->first;
            std::map<Exp*, LocationSet, lessExpStar>::iterator it =
              equates.find(lhs);
            if (it != equates.end()) {
                // Possibly new constraints that 
                // typeof(elements in it->second) == val
                Exp* val = kk->second;
                LocationSet& ls = it->second;
                LocationSet::iterator ll;
                for (ll = ls.begin(); ll != ls.end(); ll++) {
                    ConstraintMap::iterator ff;
                    ff = fixed.find(*ll);
                    if (ff != fixed.end()) {
                        if (!unify(val, ff->second, extra)) {
                            if (VERBOSE || DEBUG_TA)
                                std::cerr << "Constraint failure: " <<
                                  *ll << " constrained to be " <<
                                  ((TypeVal*)val)->getType()->getCtype() <<
                                  " and " <<
                                  ((TypeVal*)ff->second)->getType()->getCtype()
                                  << "\n";
                            return false;
                        }
                    } else
                        extra[*ll] = val;   // A new constant constraint
                }
                // Remove the equate
                equates.erase(it);
            }
        }
        fixed.makeUnion(extra);
        in = extra;     // Take care of any "ripple effect"
    }                   // Repeat until no ripples

std::cerr << "\n" << fixed.size() << " fixed: " << fixed.prints();
std::cerr << equates.size() << " equates: " << equates.prints();

    ConstraintMap soln;
    return doSolve(disjunctions.begin(), soln, solns);
}

static int level = 0;
// Constraints up to but not including iterator it have been unified.
// The current solution is soln
// The set of all solutions is in solns
bool Constraints::doSolve(std::list<Exp*>::iterator it, ConstraintMap& soln,
  std::list<ConstraintMap>& solns) {
std::cerr << "Begin doSolve at level " << ++level << "\n";
std::cerr << "Soln now: " << soln.prints() << "\n";
    if (it == disjunctions.end()) {
        // We have gotten to the end with no unification failures
        // Copy the current set of constraints as a solution
        //if (soln.size() == 0)
            // Awkward. There is a trivial solution, but we have no constraints
            // So make a constraint of always-true
            //soln.insert(new Terminal(opTrue));
        // Copy the fixed constraints
        soln.makeUnion(fixed);
        solns.push_back(soln);
std::cerr << "Exiting doSolve at level " << level-- << " returning true\n";
        return true;
    }

    Exp* dj = *it;
    // Iterate through each disjunction d of dj
    Exp* rem1 = dj;       // Remainder
    bool anyUnified = false;
    Exp* d;
    while ((d = nextDisjunct(rem1)) != NULL) {
std::cerr << " $$ d is " << d << ", rem1 is " << ((rem1==0)?"NULL":rem1->prints()) << " $$\n";
        // Match disjunct d against the fixed types; it could be compatible,
        // compatible and generate an additional constraint, or be
        // incompatible
        ConstraintMap extra;      // Just for this disjunct
        Exp* c;
        Exp* rem2 = d;
        bool unified = true;
        while ((c = nextConjunct(rem2)) != NULL) {
std::cerr << "   $$ c is " << c << ", rem2 is " << ((rem2==0)?"NULL":rem2->prints()) << " $$\n";
            assert(c->isEquality());
            Exp* lhs = ((Binary*)c)->getSubExp1();
            Exp* rhs = ((Binary*)c)->getSubExp2();
            extra.insert(lhs, rhs);
            ConstraintMap::iterator kk;
            kk = fixed.find(lhs);
            if (kk != fixed.end()) {
                unified &= unify(rhs, kk->second, extra);
std::cerr << "Unified now " << unified << "; extra now " << extra.prints() << "\n";
                if (!unified) break;
            }
        }
        if (unified)
            // True if any disjuncts had all the conjuncts satisfied
            anyUnified = true;
        if (!unified) continue;
        // Use this disjunct
        // We can't just difference out extra if this fails; it may remove
        // elements from soln that should not be removed
        // So need a copy of the old set in oldSoln
        ConstraintMap oldSoln = soln;
        soln.makeUnion(extra);
        doSolve(++it, soln, solns);
        // Revert to the previous soln (whether doSolve returned true or not)
        soln = oldSoln;
std::cerr << "After doSolve returned: soln back to: " << soln.prints() << "\n";
        // Back to the current disjunction
        it--;
        // Continue for more disjuncts this disjunction
    }
    // We have run out of disjuncts. Return true if any disjuncts had no
    // unification failures
std::cerr << "Exiting doSolve at level " << level-- << " returning " << anyUnified << "\n";
    return anyUnified;
}

bool Constraints::unify(Exp* x, Exp* y, ConstraintMap& extra) {
std::cerr << "Unifying " << x << " with " << y << " result ";
    assert(x->isTypeVal());
    assert(y->isTypeVal());
    Type* xtype = ((TypeVal*)x)->getType();
    Type* ytype = ((TypeVal*)y)->getType();
    if (xtype->isPointer() && ytype->isPointer()) {
        Type* xPointsTo = ((PointerType*)xtype)->getPointsTo();
        Type* yPointsTo = ((PointerType*)ytype)->getPointsTo();
        if (((PointerType*)xtype)->pointsToAlpha() ||
            ((PointerType*)ytype)->pointsToAlpha()) {
            // A new constraint: xtype must be equal to ytype; at least
            // one of these is a variable type
            if (((PointerType*)xtype)->pointsToAlpha())
                extra.constrain(xPointsTo, yPointsTo);
            else
                extra.constrain(yPointsTo, xPointsTo);
std::cerr << "true\n";
            return true;
        }
std::cerr << (*xPointsTo == *yPointsTo) << "\n";
        return *xPointsTo == *yPointsTo;
    }
std::cerr << (*xtype == *ytype) << "\n";
    return *xtype == *ytype;
}
