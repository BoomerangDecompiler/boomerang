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

void Constraints::solve(std::list<LocationSet>& solns) {
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

    // Sort constraints into a few categories. Disjuncts go to a special
    // list, always true is just ignored, and constraints of the form
    // typeof(x) = y (where y is a type value) go to a set called constants.
    // Where y is a another typeof, the constraint goes into a set called
    // equates1 (as typeof(x) = y) and equates2 (as y = typeof(x)),
    // for fast lookup
    for (cc = conSet.begin(); cc != conSet.end(); cc++) {
        Exp* c = *cc;
        if (c->isTrue()) continue;
        if (c->isFalse()) {
            if (VERBOSE || DEBUG_TA)
                std::cerr << "Constraint failure: always false constraint\n";
            return;
        }
        if (c->isDisjunction()) {
            disjunctions.push_back(c);
            continue;
        }
        assert(c->isEquality());
        Exp* rhs = ((Binary*)c)->getSubExp2();
        if (rhs->isTypeOf()) {
            // Of the form typeof(x) = typeof(z)
            // Insert into equates1 typeof(x) == typeof(z)
            // Insert into equates2 typeof(z) == typeof(x)
            Exp* lhs = ((Binary*)c)->getSubExp1();
            equates1[lhs] = rhs;
            equates2[rhs] = lhs;
        } else {
            assert(rhs->isTypeVal());
            constants.insert(c);
        }
    }

{std::cerr << "\ndisjunctions: "; std::list<Exp*>::iterator dd; for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++) std::cerr << *dd << ", "; std::cerr << "\n";}
{std::cerr << "constants: "; LocationSet::iterator kk; for (kk = constants.begin(); kk != constants.end(); kk++) std::cerr << *kk << ", "; std::cerr << "\n";}
{std::cerr << "equates1: "; std::map<Exp*, Exp*, lessExpStar>::iterator ee; for (ee = equates1.begin(); ee != equates1.end(); ee++) std::cerr << ee->first << " = " << ee->second << ", "; std::cerr << "\n";
std::cerr << "equates2: "; for (ee = equates2.begin(); ee != equates2.end(); ee++) std::cerr << ee->first << " = " << ee->second << ", "; std::cerr << "\n";}

    // Substitute the constants into the disjunctions
    LocationSet::iterator kk;
    for (kk = constants.begin(); kk != constants.end(); kk++) {
        Binary* k = (Binary*)*kk;
        Exp* from = k->getSubExp1();
        Exp* to = k->getSubExp2();
        bool ch;
        std::list<Exp*>::iterator dd;
        for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++)
            (*dd)->searchReplaceAll(from, to, ch);
    }

    // Substitute the constants into the equates. This may generate more
    // constants
    LocationSet extra;
    LocationSet in = constants;
    while (in.size()) {
        extra.clear();
        for (kk = in.begin(); kk != in.end(); kk++) {
            Binary* k = (Binary*)*kk;
            Exp* lhs = k->getSubExp1();
            std::map<Exp*, Exp*, lessExpStar>::iterator it = equates1.find(lhs);
            if (it != equates1.end()) {
                // A possibly new constraint that it->second == val
                Exp* rhs = k->getSubExp2();
                Binary* c = new Binary(opEquals,
                    it->second,
                    rhs);
                if (!constants.find(c))
                    extra.insert(c);            // A new constant constraint
                // Remove the equate, and its inverse in equate2
                equates1.erase(it);
                equates2.erase(rhs);
            }
            it = equates2.find(lhs);
            if (it != equates2.end()) {
                // A possibly new constraint that it->second == val
                Exp* rhs = k->getSubExp2();
                Binary* c = new Binary(opEquals,
                    it->second,
                    rhs);
                if (!constants.find(c))
                    extra.insert(c);            // A new constant constraint
                // Remove the equate, and its inverse in equate1
                equates2.erase(it);
                equates1.erase(rhs);
            }
        }
        constants.makeUnion(in);
        in = extra;
    }
{std::cerr << "constants: "; LocationSet::iterator kk; for (kk = constants.begin(); kk != constants.end(); kk++) std::cerr << *kk << ", "; std::cerr << "\n";}
{std::cerr << "equates1: "; std::map<Exp*, Exp*, lessExpStar>::iterator ee; for (ee = equates1.begin(); ee != equates1.end(); ee++) std::cerr << ee->first << " = " << ee->second << ", "; std::cerr << "\n";
std::cerr << "equates2: "; for (ee = equates2.begin(); ee != equates2.end(); ee++) std::cerr << ee->first << " = " << ee->second << ", "; std::cerr << "\n";
}

    LocationSet soln;
    doSolve(disjunctions.begin(), soln, solns);
}

static int level = 0;
// Constraints up to but not including it have been unified.
bool Constraints::doSolve(std::list<Exp*>::iterator it, LocationSet& soln,
  std::list<LocationSet>& solns) {
std::cerr << "Begin doSolve at level " << ++level << "\n";
    if (it == disjunctions.end()) {
        // We have gotten to the end with no unification failures
        // Copy the current set of constraints as a solution
        if (soln.size() == 0)
            // Awkward. There is a trivial solution, but we have no constraints
            // So make a constraint of always-true
            soln.insert(new Terminal(opTrue));
        solns.push_back(soln);
std::cerr << "Exiting doSolve at level " << level-- << " returning true\n";
        return true;
    }

    Exp* dj = *it;
    // Iterate through each disjunction d of dj
    Exp* rem = dj;       // Remainder
    bool anyUnified = false;
    Exp* d;
    while ((d = nextDisjunct(rem)) != NULL) {
std::cerr << " $$ d is " << d << ", rem is " << ((rem==0)?"NULL":rem->prints()) << " $$\n";
        // Match disjunct d with every constant; it could be compatible,
        // compatible and generate an additional constraint, or be
        // incompatible
        LocationSet extra;      // Just for this disjunct
        extra.insert(d);
        bool unified;
        
        LocationSet::iterator kk;
        for (kk = constants.begin(); kk != constants.end(); kk++) {
std::cerr << "Unifying ``" << *kk << "'' with ``" << d << "''\n";
            unified = unify(*kk, d, extra);
std::cerr << "Unification returned " << unified << "; extra now " << extra.prints() << "\n";
            if (unified)
                anyUnified = true;
            else
                // This disjunct is not usable
                break;
        }
        if (!unified) continue;
        // Use this disjunct
        // We can't just difference out extra if this fails; it may remove
        // elements from soln that should not be removed
        // So need a copy of the old set in oldSoln
        LocationSet oldSoln = soln;
        soln.makeUnion(extra);
        if (!doSolve(++it, soln, solns)) {
            // This disjunct did not work out. Revert to the previous soln
            soln = oldSoln;
        }
        // Back to the current disjunction
        it--;
        // Continue for more disjuncts this disjunction
    }
    // We have run out of disjuncts. Return true if any disjuncts had no
    // unification failures
std::cerr << "Exiting doSolve at level " << level-- << " returning " << anyUnified << "\n";
    return anyUnified;
}

bool Constraints::unify(Exp* x, Exp* y, LocationSet& extra) {
    // Y could be a conjunction. If so, unify all conjunts
    Exp* rem = y;
    Exp* term;
    LocationSet extras;
    while ((term = nextConjunct(rem)) != NULL) {
        if (!unifyTerm(x, term, extras))
            return false;
    }
    extra.makeUnion(extras);
    return true;
}

bool Constraints::unifyTerm(Exp* x, Exp* y, LocationSet& extra) {
std::cerr << "Unifying ``" << x << "'' with term ``" << y << "''\n";
    assert(x->getOper() == opEquals);
    assert(y->getOper() == opEquals);
    Exp* xleft = ((Binary*)x)->getSubExp1();
    Exp* yleft = ((Binary*)y)->getSubExp1();
    if (*xleft == *yleft) {
        Exp* xright = ((Binary*)x)->getSubExp2();
        Exp* yright = ((Binary*)y)->getSubExp2();
        assert(xright->isTypeVal());
        assert(yright->isTypeVal());
        Type* xtype = ((TypeVal*)xright)->getType();
        Type* ytype = ((TypeVal*)yright)->getType();
        if (xtype->isPointer() && ytype->isPointer()) {
            Type* xPointsTo = ((PointerType*)xtype)->getPointsTo();
            Type* yPointsTo = ((PointerType*)ytype)->getPointsTo();
            if (xPointsTo->isNamed()) {
                if (yPointsTo->isNamed()) {
                    extra.insert(new Binary(opEquals,
                        new Unary(opTypeOf,
                            new TypeVal(xPointsTo)),
                        new Unary(opTypeOf,
                            new TypeVal(xPointsTo))));
                    return true;
                }
                extra.insert(new Binary(opEquals,
                    new Unary(opTypeOf,
                        new TypeVal(xPointsTo)),
                    new TypeVal(yPointsTo)));
                return true;
            }
            if (yPointsTo->isNamed()) {
                extra.insert(new Binary(opEquals,
                    new Unary(opTypeOf,
                        new TypeVal(yPointsTo)),
                    new TypeVal(xPointsTo)));
                return true;
            }
            return *xPointsTo == *yPointsTo;
        }
        return *xtype == *ytype;
    }
    return true;
}
