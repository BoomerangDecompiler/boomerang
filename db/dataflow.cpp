/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       dataflow.cpp
 * OVERVIEW:   Implementation of the dataflow classes.
 *============================================================================*/

/*
 * $Revision$
 * 03 Jul 02 - Trent: Created
 * 09 Jan 03 - Mike: Untabbed, reformatted
 * 03 Feb 03 - Mike: cached dataflow (uses and usedBy)
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include "dataflow.h"
#include "exp.h"
#include "cfg.h"
#include "proc.h"
#include "boomerang.h"

#define VERBOSE Boomerang::get()->vFlag



// Flush the cached dataflow
void Statement::flushDataFlow() {
    if (uses) delete uses;
    uses = NULL;
    if (usedBy) delete usedBy;
    usedBy = NULL;
}

// Finds a use for a given expression
Statement *Statement::findUse(Exp *e) {
    updateUses();
    for (std::set<Statement*>::iterator it = uses->begin(); it != uses->end();
      it++) {
        if ((*it)->getLeft() && *(*it)->getLeft() == *e)
            return *it;
    }
    return NULL;
}

void Statement::calcUses(std::set<Statement*> &uses) {
    std::set<Statement*> liveIn;
    getLiveIn(liveIn);
    for (std::set<Statement*>::iterator it = liveIn.begin(); it != liveIn.end();
      it++) {
        assert(*it);
        Exp *left = (*it)->getLeft();
        if (left == NULL) continue;     // E.g. HLCall with no return value
        if (usesExp(left)) {
            uses.insert(*it);
        }
    }
}

void Statement::calcUsedBy(std::set<Statement*> &useBy) {
    if (getLeft() == NULL) return;
    std::set<Statement*> stmts;
    proc->getStatements(stmts);
    for (std::set<Statement*>::iterator it = stmts.begin(); it != stmts.end(); 
      it++) 
        if ((*it)->findUse(getLeft()) == this)
            useBy.insert(*it);
}

/* Goes through the definitions live at this expression and creates a
   link from any definition that is used by this expression to this 
   expression.
 */
void Statement::calcUseLinks() {
    updateUses();
    updateUsedBy();
}

// replace a use in this statement
void Statement::replaceUse(Statement *use) {
    if (VERBOSE) {
        std::cerr << "replace ";
        use->printAsUse(std::cerr);
        std::cerr << " in ";
        printAsUse(std::cerr);
        std::cerr << std::endl;
    }

    // Fix dataflow. Both directions need fixing
    //   Before           After
    //     (1)             (1)
    //     ^ |usedBy       ^ |
    // uses| v             | |
    //     (2) = *use  uses| |usedBy
    //     ^ |usedBy       | |
    // uses| v             | v
    //     (3) = this      (3)
    // Fix my ud chain; no longer using *use
    updateUses();
    std::set<Statement*>::iterator pos;
    pos = uses->find(use);
    if (pos != uses->end())
        uses->erase(pos);
    // However, we are now using whatever *use was using
    // Actually, it's possible *use had uses on it's left that will not be
    // propogated in the replacement, we have to remove these later - trent
    std::set<Statement*>::iterator ii;
    for (ii=use->uses->begin(); ii != use->uses->end(); ii++)
        uses->insert(*ii);
    // Fix the du chains that pointed in to the statement that will
    // be removed; they now point to this 
    use->updateUses();
    for (ii=use->uses->begin(); ii!= use->uses->end(); ii++) {
        (*ii)->updateUsedBy();
        pos = (*ii)->usedBy->find(use);
        if (pos != (*ii)->usedBy->end())
            (*ii)->usedBy->erase(pos);
        // They now point to this
        (*ii)->usedBy->insert(this);
    }

    // do the replacement
    doReplaceUse(use);

    // remove any uses that are not actually used by this statement
    bool change = true;
    while (change) {
        change = false;
        for (ii = uses->begin(); ii != uses->end(); ii++) {
            if (!(*ii)->getLeft() || !usesExp((*ii)->getLeft())) {
                assert(*ii); 
                if ((*ii)->usedBy) {
                    pos = (*ii)->usedBy->find(this);
                    if (pos != (*ii)->usedBy->end())
                        (*ii)->usedBy->erase(pos);
                }
                uses->erase(ii);
                change = true; 
                break;
            }
        }
    }
    if (VERBOSE) {
        std::cerr << "   after: ";
        printAsUse(std::cerr);
        std::cerr << std::endl;
    }
}

/* Get everything that is live before this assignment.
   To get the liveout, use getLiveIn(liveset), calcLiveOut(liveset).
 */
void Statement::getLiveIn(std::set<Statement*> &livein) {
    assert(pbb);
    pbb->getLiveInAt(this, livein);
}

bool Statement::mayAlias(Exp *e1, Exp *e2, int size) { 
    if (*e1 == *e2) return true;

    bool b = (calcAlias(e1, e2, size) && calcAlias(e2, e1, size)); 
    if (b && 0) {           // ??
        if (VERBOSE) {
            std::cerr << "mayAlias: *" << size << "* ";
            e1->print(std::cerr);
            std::cerr << " ";
            e2->print(std::cerr);
            std::cerr << " : yes" << std::endl;
        }
    }
    return b;
}

// returns true if e1 may alias e2
bool Statement::calcAlias(Exp *e1, Exp *e2, int size) {
    // currently only considers memory aliasing..
    if (!e1->isMemOf() || !e2->isMemOf()) {
        return false;
    }
    Exp *e1a = e1->getSubExp1();
    Exp *e2a = e2->getSubExp1();
    // constant memory accesses
    if (e1a->isIntConst() && 
        e2a->isIntConst()) {
        ADDRESS a1 = ((Const*)e1a)->getAddr();
        ADDRESS a2 = ((Const*)e2a)->getAddr();
        int diff = a1 - a2;
        if (diff < 0) diff = -diff;
        if (diff*8 >= size) return false;
    }
    // same left op constant memory accesses
    if (
      e1a->getArity() == 2 &&
      e1a->getOper() == e2a->getOper() &&
      e1a->getSubExp2()->isIntConst() &&
      e2a->getSubExp2()->isIntConst() &&
      *e1a->getSubExp1() == *e2a->getSubExp1()) {
        int i1 = ((Const*)e1a->getSubExp2())->getInt();
        int i2 = ((Const*)e2a->getSubExp2())->getInt();
        int diff = i1 - i2;
        if (diff < 0) diff = -diff;
        if (diff*8 >= size) return false;
    }
    // [left] vs [left +/- constant] memory accesses
    if (
      (e2a->getOper() == opPlus || e2a->getOper() == opMinus) &&
      *e1a == *e2a->getSubExp1() &&
      e2a->getSubExp2()->isIntConst()) {
        int i1 = 0;
        int i2 = ((Const*)e2a->getSubExp2())->getInt();
        int diff = i1 - i2;
        if (diff < 0) diff = -diff;
        if (diff*8 >= size) return false;
    }
    return true;
}

/* calculates the definitions that are "live" after this assignment.
   If the live set is empty, it will contain anything this assignment defines.
   If the live set is not empty, then it will not contain anything this
      assignment kills.
 */
void Statement::calcLiveOut(std::set<Statement*> &live) {
    // calculate kills
    killLive(live);
    // add this def
    if (getLeft() != NULL)
        live.insert(this);
}

/* 
 * Returns true if the statement can be propagated to all uses (and
 * therefore can be removed).
 * Returns false otherwise.
 *
 * To completely propagate a statement which does not kill any of it's
 * own uses it is sufficient to show that all the uses of the statement
 * are still live at the expression to be propagated to.
 *
 * A statement that kills one or more of it's own uses is slightly more 
 * complicated.  All the uses that are not killed must still be live at
 * the expression to be propagated to, but the uses that were killed must
 * be live at the expression to be propagated to after the statement is 
 * removed.  This is clearly the case if the only use killed by a 
 * statement is the same as the left hand side, however, if multiple uses
 * are killed a search must be conducted to ensure that no statement between
 * the source and the destination kills the other uses.  This is considered
 * too complex a task and is therefore defered for later experimentation.
 */
bool Statement::canPropagateToAll() {
    std::set<Statement*> tmp_uses;
    updateUses();
    tmp_uses = *uses;
    int nold = tmp_uses.size();
    killLive(tmp_uses);
    if (nold - tmp_uses.size() > 1) {
        // See comment above.
        if (VERBOSE) {
            std::cerr << "too hard failure in canPropogateToAll: ";
            printWithUses(std::cerr);
            std::cerr << std::endl;
        }
        return false;
    }

    updateUsedBy();
    if (usedBy->size() == 0) return false;

    for (std::set<Statement*>::iterator it = usedBy->begin();
      it != usedBy->end(); it++) {
        std::set<Statement*> in;
        (*it)->getLiveIn(in);
        // all uses must be live at the destination
        for (std::set<Statement*>::iterator iuse = tmp_uses.begin();
             iuse != tmp_uses.end(); iuse++) {
            if (in.find(*iuse) == in.end()) return false;
        }
        // no false uses must be created
        for (std::set<Statement*>::iterator ilive = in.begin();
             ilive != in.end(); ilive++) {
            if (*ilive == this) continue;
            Exp *left = (*ilive)->getLeft();
            if (left == NULL) return false;
            if (usesExp(left) && findUse(left) == NULL) return false;
        }
    }
    return true;
}

// assumes canPropagateToAll has returned true
// assumes this statement will be removed by the caller
void Statement::propagateToAll() {
    updateUsedBy();
    for (std::set<Statement*>::iterator it = usedBy->begin();
      it != usedBy->end(); it++) {
        Statement *e = *it;
        e->replaceUse(this);
    }
}

// Update the dataflow for this stmt. This stmt is about to be deleted.
//   Before           After
//     (1)           nothing!
//     ^ |usedBy       ^ |
// uses| v             | v
//     (2) = this      (2)
//     ^ |usedBy       | |
// uses| v             | v
//  nothing!         nothing!
//
void Statement::updateDfForErase() {
    std::set<Statement*>::iterator it;
    updateUses();
    for (it = uses->begin(); it != uses->end(); it++) {
        Statement* ss = *it;
        if (ss->usedBy == NULL) continue;
        std::set<Statement*>::iterator pos;
        pos = ss->usedBy->find(this);
        if (pos != ss->usedBy->end())
            // Erase this use of my definition, since I'm about to be deleted
            ss->usedBy->erase(pos);
    }
}

// Flush all dataflow for the whole procedure
void Statement::flushProc() {
    std::set<Statement*> stmts;
    proc->getStatements(stmts);
    std::set<Statement*>::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        (*it)->flushDataFlow();
    }
}

void Statement::printWithUses(std::ostream& os) {
    print(os);
    os << "   uses: ";
    updateUses();
    for (std::set<Statement*>::iterator it = uses->begin(); it != uses->end();
      it++) {
        (*it)->printAsUse(os);
        os << ", ";
    }
    os << "   used by: ";
    updateUsedBy();
    for (std::set<Statement*>::iterator it = usedBy->begin();
      it != usedBy->end(); it++) {
        (*it)->printAsUseBy(os);
        os << ", ";
    }
#if 0       // Note: if you change this, you need to update DataflowTest.cpp!
    os << "   live: ";
    std::set<Statement*> liveIn;
    getLiveIn(liveIn);
    for (std::set<Statement*>::iterator it = liveIn.begin(); it != liveIn.end();
      it++) {
        (*it)->print(os);
        os << ", ";
    }
#endif
}

