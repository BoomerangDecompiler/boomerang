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

#include <set>
#include "exp.h"
#include "rtl.h"
#include "dataflow.h"

Def::Def() : rtl(NULL), it(std::list<Exp*>::iterator())
{
}

Def::Def(RTL *r, std::list<Exp*>::iterator i) : rtl(r), it(i)
{
	assert(getExp()->isAssign());
}

bool Def::operator<(const Def &other) const
{
	return getExp() < other.getExp();
}

Exp *Def::getLeft() const
{
	return getExp()->getSubExp1();
}

Exp *Def::getRight() const
{
	return getExp()->getSubExp2();
}

void Def::remove()
{
	rtl->getList().erase(it);
}

Use::Use(Exp* &e, bool isdef) : exp(e), def(isdef)
{
}

bool Use::operator<(const Use &other) const
{
	return exp < other.exp;	
}

void Use::subscript(SSACounts &counts)
{
	// get the expression reference
	Exp *eval = exp;
	if (eval->getOper() == opSubscript)
		eval = eval->getSubExp1();
	// get a subscript for this expression
	int count = counts.getSubscriptFor(eval);
	// if already subscripted, check the subscript
	if (exp->getOper() == opSubscript) {
		assert(exp->getSubExp2()->getOper() == opIntConst);
		int ocount = ((Const*)exp->getSubExp2())->getInt();
		if (count != ocount) {
			// update it
			((Const*)exp->getSubExp2())->setInt(count);
		}
	} else
		// subscript it
		exp = new Binary(opSubscript, exp, new Const(count));
}

void DefSet::insert(Def &d)
{
	defs.push_back(d);
}

void DefSet::merge(DefSet &other)
{
	for (DefSet::iterator it = other.begin(); it != other.end(); it++)
		defs.push_back(*it);
}

void DefSet::remove(DefSet &other)
{
	for (DefSet::iterator it = other.begin(); it != other.end(); it++)
		for (std::list<Def>::iterator it1 = defs.begin(); it1 != defs.end(); it1++)
			if (*(*it).getLeft() == *(*it1).getLeft())
				defs.erase(it1);
}

void DefSet::remove(Def &d)
{
	for (std::list<Def>::iterator it1 = defs.begin(); it1 != defs.end(); it1++)
		if (*d.getLeft() == *(*it1).getLeft())
			defs.erase(it1);
}

bool DefSet::find(Exp &left, Def &d)
{
	for (std::list<Def>::iterator it = defs.begin(); it != defs.end(); it++)
		if (*(*it).getLeft() == left) {
			d = *it;
			return true;
		}
	return false;
}

void UseSet::insert(Exp* &e)
{
	uses.push_back(Use(e));
}

void UseSet::merge(UseSet &other)
{
	for (UseSet::iterator it = other.begin(); it != other.end(); it++)
		uses.push_back(*it);
}

bool UseSet::find(Exp *e, Use &u)
{
	for (std::list<Use>::iterator it = uses.begin(); it != uses.end(); it++)
		if ((*it).expIs(e)) {
			u = *it;
			return true;
		}
	return false;
}

bool UseSet::contains(Exp *e)
{
	for (std::list<Use>::iterator it = uses.begin(); it != uses.end(); it++)
		if ((*it).expIs(e)) {
			return true;
		}
	return false;
}

void UseSet::remove(Exp *e)
{
	for (std::list<Use>::iterator it = uses.begin(); it != uses.end(); it++)
		if ((*it).expIs(e)) {
			it = uses.erase(it);
		}
}

int SSACounts::getSubscriptFor(Exp *e)
{
	for (std::list<std::pair<Exp*, int> >::iterator it = counts.begin(); it != counts.end(); it++)
		if (*e == *(*it).first) {
			return (*it).second;
		}
	std::pair<Exp*, int> p;
	p.first = e->clone();
	p.second = 0;
	counts.push_back(p);
	return 0;	
}

void SSACounts::incSubscriptFor(Exp *e)
{
	bool found = false;
	int n;
	// increase the max
	for (std::list<std::pair<Exp*, int> >::iterator it = maxes.begin(); it != maxes.end(); it++)
		if (*e == *(*it).first) {
			(*it).second++;
			found = true;
			n = (*it).second;
			break;
		}
	if (!found) {
		std::pair<Exp*, int> p;
		p.first = e->clone();
		p.second = 1;
		maxes.push_back(p);
		n = 1;
	}
	// make the max the current
	for (std::list<std::pair<Exp*, int> >::iterator cit = counts.begin(); cit != counts.end(); cit++)
		if (*e == *(*cit).first) {
			(*cit).second = n;
			return;
		}
	std::pair<Exp*, int> p;
	p.first = e->clone();
	p.second = n;
	counts.push_back(p);	
}

std::list<std::pair<Exp*, int> > SSACounts::maxes;

int SSACounts::getMaxSubscriptFor(Exp *e)
{
	for (std::list<std::pair<Exp*, int> >::iterator mit = maxes.begin(); mit != maxes.end(); mit++)
		if (*e == *(*mit).first) {
			return (*mit).second;
		}
	return 0;
}

void SSACounts::clearMaxes()
{
	maxes.clear();
}


void UseSet::setDef(Exp *e, bool b)
{
	for (std::list<Use>::iterator it = uses.begin(); it != uses.end(); it++)
		if ((*it).expIs(e)) {
			(*it).setDef(b);
			break;
		}
}
