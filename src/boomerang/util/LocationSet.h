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

class StatementSet;
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

public:
    // Make this set the union of itself and other
    void makeUnion(const LocationSet& other);           ///< Set union

    // Make this set the set difference of itself and other
    void makeDiff(const LocationSet& other);            ///< Set difference

    void clear() { lset.clear(); }                ///< Clear the set

    iterator begin() { return lset.begin(); }
    iterator end() { return lset.end(); }
    const_iterator begin() const { return lset.begin(); }
    const_iterator end() const { return lset.begin(); }

    void insert(SharedExp loc) { lset.insert(loc); }  ///< Insert the given location

    /// \param loc is not modified, and could be const'd if not for std::set requirements
    void remove(SharedExp loc);                       ///< Remove the given location

    void remove(iterator ll) { lset.erase(ll); } ///< Remove location, given iterator

    // Remove locations defined by any of the given set of statements
    // Used for killing in liveness sets
    void removeIfDefines(StatementSet& given); ///< Remove locs defined in given

    size_t size() const { return lset.size(); }  ///< Number of elements
    bool operator==(const LocationSet& o) const; ///< Compare

    // Substitute s into all members of the set
    void substitute(Assign& a);                  ///< Substitute the given assignment to all
    void print(QTextStream& os) const;           ///< Print to os
    char *prints() const;                        ///< Print to string for debugging
    void dump() const;
    void printDiff(LocationSet *o) const;        ///< Diff 2 location sets to LOG_STREAM()
    bool exists(SharedConstExp e) const;         ///< Return true if the location exists in the set

    /// Find location e (no subscripts); nullptr if not found
    /// This set is assumed to be of subscripted locations (e.g. a Collector), and we want to find the unsubscripted
    /// location e in the set
    SharedExp findNS(SharedExp e);

    // Given an unsubscripted location e, return true if e{-} or e{0} exists in the set
    bool existsImplicit(SharedExp e) const;

    /// Return an iterator to the found item (or end() if not). Only really makes sense if e has a wildcard
    iterator find(SharedExp e)       { return lset.find(e); }
    const_iterator find(SharedExp e) const { return lset.find(e); }

    /// Find a location with a different def, but same expression. For example, pass r28{10},
    /// return true if r28{20} in the set. If return true, dr points to the first different ref
    bool findDifferentRef(const std::shared_ptr<RefExp>& e, SharedExp& dr);

    /// Add a subscript (to definition d) to each element
    void addSubscript(Statement *def); ///< Add a subscript to all elements

private:
    /// We use a standard set, but with a special "less than" operator
    /// so that the sets are ordered by expression value. If this
    /// is not done, then two expressions with the same value (say r[10])
    /// but that happen to have different addresses (because they came
    /// from different statements) would both be stored in the set
    /// (instead of the required set behaviour, where only one is stored)
    ExpSet lset;
};
