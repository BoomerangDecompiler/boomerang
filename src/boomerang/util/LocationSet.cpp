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

#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/util/StatementSet.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/Log.h"


LocationSet::LocationSet(const std::initializer_list<SharedExp> &exps)
    : ExpSet<Exp, lessExpStar>(exps)
{
}


LocationSet &LocationSet::operator=(const LocationSet &otherSet)
{
    m_set.clear();

    for (const SharedExp &loc : otherSet) {
        insert(loc->clone());
    }

    return *this;
}


LocationSet::LocationSet(const LocationSet &o)
    : ExpSet<Exp, lessExpStar>(o)
{
}


QString LocationSet::toString() const
{
    QString tgt;
    OStream ost(&tgt);

    for (const_iterator it = begin(); it != end(); ++it) {
        if (it != begin()) {
            ost << ", ";
        }

        ost << *it;
    }

    return tgt;
}


SharedExp LocationSet::findNS(SharedExp e)
{
    if (e == nullptr) {
        return nullptr;
    }

    // Note: can't search with a wildcard, since it doesn't have the weak ordering required (I
    // think)
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

    auto r(RefExp::get(e, nullptr));
    iterator it = m_set.lower_bound(r); // First element >= r

    // Note: the below relies on the fact that nullptr is less than any other pointer. Try later
    // entries in the set:
    while (it != m_set.end()) {
        if (!(*it)->isSubscript()) {
            return false; // Looking for e{something} (could be e.g. %pc)
        }

        if (!(*(*it)->getSubExp1() == *e)) { // Gone past e{anything}?
            return false;                    // Yes, then e{-} or e{0} cannot exist
        }

        if ((*it)->access<RefExp>()->isImplicitDef()) { // Check for e{-} or e{0}
            return true;                                // Found
        }

        ++it; // Else check next entry
    }

    return false;
}


bool LocationSet::findDifferentRef(const std::shared_ptr<RefExp> &ref, SharedExp &differentRef)
{
    if (!ref) {
        return false;
    }

    auto search  = RefExp::get(ref->getSubExp1()->clone(), STMT_WILD);
    iterator pos = m_set.find(search);

    if (pos == m_set.end()) {
        return false;
    }

    while (pos != m_set.end()) {
        assert(*pos);

        // Exit if we've gone to a new base expression
        // E.g. searching for r13{10} and **pos is r14{0}
        // Note: we want a ref-sensitive compare, but with the outer refs stripped off
        // For example: m[r29{10} - 16]{any} is different from m[r29{20} - 16]{any}
        if (!(*(*pos)->getSubExp1() == *ref->getSubExp1())) {
            break;
        }

        // Bases are the same; return true if only different ref
        if (!(**pos == *ref)) {
            differentRef = *pos;
            return true;
        }

        ++pos;
    }

    return false;
}


void LocationSet::addSubscript(const SharedStmt &d)
{
    Set newSet;

    for (SharedExp it : m_set) {
        newSet.insert(it->expSubscriptVar(it, d));
    }

    // Note: don't delete the old exps; they are copied in the new set
    m_set = newSet;
}
