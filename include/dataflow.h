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
 * 3 July 02 - Trent: created.
 *
 */

#ifndef _DATAFLOW_H_
#define _DATAFLOW_H_

#include <set>

class Exp;
class UseSet;
class SSACounts;

class Def {
private:
	RTL* rtl;
	std::list<Exp*>::iterator it;

public:
	Def();
	Def(RTL *r, std::list<Exp*>::iterator i);

	Exp *getLeft() const;
	Exp *getRight() const;
	Def &operator=(Def &other) { rtl = other.rtl; it = other.it; return *this; }
	bool operator<(const Def &other) const;
	Exp* &getExp() const { return *it; }

	void remove();
};

class Use {
private:
	Exp* &exp;
	bool def;

public:
	Use(Exp* &e, bool isdef = false);
	Use &operator=(Use &other) { exp = other.exp; return *this; }
	bool expIs(Exp *e) { return exp == e; }
	bool operator<(const Use &other) const;
	bool isDef() { return def; }
	void setDef(bool b) { def = b; }
	Exp* &getExp() { return exp; }
	void subscript(SSACounts &counts);
};

class DefSet {
private:
	std::list<Def> defs;

public:
	DefSet() {}

	void insert(Def &d);
	void merge(DefSet &other);
	void remove(DefSet &other);
	void remove(Def &d);
	bool find(Exp &left, Def &d);
	void clear() { defs.clear(); }

	typedef std::list<Def>::iterator iterator;
	iterator begin() { return defs.begin(); }
	iterator end() { return defs.end(); }
};

class UseSet {
private:
	std::list<Use> uses;
	
public:
	UseSet() {}

	void insert(Exp* &e);
	void merge(UseSet &other);

	bool find(Exp *e, Use &u);
	bool contains(Exp *e);
	void remove(Exp *e);
	bool empty() { return uses.begin() == uses.end(); }
	void setDef(Exp *e, bool b);	

	typedef std::list<Use>::iterator iterator;
	iterator begin() { return uses.begin(); }
	iterator end() { return uses.end(); }

	iterator erase(iterator it) { return uses.erase(it); }	
};

class SSACounts {
private:
	std::list<std::pair<Exp*, int> > counts;	
	static std::list<std::pair<Exp*, int> > maxes;

public:
	SSACounts() { }

	int getSubscriptFor(Exp *e);
	void incSubscriptFor(Exp *e);
	int getMaxSubscriptFor(Exp *e);
	void clearMaxes();
};

#endif // DATAFLOW
