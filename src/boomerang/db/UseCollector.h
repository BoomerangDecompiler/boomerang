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


#include "boomerang/util/LocationSet.h"

class UserProc;


/**
 * UseCollector class. This class collects all uses (live variables)
 * that will be defined by the statement that contains this collector
 * (or the UserProc that contains it).
 *
 * Typically the entries are not subscripted,
 * like parameters or locations on the LHS of assignments
 */
class UseCollector
{
public:
    typedef LocationSet::iterator         iterator;
    typedef LocationSet::const_iterator   const_iterator;

public:
    UseCollector();

    bool operator==(const UseCollector& other) const;
    bool operator!=(const UseCollector& other) const { return !(*this == other); }

    inline iterator begin() { return m_locs.begin(); }
    inline iterator end()   { return m_locs.end(); }
    inline const_iterator begin() const { return m_locs.begin(); }
    inline const_iterator end() const { return m_locs.end(); }

public:
    /// clone the given Collector into this one
    void makeCloneOf(UseCollector& other);

    /// \returns true if initialised
    inline bool isInitialised() const { return m_initialised; }

    /// Clear the location set
    void clear();

    /// Insert a new member
    void insert(SharedExp e);

    /// Print the collected locations to stream \p os
    void print(QTextStream& os, bool html = false) const;

    /// Print to string (for debugging)
    char *prints() const;
    void dump() const;

    /// \returns true if \p e is in the collection
    inline bool exists(SharedExp e) { return m_locs.exists(e); }
    LocationSet& getLocSet() { return m_locs; }

public:
    /// Add a new use from Statement \p stmt
    void updateLocs(Statement *stmt);

    /// Remove the given location
    void remove(SharedExp loc);

    /// Remove the current location
    void remove(iterator it);

    /// Translate out of SSA form
    /// Called from CallStatement::fromSSAForm. The UserProc is needed for the symbol map
    void fromSSAForm(UserProc *proc, Statement *def);

private:
    /// True if initialised. When not initialised, callees should not
    /// subscript parameters inserted into the associated CallStatement
    bool m_initialised;

    /// The set of locations. Use lessExpStar to compare properly
    LocationSet m_locs;
};
