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

// finds a use for a given expression
Statement *Statement::findUse(Exp *e) {
    for (std::set<Statement*>::iterator it = uses.begin(); it != uses.end();
            it++)
        if (*(*it)->getLeft() == *e)
	    return *it;
    return NULL;
}

/* Goes through the definitions live at this expression and creates a
   link from any definition that is used by this expression to this 
   expression.
 */
void Statement::calcUseLinks()
{
    uses.clear();
    std::set<Statement*> live;
    getLiveIn(live);
    for (std::set<Statement*>::iterator it = live.begin(); it != live.end(); 
	    it++) {
        assert(*it);
        Exp *left = (*it)->getLeft();
        assert(left);
        if (usesExp(left)) {
	    uses.insert(*it);
            (*it)->addUseBy(this);
	}
    }
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
    // update statements that use this statement
    std::set<Statement*> tmp_useBy;
    for (std::set<Statement*>::iterator it = useBy.begin(); 
	     it != useBy.end(); it++) {
	    tmp_useBy.insert(*it);
    }
    useBy.clear();
    for (std::set<Statement*>::iterator it = tmp_useBy.begin(); 
	     it != tmp_useBy.end(); it++) {
	    (*it)->calcUseLinks();
    }
    // update statements used by this statement
    for (std::set<Statement*>::iterator it = uses.begin(); it != uses.end();
		    it++)
	    (*it)->getUseBy().erase(this);
    use->getUseBy().erase(this);
    calcUseLinks();
}

/* get everything that is live before this assignment.
   To get the liveout, use getLiveIn(liveset), calcLiveOut(liveset).
 */
void Statement::getLiveIn(std::set<Statement*> &livein)
{
	assert(pbb);
	pbb->getLiveInAt(this, livein);
}

// returns true if e1 may alias e2
bool Statement::mayAlias(Exp *e1, Exp *e2, int size)
{
//    std::cerr << "mayAlias: ";
//    e1->print(std::cerr);
//    std::cerr << " ";
//    e2->print(std::cerr);
//    std::cerr << " : ";
    // currently only considers memory aliasing..
    if (!e1->isMemOf() || !e2->isMemOf()) {
//	    std::cerr << "no" << std::endl;
	    return false;
    }
    // constant memory accesses
    if (e1->getSubExp1()->isAddrConst() && 
        e2->getSubExp1()->isAddrConst()) {
        ADDRESS a1 = ((Const*)e1->getSubExp1())->getAddr();
        ADDRESS a2 = ((Const*)e2->getSubExp1())->getAddr();
	int diff = a1 - a2;
	if (diff < 0) diff = -diff;
	if (diff*8 >= size) {
//	        std::cerr << "no" << std::endl;
		return false;
	}
    }
    // same register op constant memory accesses
    if (e1->getSubExp1()->getArity() == 2 &&
        e2->getSubExp1()->getArity() == 2 &&
	e1->getSubExp1()->getSubExp1()->isRegOf() &&
        e2->getSubExp1()->getSubExp1()->isRegOf() &&
	*e1->getSubExp1()->getSubExp1() == *e2->getSubExp1()->getSubExp1() &&
	e1->getSubExp1()->getSubExp2()->isIntConst() &&
	e2->getSubExp1()->getSubExp2()->isIntConst()) {
        int i1 = ((Const*)e1->getSubExp1())->getInt();
        int i2 = ((Const*)e2->getSubExp1())->getInt();
	int diff = i1 - i2;
	if (diff < 0) diff = -diff;
	if (diff*8 >= size) {
//	        std::cerr << "no" << std::endl;
		return false;
	}
    }
    // same register op constant / same register memory accesses
    if (e1->getSubExp1()->getArity() == 2 &&
        e2->getSubExp1()->isRegOf() &&
	e1->getSubExp1()->getSubExp1()->isRegOf() &&
	*e1->getSubExp1()->getSubExp1() == *e2->getSubExp1() &&
	e1->getSubExp1()->getSubExp2()->isIntConst()) {
        int i1 = ((Const*)e1->getSubExp1())->getInt();
	if (i1*8 >= size) {
//	        std::cerr << "no" << std::endl;
		return false;
	}
    }
    // same register / same register op constant memory accesses
    if (e2->getSubExp1()->getArity() == 2 &&
        e1->getSubExp1()->isRegOf() &&
	e2->getSubExp1()->getSubExp1()->isRegOf() &&
	*e2->getSubExp1()->getSubExp1() == *e1->getSubExp1() &&
	e2->getSubExp1()->getSubExp2()->isIntConst()) {
        int i2 = ((Const*)e2->getSubExp1())->getInt();
	if (i2*8 >= size) {
//	        std::cerr << "no" << std::endl;
		return false;
	}
    }
//    std::cerr << "yes" << std::endl;
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
    for (std::set<Statement*>::iterator it = uses.begin(); it != uses.end(); 
		    it++)
        tmp_uses.insert(*it);
    int nold = tmp_uses.size();
    killLive(tmp_uses);
    if (nold - tmp_uses.size() > 1) {
        // see comment.
	return false;
    }

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
    while(useBy.begin() != useBy.end()) {
	Statement *e = *useBy.begin();
        e->replaceUse(this);
	assert(useBy.begin() == useBy.end() || e != *useBy.begin());
    }
}


