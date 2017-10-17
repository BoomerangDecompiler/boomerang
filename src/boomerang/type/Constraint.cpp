#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Constraint.h"


/***************************************************************************/ /**
 * \file   constraint.cpp
 * \brief  Implementation of objects related to type constraints
 ******************************************************************************/
#include "boomerang/core/Boomerang.h"

#include "boomerang/db/proc/Proc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/exp/TypeVal.h"
#include "boomerang/db/Managed.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/util/Log.h"

#include <sstream>
#include <cstring>


void ConstraintMap::print(QTextStream& os)
{
    iterator kk;
    bool     first = true;

    for (kk = cmap.begin(); kk != cmap.end(); kk++) {
        if (first) {
            first = false;
        }
        else {
            os << ", ";
        }

        os << kk->first << " = " << kk->second;
    }

    os << "\n";
}


char *ConstraintMap::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void ConstraintMap::makeUnion(ConstraintMap& o)
{
    std::pair<std::map<SharedExp, SharedExp, lessExpStar>::iterator, bool> ret;

    for (std::map<SharedExp, SharedExp, lessExpStar>::iterator it = o.cmap.begin(); it != o.cmap.end(); it++) {
        // Note: *it is a std::pair<Exp*, Exp*>
        ret = cmap.insert(*it);

        // If an insertion occured, ret will be std::pair<where, true>
        // If no insertion occured, ret will be std::pair<where, false>
        if (ret.second == false) {
            // LOG_STREAM() << "ConstraintMap::makeUnion: want to overwrite " << ret.first->first
            // << " -> " << ret.first->second << " with " << it->first << " -> " << it->second << "\n";
            auto       Tret = std::static_pointer_cast<TypeVal>(ret.first->second);
            SharedType ty1  = Tret->getType();
            auto       Toth = it->second->access<TypeVal>();
            SharedType ty2  = Toth->getType();

            if (ty1 && ty2 && (*ty1 != *ty2)) {
                Tret->setType(ty1->mergeWith(ty2));
                // LOG_STREAM() << "Now " << ret.first->first << " -> " << ret.first->second << "\n";
            }
        }
    }
}


void ConstraintMap::constrain(SharedExp loc1, SharedExp loc2)
{
    cmap[Unary::get(opTypeOf, loc1)] = Unary::get(opTypeOf, loc2);
}


void ConstraintMap::constrain(SharedExp loc, SharedType t)
{
    cmap[Unary::get(opTypeOf, loc)] = std::make_shared<TypeVal>(t);
}


void ConstraintMap::constrain(SharedType t1, SharedType t2)   // Example: alpha1 = alpha2
{
    cmap[std::make_shared<TypeVal>(t1)] = std::make_shared<TypeVal>(t2);
}


void ConstraintMap::insert(SharedExp term)
{
    assert(term->isEquality());
    SharedExp lhs = term->getSubExp1();
    SharedExp rhs = term->getSubExp2();
    cmap[lhs] = rhs;
}


void EquateMap::print(QTextStream& os)
{
    iterator ee;

    for (ee = emap.begin(); ee != emap.end(); ee++) {
        os << "     " << ee->first << " = " << ee->second.prints();
    }

    os << "\n";
}


char *EquateMap::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void ConstraintMap::substitute(ConstraintMap& other)
{
    std::map<SharedExp, SharedExp, lessExpStar>::iterator oo, cc;

    for (oo = other.cmap.begin(); oo != other.cmap.end(); oo++) {
        bool ch;

        for (cc = cmap.begin(); cc != cmap.end(); cc++) {
            SharedExp newVal = cc->second->searchReplaceAll(*oo->first, oo->second, ch);

            if (ch) {
                if (*cc->first == *newVal) {
                    // e.g. was <char*> = <alpha6> now <char*> = <char*>
                    cc = cmap.erase(cc);
                }
                else {
                    cmap[cc->first] = newVal;
                }
            }
            else {
                // The existing value
                newVal = cc->second;
            }

            SharedExp newKey = cc->first->searchReplaceAll(*oo->first, oo->second, ch);

            if (ch) {
                cmap.erase(cc->first);

                // Often end up with <char*> = <char*>
                if (!(*newKey == *newVal)) {
                    cmap[newKey] = newVal;
                }
            }
        }
    }
}


void ConstraintMap::substAlpha()
{
    ConstraintMap alphaDefs;

    std::map<SharedExp, SharedExp, lessExpStar>::iterator cc;

    for (cc = cmap.begin(); cc != cmap.end(); cc++) {
        // Looking for entries with two TypeVals, where exactly one is an alpha
        if (!cc->first->isTypeVal() || !cc->second->isTypeVal()) {
            continue;
        }

        SharedType t1       = cc->first->access<TypeVal>()->getType();
        SharedType t2       = cc->second->access<TypeVal>()->getType();
        int        numAlpha = 0;

        if (t1->isPointerToAlpha()) {
            numAlpha++;
        }

        if (t2->isPointerToAlpha()) {
            numAlpha++;
        }

        if (numAlpha != 1) {
            continue;
        }

        // This is such an equality. Copy it to alphaDefs
        if (t1->isPointerToAlpha()) {
            alphaDefs.cmap[cc->first] = cc->second;
        }
        else {
            alphaDefs.cmap[cc->second] = cc->first;
        }
    }

    // Remove these from the solution
    for (cc = alphaDefs.begin(); cc != alphaDefs.end(); cc++) {
        cmap.erase(cc->first);
    }

    // Now substitute into the remainder
    substitute(alphaDefs);
}


Constraints::~Constraints()
{
}


void Constraints::substIntoDisjuncts(ConstraintMap& in)
{
    ConstraintMap::iterator kk;

    for (kk = in.begin(); kk != in.end(); kk++) {
        SharedExp from = kk->first;
        SharedExp to   = kk->second;
        bool      ch;
        std::list<SharedExp>::iterator dd;

        for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++) {
            (*dd)->searchReplaceAll(*from, to, ch);
            *dd = (*dd)->simplifyConstraint();
        }
    }

    // Now do alpha substitution
    alphaSubst();
}


void Constraints::substIntoEquates(ConstraintMap& in)
{
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
                SharedExp             val = kk->second;
                LocationSet&          ls  = it->second;
                LocationSet::iterator ll;

                for (ll = ls.begin(); ll != ls.end(); ll++) {
                    ConstraintMap::iterator ff;
                    ff = fixed.find(*ll);

                    if (ff != fixed.end()) {
                        if (!unify(val, ff->second, extra)) {
                            if (DEBUG_TA) {
                                LOG_WARN("Constraint failure: %1 constrained to be %2 and %3", *ll,
                                         val->access<TypeVal>()->getType()->getCtype(),
                                         ff->second->access<TypeVal>()->getType()->getCtype());
                            }

                            return;
                        }
                    }
                    else {
                        extra[*ll] = val; // A new constant constraint
                    }
                }

                if (val->access<TypeVal>()->getType()->isComplete()) {
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
SharedExp nextDisjunct(SharedExp& remainder)
{
    if (remainder == nullptr) {
        return nullptr;
    }

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


SharedExp nextConjunct(SharedExp& remainder)
{
    if (remainder == nullptr) {
        return nullptr;
    }

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


bool Constraints::solve(std::list<ConstraintMap>& solns)
{
    QString     tgt_s;
    QTextStream os(&tgt_s);

    conSet.print(os);
    LOG_MSG("%1 constraints: %2", conSet.size(), tgt_s);

    // Replace Ta[loc] = ptr(alpha) with
    //           Tloc = alpha
    LocationSet::iterator cc;

    for (cc = conSet.begin(); cc != conSet.end(); cc++) {
        SharedExp c = *cc;

        if (!c->isEquality()) {
            continue;
        }

        SharedExp left = c->getSubExp1();

        if (!left->isTypeOf()) {
            continue;
        }

        SharedExp leftSub = left->getSubExp1();

        if (!leftSub->isAddrOf()) {
            continue;
        }

        SharedExp right = c->getSubExp2();

        if (!right->isTypeVal()) {
            continue;
        }

        SharedType t = right->access<TypeVal>()->getType();

        if (!t->isPointer()) {
            continue;
        }

        // Don't modify a key in a map
        SharedExp clone = c->clone();
        // left is typeof(addressof(something)) -> typeof(something)
        left    = clone->getSubExp1();
        leftSub = left->getSubExp1();
        SharedExp something = leftSub->getSubExp1();
        left->setSubExp1(something);
        leftSub->setSubExp1(nullptr);
        leftSub = nullptr;
        // right is <alpha*> -> <alpha>
        right = clone->getSubExp2();
        t     = right->access<TypeVal>()->getType();
        right->access<TypeVal>()->setType(t->as<PointerType>()->getPointsTo()->clone());
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

        if (c->isTrue()) {
            continue;
        }

        if (c->isFalse()) {
            if (DEBUG_TA) {
                LOG_WARN("Constraint failure: always false constraint");
            }

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
            }
            else {
                // Of the form typeof(x) = <typeval>
                // Insert into fixed
                assert(rhs->isTypeVal());
                fixed[lhs] = rhs;
            }
        }
    }

    {
        LOG_MSG("  %1 disjunctions:", disjunctions.size());

        for (std::list<SharedExp>::iterator dd = disjunctions.begin(); dd != disjunctions.end(); dd++) {
            LOG_MSG("    %1,", *dd);
        }
    }

    LOG_MSG("  %1 fixed:   %2", fixed.size(), fixed.prints());
    LOG_MSG("  %1 equates: %2", equates.size(), equates.prints());

    // Substitute the fixed types into the disjunctions
    substIntoDisjuncts(fixed);

    // Substitute the fixed types into the equates. This may generate more
    // fixed types
    substIntoEquates(fixed);

    LOG_MSG("After substitute fixed into equates:");
    {
        LOG_MSG("  %1 disjunctions:", disjunctions.size());
        std::list<SharedExp>::iterator dd;

        for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++) {
            LOG_MSG("    %1,", *dd);
        }
    }

    LOG_MSG("  %1 fixed:   %2", fixed.size(), fixed.prints());
    LOG_MSG("  %1 equates: %2", equates.size(), equates.prints());

    // Substitute again the fixed types into the disjunctions
    // (since there may be more fixed types from the above)
    substIntoDisjuncts(fixed);

    LOG_MSG("After second substitute fixed into disjunctions:");
    {
        LOG_MSG("  %1 disjunction:", disjunctions.size());
        std::list<SharedExp>::iterator dd;

        for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++) {
            LOG_MSG("    %1,", *dd);
        }
    }

    LOG_MSG("  %1 fixed:   %2", fixed.size(), fixed.prints());
    LOG_MSG("  %1 equates: %2", equates.size(), equates.prints());

    ConstraintMap soln;
    bool          ret = doSolve(disjunctions.begin(), soln, solns);

    if (ret) {
        // For each solution, we need to find disjunctions of the form
        // <alphaN> = <type>      or
        // <type>    = <alphaN>
        // and substitute these into each part of the solution

        for (auto& sol : solns) {
            sol.substAlpha();
        }
    }

    return ret;
}


static int level = 0;

bool Constraints::doSolve(std::list<SharedExp>::iterator it, ConstraintMap& soln, std::list<ConstraintMap>& solns)
{
    LOG_MSG("Begin doSolve at level %1", ++level);
    LOG_MSG("Soln now: %1", soln.prints());

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
        LOG_MSG("Exiting doSolve at level %1 returning true", level--);
        return true;
    }

    SharedExp dj = *it;
    // Iterate through each disjunction d of dj
    SharedExp rem1       = dj; // Remainder
    bool      anyUnified = false;
    SharedExp d;

    while ((d = nextDisjunct(rem1)) != nullptr) {
        LOG_MSG(" $$ d is %1, rem1 is %2 $$", d, ((rem1 == nullptr) ? "NULL" : rem1->prints()));

        // Match disjunct d against the fixed types; it could be compatible,
        // compatible and generate an additional constraint, or be
        // incompatible
        ConstraintMap extra; // Just for this disjunct
        SharedExp     c;
        SharedExp     rem2    = d;
        bool          unified = true;

        while ((c = nextConjunct(rem2)) != nullptr) {
            LOG_MSG("   $$ c is %1, rem2 is %2 $$", d, ((rem2 == nullptr) ? "NULL" : rem2->prints()));

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
                LOG_MSG("Unified now %1; extra now %2", unified, extra.prints());

                if (!unified) {
                    break;
                }
            }
        }

        if (unified) {
            // True if any disjuncts had all the conjuncts satisfied
            anyUnified = true;
        }

        if (!unified) {
            continue;
        }

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
        LOG_MSG("After doSolve returned: soln back to: %1", soln.prints());

        // Back to the current disjunction
        it--;
        // Continue for more disjuncts this disjunction
    }

    // We have run out of disjuncts. Return true if any disjuncts had no
    // unification failures
    LOG_MSG("Exiting doSolve at level %1 returning %2", level--, anyUnified);
    return anyUnified;
}


bool Constraints::unify(SharedExp x, SharedExp y, ConstraintMap& extra)
{
    assert(x->isTypeVal());
    assert(y->isTypeVal());


    SharedType xtype = x->access<TypeVal>()->getType();
    SharedType ytype = y->access<TypeVal>()->getType();

    bool unified = false;

    if (xtype->isPointer() && ytype->isPointer()) {
        auto xPointsTo = xtype->as<PointerType>()->getPointsTo();
        auto yPointsTo = ytype->as<PointerType>()->getPointsTo();

        if (xtype->as<PointerType>()->pointsToAlpha() || ytype->as<PointerType>()->pointsToAlpha()) {
            // A new constraint: xtype must be equal to ytype; at least
            // one of these is a variable type
            if (xtype->as<PointerType>()->pointsToAlpha()) {
                extra.constrain(xPointsTo, yPointsTo);
            }
            else {
                extra.constrain(yPointsTo, xPointsTo);
            }

            unified = true;
        }
        else {
            unified = (*xPointsTo == *yPointsTo);
        }
    }
    else if (xtype->getSize() || ytype->isSize()) {
        // Assume size=0 means unknown
        unified = xtype->getSize() == 0 || ytype->getSize() == 0 ||
                  xtype->getSize() == ytype->getSize();
    }
    else {
        // Otherwise, just compare the sizes
        unified = (*xtype == *ytype);
    }

    LOG_MSG("Unifying %1 and %2, result: %3", x, y, unified);
    return unified;
}


void Constraints::alphaSubst()
{
    std::list<SharedExp>::iterator it;

    for (it = disjunctions.begin(); it != disjunctions.end(); it++) {
        // This should be a conjuction of terms
        if (!(*it)->isConjunction()) {
            // A single term will do no good...
            continue;
        }

        // Look for a term like alphaX* == fixedType*
        SharedExp  temp = (*it)->clone();
        SharedExp  term;
        bool       found = false;
        SharedExp  trm1 = nullptr;
        SharedExp  trm2 = nullptr;
        SharedType t1 = nullptr, t2;

        while ((term = nextConjunct(temp)) != nullptr) {
            if (!term->isEquality()) {
                continue;
            }

            trm1 = term->getSubExp1();

            if (!trm1->isTypeVal()) {
                continue;
            }

            trm2 = term->getSubExp2();

            if (!trm2->isTypeVal()) {
                continue;
            }

            // One of them has to be a pointer to an alpha
            t1 = trm1->access<TypeVal>()->getType();

            if (t1->isPointerToAlpha()) {
                found = true;
                break;
            }

            t2 = trm2->access<TypeVal>()->getType();

            if (t2->isPointerToAlpha()) {
                found = true;
                break;
            }
        }

        if (!found) {
            continue;
        }

        // We have a alpha value; get the value
        SharedExp val, alpha;

        if (t1->isPointerToAlpha()) {
            alpha = trm1;
            val   = trm2;
        }
        else {
            val   = trm1;
            alpha = trm2;
        }

        assert(alpha);
        // Now substitute
        bool change;
        *it = (*it)->searchReplaceAll(*alpha, val, change);
        *it = (*it)->simplifyConstraint();
    }
}


void Constraints::print(QTextStream& os)
{
    os << "\n" << (int)disjunctions.size() << " disjunctions: ";
    std::list<SharedExp>::iterator dd;

    for (dd = disjunctions.begin(); dd != disjunctions.end(); dd++) {
        os << *dd << ",\n";
    }

    os << "\n";
    os << (int)fixed.size() << " fixed: ";
    fixed.print(os);
    os << (int)equates.size() << " equates: ";
    equates.print(os);
}


char *Constraints::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}
