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


LocationSet& LocationSet::operator=(const LocationSet& o)
{
    m_set.clear();

    for (const_iterator it = o.begin(); it != o.end(); ++it) {
        insert((*it)->clone());
    }

    return *this;
}


LocationSet::LocationSet(const LocationSet& o)
{
    for (auto it = o.begin(); it != o.end(); ++it) {
        insert((*it)->clone());
    }
}


char *LocationSet::prints() const
{
    QString     tgt;
    QTextStream ost(&tgt);

    for (const_iterator it = begin(); it != end(); ++it) {
        if (it != begin()) {
            ost << ", ";
        }

        ost << *it;
    }

    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


SharedExp LocationSet::findNS(SharedExp e)
{
    if (e == nullptr) {
        return nullptr;
    }

    // Note: can't search with a wildcard, since it doesn't have the weak ordering required (I think)
    auto ref = RefExp::get(e, nullptr);

    // Note: the below assumes that nullptr is less than any other pointer
    iterator it = m_set.lower_bound(ref);

    if (it == m_set.end()) {
        return nullptr;
    }

    if ((*(*it)->getSubExp1() == *e)) {
        return *it;
    }
    else {
        return nullptr;
    }
}


bool LocationSet::containsImplicit(SharedExp e) const
{
    if (e == nullptr) {
        return false;
    }

    auto     r(RefExp::get(e, nullptr));
    iterator it = m_set.lower_bound(r); // First element >= r

    // Note: the below relies on the fact that nullptr is less than any other pointer. Try later entries in the set:
    while (it != m_set.end()) {
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

    auto     search = RefExp::get(e->getSubExp1()->clone(), STMT_WILD);
    iterator pos    = m_set.find(search);

    if (pos == m_set.end()) {
        return false;
    }

    while (pos != m_set.end()) {
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
    Set newSet;

    for (SharedExp it : m_set) {
        newSet.insert(it->expSubscriptVar(it, d));
    }

    // Note: don't delete the old exps; they are copied in the new set
    m_set = newSet;
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

    for (SharedExp loc : m_set) {
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
}

