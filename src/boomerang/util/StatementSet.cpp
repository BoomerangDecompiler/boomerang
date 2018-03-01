#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementSet.h"


#include "boomerang/db/exp/Exp.h"
#include "boomerang/db/statements/Statement.h"


#include <QTextStream>


void StatementSet::insert(Statement *stmt)
{
    assert(stmt != nullptr);
    m_set.insert(stmt);
}


void StatementSet::makeUnion(const StatementSet& other)
{
    for (Statement *stmt : other) {
        m_set.insert(stmt);
    }
}


void StatementSet::makeDiff(const StatementSet& other)
{
    if (&other == this) {
        m_set.clear(); // A \ A == empty set
        return;
    }

    for (Statement *stmt : other) {
        m_set.erase(stmt);
    }
}


void StatementSet::makeIsect(const StatementSet& other)
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


bool StatementSet::isSubSetOf(const StatementSet& other)
{
    if (m_set.size() > other.m_set.size()) {
        return false;
    }

    for (Statement *stmt : *this) {
        if (!other.contains(stmt)) {
            return false;
        }
    }

    return true;
}


bool StatementSet::remove(Statement *s)
{
    if (this->contains(s)) {
        m_set.erase(s);
        return true;
    }

    return false;
}


bool StatementSet::definesLoc(SharedExp loc)
{
    if (!loc) {
        return false;
    }

    return std::any_of(m_set.begin(), m_set.end(),
        [loc] (const Statement *stmt) {
            return stmt->definesLoc(loc);
        });
}


bool StatementSet::contains(Statement *stmt) const
{
    return m_set.find(stmt) != m_set.end();
}
