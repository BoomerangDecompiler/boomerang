#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once

#include <set>
#include <memory>


class Statement;

using SharedExp = std::shared_ptr<class Exp>;


/**
 * A class to implement sets of statements
 */
class StatementSet
{
    typedef std::set<Statement *> Set;
    typedef Set::iterator iterator;
    typedef Set::const_iterator const_iterator;

public:
    iterator begin() { return m_set.begin(); }
    iterator end()   { return m_set.end(); }

    const_iterator begin() const { return m_set.begin(); }
    const_iterator end()   const { return m_set.end();   }

public:
    void insert(Statement *stmt);

    /// Remove this Statement.
    /// \returns true if removed, false if not found
    bool remove(Statement *s);

    bool contains(Statement *stmt) const;

    /// \returns true if any statement in this set defines \p loc
    bool definesLoc(SharedExp loc);

    /// \returns true if this set is a subset of \p other
    bool isSubSetOf(const StatementSet& other);

    /// Set union: this = this union \p other
    void makeUnion(const StatementSet& other);

    /// Set intersection: this = this intersect other
    void makeIsect(const StatementSet& other);

    /// Set difference: this = this - other
    void makeDiff(const StatementSet& other);

private:
    Set m_set;
};
