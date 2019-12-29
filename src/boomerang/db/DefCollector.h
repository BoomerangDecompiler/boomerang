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


#include "boomerang/ssl/exp/ExpHelp.h"
#include "boomerang/util/StatementSet.h"

#include <map>
#include <stack>


class Statement;
class UserProc;


/**
 * This class collects all definitions that reach the statement
 * that contains this collector.
 */
class BOOMERANG_API DefCollector
{
public:
    typedef AssignSet::const_iterator const_iterator;
    typedef AssignSet::iterator iterator;

public:
    DefCollector()                          = default;
    DefCollector(const DefCollector &other) = delete;
    DefCollector(DefCollector &&other)      = default;

    ~DefCollector();

    DefCollector &operator=(const DefCollector &other) = delete;
    DefCollector &operator=(DefCollector &&other) = default;

public:
    iterator begin() { return m_defs.begin(); }
    iterator end() { return m_defs.end(); }
    const_iterator begin() const { return m_defs.begin(); }
    const_iterator end() const { return m_defs.end(); }

public:
    /// Clone the given Collector into this one (discard existing data)
    void makeCloneOf(const DefCollector &other);

    /// Remove all collected definitions
    void clear();

    /// Insert a new collected def. If the LHS of \p a already exists, nothing happens.
    void collectDef(const std::shared_ptr<Assign> &a);

    /// \returns true if any assignment in this collector assigns to \p e on the LHS.
    bool hasDefOf(const SharedExp &e) const;

    /// Find the RHS expression for the LHS \p e.
    /// If not found, returns nullptr.
    SharedExp findDefFor(const SharedExp &e) const;

    /// Update the definitions with the current set of reaching definitions
    /// \p proc is the enclosing procedure
    void updateDefs(std::map<SharedExp, std::stack<SharedStmt>, lessExpStar> &Stacks,
                    UserProc *proc);

    /// Search and replace all occurrences
    void searchReplaceAll(const Exp &pattern, SharedExp replacement, bool &change);

public:
    /// Print the collected locations to stream \p os
    void print(OStream &os) const;

private:
    AssignSet m_defs; ///< The set of definitions.
};
