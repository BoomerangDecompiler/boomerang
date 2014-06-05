/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**=================
  * \file       managed.h
  * OVERVIEW:   Definition of "managed" classes such as StatementSet, which feature makeUnion etc
  * CLASSES:        StatementSet
  *                AssignSet
  *                StatementList
  *                StatementVec
  *                LocationSet
  *                //LocationList
  *                ConnectionGraph
  *==============================================================================================*/

#ifndef __MANAGED_H__
#define __MANAGED_H__

#include <list>
#include <set>
#include <vector>

#include "exphelp.h" // For lessExpStar

class Instruction;
class Assign;
class Exp;
class RefExp;
class Cfg;
class LocationSet;
class QTextStream;

// A class to implement sets of statements
class StatementSet : public std::set<Instruction *> {

  public:
    ~StatementSet() {}
    void makeUnion(StatementSet &other);  // Set union
    void makeDiff(StatementSet &other);   // Set difference
    void makeIsect(StatementSet &other);  // Set intersection
    bool isSubSetOf(StatementSet &other); // Subset relation

    bool remove(Instruction *s);                   // Removal; rets false if not found
    bool removeIfDefines(Exp *given);            // Remove if given exp is defined
    bool removeIfDefines(StatementSet &given);   // Remove if any given is def'd
    bool exists(Instruction *s);                   // Search; returns false if !found
    bool definesLoc(Exp *loc);                   // Search; returns true if any
                                                 // statement defines loc
    bool operator<(const StatementSet &o) const; // Compare if less
    void print(QTextStream &os) const;          // Print to os
    void printNums(QTextStream &os);            // Print statements as numbers
    const char *prints();                              // Print to string (for debug)
    void dump();                                 // Print to standard error for debugging
};                                               // class StatementSet

// As above, but the Statements are known to be Assigns, and are sorted sensibly
class AssignSet : public std::set<Assign *, lessAssign> {
  public:
    ~AssignSet() {}
    void makeUnion(AssignSet &other);       // Set union
    void makeDiff(AssignSet &other);        // Set difference
    void makeIsect(AssignSet &other);       // Set intersection
    bool isSubSetOf(AssignSet &other);      // Subset relation
    bool remove(Assign *a);                 // Removal; rets false if not found
    bool removeIfDefines(Exp *given);       // Remove if given exp is defined
    bool removeIfDefines(AssignSet &given); // Remove if any given is def'd
    bool exists(Assign *s);                 // Search; returns false if !found
    bool definesLoc(Exp *loc);              // Search; returns true if any assignment defines loc
    Assign *lookupLoc(Exp *loc);            // Search for loc on LHS, return ptr to Assign if found

    bool operator<(const AssignSet &o) const; // Compare if less
    void print(QTextStream &os) const;       // Print to os
    void printNums(QTextStream &os);         // Print statements as numbers
    char *prints();                           // Print to string (for debug)
    void dump();                              // Print to standard error for debugging
};                                            // class AssignSet

class StatementList : public std::list<Instruction *> {

  public:
    ~StatementList() {}

    // A special intersection operator; this becomes the intersection of StatementList a (assumed to be a list of
    // Assignment*s) with the LocationSet b.
    // Used for calculating returns for a CallStatement
    void makeIsect(StatementList &a, LocationSet &b);

    void append(Instruction *s) { push_back(s); } // Insert at end
    void append(StatementList &sl);             // Append whole StatementList
    void append(StatementSet &sl);              // Append whole StatementSet
    bool remove(Instruction *s);                  // Removal; rets false if not found
    void removeDefOf(Exp *loc);                 // Remove definitions of loc
    // This one is needed where you remove in the middle of a loop
    // Use like this: it = mystatementlist.erase(it);
    bool exists(Instruction *s);          //!< Search; returns false if not found
    char *prints();                     //!< Print to string (for debugging)
    void dump();                        //!< Print to standard error for debugging
    void makeCloneOf(StatementList &o); //!< Make this a clone of o
    bool existsOnLeft(Exp *loc);        //!< True if loc exists on the LHS of any Assignment in this list
    Assignment *findOnLeft(Exp *loc);   //!< Return the first stmt with loc on the LHS
};                                      // class StatementList

class StatementVec {
    std::vector<Instruction *> svec; // For now, use use standard vector

  public:
    typedef std::vector<Instruction *>::iterator iterator;
    typedef std::vector<Instruction *>::reverse_iterator reverse_iterator;
    size_t size() { return svec.size(); } // Number of elements
    iterator begin() { return svec.begin(); }
    iterator end() { return svec.end(); }
    reverse_iterator rbegin() { return svec.rbegin(); }
    reverse_iterator rend() { return svec.rend(); }
    // Get/put at position idx (0 based)
    Instruction *operator[](size_t idx) { return svec[idx]; }
    void putAt(int idx, Instruction *s);
    iterator remove(iterator it);
    char *prints(); // Print to string (for debugging)
    void dump();    // Print to standard error for debugging
    void printNums(QTextStream &os);
    void clear() { svec.clear(); }
    bool operator==(const StatementVec &o) const // Compare if equal
    {
        return svec == o.svec;
    }
    bool operator<(const StatementVec &o) const // Compare if less
    {
        return svec < o.svec;
    }
    void append(Instruction *s) { svec.push_back(s); }
    void erase(iterator it) { svec.erase(it); }
}; // class StatementVec

// For various purposes, we need sets of locations (registers or memory)
class LocationSet {
    // We use a standard set, but with a special "less than" operator so that the sets are ordered
    // by expression value. If this is not done, then two expressions with the same value (say r[10])
    // but that happen to have different addresses (because they came from different statements)
    // would both be stored in the set (instead of the required set behaviour, where only one is stored)
    std::set<Exp *, lessExpStar> lset;

  public:
    typedef std::set<Exp *, lessExpStar>::iterator iterator;
    typedef std::set<Exp *, lessExpStar>::const_iterator const_iterator;
    LocationSet() {}                              // Default constructor
    ~LocationSet() {}                             // virtual destructor kills warning
    LocationSet(const LocationSet &o);            // Copy constructor
    LocationSet &operator=(const LocationSet &o); // Assignment
    void makeUnion(LocationSet &other);           // Set union
    void makeDiff(LocationSet &other);            // Set difference
    void clear() { lset.clear(); }                // Clear the set
    iterator begin() { return lset.begin(); }
    iterator end() { return lset.end(); }
    const_iterator begin() const { return lset.begin(); }
    const_iterator end() const { return lset.begin(); }
    void insert(Exp *loc) { lset.insert(loc); }  // Insert the given location
    void remove(Exp *loc);                       // Remove the given location
    void remove(iterator ll) { lset.erase(ll); } // Remove location, given iterator
    void removeIfDefines(StatementSet &given);   // Remove locs defined in given
    size_t size() const { return lset.size(); }  // Number of elements
    bool operator==(const LocationSet &o) const; // Compare
    void substitute(Assign &a);                  // Substitute the given assignment to all
    void print(QTextStream &os) const;          // Print to os
    char *prints();                              // Print to string for debugging
    void dump();
    void diff(LocationSet *o);   // Diff 2 location sets to LOG_STREAM()
    bool exists(Exp *e);         // Return true if the location exists in the set
    Exp *findNS(Exp *e);         // Find location e (no subscripts); nullptr if not found
    bool existsImplicit(Exp *e); // Search for location e{-} or e{0} (e has no subscripts)
    // Return an iterator to the found item (or end() if not). Only really makes sense if e has a wildcard
    iterator find(Exp *e) { return lset.find(e); }
    // Find a location with a different def, but same expression. For example, pass r28{10},
    // return true if r28{20} in the set. If return true, dr points to the first different ref
    bool findDifferentRef(RefExp *e, Exp *&dr);
    void addSubscript(Instruction *def /* , Cfg* cfg */); // Add a subscript to all elements
};                                                      // class LocationSet

class Range {
  protected:
    int stride, lowerBound, upperBound;
    Exp *base;

  public:
    Range();
    Range(int stride, int lowerBound, int upperBound, Exp *base);

    Exp *getBase() { return base; }
    int getStride() { return stride; }
    int getLowerBound() { return lowerBound; }
    int getUpperBound() { return upperBound; }
    void unionWith(Range &r);
    void widenWith(Range &r);
    void print(QTextStream &os) const;
    bool operator==(Range &other);

    static const int MAX = 2147483647;
    static const int MIN = -2147483647;
};

class RangeMap {
  protected:
    std::map<Exp *, Range, lessExpStar> ranges;

  public:
    RangeMap() {}
    void addRange(Exp *loc, Range &r) { ranges[loc] = r; }
    bool hasRange(Exp *loc) { return ranges.find(loc) != ranges.end(); }
    Range &getRange(Exp *loc);
    void unionwith(RangeMap &other);
    void widenwith(RangeMap &other);
    void print(QTextStream &os) const;
    Exp *substInto(Exp *e, std::set<Exp *, lessExpStar> *only = nullptr);
    void killAllMemOfs();
    void clear() { ranges.clear(); }
    bool isSubset(RangeMap &other);
    bool empty() const { return ranges.empty(); }
};

/// A class to store connections in a graph, e.g. for interferences of types or live ranges, or the phi_unite relation
/// that phi statements imply
/// If a is connected to b, then b is automatically connected to a
// This is implemented in a std::multimap, even though Appel suggests a bitmap (e.g. std::vector<bool> does this in a
// space efficient manner), but then you still need maps from expression to bit number. So here a standard map is used,
// and when a -> b is inserted, b->a is redundantly inserted.
class ConnectionGraph {
    std::multimap<Exp *, Exp *, lessExpStar> emap; // The map
  public:
    typedef std::multimap<Exp *, Exp *, lessExpStar>::iterator iterator;
    typedef std::multimap<Exp *, Exp *, lessExpStar>::const_iterator const_iterator;
    ConnectionGraph() {}

    void add(Exp *a, Exp *b); // Add pair with check for existing
    void connect(Exp *a, Exp *b);
    iterator begin() { return emap.begin(); }
    iterator end() { return emap.end(); }
    const_iterator begin() const { return emap.begin(); }
    const_iterator end() const { return emap.end(); }
    int count(Exp *a) const;
    bool isConnected(Exp *a, const Exp &b) const;
    void update(Exp *a, Exp *b, Exp *c);
    iterator remove(iterator aa); // Remove the mapping at *aa
    void dump() const;            // Dump for debugging
};
QTextStream &operator<<(QTextStream &os, const AssignSet *as);
QTextStream &operator<<(QTextStream &os, const StatementSet *ss);
QTextStream &operator<<(QTextStream &os, const LocationSet *ls);
#endif // #ifdef __MANAGED_H__
