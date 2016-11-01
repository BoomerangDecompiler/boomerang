/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file       constraint.cpp
  * \brief   Implementation of objects related to type constraints
  ******************************************************************************/

#include "constraint.h"
#include "managed.h"
#include "exp.h"
#include "boomerang.h"
#include "log.h"
#include "proc.h"
#include "prog.h"
#include "type.h"
#include <sstream>
#include <cstring>

void ConstraintMap::print(QTextStream &os) {
    iterator kk;
    bool first = true;
    for (kk = cmap.begin(); kk != cmap.end(); kk++) {
        if (first)
            first = false;
        else
            os << ", ";
        os << kk->first << " = " << kk->second;
    }
    os << "\n";
}

extern char debug_buffer[];
char *ConstraintMap::prints() {
    QString tgt;
    QTextStream ost(&tgt);
    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}

void ConstraintMap::makeUnion(ConstraintMap &o) {
    std::map<SharedExp, SharedExp, lessExpStar>::iterator it;
    std::pair<std::map<SharedExp, SharedExp, lessExpStar>::iterator, bool> ret;
    for (it = o.cmap.begin(); it != o.cmap.end(); it++) {
        // Note: *it is a std::pair<Exp*, Exp*>
        ret = cmap.insert(*it);
        // If an insertion occured, ret will be std::pair<where, true>
        // If no insertion occured, ret will be std::pair<where, false>
        if (ret.second == false) {
            // LOG_STREAM() << "ConstraintMap::makeUnion: want to overwrite " << ret.first->first
            // << " -> " << ret.first->second << " with " << it->first << " -> " << it->second << "\n";
            auto Tret = std::static_pointer_cast<TypeVal>(ret.first->second);
            SharedType ty1 = Tret->getType();
            auto Toth = it->second->subExp<TypeVal>();
            SharedType ty2 = Toth->getType();
            if (ty1 && ty2 && *ty1 != *ty2) {
                Tret->setType(ty1->mergeWith(ty2));
                // LOG_STREAM() << "Now " << ret.first->first << " -> " << ret.first->second << "\n";
            }
        }
    }
}

void ConstraintMap::constrain(SharedExp loc1, SharedExp loc2) { cmap[Unary::get(opTypeOf, loc1)] = Unary::get(opTypeOf, loc2); }
//! Insert a constraint given a location and a Type
void ConstraintMap::constrain(SharedExp loc, SharedType t) { cmap[Unary::get(opTypeOf, loc)] = std::make_shared<TypeVal>(t); }
//! Insert a constraint given two Types (at least one variable)
void ConstraintMap::constrain(SharedType t1, SharedType t2) { // Example: alpha1 = alpha2
    cmap[std::make_shared<TypeVal>(t1)] = std::make_shared<TypeVal>(t2);
}

//! Insert a constraint given two locations (i.e. Tloc1 = Tloc2)
void ConstraintMap::insert(SharedExp term) {
    assert(term->isEquality());
    SharedExp lhs = term->getSubExp1();
    SharedExp rhs = term->getSubExp2();
    cmap[lhs] = rhs;
}

void EquateMap::print(QTextStream &os) {
    iterator ee;
    for (ee = emap.begin(); ee != emap.end(); ee++) {
        os << "     " << ee->first << " = " << ee->second.prints();
    }
    os << "\n";
}

char *EquateMap::prints() {
    QString tgt;
    QTextStream ost(&tgt);
    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}

// Substitute the given constraints into this map
void ConstraintMap::substitute(ConstraintMap &other) {
    std::map<SharedExp, SharedExp, lessExpStar>::iterator oo, cc;
    for (oo = other.cmap.begin(); oo != other.cmap.end(); oo++) {
        bool ch;
        for (cc = cmap.begin(); cc != cmap.end(); cc++) {
            SharedExp newVal = cc->second->searchReplaceAll(*oo->first, oo->second, ch);
            if (ch) {
                if (*cc->first == *newVal)
                    // e.g. was <char*> = <alpha6> now <char*> = <char*>
                    cmap.erase(cc);
                else
                    cmap[cc->first] = newVal;
            } else
                // The existing value
                newVal = cc->second;
            SharedExp newKey = cc->first->searchReplaceAll(*oo->first, oo->second, ch);
            if (ch) {
                cmap.erase(cc->first);
                // Often end up with <char*> = <char*>
                if (!(*newKey == *newVal))
                    cmap[newKey] = newVal;
            }
        }
    }
}

void ConstraintMap::substAlpha() {
    ConstraintMap alphaDefs;
    std::map<SharedExp, SharedExp, lessExpStar>::iterator cc;
    for (cc = cmap.begin(); cc != cmap.end(); cc++) {
        // Looking for entries with two TypeVals, where exactly one is an alpha
        if (!cc->first->isTypeVal() || !cc->second->isTypeVal())
            continue;
        SharedType t1 = cc->first->subExp<TypeVal>()->getType();
        SharedType t2 = cc->second->subExp<TypeVal>()->getType();
        int numAlpha = 0;
        if (t1->isPointerToAlpha())
            numAlpha++;
        if (t2->isPointerToAlpha())
            numAlpha++;
        if (numAlpha != 1)
            continue;
        // This is such an equality. Copy it to alphaDefs
        if (t1->isPointerToAlpha())
            alphaDefs.cmap[cc->first] = cc->second;
        else
            alphaDefs.cmap[cc->second] = cc->first;
    }

    // Remove these from the solution
    for (cc = alphaDefs.begin(); cc != alphaDefs.end(); cc++)
        cmap.erase(cc->first);

    // Now substitute into the remainder
    substitute(alphaDefs);
}

Constraints::~Constraints() {
//    LocationSet::iterator cc;
//    for (cc = conSet.begin(); cc != conSet.end(); cc++) {
//        delete *cc;
//    }
}

void Constraints::substIntoDisjuncts(ConstraintMap &in) {
    ConstraintMap::iterator kk;
    for (kk = in.begin(); kk != in.end(); kk++) {
        SharedExp from = kk->first;
        SharedExp to = kk->second;
        bool ch;
        std::list<SharedExp>::iterator dd;
        for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++) {
            (*dd)->searchReplaceAll(*from, to, ch);
            *dd = (*dd)->simplifyConstraint();
        }
    }
    // Now do alpha substitution
    alphaSubst();
}

void Constraints::substIntoEquates(ConstraintMap &in) {
    // Substitute the fixed types into the equates. This may generate more
    // fixed types
    ConstraintMap extra;
    ConstraintMap cur = in;
    while (cur.size()) {
        extra.clear();
        ConstraintMap::iterator kk;
        for (kk = cur.begin(); kk != cur.end(); kk++) {
            SharedExp lhs = kk->first;
            std::map<SharedExp, LocationSet, lessExpStar>::iterator it = equates.find(lhs);
            if (it != equates.end()) {
                // Possibly new constraints that
                // typeof(elements in it->second) == val
                SharedExp val = kk->second;
                LocationSet &ls = it->second;
                LocationSet::iterator ll;
                for (ll = ls.begin(); ll != ls.end(); ll++) {
                    ConstraintMap::iterator ff;
                    ff = fixed.find(*ll);
                    if (ff != fixed.end()) {
                        if (!unify(val, ff->second, extra)) {
                            LOG_VERBOSE(DEBUG_TA) << "Constraint failure: " << *ll << " constrained to be "
                                                  << val->subExp<TypeVal>()->getType()->getCtype() << " and "
                                                  << ff->second->subExp<TypeVal>()->getType()->getCtype() << "\n";
                            return;
                        }
                    } else
                        extra[*ll] = val; // A new constant constraint
                }
                if (val->subExp<TypeVal>()->getType()->isComplete()) {
                    // We have a complete type equal to one or more variables
                    // Remove the equate, and generate more fixed
                    // e.g. Ta = Tb,Tc and Ta = K => Tb=K, Tc=K
                    for (ll = ls.begin(); ll != ls.end(); ll++) {
                        SharedExp newFixed = Binary::get(opEquals,
                                                    *ll,  // e.g. Tb
                                                    val); // e.g. K
                        extra.insert(newFixed);
                    }
                    equates.erase(it);
                }
            }
        }
        fixed.makeUnion(extra);
        cur = extra; // Take care of any "ripple effect"
    }                // Repeat until no ripples
}

// Get the next disjunct from this disjunction
// Assumes that the remainder is of the for a or (b or c), or (a or b) or c
// But NOT (a or b) or (c or d)
// Could also be just a (a conjunction, or a single constraint)
// Note: remainder is changed by this function
SharedExp nextDisjunct(SharedExp&remainder) {
    if (remainder == nullptr)
        return nullptr;
    if (remainder->isDisjunction()) {
        SharedExp d1 = remainder->getSubExp1();
        SharedExp d2 = remainder->getSubExp2();
        if (d1->isDisjunction()) {
            remainder = d1;
            return d2;
        }
        remainder = d2;
        return d1;
    }
    // Else, we have one disjunct. Return it
    SharedExp ret = remainder;
    remainder = nullptr;
    return ret;
}

SharedExp nextConjunct(SharedExp&remainder) {
    if (remainder == nullptr)
        return nullptr;
    if (remainder->isConjunction()) {
        SharedExp c1 = remainder->getSubExp1();
        SharedExp c2 = remainder->getSubExp2();
        if (c1->isConjunction()) {
            remainder = c1;
            return c2;
        }
        remainder = c2;
        return c1;
    }
    // Else, we have one conjunct. Return it
    SharedExp ret = remainder;
    remainder = nullptr;
    return ret;
}

bool Constraints::solve(std::list<ConstraintMap> &solns) {
    LOG << conSet.size() << " constraints:";
    QString tgt_s;
    QTextStream os(&tgt_s);
    conSet.print(os);
    LOG << tgt_s;
    // Replace Ta[loc] = ptr(alpha) with
    //           Tloc = alpha
    LocationSet::iterator cc;
    for (cc = conSet.begin(); cc != conSet.end(); cc++) {
        SharedExp c = *cc;
        if (!c->isEquality())
            continue;
        SharedExp left = c->getSubExp1();
        if (!left->isTypeOf())
            continue;
        SharedExp leftSub = left->getSubExp1();
        if (!leftSub->isAddrOf())
            continue;
        SharedExp right = c->getSubExp2();
        if (!right->isTypeVal())
            continue;
        SharedType t = right->subExp<TypeVal>()->getType();
        if (!t->isPointer())
            continue;
        // Don't modify a key in a map
        SharedExp clone = c->clone();
        // left is typeof(addressof(something)) -> typeof(something)
        left = clone->getSubExp1();
        leftSub = left->getSubExp1();
        SharedExp something = leftSub->getSubExp1();
        left->setSubExp1(something);
        leftSub->setSubExp1(nullptr);
        leftSub = nullptr;
        // right is <alpha*> -> <alpha>
        right = clone->getSubExp2();
        t = right->subExp<TypeVal>()->getType();
        right->subExp<TypeVal>()->setType(t->as<PointerType>()->getPointsTo()->clone());
        conSet.remove(c);
        conSet.insert(clone);
    }

    // Sort constraints into a few categories. Disjunctions go to a special
    // list, always true is just ignored, and constraints of the form
    // typeof(x) = y (where y is a type value) go to a map called fixed.
    // Constraint terms of the form Tx = Ty go into a map of LocationSets
    // called equates for fast lookup
    for (cc = conSet.begin(); cc != conSet.end(); cc++) {
        SharedExp c = *cc;
        if (c->isTrue())
            continue;
        if (c->isFalse()) {
            LOG_VERBOSE(DEBUG_TA) << "Constraint failure: always false constraint\n";
            return false;
        }
        if (c->isDisjunction()) {
            disjunctions.push_back(c);
            continue;
        }
        // Break up conjunctions into terms
        SharedExp rem = c, term;
        while ((term = nextConjunct(rem)) != nullptr) {
            assert(term->isEquality());
            SharedExp lhs = term->getSubExp1();
            SharedExp rhs = term->getSubExp2();
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

    {
        LOG << "\n" << disjunctions.size() << " disjunctions: ";
        std::list<SharedExp>::iterator dd;
        for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++)
            LOG << *dd << ",\n";
        LOG << "\n";
    }
    LOG << fixed.size() << " fixed: " << fixed.prints();
    LOG << equates.size() << " equates: " << equates.prints();

    // Substitute the fixed types into the disjunctions
    substIntoDisjuncts(fixed);

    // Substitute the fixed types into the equates. This may generate more
    // fixed types
    substIntoEquates(fixed);

    LOG << "\nAfter substitute fixed into equates:\n";
    {
        LOG << "\n" << disjunctions.size() << " disjunctions: ";
        std::list<SharedExp>::iterator dd;
        for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++)
            LOG << *dd << ",\n";
        LOG << "\n";
    }
    LOG << fixed.size() << " fixed: " << fixed.prints();
    LOG << equates.size() << " equates: " << equates.prints();
    // Substitute again the fixed types into the disjunctions
    // (since there may be more fixed types from the above)
    substIntoDisjuncts(fixed);

    LOG << "\nAfter second substitute fixed into disjunctions:\n";
    {
        LOG << "\n" << disjunctions.size() << " disjunctions: ";
        std::list<SharedExp>::iterator dd;
        for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++)
            LOG << *dd << ",\n";
        LOG << "\n";
    }
    LOG << fixed.size() << " fixed: " << fixed.prints();
    LOG << equates.size() << " equates: " << equates.prints();

    ConstraintMap soln;
    bool ret = doSolve(disjunctions.begin(), soln, solns);
    if (ret) {
        // For each solution, we need to find disjunctions of the form
        // <alphaN> = <type>      or
        // <type>    = <alphaN>
        // and substitute these into each part of the solution
        std::list<ConstraintMap>::iterator it;
        for (it = solns.begin(); it != solns.end(); it++)
            it->substAlpha();
    }
    return ret;
}

static int level = 0;
// Constraints up to but not including iterator it have been unified.
// The current solution is soln
// The set of all solutions is in solns
bool Constraints::doSolve(std::list<SharedExp>::iterator it, ConstraintMap &soln, std::list<ConstraintMap> &solns) {
    LOG << "Begin doSolve at level " << ++level << "\n";
    LOG << "Soln now: " << soln.prints() << "\n";
    if (it == disjunctions.end()) {
        // We have gotten to the end with no unification failures
        // Copy the current set of constraints as a solution
        // if (soln.size() == 0)
        // Awkward. There is a trivial solution, but we have no constraints
        // So make a constraint of always-true
        // soln.insert(Terminal::get(opTrue));
        // Copy the fixed constraints
        soln.makeUnion(fixed);
        solns.push_back(soln);
        LOG << "Exiting doSolve at level " << level-- << " returning true\n";
        return true;
    }

    SharedExp dj = *it;
    // Iterate through each disjunction d of dj
    SharedExp rem1 = dj; // Remainder
    bool anyUnified = false;
    SharedExp d;
    while ((d = nextDisjunct(rem1)) != nullptr) {
        LOG << " $$ d is " << d << ", rem1 is " << ((rem1 == nullptr) ? "NULL" : rem1->prints()) << " $$\n";
        // Match disjunct d against the fixed types; it could be compatible,
        // compatible and generate an additional constraint, or be
        // incompatible
        ConstraintMap extra; // Just for this disjunct
        SharedExp c;
        SharedExp rem2 = d;
        bool unified = true;
        while ((c = nextConjunct(rem2)) != nullptr) {
            LOG << "   $$ c is " << c << ", rem2 is " << ((rem2 == nullptr) ? "NULL" : rem2->prints()) << " $$\n";
            if (c->isFalse()) {
                unified = false;
                break;
            }
            assert(c->isEquality());
            SharedExp lhs = c->getSubExp1();
            SharedExp rhs = c->getSubExp2();
            extra.insert(lhs, rhs);
            ConstraintMap::iterator kk;
            kk = fixed.find(lhs);
            if (kk != fixed.end()) {
                unified &= unify(rhs, kk->second, extra);
                LOG << "Unified now " << unified << "; extra now " << extra.prints() << "\n";
                if (!unified)
                    break;
            }
        }
        if (unified)
            // True if any disjuncts had all the conjuncts satisfied
            anyUnified = true;
        if (!unified)
            continue;
        // Use this disjunct
        // We can't just difference out extra if this fails; it may remove
        // elements from soln that should not be removed
        // So need a copy of the old set in oldSoln
        ConstraintMap oldSoln = soln;
        soln.makeUnion(extra);
        doSolve(++it, soln, solns);
        // Revert to the previous soln (whether doSolve returned true or not)
        // If this recursion did any good, it will have gotten to the end and
        // added the resultant soln to solns
        soln = oldSoln;
        LOG << "After doSolve returned: soln back to: " << soln.prints() << "\n";
        // Back to the current disjunction
        it--;
        // Continue for more disjuncts this disjunction
    }
    // We have run out of disjuncts. Return true if any disjuncts had no
    // unification failures
    LOG << "Exiting doSolve at level " << level-- << " returning " << anyUnified << "\n";
    return anyUnified;
}

bool Constraints::unify(SharedExp x, SharedExp y, ConstraintMap &extra) {
    LOG << "Unifying " << x << " with " << y << " result ";
    assert(x->isTypeVal());
    assert(y->isTypeVal());
    SharedType xtype = x->subExp<TypeVal>()->getType();
    SharedType ytype = y->subExp<TypeVal>()->getType();
    if (xtype->isPointer() && ytype->isPointer()) {
        auto xPointsTo = xtype->as<PointerType>()->getPointsTo();
        auto yPointsTo = ytype->as<PointerType>()->getPointsTo();
        if (xtype->as<PointerType>()->pointsToAlpha() || ytype->as<PointerType>()->pointsToAlpha()) {
            // A new constraint: xtype must be equal to ytype; at least
            // one of these is a variable type
            if (xtype->as<PointerType>()->pointsToAlpha())
                extra.constrain(xPointsTo, yPointsTo);
            else
                extra.constrain(yPointsTo, xPointsTo);
            LOG << "true\n";
            return true;
        }
        LOG << (*xPointsTo == *yPointsTo) << "\n";
        return *xPointsTo == *yPointsTo;
    } else if (xtype->isSize()) {
        if (ytype->getSize() == 0) { // Assume size=0 means unknown
            LOG << "true\n";
            return true;
        } else {
            LOG << (xtype->getSize() == ytype->getSize()) << "\n";
            return xtype->getSize() == ytype->getSize();
        }
    } else if (ytype->isSize()) {
        if (xtype->getSize() == 0) { // Assume size=0 means unknown
            LOG << "true\n";
            return true;
        } else {
            LOG << (xtype->getSize() == ytype->getSize()) << "\n";
            return xtype->getSize() == ytype->getSize();
        }
    }
    // Otherwise, just compare the sizes
    LOG << (*xtype == *ytype) << "\n";
    return *xtype == *ytype;
}
/**
 Perform "alpha substitution". Example:
     <fixedtype*> = alphaX* and T[Y] = alphaX*    ->
     T[Y] = <fixedtype*>
*/
void Constraints::alphaSubst() {
    std::list<SharedExp>::iterator it;
    for (it = disjunctions.begin(); it != disjunctions.end(); it++) {
        // This should be a conjuction of terms
        if (!(*it)->isConjunction())
            // A single term will do no good...
            continue;
        // Look for a term like alphaX* == fixedType*
        SharedExp temp = (*it)->clone();
        SharedExp term;
        bool found = false;
        SharedExp trm1 = nullptr;
        SharedExp trm2 = nullptr;
        SharedType t1 = nullptr, t2;
        while ((term = nextConjunct(temp)) != nullptr) {
            if (!term->isEquality())
                continue;
            trm1 = term->getSubExp1();
            if (!trm1->isTypeVal())
                continue;
            trm2 = term->getSubExp2();
            if (!trm2->isTypeVal())
                continue;
            // One of them has to be a pointer to an alpha
            t1 = trm1->subExp<TypeVal>()->getType();
            if (t1->isPointerToAlpha()) {
                found = true;
                break;
            }
            t2 = trm2->subExp<TypeVal>()->getType();
            if (t2->isPointerToAlpha()) {
                found = true;
                break;
            }
        }
        if (!found)
            continue;
        // We have a alpha value; get the value
        SharedExp val, alpha;
        if (t1->isPointerToAlpha()) {
            alpha = trm1;
            val = trm2;
        } else {
            val = trm1;
            alpha = trm2;
        }
        assert(alpha);
        // Now substitute
        bool change;
        *it = (*it)->searchReplaceAll(*alpha, val, change);
        *it = (*it)->simplifyConstraint();
    }
}

void Constraints::print(QTextStream &os) {
    os << "\n" << (int)disjunctions.size() << " disjunctions: ";
    std::list<SharedExp>::iterator dd;
    for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++)
        os << *dd << ",\n";
    os << "\n";
    os << (int)fixed.size() << " fixed: ";
    fixed.print(os);
    os << (int)equates.size() << " equates: ";
    equates.print(os);
}

char *Constraints::prints() {
    QString tgt;
    QTextStream ost(&tgt);
    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}
