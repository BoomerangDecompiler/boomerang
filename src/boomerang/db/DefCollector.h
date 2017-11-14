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


#include "boomerang/util/AssignSet.h"

#include <deque>

class Statement;
class UserProc;


/**
 * This class collects all definitions that reach the statement
 * that contains this collector.
 */
class DefCollector
{
public:
    DefCollector();

    ~DefCollector();

    /// Clone the given Collector into this one
    void makeCloneOf(const DefCollector& other);

    /// \returns true if initialised
    inline bool isInitialised() const { return m_initialised; }

    /// Clear the location set
    void clear()
    {
        m_defs.clear();
        m_initialised = false;
    }

    /**
     * Insert a new member (make sure none exists yet).
     * Takes ownership of the pointer. Deletes \p a
     * if the LHS of \p a is already present.
     */
    void insert(Assign *a);


    /// Print the collected locations to stream os
    void print(QTextStream& os, bool html = false) const;

    /*
     * Print to string (for debugging)
     */
    char *prints() const;
    void dump() const;
    Assign *dumpAddrOfFourth();

    /*
     * begin() and end() so we can iterate through the locations
     */
    typedef AssignSet::const_iterator   const_iterator;
    typedef AssignSet::iterator         iterator;

    iterator begin() { return m_defs.begin(); }
    iterator end() { return m_defs.end(); }

    const_iterator begin() const { return m_defs.begin(); }
    const_iterator end() const { return m_defs.end(); }

    bool existsOnLeft(SharedExp e) const { return m_defs.definesLoc(e); }

    /*
     * Update the definitions with the current set of reaching definitions
     * proc is the enclosing procedure
     */
    void updateDefs(std::map<SharedExp, std::deque<Statement *>, lessExpStar>& Stacks, UserProc *proc);

    /**
     * Find the definition for a location.
     * Find the definition for e that reaches this Collector.
     * If none reaches here, return nullptr
     */
    SharedExp findDefFor(SharedExp e) const;

    /**
     * Search and replace all occurrences
     */
    void searchReplaceAll(const Exp& from, SharedExp to, bool& change);

private:
    /*
     * True if initialised. When not initialised, callees should not subscript parameters inserted into the
     * associated CallStatement
     */
    bool m_initialised;
    AssignSet m_defs; ///< The set of definitions.
};

