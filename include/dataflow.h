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

class Exp;
class BasicBlock;
typedef BasicBlock *PBB;
class Prog;
class UserProc;
class Type;
class Statement;

// A class to implement sets of statements
// We may choose to implement these very differently one day
typedef std::set<Statement*>::iterator StmtSetIter;
class StatementSet {
    std::set<Statement*> sset;          // For now, use use standard sets

public:
    void make_union(StatementSet& other);    // Set union
    void make_diff (StatementSet& other);    // Set difference
    void make_isect(StatementSet& other);    // Set intersection

    int size() {return sset.size();}        // Number of elements
    Statement* getFirst(StmtSetIter& it);   // Get the first Statement
    Statement* getNext (StmtSetIter& it);   // Get next
    void insert(Statement* s) {sset.insert(s);} // Insertion
    bool remove(Statement* s);             // Removal; rets false if not found
    bool exists(Statement* s);             // Search; returns false if not found
    void clear() {sset.clear();}           // Clear the set
    bool operator==(const StatementSet& o) const // Compare
        { return sset == o.sset;}
    bool operator!=(const StatementSet& o) const // Compare
        { return sset != o.sset;}
};

// Ugh - we also need lists of Statements for the internal statements
typedef std::list<Statement*>::iterator StmtListIter;
class StatementList {
    std::list<Statement*> slist;          // For now, use use standard list

public:
    int size() {return slist.size();}        // Number of elements
    Statement* getFirst(StmtListIter& it);   // Get the first Statement
    Statement* getNext (StmtListIter& it);   // Get next
    void append(Statement* s) {slist.push_back(s);} // Insert at end
    void append(StatementList& sl);         // Append whole StatementList
    void append(StatementSet& sl);          // Append whole StatementSet
    bool remove(Statement* s);              // Removal; rets false if not found
    bool exists(Statement* s);  // Find; returns false if not found
};


/* Statements define values that are used in expressions.
 * They are akin to "definition" in the Dragon Book.
 */
class Statement {
protected:
    PBB pbb;  // contains a pointer to the enclosing BB
    UserProc *proc; // procedure containing this statement
    // The following pointers are initially null, but if non null are
    // considered valid
    StatementSet uses;          // ud chain: my uses' defs
    StatementSet usedBy;        // du chain: my def's uses
public:

    Statement() : pbb(NULL), proc(NULL) { }
    virtual ~Statement() {
    }

    void setProc(UserProc *p) { proc = p; }

    // calculates the reaching definitions set after this statement
    virtual void calcReachOut(StatementSet &reachout);

    // gets the reaching definitions set before this statement
    virtual void getReachIn(StatementSet &reachin);

    // removes any statement from the reaching definitions set which is
    // killed by this statement
    virtual void killReach(StatementSet &reach) = 0;

    // get the available definitions (not reassigned on any path) before
    // this statement
    // NOTE: needs separate calculation! For now, use Trent's approximation
    virtual void getAvailIn(StatementSet& availin) {getReachIn(availin);}

    // creates a set of statements that are killed by this statement
    // and have no uses
    virtual void getDeadStatements(StatementSet &dead) = 0;

    // calculates the uses/usedBy links for this statement
    virtual void calcUseLinks();

    // returns an expression that would be used to reference the value
    // defined by this statement
    virtual Exp* getLeft() = 0;

    // returns a type for the left
    virtual Type* getLeftType() = 0;

    // returns an expression that would be used to replace this statement
    // in a use
    virtual Exp* getRight() = 0;

    // returns true if this statement uses the given expression
    virtual bool usesExp(Exp *e) = 0;

    // returns the statement which is used by this statement and has a
    // left like the given expression
    // MVE: is this useful?
    virtual Statement *findDef(Exp *e);

    // 
    // get my uses' definitions (ud chain)
    // 
    void calcUses(StatementSet &uses);
    int getNumUses() { return uses.size(); }
    StatementSet &getUses() { return uses; }
    void clearUses() {uses.clear(); usedBy.clear();}
 
    // 
    // usedBy: du chain (my def's uses)
    //
    void calcUsedBy(StatementSet &usedBy);
    int getNumUseBy() { return usedBy.size(); }

    // update my data flow (I'm about to be deleted)
    void updateDfForErase();

    // get/set the enclosing BB
    PBB getBB() { return pbb; }
    void setBB(PBB bb) { pbb = bb; }

    // returns true if this statement can be propagated to all its
    // uses and removed
    virtual bool canPropagateToAll();

    // propagates this statement to all its uses, caller must remove
    virtual void propagateToAll();

    // replaces a use of the given statement with an expression
    virtual void replaceUse(Statement *use);

    // statements should be printable (for debugging)
    virtual void print(std::ostream &os) = 0;
    virtual void printWithUses(std::ostream& os);
    virtual void printAsUse(std::ostream &os) = 0;
    virtual void printAsUseBy(std::ostream &os) = 0;

    // inline / decode any constants in the statement
    virtual void processConstants(Prog *prog) = 0;

    // general search
    virtual bool search(Exp *search, Exp *&result) = 0;

    // general search and replace
    virtual void searchAndReplace(Exp *search, Exp *replace) = 0;

    // update the type information for an expression in this statement
    virtual Type *updateType(Exp *e, Type *curType) = 0;

protected:
    virtual void doReplaceUse(Statement *use) = 0;
    bool calcAlias(Exp *e1, Exp *e2, int size);
    bool mayAlias(Exp *e1, Exp *e2, int size);
};

// Print the Statement poited to by p
std::ostream& operator<<(std::ostream& os, Statement* s);

#endif // DATAFLOW
