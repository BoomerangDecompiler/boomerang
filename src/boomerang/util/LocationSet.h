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


#include "boomerang/db/exp/ExpHelp.h"

#include <set>

class RefExp;
class Statement;
class QTextStream;
class Assign;


/// For various purposes, we need sets of locations (registers or memory)
class LocationSet
{
    typedef std::set<SharedExp, lessExpStar> ExpSet;

public:
    typedef ExpSet::iterator       iterator;
    typedef ExpSet::const_iterator const_iterator;

    LocationSet() = default;
    LocationSet(const LocationSet& other);
    LocationSet(LocationSet&& other) = default;

    ~LocationSet() = default;

    LocationSet& operator=(const LocationSet& other);
    LocationSet& operator=(LocationSet&& other) = default;

    bool operator==(const LocationSet& other) const; ///< Compare
    bool operator!=(const LocationSet& other) const { return !(*this == other); }

public:
    iterator begin() { return lset.begin(); }
    iterator end()   { return lset.end();   }
    const_iterator begin() const { return lset.begin(); }
    const_iterator end()   const { return lset.end();   }

public:
    bool empty() const { return lset.empty(); }

    size_t size() const { return lset.size(); }

    void clear() { lset.clear(); }

    void insert(SharedExp loc) { lset.insert(loc); }  ///< Insert the given location

    /// \param loc is not modified, and could be const'd if not for std::set requirements
    void remove(SharedExp loc);                       ///< Remove the given location

    void remove(iterator ll) { lset.erase(ll); } ///< Remove location, given iterator

    bool contains(SharedConstExp e) const;       ///< Return true if the location exists in the set

    // Given an unsubscripted location e, return true if e{-} or e{0} exists in the set
    bool existsImplicit(SharedExp e) const;

    /**
     * Given a not subscripted location \p e, return the subscripted location matching \p e.
     * Example: Given e == r32, return r32{-}.
     * Returns nullptr if not found.
     * \note This set is assumed to be of subscripted locations (e.g. a Collector).
     */
    SharedExp findNS(SharedExp e);

    /// Find a location with a different def, but same expression. For example, pass r28{10},
    /// return true if r28{20} in the set. If return true, dr points to the first different ref
    bool findDifferentRef(const std::shared_ptr<RefExp>& e, SharedExp& dr);

    /// Add a subscript (to definition \p def) to each element.
    /// Existing exps are not re-subscripted.
    void addSubscript(Statement *def);

    /// Make this set the union of itself and other
    void makeUnion(const LocationSet& other);

    /// Make this set the set difference of itself and other
    void makeDiff(const LocationSet& other);

    /// Substitute all occurrences of the LHS of the assignment
    /// by the RHS of the assignment
    void substitute(Assign& a);

    void print(QTextStream& os) const;           ///< Print to os
    char *prints() const;                        ///< Print to string for debugging

private:
    /// We use a standard set, but with a special "less than" operator
    /// so that the sets are ordered by expression value. If this
    /// is not done, then two expressions with the same value (say r[10])
    /// but that happen to have different addresses (because they came
    /// from different statements) would both be stored in the set
    /// (instead of the required set behaviour, where only one is stored)
    ExpSet lset;
};
