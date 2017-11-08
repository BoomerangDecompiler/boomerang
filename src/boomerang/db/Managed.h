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


/**
 * \brief   Definition of "managed" classes such as InstructionSet, which feature makeUnion etc
 * CLASSES: InstructionSet
 *          AssignSet
 *          StatementList
 *          StatementVec
 *          LocationSet
 *          ConnectionGraph
 */

#include "boomerang/db/exp/ExpHelp.h" // For lessExpStar

#include <list>
#include <set>
#include <vector>
#include <memory>

class Statement;
class Assign;
class Exp;
using SharedExp = std::shared_ptr<Exp>;
class RefExp;
class Cfg;
class LocationSet;
class QTextStream;

/// A class to implement sets of statements
class InstructionSet : public std::set<Statement *>
{
public:
    ~InstructionSet() {}
    void makeUnion(InstructionSet& other);       ///< Set union
    void makeDiff(InstructionSet& other);        ///< Set difference

    /// Make this set the intersection of itself and other
    void makeIsect(InstructionSet& other);       ///< Set intersection

    /// Check for the subset relation, i.e. are all my elements also in the set
    /// other. Effectively (this intersect other) == this
    bool isSubSetOf(InstructionSet& other);      ///< Subset relation

    // Remove this Statement. Return false if it was not found
    bool remove(Statement *s);                   ///< Removal; rets false if not found
    bool removeIfDefines(SharedExp given);       ///< Remove if given exp is defined
    bool removeIfDefines(InstructionSet& given); ///< Remove if any given is def'd

    // Search for s in this Statement set. Return true if found
    bool exists(Statement *s);                 ///< Search; returns false if !found

    // Find a definition for loc in this Statement set. Return true if found
    bool definesLoc(SharedExp loc);              ///< Search; returns true if any

    // statement defines loc
    bool operator<(const InstructionSet& o) const; ///< Compare if less
    void print(QTextStream& os) const;             ///< Print to os

    // Print just the numbers to stream os
    void printNums(QTextStream& os);               ///< Print statements as numbers

    // Print to a string, for debugging
    const char *prints();                          ///< Print to string (for debug)
    void dump();                                   ///< Print to standard error for debugging
};


// As above, but the Statements are known to be Assigns, and are sorted sensibly
class AssignSet : public std::set<Assign *, lessAssign>
{
public:
    ~AssignSet() {}

    // Make this set the union of itself and other
    void makeUnion(AssignSet& other);         ///< Set union

    // Make this set the difference of itself and other
    void makeDiff(AssignSet& other);          ///< Set difference

    /// Make this set the intersection of itself and other
    void makeIsect(AssignSet& other);         ///< Set intersection

    // Check for the subset relation, i.e. are all my elements also in the set
    // other. Effectively (this intersect other) == this
    bool isSubSetOf(AssignSet& other);        ///< Subset relation

    // Remove this Assign. Return false if it was not found
    bool remove(Assign *a);                   ///< Removal; rets false if not found
    bool removeIfDefines(SharedExp given);    ///< Remove if given exp is defined
    bool removeIfDefines(AssignSet& given);   ///< Remove if any given is def'd

    // Search for a in this Assign set. Return true if found
    bool exists(Assign *s);                   ///< Search; returns false if !found

    // Find a definition for loc in this Assign set. Return true if found
    bool definesLoc(SharedExp loc) const;     ///< Search; returns true if any assignment defines loc

    // Find a definition for loc on the LHS in this Assign set. If found, return pointer to the Assign with that LHS
    Assign *lookupLoc(SharedExp loc);         ///< Search for loc on LHS, return ptr to Assign if found

    bool operator<(const AssignSet& o) const; ///< Compare if less

    void print(QTextStream& os) const;        ///< Print to os

    // Print just the numbers to stream os
    void printNums(QTextStream& os);          ///< Print statements as numbers

    // Print to a string, for debugging
    char *prints();                           ///< Print to string (for debug)
    void dump();                              ///< Print to standard error for debugging
};                                            ///< class AssignSet


class StatementList : public std::list<Statement *>
{
public:
    ~StatementList() {}

    // A special intersection operator; this becomes the intersection of StatementList a (assumed to be a list of
    // Assignment*s) with the LocationSet b.
    // Used for calculating returns for a CallStatement
    // Special intersection method: this := a intersect b
    void makeIsect(StatementList& a, LocationSet& b);

    void append(Statement *s) { push_back(s); } ///< Insert at end
    void append(const StatementList& sl);         ///< Append whole StatementList
    void append(const InstructionSet& sl);        ///< Append whole InstructionSet

    bool remove(Statement *s);                  ///< Removal; rets false if not found

    /// Remove the first definition where loc appears on the left
    /// \note statements in this list are assumed to be assignments
    void removeDefOf(SharedExp loc);              ///< Remove definitions of loc

    // This one is needed where you remove in the middle of a loop
    // Use like this: it = mystatementlist.erase(it);
    bool exists(Statement *s);            ///< Search; returns false if not found
    char *prints();                       ///< Print to string (for debugging)
    void dump();                          ///< Print to standard error for debugging
    void makeCloneOf(StatementList& o);   ///< Make this a clone of o

    /// Return true if loc appears on the left of any statements in this list
    /// Note: statements in this list are assumed to be assignments
    bool existsOnLeft(const SharedExp& loc) const; ///< True if loc exists on the LHS of any Assignment in this list

    /// Find the first Assignment with loc on the LHS
    Assignment *findOnLeft(SharedExp loc) const;   ///< Return the first stmt with loc on the LHS
};


class StatementVec
{
    std::vector<Statement *> svec; // For now, use use standard vector

public:
    typedef std::vector<Statement *>::iterator           iterator;
    typedef std::vector<Statement *>::reverse_iterator   reverse_iterator;

    size_t size() const { return svec.size(); } ///< Number of elements
    iterator begin() { return svec.begin(); }
    iterator end() { return svec.end(); }
    reverse_iterator rbegin() { return svec.rbegin(); }
    reverse_iterator rend() { return svec.rend(); }

    // Get/put at position idx (0 based)
    Statement *operator[](size_t idx) { return svec[idx]; }
    void putAt(int idx, Statement *s);
    iterator remove(iterator it);
    char *prints(); ///< Print to string (for debugging)
    void dump();    ///< Print to standard error for debugging

    // Print just the numbers to stream os
    void printNums(QTextStream& os);

    void clear() { svec.clear(); }
    bool operator==(const StatementVec& o) const ///< Compare if equal
    {
        return svec == o.svec;
    }

    bool operator<(const StatementVec& o) const ///< Compare if less
    {
        return svec < o.svec;
    }

    void append(Statement *s) { svec.push_back(s); }
    void erase(iterator it) { svec.erase(it); }
};


typedef std::set<SharedExp, lessExpStar> ExpSet;


// For various purposes, we need sets of locations (registers or memory)
class LocationSet
{
    // We use a standard set, but with a special "less than" operator so that the sets are ordered
    // by expression value. If this is not done, then two expressions with the same value (say r[10])
    // but that happen to have different addresses (because they came from different statements)
    // would both be stored in the set (instead of the required set behaviour, where only one is stored)
    ExpSet lset;

public:
    typedef ExpSet::iterator       iterator;
    typedef ExpSet::const_iterator const_iterator;

    LocationSet() {}                              ///< Default constructor
    ~LocationSet() {}                             ///< virtual destructor kills warning
    LocationSet(const LocationSet& o);            ///< Copy constructor

    LocationSet& operator=(const LocationSet& o); ///< Assignment

    // Make this set the union of itself and other
    void makeUnion(LocationSet& other);           ///< Set union

    // Make this set the set difference of itself and other
    void makeDiff(LocationSet& other);            ///< Set difference

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
    void removeIfDefines(InstructionSet& given); ///< Remove locs defined in given

    size_t size() const { return lset.size(); }  ///< Number of elements
    bool operator==(const LocationSet& o) const; ///< Compare

    // Substitute s into all members of the set
    void substitute(Assign& a);                  ///< Substitute the given assignment to all
    void print(QTextStream& os) const;           ///< Print to os
    char *prints() const;                        ///< Print to string for debugging
    void dump() const;
    void printDiff(LocationSet *o) const;        ///< Diff 2 location sets to LOG_STREAM()
    bool exists(SharedExp e) const;              ///< Return true if the location exists in the set

    // This set is assumed to be of subscripted locations (e.g. a Collector), and we want to find the unsubscripted
    // location e in the set
    SharedExp findNS(SharedExp e);               ///< Find location e (no subscripts); nullptr if not found

    // Given an unsubscripted location e, return true if e{-} or e{0} exists in the set
    bool existsImplicit(SharedExp e) const;      ///< Search for location e{-} or e{0} (e has no subscripts)

    /// Return an iterator to the found item (or end() if not). Only really makes sense if e has a wildcard
    iterator find(SharedExp e)       { return lset.find(e); }
    const_iterator find(SharedExp e) const { return lset.find(e); }

    // Find a location with a different def, but same expression. For example, pass r28{10},
    // return true if r28{20} in the set. If return true, dr points to the first different ref
    bool findDifferentRef(const std::shared_ptr<RefExp>& e, SharedExp& dr);

    /// Add a subscript (to definition d) to each element
    void addSubscript(Statement *def /* , Cfg* cfg */); ///< Add a subscript to all elements
};


/**
 * A class to store connections in a graph, e.g. for interferences of types or live ranges, or the phi_unite relation
 * that phi statements imply
 * If a is connected to b, then b is automatically connected to a
 * \internal This is implemented in a std::multimap, even though Appel suggests a bitmap (e.g. std::vector<bool> does this in a
 * space efficient manner), but then you still need maps from expression to bit number. So here a standard map is used,
 * and when a -> b is inserted, b->a is redundantly inserted.
 */
class ConnectionGraph
{
    std::multimap<SharedExp, SharedExp, lessExpStar> emap;   ///< The map

public:
    typedef std::multimap<SharedExp, SharedExp, lessExpStar>::iterator       iterator;
    typedef std::multimap<SharedExp, SharedExp, lessExpStar>::const_iterator const_iterator;

    ConnectionGraph() = default;

    /// Add pair with check for existing
    void add(SharedExp a, SharedExp b);
    void connect(SharedExp a, SharedExp b);

    iterator begin()       { return emap.begin(); }
    iterator end()         { return emap.end(); }
    const_iterator begin() const { return emap.begin(); }
    const_iterator end()   const { return emap.end(); }

    /// Return a count of locations connected to \a e
    int count(SharedExp a) const;

    /// Return true if a is connected to b
    bool isConnected(SharedExp a, const Exp& b) const;
    bool allRefsHaveDefs() const;

    // Modify the map so that a <-> b becomes a <-> c
    /// Update the map that used to be a <-> b, now it is a <-> c
    void update(SharedExp a, SharedExp b, SharedExp c);

    // Remove the mapping at *aa, and return a valid iterator for looping
    iterator remove(iterator aa); ///< Remove the mapping at *aa
    void dump() const;            ///< Dump for debugging

private:
    std::vector<SharedExp> allConnected(SharedExp a);
};


QTextStream& operator<<(QTextStream& os, const AssignSet *as);
QTextStream& operator<<(QTextStream& os, const InstructionSet *ss);
QTextStream& operator<<(QTextStream& os, const LocationSet *ls);
