#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "LocationSet.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/StatementSet.h"

#include <QTextStream>


QTextStream& operator<<(QTextStream& os, const LocationSet *ls)
{
    ls->print(os);
    return os;
}


LocationSet& LocationSet::operator=(const LocationSet& o)
{
    lset.clear();

    for (ExpSet::const_iterator it = o.lset.begin(); it != o.lset.end(); ++it) {
        lset.insert((*it)->clone());
    }

    return *this;
}


LocationSet::LocationSet(const LocationSet& o)
{
    ExpSet::const_iterator it;

    for (it = o.lset.begin(); it != o.lset.end(); ++it) {
        lset.insert((*it)->clone());
    }
}


char *LocationSet::prints() const
{
    QString     tgt;
    QTextStream ost(&tgt);

    for (ExpSet::iterator it = lset.begin(); it != lset.end(); ++it) {
        if (it != lset.begin()) {
            ost << ", ";
        }

        ost << *it;
    }

    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void LocationSet::dump() const
{
    QTextStream ost(stderr);
    print(ost);
}


void LocationSet::print(QTextStream& os) const
{
    for (auto it = lset.begin(); it != lset.end(); ++it) {
        if (it != lset.begin()) {
            os << ",\t";
        }

        os << *it;
    }
}


void LocationSet::remove(SharedExp given)
{
    ExpSet::iterator it = lset.find(given);

    if (it == lset.end()) {
        return;
    }

    // NOTE: if the below uncommented, things go crazy. Valgrind says that
    // the deleted value gets used next in LocationSet::operator== ?!
    // delete *it;          // These expressions were cloned when created
    lset.erase(it);
}


void LocationSet::makeUnion(const LocationSet& other)
{
    for (const SharedExp& exp : other) {
        lset.insert(exp);
    }
}


void LocationSet::makeDiff(const LocationSet& other)
{
    for (const SharedExp& exp : other) {
        lset.erase(exp);
    }
}


bool LocationSet::operator==(const LocationSet& o) const
{
    // We want to compare the locations, not the pointers
    if (size() != o.size()) {
        return false;
    }

    return std::equal(lset.begin(), lset.end(), o.lset.begin(),
        [](const SharedConstExp& e1, const SharedConstExp& e2) {
            return *e1 == *e2;
        });
}


bool LocationSet::contains(SharedConstExp e) const
{
    return lset.find(std::const_pointer_cast<Exp>(e)) != lset.end();
}


SharedExp LocationSet::findNS(SharedExp e)
{
    if (e == nullptr) {
        return nullptr;
    }

    // Note: can't search with a wildcard, since it doesn't have the weak ordering required (I think)
    auto ref = RefExp::get(e, nullptr);

    // Note: the below assumes that nullptr is less than any other pointer
    iterator it = lset.lower_bound(ref);

    if (it == lset.end()) {
        return nullptr;
    }

    if ((*(*it)->getSubExp1() == *e)) {
        return *it;
    }
    else {
        return nullptr;
    }
}


bool LocationSet::existsImplicit(SharedExp e) const
{
    if (e == nullptr) {
        return false;
    }

    auto     r(RefExp::get(e, nullptr));
    iterator it = lset.lower_bound(r); // First element >= r

    // Note: the below relies on the fact that nullptr is less than any other pointer. Try later entries in the set:
    while (it != lset.end()) {
        if (!(*it)->isSubscript()) {
            return false;                            // Looking for e{something} (could be e.g. %pc)
        }

        if (!(*(*it)->getSubExp1() == *e)) { // Gone past e{anything}?
            return false;                    // Yes, then e{-} or e{0} cannot exist
        }

        if ((*it)->access<RefExp>()->isImplicitDef()) { // Check for e{-} or e{0}
            return true;                                // Found
        }

        ++it;                                           // Else check next entry
    }

    return false;
}


bool LocationSet::findDifferentRef(const std::shared_ptr<RefExp>& e, SharedExp& dr)
{
    if (!e) {
        return false;
    }

    auto             search = RefExp::get(e->getSubExp1()->clone(), STMT_WILD);
    ExpSet::iterator pos    = lset.find(search);

    if (pos == lset.end()) {
        return false;
    }

    while (pos != lset.end()) {
        assert(*pos);

        // Exit if we've gone to a new base expression
        // E.g. searching for r13{10} and **pos is r14{0}
        // Note: we want a ref-sensitive compare, but with the outer refs stripped off
        // For example: m[r29{10} - 16]{any} is different from m[r29{20} - 16]{any}
        if (!(*(*pos)->getSubExp1() == *e->getSubExp1())) {
            break;
        }

        // Bases are the same; return true if only different ref
        if (!(**pos == *e)) {
            dr = *pos;
            return true;
        }

        ++pos;
    }

    return false;
}


void LocationSet::addSubscript(Statement *d)
{
    ExpSet newSet;

    for (SharedExp it : lset) {
        newSet.insert(it->expSubscriptVar(it, d));
    }

    // Note: don't delete the old exps; they are copied in the new set
    lset = newSet;
}


void LocationSet::substitute(Assign& a)
{
    SharedExp lhs = a.getLeft();
    SharedExp rhs = a.getRight();

    if (!lhs || !rhs) {
        return;
    }

    // Note: it's important not to change the pointer in the set of pointers to expressions, without removing and
    // inserting again. Otherwise, the set becomes out of order, and operations such as set comparison fail!
    // To avoid any funny behaviour when iterating the loop, we use the following two sets
    LocationSet removeSet;       // These will be removed after the loop
    LocationSet removeAndDelete; // These will be removed then deleted
    LocationSet insertSet;       // These will be inserted after the loop
    bool        change;

    for (SharedExp loc : lset) {
        SharedExp replace;
        if (!loc->search(*lhs, replace)) {
            continue;
        }

        if (rhs->isTerminal()) {
            // This is no longer a location of interest (e.g. %pc)
            removeSet.insert(loc);
            continue;
        }

        loc = loc->clone()->searchReplaceAll(*lhs, rhs, change);

        if (change) {
            loc = loc->simplifyArith()->simplify();

            // If the result is no longer a register or memory (e.g.
            // r[28]-4), then delete this expression and insert any
            // components it uses (in the example, just r[28])
            if (!loc->isRegOf() && !loc->isMemOf()) {
                // Note: can't delete the expression yet, because the
                // act of insertion into the remove set requires silent
                // calls to the compare function
                removeAndDelete.insert(loc);
                loc->addUsedLocs(insertSet);
                continue;
            }

            // Else we just want to replace it
            // Regardless of whether the top level expression pointer has
            // changed, remove and insert it from the set of pointers
            removeSet.insert(loc); // Note: remove the unmodified ptr
            insertSet.insert(loc);
        }
    }

    makeDiff(removeSet);       // Remove the items to be removed
    makeDiff(removeAndDelete); // These are to be removed as well
    makeUnion(insertSet);      // Insert the items to be added
    // Now delete the expressions that are no longer needed
//    std::set<SharedExp , lessExpStar>::iterator dd;
//    for (dd = removeAndDelete.lset.begin(); dd != removeAndDelete.lset.end(); ++dd)
//        delete *dd; // Plug that memory leak
}



void LocationSet::printDiff(LocationSet *o) const
{
    bool printed2not1 = false;

    for (const SharedExp& oe : o->lset) {
        if (lset.find(oe) == lset.end()) {
            if (!printed2not1) {
                printed2not1 = true;
                LOG_MSG("In set 2 but not set 1:");
            }

            LOG_MSG("  %1", oe);
        }
    }

    bool printed1not2 = false;

    for (const SharedExp& e : lset) {
        if (o->lset.find(e) == o->lset.end()) {
            if (!printed1not2) {
                printed1not2 = true;
                LOG_MSG("In set 1 but not set 2:");
            }

            LOG_MSG("  %1", e);
        }
    }
}

