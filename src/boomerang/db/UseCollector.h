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
#include "boomerang/ssl/statements/Statement.h"


class UserProc;


/**
 * UseCollector class. This class collects all uses (live variables)
 * that will be defined by the statement that contains this collector
 * (or the UserProc that contains it).
 *
 * Typically the entries are not subscripted,
 * like parameters or locations on the LHS of assignments
 */
class BOOMERANG_API UseCollector
{
public:
    typedef LocationSet::iterator iterator;
    typedef LocationSet::const_iterator const_iterator;

public:
    UseCollector();

public:
    bool operator==(const UseCollector &other) const;
    bool operator!=(const UseCollector &other) const { return !(*this == other); }

    inline iterator begin() { return m_locs.begin(); }
    inline iterator end() { return m_locs.end(); }
    inline const_iterator begin() const { return m_locs.begin(); }
    inline const_iterator end() const { return m_locs.end(); }

public:
    /// clone the given Collector into this one
    void makeCloneOf(const UseCollector &other);

    /// \returns true if initialised
    inline bool isInitialised() const { return m_initialised; }

    /// Clear the location set
    void clear();

    /// Insert a new member
    void insert(SharedExp e);

    /// Print the collected locations to stream \p os
    void print(OStream &os) const;

    /// \returns true if \p e is in the collection
    inline bool exists(SharedExp e) const { return m_locs.contains(e); }
    LocationSet &getLocSet() { return m_locs; }

public:
    /// Remove the given location
    void remove(SharedExp loc);

    /// Remove the current location
    void remove(iterator it);

    /// Translate out of SSA form
    /// Called from CallStatement::fromSSAForm. The UserProc is needed for the symbol map
    void fromSSAForm(UserProc *proc, const SharedStmt &def);

private:
    /// True if initialised. When not initialised, callees should not
    /// subscript parameters inserted into the associated CallStatement
    bool m_initialised;

    /// The set of locations. Use lessExpStar to compare properly
    LocationSet m_locs;
};
