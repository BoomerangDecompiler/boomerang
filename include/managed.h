/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       managed.h
 * OVERVIEW:   Definition of "managed" classes such as StatementSet, which
 *              feature makeUnion etc
 *============================================================================*/

/*
 * 26/Aug/03 - Mike: Split off from statement.h
 */

#ifndef __MANAGED_H__
#define __MANAGED_H__

#include <list>
#include <set>
#include <vector>

#include "exphelp.h"        // For lessExpStar

class Statement;
class Exp;
class RefExp;

class ManagedIter {
public:
    virtual void operator++(int) = 0;   // Postfix; dummy arg
    virtual bool operator==(const ManagedIter& o) = 0;
    virtual bool operator!=(const ManagedIter& o) = 0;
};

class StmtSetIt : public ManagedIter {
    std::set<Statement*>::iterator iter;
public:
    StmtSetIt() {}
    StmtSetIt(std::set<Statement*>::iterator it) : iter(it) {}
    virtual void operator++(int) {iter++;}
    Statement*   operator*() {return *iter;}
    virtual bool operator==(const ManagedIter& o) {
        return iter == ((StmtSetIt&)o).iter;}
    virtual bool operator!=(const ManagedIter& o) {
        return iter != ((StmtSetIt&)o).iter;}
};

class LocSetIt : public ManagedIter {
    std::set<Exp*>::iterator iter;
public:
    LocSetIt() {}
    LocSetIt(std::set<Exp*>::iterator it) : iter(it) {}
    virtual void operator++(int) {iter++;}
    Exp*         operator*() {return *iter;}
    virtual bool operator==(const ManagedIter& o) {
        return iter == ((LocSetIt&)o).iter;}
    virtual bool operator!=(const ManagedIter& o) {
        return iter != ((LocSetIt&)o).iter;}
};

class Managed {
public:
    virtual ManagedIter& begin() = 0;
    virtual ManagedIter& end() = 0;
};

// A class to implement sets of statements
// We may choose to implement these very differently one day
typedef std::set<Statement*>::iterator StmtSetIter;
class StatementSet /*: public Managed */{
    std::set<Statement*> sset;          // For now, use use standard sets

public:
    virtual ~StatementSet() {}
    void makeUnion(StatementSet& other);    // Set union
    void makeDiff (StatementSet& other);    // Set difference
    void makeIsect(StatementSet& other);    // Set intersection
    bool isSubSetOf(StatementSet& other);    // subset relation

    int size() {return sset.size();}        // Number of elements
    Statement* getFirst(StmtSetIter& it);   // Get the first Statement
    Statement* getNext (StmtSetIter& it);   // Get next
    virtual ManagedIter& begin() {return *new StmtSetIt(sset.begin());}
    virtual ManagedIter& end()   {return *new StmtSetIt(sset.end());}
    
    void insert(Statement* s) {sset.insert(s);} // Insertion
    bool remove(Statement* s);              // Removal; rets false if not found
    bool removeIfDefines(Exp* given);       // Remove if given exp is defined
    bool removeIfDefines(StatementSet& given);// Remove if any given is def'd
    bool exists(Statement* s);              // Search; returns false if !found
    bool defines(Exp* loc);                 // Search; returns true if any
                                            // statement defines loc
    void clear() {sset.clear();}            // Clear the set
    bool operator==(const StatementSet& o) const    // Compare if equal
        { return sset == o.sset;}
    bool operator<(const StatementSet& o) const;    // Compare if less
    char* prints();                         // Print to std::cerr (for debug)
    void  print(std::ostream& os);          // Print to os
    void printNums(std::ostream& os);       // Print statements as numbers
    bool isLast(StmtSetIter& it);           // returns true if it is at end
};  // class StatementSet

typedef std::list<Statement*>::iterator StmtListIter;
typedef std::list<Statement*>::reverse_iterator StmtListRevIter;
class StatementList {
    std::list<Statement*> slist;          // For now, use use standard list

public:
    int size() {return slist.size();}        // Number of elements
    Statement* getFirst(StmtListIter& it);   // Get the first Statement
    Statement* getNext (StmtListIter& it);   // Get next
    Statement* getLast (StmtListRevIter& it);// Get the last Statement
    Statement* getPrev (StmtListRevIter& it);// Get previous
    void append(Statement* s) {slist.push_back(s);} // Insert at end
    void append(StatementList& sl);         // Append whole StatementList
    void append(StatementSet& sl);          // Append whole StatementSet
    bool remove(Statement* s);              // Removal; rets false if not found
    // This one is needed where you remove in the middle of a loop
    // Use like this: s = mystatementlist.remove(it);
    Statement* StatementList::remove(StmtListIter& it);
    bool exists(Statement* s);  // Find; returns false if not found
    char*  prints();                        // Print to string (for debugging)
    void clear() { slist.clear(); }
};  // class StatementList

typedef std::vector<Statement*>::iterator StmtVecIter;
typedef std::vector<Statement*>::reverse_iterator StmtVecRevIter;
class StatementVec {
    std::vector<Statement*> svec;           // For now, use use standard vector

public:
    int size() {return svec.size();}         // Number of elements
    Statement* getFirst(StmtVecIter& it);    // Get the first Statement
    Statement* getNext (StmtVecIter& it);    // Get next
    Statement* getLast (StmtVecRevIter& it); // Get the last Statement
    Statement* getPrev (StmtVecRevIter& it); // Get previous
    // returns true if it is at end
    bool isLast(StmtVecIter& it) {return it == svec.end();}
    Statement* getAt(int idx) {return svec[idx];}
    // Put at position idx (0 based)
    void   putAt(int idx, Statement* s);
    char*  prints();                        // Print to string (for debugging)
    void   printNums(std::ostream& os);
    void   clear() { svec.clear(); }
    bool operator==(const StatementVec& o) const    // Compare if equal
        { return svec == o.svec;}
    bool operator<(const StatementVec& o) const     // Compare if less
        { return svec < o.svec;}
};  // class StatementVec

// For liveness, we need sets of locations (registers or memory)
typedef std::set<Exp*, lessExpStar>::iterator LocSetIter;
class LocationSet {
    // We use a standard set, but with a special "less than" operator
    // so that the sets are ordered by expression value. If this is not done,
    // then two expressions with the same value (say r[10]) but that happen to
    // have different addresses (because they came from different statements)
    // would both be stored in the set (instead of the required set 
    // behaviour, where only one is stored)
    std::set<Exp*, lessExpStar> sset; 
public:
    LocationSet() {}                        // Default constructor
    virtual ~LocationSet() {}               // virtual destructor kills warning
    LocationSet(const LocationSet& o);      // Copy constructor
    LocationSet& operator=(const LocationSet& o); // Assignment
    void makeUnion(LocationSet& other);     // Set union
    void makeDiff (LocationSet& other);     // Set difference
    void clear() {sset.clear();}            // Clear the set
    Exp* getFirst(LocSetIter& it);          // Get the first Statement
    Exp* getNext (LocSetIter& it);          // Get next
    virtual ManagedIter& begin() {return *new LocSetIt(sset.begin());}
    virtual ManagedIter& end()   {return *new LocSetIt(sset.end());}
    void insert(Exp* loc) {sset.insert(loc);}// Insert the given location
    void remove(Exp* loc);                  // Remove the given location
    void remove(LocSetIter ll);             // Remove location, given iterator
    void removeIfDefines(StatementSet& given);// Remove locs defined in given
    int  size() const {return sset.size();} // Number of elements
    bool operator==(const LocationSet& o) const; // Compare
    void substitute(Statement& s);          // Substitute the statement to all
    char* prints();                         // Print to cerr for debugging
    void  print(std::ostream& os);          // Print to os
    // Return true if the location exists in the set
    bool find(Exp* e);
    // Find a location with a different def, but same expression
    // For example, pass r28{10}, return true if r28{20} in the set
    bool findDifferentRef(RefExp* e);
    void addSubscript(Statement* def);      // Add a subscript to all elements
};  // class LocationSet

#endif  // #ifdef __MANAGED_H__
