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
 *
 */

#ifndef _DATAFLOW_H_
#define _DATAFLOW_H_

#include <set>

class Exp;
class BasicBlock;
typedef BasicBlock *PBB;

/* Statements define values that are used in expressions.
 * They are akin to "definition" in the Dragon Book.
 */
class Statement {
protected:
    PBB pbb;  // contains a pointer to the enclosing BB
    std::set<Statement*> uses;
    std::set<Statement*> useBy;
public:

    Statement() : pbb(NULL) { }
    virtual ~Statement() { }

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

    // calculates the uses/useBy links for this statement
    virtual void calcUseLinks();

    // returns an expression that would be used to reference the value
    // defined by this statement
    virtual Exp* getLeft() = 0;

    // returns true if this statement uses the given expression
    virtual bool usesExp(Exp *e) = 0;

    // returns the statement which is used by this statement and has a
    // left like the given expression
    virtual Statement *findUse(Exp *e);

    // get the uses
    std::set<Statement*> &getUses() { return uses; }
    std::set<Statement*> &getUseBy() { return useBy; }

    // adds a new statement to the useBy set
    void addUseBy(Statement *stmt) { useBy.insert(stmt); }

    // get/set the enclosing BB
    PBB getBB() { return pbb; }
    void setBB(PBB bb) { pbb = bb; }

    // returns true if this statement can be propogated to all it's
    // uses and removed
    virtual bool canPropogateToAll() = 0;

    // propogates this statement to all it's uses, caller must remove
    virtual void propogateToAll() = 0;

    // replaces a use of the given statement with an expression
    virtual void replaceUse(Statement *use, Exp *with);

    // statements should be printable (for debugging)
    virtual void print(std::ostream &os) = 0;
    virtual void printWithLives(std::ostream& os) = 0;
    virtual void printWithUses(std::ostream& os) = 0;
    virtual void printAsUse(std::ostream &os) = 0;
    virtual void printAsUseBy(std::ostream &os) = 0;

protected:
    virtual void doReplaceUse(Statement *use, Exp *with) = 0;
    bool mayAlias(Exp *e1, Exp *e2, int size);
};

#endif // DATAFLOW
