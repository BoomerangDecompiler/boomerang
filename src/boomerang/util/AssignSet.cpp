#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "AssignSet.h"


#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/exp/Terminal.h"


bool AssignSet::lessAssign::operator()(const Assign* x, const Assign* y) const
{
    return *x->getLeft() < *y->getLeft();
}


void AssignSet::insert(Assign *assign)
{
    m_set.insert(assign);
}


void AssignSet::makeUnion(const AssignSet& other)
{
    for (Assign *as : other) {
        m_set.insert(as);
    }
}


void AssignSet::makeDiff(const AssignSet& other)
{
    for (Assign *as : other) {
        m_set.erase(as);
    }
}


void AssignSet::makeIsect(const AssignSet& other)
{
    for (auto it = m_set.begin(); it != m_set.end(); ) {
        if (!other.contains(*it)) {
            // Not in both sets
            it = m_set.erase(it);
        }
        else {
            ++it;
        }
    }
}


bool AssignSet::isSubSetOf(const AssignSet& other)
{
    if (m_set.size() > other.m_set.size()) {
        return false;
    }

    for (Assign *asgn : *this) {
        if (!other.contains(asgn)) {
            return false;
        }
    }

    return true;
}


bool AssignSet::remove(Assign *a)
{
    if (this->contains(a)) {
        m_set.erase(a);
        return true;
    }

    return false;
}


bool AssignSet::definesLoc(SharedExp loc) const
{
    Assign as(loc, Terminal::get(opWild));

    return m_set.find(&as) != end();
}


Assign *AssignSet::lookupLoc(SharedExp loc)
{
    Assign   as(loc, Terminal::get(opWild));
    iterator ff = m_set.find(&as);

    if (ff == end()) {
        return nullptr;
    }

    return *ff;
}
