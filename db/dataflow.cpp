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
 * 03 July 02 - Trent: Created
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

// finds a use for a given expression
Statement *Statement::findUse(Exp *e) {
    std::set<Statement*> uses;
    calcUses(uses);
    for (std::set<Statement*>::iterator it = uses.begin(); it != uses.end();
            it++)
        if (*(*it)->getLeft() == *e)
	    return *it;
    return NULL;
}

void Statement::calcUses(std::set<Statement*> &uses) {
    std::set<Statement*> live;
    getLiveIn(live);
    for (std::set<Statement*>::iterator it = live.begin(); it != live.end();
		    it++) {
        assert(*it);
        Exp *left = (*it)->getLeft();
        assert(left);
        if (usesExp(left)) {
	    uses.insert(*it);
	}
    }
}

void Statement::calcUseBy(std::set<Statement*> &useBy) {
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
void Statement::calcUseLinks()
{
    std::set<Statement*> uses;
    calcUses(uses);
}

// replace a use in this statement
void Statement::replaceUse(Statement *use)
{
    std::cerr << "replace ";
    use->printAsUse(std::cerr);
    std::cerr << " in ";
    printAsUse(std::cerr);
    std::cerr << std::endl;

    // do the replacement
    doReplaceUse(use);
    std::cerr << "   after: ";
    printAsUse(std::cerr);
    std::cerr << std::endl;
}

/* get everything that is live before this assignment.
   To get the liveout, use getLiveIn(liveset), calcLiveOut(liveset).
 */
void Statement::getLiveIn(std::set<Statement*> &livein)
{
	assert(pbb);
	pbb->getLiveInAt(this, livein);
}

bool Statement::mayAlias(Exp *e1, Exp *e2, int size) { 
    if (*e1 == *e2) return true;

    bool b = (calcAlias(e1, e2, size) && calcAlias(e2, e1, size)); 
    if (b && 0) {
        std::cerr << "mayAlias: *" << size << "* ";
        e1->print(std::cerr);
        std::cerr << " ";
        e2->print(std::cerr);
        std::cerr << " : yes" << std::endl;
    }
    return b;
}

// returns true if e1 may alias e2
bool Statement::calcAlias(Exp *e1, Exp *e2, int size)
{
    // currently only considers memory aliasing..
    if (!e1->isMemOf() || !e2->isMemOf()) {
	    return false;
    }
    Exp *e1a = e1->getSubExp1();
    Exp *e2a = e2->getSubExp1();
    // constant memory accesses
    if (e1a->isAddrConst() && 
        e2a->isAddrConst()) {
        ADDRESS a1 = ((Const*)e1a)->getAddr();
        ADDRESS a2 = ((Const*)e2a)->getAddr();
	int diff = a1 - a2;
	if (diff < 0) diff = -diff;
	if (diff*8 >= size) {
		return false;
	}
    }
    // same left op constant memory accesses
    if (e1a->getArity() == 2 &&
        e1a->getOper() == e2a->getOper() &&
	e1a->getSubExp2()->isIntConst() &&
	e2a->getSubExp2()->isIntConst() &&
	*e1a->getSubExp1() == *e2a->getSubExp1()) {
        int i1 = ((Const*)e1a->getSubExp2())->getInt();
        int i2 = ((Const*)e2a->getSubExp2())->getInt();
	int diff = i1 - i2;
	if (diff < 0) diff = -diff;
	if (diff*8 >= size) {
		return false;
	}
    }
    // [left] vs [left +/- constant] memory accesses
    if ((e2a->getOper() == opPlus || e2a->getOper() == opMinus) &&
        *e1a == *e2a->getSubExp1() &&
	e2a->getSubExp2()->isIntConst()) {
        int i1 = 0;
        int i2 = ((Const*)e2a->getSubExp2())->getInt();
	int diff = i1 - i2;
	if (diff < 0) diff = -diff;
	if (diff*8 >= size) {
		return false;
	}
    }
    return true;
}

/* calculates the definitions that are "live" after this assignment.
   If the live set is empty, it will contain anything this assignment defines.
   If the live set is not empty, then it will not contain anything this
      assignment kills.
 */
void Statement::calcLiveOut(std::set<Statement*> &live)
{
	// calculate kills
        killLive(live);
	// add this def
	if (getLeft() != NULL)
	    live.insert(this);
}

/* 
 * Returns true if the statement can be propogated to all uses (and
 * therefore can be removed).
 * Returns false otherwise.
 *
 * To completely propogate a statement which does not kill any of it's
 * own uses it is sufficient to show that all the uses of the statement
 * are still live at the expression to be propogated to.
 *
 * A statement that kills one or more of it's own uses is slightly more 
 * complicated.  All the uses that are not killed must still be live at
 * the expression to be propogated to, but the uses that were killed must
 * be live at the expression to be propogated to after the statement is 
 * removed.  This is clearly the case if the only use killed by a 
 * statement is the same as the left hand side, however, if multiple uses
 * are killed a search must be conducted to ensure that no statement between
 * the source and the destination kills the other uses.  This is considered
 * too complex a task and is therefore defered for later experimentation.
 */
bool Statement::canPropogateToAll()
{
    std::set<Statement*> tmp_uses;
    calcUses(tmp_uses);
    int nold = tmp_uses.size();
    killLive(tmp_uses);
    if (nold - tmp_uses.size() > 1) {
        // see comment.
	return false;
    }

    std::set<Statement*> useBy;
    calcUseBy(useBy);

    if (useBy.size() == 0) return false;

    for (std::set<Statement*>::iterator it = useBy.begin(); it != useBy.end(); 
		    it++) {
	std::set<Statement*> in;
	(*it)->getLiveIn(in);
	// all uses must be live at the destination
	for (std::set<Statement*>::iterator iuse = tmp_uses.begin();
	         iuse != tmp_uses.end(); iuse++)
	    if (in.find(*iuse) == in.end()) return false;
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

// assumes canPropogateToAll has returned true
// assumes this statement will be removed by the caller
void Statement::propogateToAll()
{
    std::set<Statement*> useBy;
    calcUseBy(useBy);
    for (std::set<Statement*>::iterator it = useBy.begin(); it != useBy.end(); 
	 it++) {
	Statement *e = *it;
        e->replaceUse(this);
    }
}


