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



// Finds a definition for a given location
// MVE: Is this useful?
Statement *Statement::findDef(Exp *e) {
    StmtSetIter it;
    for (Statement* s = uses.getFirst(it); s; s = uses.getNext(it)) {
        if (s->getLeft() && *s->getLeft() == *e)
            return s;
    }
    return NULL;
}

// From the set of reaching statements, find those which assign to a location
// that I use (i.e. are in my RHS, or in a m[] on my LHS), parameter, etc
// This is the set of statements that this statement uses (relies on)
// Also calculates usedBy
void Statement::calcUses(StatementSet &uses) {
    StatementSet reachIn;
    getReachIn(reachIn);
    StmtSetIter it;
    for (Statement* s = reachIn.getFirst(it); s; s = reachIn.getNext(it)) {
        assert(s);
        Exp *left = s->getLeft();
        if (left == NULL) continue;     // E.g. HLCall with no return value
        if (usesExp(left)) {
            uses.insert(s);             // This statement uses s
            s->usedBy.insert(this);     // s is usedBy this Statement
        }
    }
}

// From all statements in this proc, find those which use my LHS
// These statements rely on my assignment; this statement is usedBy these
void Statement::calcUsedBy(StatementSet &usedBy) {
#if 0       // Done in calcUses now
    if (getLeft() == NULL) return;
    StatementList stmts;
    proc->getStatements(stmts);
    StmtListIter it;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        if (s->findDef(getLeft()) == this)
            usedBy.insert(s);
    }
#endif
}

/* Goes through the definitions which reach this expression and creates a
   link from any definition that is used by this expression to this 
   expression.
 */
void Statement::calcUseLinks() {
    calcUses(uses);             // Does both uses and usedBy now
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
    uses.remove(use);
    // However, we are now using whatever *use was using
    // Actually, it's possible *use had uses on it's left that will not be
    // propogated in the replacement, we have to remove these later - trent
    uses.make_union(use->uses);
    // Fix the du chains that pointed in to the statement that will
    // be removed; they now point to this 
    StmtSetIter ii;
    StatementSet& useUses = use->uses;
    for (Statement* s = useUses.getFirst(ii); s; s = useUses.getNext(ii)) {
        s->usedBy.remove(use);
        // They now point to this
        s->usedBy.insert(this);
    }

    // do the replacement
    doReplaceUse(use);

    // remove any uses that are not actually used by this statement
    bool change = true;
    while (change) {
        change = false;
        for (Statement* s = uses.getFirst(ii); s; s = uses.getNext(ii)) {
            if (!s->getLeft() || !usesExp(s->getLeft())) {
                assert(s); 
                s->usedBy.remove(this);
                uses.remove(s);
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

/* Get everything that reaches this assignment.
   To get the reachout, use getReachIn(reachset), calcReachOut(reachset).
 */
void Statement::getReachIn(StatementSet &reachin) {
    assert(pbb);
    pbb->getReachInAt(this, reachin);
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

/* calculates the definitions that are not killed by this assignment.
   If the reach set is empty, it will contain anything this assignment defines.
   If the reach set is not empty, then it will not contain anything this
      assignment kills.
 */
void Statement::calcReachOut(StatementSet &reach) {
    // calculate kills
    killReach(reach);
    // add this def
    if (getLeft() != NULL)
        reach.insert(this);
}

/* 
 * Returns true if the statement can be propagated to all uses (and
 * therefore can be removed).
 * Returns false otherwise.
 *
 * To completely propagate a statement which does not kill any of its
 * own uses it is sufficient to show that all the uses of the statement
 * are still available at the expression to be propagated to.
 *
 * A statement that kills one or more of its own uses is slightly more 
 * complicated.  All the uses that are not killed must still be available at
 * the expression to be propagated to, but the uses that were killed must
 * be available at the expression to be propagated to after the statement is 
 * removed.  This is clearly the case if the only use killed by a 
 * statement is the same as the left hand side, however, if multiple uses
 * are killed a search must be conducted to ensure that no statement between
 * the source and the destination kills the other uses.  This is considered
 * too complex a task and is therefore defered for later experimentation.
 */
bool Statement::canPropagateToAll() {
    StatementSet tmp_uses;
    tmp_uses = uses;
    int nold = tmp_uses.size();
    killReach(tmp_uses);
    if (nold - tmp_uses.size() > 1) {
        // See comment above.
        if (VERBOSE) {
            std::cerr << "too hard failure in canPropogateToAll: ";
            printWithUses(std::cerr);
            std::cerr << std::endl;
        }
        return false;
    }

    if (usedBy.size() == 0) {
        return false;
    }

    StmtSetIter it;
    for (Statement* sdest = usedBy.getFirst(it); sdest;
      sdest = usedBy.getNext(it)) {
        StatementSet destIn;
        sdest->getAvailIn(destIn);
        // all uses must be available at the destination
        StmtSetIter ituse;
        for (Statement* tuse = tmp_uses.getFirst(ituse); tuse;
          tuse = tmp_uses.getNext(ituse)) {
            if (!destIn.exists(tuse)) {
                return false;
            }
        }
        // no false uses must be created
        StmtSetIter iav;
        for (Statement* savail = destIn.getFirst(iav); savail;
          savail = destIn.getNext(iav)) {
            if (savail == this) continue;
            Exp *left = savail->getLeft();
            if (left == NULL) {
                return false;
            }
            if (usesExp(left) && findDef(left) == NULL) {
                return false;
            }
        }
    }
    return true;
}

// assumes canPropagateToAll has returned true
// assumes this statement will be removed by the caller
void Statement::propagateToAll() {
    StmtSetIter it;
    for (Statement* s = usedBy.getFirst(it); s; s = usedBy.getNext(it)) {
        s->replaceUse(this);
    }
}

// Update the dataflow for this stmt. This stmt is about to be deleted.
// Don't assume the statement being erased has no dataflow; it could be
// of the form x := x
// 
//   Before           After
//     (1)             (1)
//     ^ |usedBy       ^ |
// uses| v             | |
//     (2) = this      | |
//     ^ |usedBy       | |
// uses| v             | v
//     (3)             (3)
//
void Statement::updateDfForErase() {
    // First fix the down arrows (usedBy)
    StmtSetIter it, uu;
    for (Statement* ss = uses.getFirst(it); ss; ss = uses.getNext(it)) {
        // it is iterating through the (1) set
        // This is the usedBy entry from this (1) to (2)
        // Erase this use of my definition, since I'm about to be deleted
        ss->usedBy.remove(this);
        // The use from this (1) to each (3) comes next
        for (Statement* su = usedBy.getFirst(uu); su;
          su = usedBy.getNext(uu))
            ss->usedBy.insert(su);        // This (3) usedby this (1)
    }
    // Next, fix the up arrows (uses)
    for (Statement* ss = usedBy.getFirst(it); ss; ss = usedBy.getNext(it)) {
        // it is iterating through the (3) set
        // This is the uses entry from this (3) to (2)
        // Erase this def of my rhs, since I'm about to be deleted
        ss->uses.remove(this);
        // The uses from this (3) to each (1) comes next
        for (Statement* suu = uses.getFirst(uu); suu; suu = uses.getNext(uu))
            ss->uses.insert(suu);        // This (3) uses this (1)
    }
}

void Statement::printWithUses(std::ostream& os) {
    print(os);
    os << "   uses: ";
    StmtSetIter it;
    for (Statement* s = uses.getFirst(it); s; s = uses.getNext(it)) {
        s->printAsUse(os);
        os << ", ";
    }
    os << "   used by: ";
    for (Statement* s = usedBy.getFirst(it); s; s = usedBy.getNext(it)) {
        s->printAsUseBy(os);
        os << ", ";
    }
#if 0       // Note: if you change this, you need to update DataflowTest.cpp!
    os << "   reach: ";
    StatementSet reachIn;
    getReachIn(reachIn);
    for (Statement* s = reachIn.getFirst(it); s; s = reachIn.getNext(it)) {
        s->print(os);
        os << ", ";
    }
#endif
}

/*==============================================================================
 * FUNCTION:        operator<<
 * OVERVIEW:        Output operator for Statement*
 *                  Just makes it easier to use e.g. std::cerr << myStmtStar
 * PARAMETERS:      os: output stream to send to
 *                  p: ptr to Statement to print to the stream
 * RETURNS:         copy of os (for concatenation)
 *============================================================================*/
std::ostream& operator<<(std::ostream& os, Statement* s) {
    s->print(os);
    return os;
}

//
// StatementSet methods
//

// Make this set the union of itself and other
void StatementSet::make_union(StatementSet& other) {
    StmtSetIter it;
    for (it = other.sset.begin(); it != other.sset.end(); it++) {
        sset.insert(*it);
    }
}

// Make this set the difference of itself and other
void StatementSet::make_diff(StatementSet& other) {
    StmtSetIter it;
    for (it = other.sset.begin(); it != other.sset.end(); it++) {
        sset.erase(*it);
    }
}

// Make this set the intersection of itself and other
void StatementSet::make_isect(StatementSet& other) {
    StmtSetIter it, ff;
    for (it = sset.begin(); it != sset.end(); it++) {
        ff = other.sset.find(*it);
        if (ff == other.sset.end())
            // Not in both sets
            sset.erase(it);
    }
}

Statement* StatementSet::getFirst(StmtSetIter& it) {
    it = sset.begin();
    if (it == sset.end())
        // No elements
        return NULL;
    return *it;         // Else return the first element
}

Statement* StatementSet::getNext(StmtSetIter& it) {
    if (++it == sset.end())
        // No more elements
        return NULL;
    return *it;         // Else return the next element
}

// Remove this set. Return false if it was not found
bool StatementSet::remove(Statement* s) {
    if (sset.find(s) != sset.end()) {
        sset.erase(s);
        return true;
    }
    return false;
}

// Find s in this Statement set. Return true if found
bool StatementSet::exists(Statement* s) {
    StmtSetIter it = sset.find(s);
    return (it != sset.end());
}



//
// StatementList methods
//

bool StatementList::remove(Statement* s) {
    for (StmtListIter it = slist.begin(); it != slist.end(); it++) {
        if (*it == s) {
            slist.erase(it);
            return true;
        }
    }
    return false;
}

void StatementList::append(StatementList& sl) {
    for (StmtListIter it = sl.slist.begin(); it != sl.slist.end(); it++) {
        slist.push_back(*it);
    }
}

void StatementList::append(StatementSet& ss) {
    StmtSetIter it;
    for (Statement* s  = ss.getFirst(it); s; s = ss.getNext(it)) {
        slist.push_back(s);
    }
}

Statement* StatementList::getFirst(StmtListIter& it) {
    it = slist.begin();
    if (it == slist.end())
        // No elements
        return NULL;
    return *it;         // Else return the first element
}

Statement* StatementList::getNext(StmtListIter& it) {
    if (++it == slist.end())
        // No more elements
        return NULL;
    return *it;         // Else return the next element
}

