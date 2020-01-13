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


#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/LocationSet.h"


class UserProc;


/**
 * This class collects all uses (live variables) that will be defined
 * by the statement that contains this collector (or the UserProc that contains it).
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
    UseCollector(const UseCollector &other) = delete;
    UseCollector(UseCollector &&other)      = default;

    ~UseCollector();

    UseCollector &operator=(const UseCollector &other) = delete;
    UseCollector &operator=(UseCollector &&other) = default;

public:
    bool operator==(const UseCollector &other) const;
    bool operator!=(const UseCollector &other) const { return !(*this == other); }

    inline iterator begin() { return m_locs.begin(); }
    inline iterator end() { return m_locs.end(); }
    inline const_iterator begin() const { return m_locs.begin(); }
    inline const_iterator end() const { return m_locs.end(); }

public:
    /// clone the given Collector into this one (discard existing data)
    void makeCloneOf(const UseCollector &other);

    /// Remove all collected uses
    void clear();

    /// Insert a new collected use
    void collectUse(SharedExp e);

    /// \returns true if \p e is in the collection
    inline bool hasUse(SharedExp e) const { return m_locs.contains(e); }

    /// Remove the given location
    void removeUse(SharedExp loc);

    /// Remove the current location
    iterator removeUse(iterator it);

    /// \return all collected uses
    const LocationSet &getUses() const { return m_locs; }

public:
    /// Translate out of SSA form
    /// Called from CallStatement::fromSSAForm. The UserProc is needed for the symbol map
    void fromSSAForm(UserProc *proc, const SharedStmt &def);

    /// Print the collected locations to stream \p os
    void print(OStream &os) const;

private:
    /// The set of locations. Use lessExpStar to compare properly
    LocationSet m_locs;
};
