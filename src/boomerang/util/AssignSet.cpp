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


void AssignSet::clear()
{
    m_set.clear();
}

void AssignSet::insert(Assign *assign)
{
    assert(assign);
    assert(assign->getLeft());
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
    if (!loc) {
        return false;
    }

    Assign as(loc, Terminal::get(opWild));

    return m_set.find(&as) != end();
}


Assign *AssignSet::lookupLoc(SharedExp loc)
{
    if (!loc) {
        return nullptr;
    }

    Assign   as(loc, Terminal::get(opWild));
    iterator ff = m_set.find(&as);

    return (ff != end()) ? *ff : nullptr;
}


bool AssignSet::contains(Assign *asgn) const
{
    return m_set.find(asgn) != m_set.end();
}


bool AssignSet::empty() const
{
    return m_set.empty();
}


size_t AssignSet::size() const
{
    return m_set.size();
}

