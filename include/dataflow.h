/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       dataflow.h
 * OVERVIEW:   Dataflow analysis classes.
 *============================================================================*/

/*
 * $Revision$
 * 25 Nov 02 - Trent: appropriated for use by new dataflow.
 * 3 July 02 - Trent: created.
 * 03 Feb 03 - Mike: cached dataflow (uses and usedBy)
 * 03 Apr 03 - Mike: Added StatementSet
 */

#ifndef _DATAFLOW_H_
#define _DATAFLOW_H_

#include <set>
#include <list>
#include <map>
#include <ostream>
#include "exphelp.h"    // For lessExpStar


class Exp;              // Can't #include exp.h, since AssignExp is derived
                        // from Statement, so must #include dataflow.h in exp.h
class BasicBlock;
typedef BasicBlock *PBB;
class Prog;
class UserProc;
class Cfg;
class Type;
class Statement;
class LocationSet;      // Actually declared in exp.h
typedef std::map<Exp*, int, lessExpStar> igraph;

// A class to implement sets of statements
// We may choose to implement these very differently one day
typedef std::set<Statement*>::iterator StmtSetIter;
class StatementSet {
    std::set<Statement*> sset;          // For now, use use standard sets

public:
    void makeUnion(StatementSet& other);    // Set union
    void makeDiff (StatementSet& other);    // Set difference
    void makeIsect(StatementSet& other);    // Set intersection
    bool isSubSetOf(StatementSet& other);    // subset relation
    // Set difference (remove all elements of this where some element of other
    // defines same location)
    void makeKillDiff (StatementSet& other);

    int size() {return sset.size();}        // Number of elements
    Statement* getFirst(StmtSetIter& it);   // Get the first Statement
    Statement* getNext (StmtSetIter& it);   // Get next
    void insert(Statement* s) {sset.insert(s);} // Insertion
    bool remove(Statement* s);              // Removal; rets false if not found
    bool removeIfDefines(Exp* given);       // Remove if given exp is defined
    bool removeIfDefines(StatementSet& given);// Remove if any given is def'd
    bool exists(Statement* s);              // Search; returns false if !found
    bool defines(Exp* loc);                 // Search; returns true if any
                                            // statement defines loc
    void clear() {sset.clear();}            // Clear the set
    bool operator==(const StatementSet& o) const // Compare
        { return sset == o.sset;}
    void prints();                          // Print to std::cerr (for debug)
    void printNums(std::ostream& os);       // Print statements as numbers
};

// Ugh - we also need lists of Statements for the internal statements
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
    void prints();                          // Print to cerr (for debugging)
};

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
    LocationSet(const LocationSet& o);      // Copy constructor
    LocationSet& operator=(const LocationSet& o); // Assignment
    void makeUnion(LocationSet& other);    // Set union
    void makeDiff (LocationSet& other);    // Set difference
    void clear() {sset.clear();}            // Clear the set
    Exp* getFirst(LocSetIter& it);          // Get the first Statement
    Exp* getNext (LocSetIter& it);          // Get next
    void insert(Exp* loc) {sset.insert(loc);}// Insert the given location
    void remove(Exp* loc);                  // Remove the given location
    void remove(LocSetIter ll);             // Remove location, given iterator
    void removeIfDefines(StatementSet& given);// Remove locs defined in given
    int  size() const {return sset.size();}  // Number of elements
    bool operator==(const LocationSet& o) const; // Compare
    void substitute(Statement& s);          // Substitute the statement to all
    void prints();                          // Print to cerr for debugging
    // Return true if the location exists in the set
    bool find(Exp* e);
};


/* Statements define values that are used in expressions.
 * They are akin to "definition" in the Dragon Book.
 */
class Statement {
protected:
    PBB     pbb;  // contains a pointer to the enclosing BB
    UserProc *proc; // procedure containing this statement
    // The following 2 are soon to be deleted!
    //StatementSet uses;          // ud chain: my uses' defs
    //StatementSet usedBy;        // du chain: my def's uses
    int     number;             // Statement number for printing
public:

    Statement() : pbb(NULL), proc(NULL), number(0) { }
    virtual ~Statement() {
    }

    bool        operator==(Statement& o);
    void        setProc(UserProc *p) { proc = p; }

    // calculates the reaching definitions set after this statement
    virtual void calcReachOut(StatementSet &reachout);

    // gets the reaching definitions set before this statement
    virtual void getReachIn(StatementSet &reachin, int phase);

    // removes any statement from the reaching or available definitions set
    // which is killed by this statement
    virtual void killDef(StatementSet &reach) = 0;

    // calculates the available definitions set after this statement
    virtual void calcAvailOut(StatementSet &availout);

    // get the available definitions (not reassigned on any path) before
    // this statement
    virtual void getAvailIn(StatementSet& availin, int phase);

    // calculates the live variables (used before definition) before this stmt
    virtual void calcLiveIn(LocationSet &livein);

    // removes any statement from the set containing live variables which is
    // killed by this statement
    virtual void killLive(LocationSet &live) = 0;

    // calculates the dead variables (defined before use) before this stmt
    virtual void calcDeadIn(LocationSet &deadin);

    // removes any statement from the set containing dead variables which is
    // killed (in a deadness sense, i.e. used) by this statement
    virtual void killDead(LocationSet &dead) = 0;

    // check live in for interference
    void checkLiveIn(LocationSet& liveout, igraph& ig);


    // creates a set of statements that are killed by this statement
    // and have no uses
    //virtual void getDeadStatements(StatementSet &dead) = 0;

    // calculates the uses/usedBy links for this statement
    //virtual void calcUseLinks();

    // returns true if this statement defines anything
    virtual bool isDefinition() = 0;

    // returns a set of locations defined by this statement
    virtual void getDefinitions(LocationSet &def);

    // returns an expression that would be used to reference the value
    // defined by this statement (if this statement is propogatable)
    virtual Exp* getLeft() = 0;

    // returns a type for the left
    virtual Type* getLeftType() = 0;

    // returns an expression that would be used to replace this statement
    // in a use
    virtual Exp* getRight() = 0;

    // returns true if this statement uses the given expression
    virtual bool usesExp(Exp *e) = 0;

    // Adds (inserts) all locations (registers or memory) used by this statement
    virtual void addUsedLocs(LocationSet& used) = 0;

    // Subscript the left hand side to "point to self"
    virtual void subscriptLeft(Statement* self) {};

    // returns the statement which is used by this statement and has a
    // left like the given expression
    // MVE: is this useful?
    //virtual Statement *findDef(Exp *e);

    // 
    // get my uses' definitions (ud chain)
    // 
    //void calcUses(StatementSet &uses);
    //int getNumUses() { return uses.size(); }
    //StatementSet &getUses() { return uses; }
    //void clearUses() {uses.clear(); usedBy.clear();}
 
    // 
    // usedBy: du chain (my def's uses)
    //
    //void calcUsedBy(StatementSet &usedBy);
    //int getNumUsedBy() { return usedBy.size(); }

    // update my data flow (I'm about to be deleted)
    //void updateDfForErase();

    // get/set the enclosing BB
    PBB getBB() { return pbb; }
    void setBB(PBB bb) { pbb = bb; }

    // returns true if this statement can be propagated to all its
    // uses and removed
    //virtual bool canPropagateToAll();

    // propagates this statement to all its uses, caller must remove
    //virtual void propagateToAll();

    // replaces a use of the given statement with an expression
            void replaceRef(Statement *use);
    // special version of the above for the "special hack"
    // (see Proc::propagateStatements, where numUses == 2)
            void specialReplaceRef(Statement* def);

    // Remove refs to statements defining a restored location
    virtual void removeRestoreRefs(StatementSet& rs) = 0;

    // statements should be printable (for debugging)
    virtual void print(std::ostream &os, bool withUses = false) = 0;
    virtual void printWithUses(std::ostream& os) {print(os, true);}
            void printAsUse(std::ostream &os)   {os << std::dec << number;}
            void printAsUseBy(std::ostream &os) {os << std::dec << number;}
            void printNum(std::ostream &os)     {os << std::dec << number;}
            char* prints();      // For use in a debugger

    // inline / decode any constants in the statement
    virtual void processConstants(Prog *prog) = 0;

    // general search
    virtual bool search(Exp *search, Exp *&result) = 0;

    // general search and replace
    virtual bool searchAndReplace(Exp *search, Exp *replace) = 0;

    // update the type information for an expression in this statement
    virtual Type *updateType(Exp *e, Type *curType) = 0;

    // get the statement number
    int     getNumber() {return number;}

    // update the statement number
    void    setNumber(int num) {number = num;}

    // true if is a null statement
    bool    isNullStatement();

    // To/from SSA form
    virtual void   toSSAform(StatementSet& reachin, int memDepth,
        StatementSet& rs) = 0;
    virtual void fromSSAform(igraph& igm) = 0;

    // Propagate to this statement
    void propagateTo(int memDepth);

protected:
    virtual void doReplaceRef(Exp* from, Exp* to) = 0;
    bool doPropagateTo(int memDepth, Statement* def, bool twoRefs);
    bool calcMayAlias(Exp *e1, Exp *e2, int size);
    bool mayAlias(Exp *e1, Exp *e2, int size);
};

// Print the Statement poited to by p
std::ostream& operator<<(std::ostream& os, Statement* s);


#endif // DATAFLOW
