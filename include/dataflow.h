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
 */

#ifndef _DATAFLOW_H_
#define _DATAFLOW_H_

#include <set>

class Exp;
class BasicBlock;
typedef BasicBlock *PBB;
class Prog;
class UserProc;
class Type;

/* Statements define values that are used in expressions.
 * They are akin to "definition" in the Dragon Book.
 */
class Statement {
protected:
    PBB pbb;  // contains a pointer to the enclosing BB
    UserProc *proc; // procedure containing this statement
    // The following pointers are initially null, but if non null are
    // considered valid
    std::set<Statement*>* uses;          // ud chain: my uses' defs
    std::set<Statement*>* usedBy;        // du chain: my def's uses
public:

    Statement() : pbb(NULL), proc(NULL), uses(NULL), usedBy(NULL) { }
    virtual ~Statement() {
        if (uses) delete uses;
        if (usedBy) delete usedBy;
    }

    void setProc(UserProc *p) { proc = p; }

    // calculates the live set after this statement
    virtual void calcLiveOut(std::set<Statement*> &liveout);

    // gets the live set before this statement
    virtual void getLiveIn(std::set<Statement*> &livein);

    // removes any statement from the live set which is killed by this 
    // statement
    virtual void killLive(std::set<Statement*> &live) = 0;

    // creates a set of statements that are killed by this statement
    // and have no uses
    virtual void getDeadStatements(std::set<Statement*> &dead) = 0;

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
    virtual Statement *findUse(Exp *e);

    // 
    // get my uses' definitions (ud chain)
    // 
    void updateUses() {
        if (uses == NULL) {
            uses = new std::set<Statement*>; calcUses(*uses); 
        } 
    }
    void calcUses(std::set<Statement*> &uses);
    int getNumUses() { 
        updateUses();
	    return uses->size(); 
    }
 
    // 
    // usedBy: du chain (my def's uses)
    //
    void updateUsedBy() {
        if (usedBy == NULL) {
            usedBy = new std::set<Statement*>; calcUsedBy(*usedBy); } }
    void calcUsedBy(std::set<Statement*> &usedBy);
    int getNumUseBy() {
        updateUsedBy(); return usedBy->size(); }

    // update my data flow (I'm about to be deleted)
    void updateDfForErase();

    // get/set the enclosing BB
    PBB getBB() { return pbb; }
    void setBB(PBB bb) { pbb = bb; }

    // returns true if this statement can be propagated to all it's
    // uses and removed
    virtual bool canPropagateToAll();

    // propagates this statement to all it's uses, caller must remove
    virtual void propagateToAll();

    // replaces a use of the given statement with an expression
    virtual void replaceUse(Statement *use);

    // Flush the cached dataflow
    void flushDataFlow();

    // flush all cached ud/du chains for the whole procedure
    void flushProc();

    // statements should be printable (for debugging)
    virtual void print(std::ostream &os) = 0;
    virtual void printWithUses(std::ostream& os) = 0;
    virtual void printAsUse(std::ostream &os) = 0;
    virtual void printAsUseBy(std::ostream &os) = 0;

    // inline any constants in the statement
    virtual void inlineConstants(Prog *prog) = 0;

    // general search and replace
    virtual void searchAndReplace(Exp *search, Exp *replace) = 0;

    // update the type information for an expression in this statement
    virtual Type *updateType(Exp *e, Type *curType) = 0;

protected:
    virtual void doReplaceUse(Statement *use) = 0;
    bool calcAlias(Exp *e1, Exp *e2, int size);
    bool mayAlias(Exp *e1, Exp *e2, int size);
};

#endif // DATAFLOW
