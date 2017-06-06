#pragma once

/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/***************************************************************************/ /**
 * \file       managed.h
 * \brief   Definition of "managed" classes such as InstructionSet, which feature makeUnion etc
 * CLASSES:        InstructionSet
 *                AssignSet
 *                StatementList
 *                StatementVec
 *                LocationSet
 *                ConnectionGraph
 *==============================================================================================*/

#include "db/exphelp.h" // For lessExpStar

#include <list>
#include <set>
#include <vector>
#include <memory>

class Instruction;
class Assign;
class Exp;
using SharedExp = std::shared_ptr<Exp>;
class RefExp;
class Cfg;
class LocationSet;
class QTextStream;

// A class to implement sets of statements
class InstructionSet : public std::set<Instruction *>
{
public:
	~InstructionSet() {}
	void makeUnion(InstructionSet& other);       ///< Set union
	void makeDiff(InstructionSet& other);        ///< Set difference
	void makeIsect(InstructionSet& other);       ///< Set intersection
	bool isSubSetOf(InstructionSet& other);      ///< Subset relation

	bool remove(Instruction *s);                 ///< Removal; rets false if not found
	bool removeIfDefines(SharedExp given);       ///< Remove if given exp is defined
	bool removeIfDefines(InstructionSet& given); ///< Remove if any given is def'd
	bool exists(Instruction *s);                 ///< Search; returns false if !found
	bool definesLoc(SharedExp loc);              ///< Search; returns true if any

	// statement defines loc
	bool operator<(const InstructionSet& o) const; ///< Compare if less
	void print(QTextStream& os) const;             ///< Print to os
	void printNums(QTextStream& os);               ///< Print statements as numbers
	const char *prints();                          ///< Print to string (for debug)
	void dump();                                   ///< Print to standard error for debugging
};                                                 ///< class InstructionSet

// As above, but the Statements are known to be Assigns, and are sorted sensibly
class AssignSet : public std::set<Assign *, lessAssign>
{
public:
	~AssignSet() {}
	void makeUnion(AssignSet& other);         ///< Set union
	void makeDiff(AssignSet& other);          ///< Set difference
	void makeIsect(AssignSet& other);         ///< Set intersection
	bool isSubSetOf(AssignSet& other);        ///< Subset relation
	bool remove(Assign *a);                   ///< Removal; rets false if not found
	bool removeIfDefines(SharedExp given);    ///< Remove if given exp is defined
	bool removeIfDefines(AssignSet& given);   ///< Remove if any given is def'd
	bool exists(Assign *s);                   ///< Search; returns false if !found
	bool definesLoc(SharedExp loc) const;     ///< Search; returns true if any assignment defines loc
	Assign *lookupLoc(SharedExp loc);         ///< Search for loc on LHS, return ptr to Assign if found

	bool operator<(const AssignSet& o) const; ///< Compare if less

	void print(QTextStream& os) const;        ///< Print to os
	void printNums(QTextStream& os);          ///< Print statements as numbers
	char *prints();                           ///< Print to string (for debug)
	void dump();                              ///< Print to standard error for debugging
};                                            ///< class AssignSet

class StatementList : public std::list<Instruction *>
{
public:
	~StatementList() {}

	// A special intersection operator; this becomes the intersection of StatementList a (assumed to be a list of
	// Assignment*s) with the LocationSet b.
	// Used for calculating returns for a CallStatement
	void makeIsect(StatementList& a, LocationSet& b);

	void append(Instruction *s) { push_back(s); } ///< Insert at end
	void append(StatementList& sl);               ///< Append whole StatementList
	void append(InstructionSet& sl);              ///< Append whole InstructionSet
	bool remove(Instruction *s);                  ///< Removal; rets false if not found
	void removeDefOf(SharedExp loc);              ///< Remove definitions of loc

	// This one is needed where you remove in the middle of a loop
	// Use like this: it = mystatementlist.erase(it);
	bool exists(Instruction *s);            ///< Search; returns false if not found
	char *prints();                         ///< Print to string (for debugging)
	void dump();                            ///< Print to standard error for debugging
	void makeCloneOf(StatementList& o);     ///< Make this a clone of o

	/// Return true if loc appears on the left of any statements in this list
	/// Note: statements in this list are assumed to be assignments
	bool existsOnLeft(const SharedExp& loc) const; ///< True if loc exists on the LHS of any Assignment in this list
	Assignment *findOnLeft(SharedExp loc) const;         ///< Return the first stmt with loc on the LHS
};

class StatementVec
{
	std::vector<Instruction *> svec; // For now, use use standard vector

public:
	typedef std::vector<Instruction *>::iterator           iterator;
	typedef std::vector<Instruction *>::reverse_iterator   reverse_iterator;

	size_t size() const { return svec.size(); } ///< Number of elements
	iterator begin() { return svec.begin(); }
	iterator end() { return svec.end(); }
	reverse_iterator rbegin() { return svec.rbegin(); }
	reverse_iterator rend() { return svec.rend(); }

	// Get/put at position idx (0 based)
	Instruction *operator[](size_t idx) { return svec[idx]; }
	void putAt(int idx, Instruction *s);
	iterator remove(iterator it);
	char *prints(); ///< Print to string (for debugging)
	void dump();    ///< Print to standard error for debugging
	void printNums(QTextStream& os);

	void clear() { svec.clear(); }
	bool operator==(const StatementVec& o) const ///< Compare if equal
	{
		return svec == o.svec;
	}

	bool operator<(const StatementVec& o) const ///< Compare if less
	{
		return svec < o.svec;
	}

	void append(Instruction *s) { svec.push_back(s); }
	void erase(iterator it) { svec.erase(it); }
};

// For various purposes, we need sets of locations (registers or memory)
class LocationSet
{
	// We use a standard set, but with a special "less than" operator so that the sets are ordered
	// by expression value. If this is not done, then two expressions with the same value (say r[10])
	// but that happen to have different addresses (because they came from different statements)
	// would both be stored in the set (instead of the required set behaviour, where only one is stored)
	std::set<SharedExp, lessExpStar> lset;

public:
	typedef std::set<SharedExp, lessExpStar>::iterator         iterator;
	typedef std::set<SharedExp, lessExpStar>::const_iterator   const_iterator;

	LocationSet() {}                              ///< Default constructor
	~LocationSet() {}                             ///< virtual destructor kills warning
	LocationSet(const LocationSet& o);            ///< Copy constructor

	LocationSet& operator=(const LocationSet& o); ///< Assignment

	void makeUnion(LocationSet& other);           ///< Set union
	void makeDiff(LocationSet& other);            ///< Set difference

	void clear() { lset.clear(); }                ///< Clear the set

	iterator begin() { return lset.begin(); }
	iterator end() { return lset.end(); }
	const_iterator begin() const { return lset.begin(); }
	const_iterator end() const { return lset.begin(); }

	void insert(SharedExp loc) { lset.insert(loc); }  ///< Insert the given location
	void remove(SharedExp loc);                       ///< Remove the given location

	void remove(iterator ll) { lset.erase(ll); } ///< Remove location, given iterator
	void removeIfDefines(InstructionSet& given); ///< Remove locs defined in given

	size_t size() const { return lset.size(); }  ///< Number of elements
	bool operator==(const LocationSet& o) const; ///< Compare
	void substitute(Assign& a);                  ///< Substitute the given assignment to all
	void print(QTextStream& os) const;           ///< Print to os
	char *prints() const;                        ///< Print to string for debugging
	void dump() const;
	void printDiff(LocationSet *o) const;        ///< Diff 2 location sets to LOG_STREAM()
	bool exists(SharedExp e) const;              ///< Return true if the location exists in the set
	SharedExp findNS(SharedExp e);               ///< Find location e (no subscripts); nullptr if not found
	bool existsImplicit(SharedExp e) const;      ///< Search for location e{-} or e{0} (e has no subscripts)

	/// Return an iterator to the found item (or end() if not). Only really makes sense if e has a wildcard
	iterator find(SharedExp e)       { return lset.find(e); }
	const_iterator find(SharedExp e) const { return lset.find(e); }

	// Find a location with a different def, but same expression. For example, pass r28{10},
	// return true if r28{20} in the set. If return true, dr points to the first different ref
	bool findDifferentRef(const std::shared_ptr<RefExp>& e, SharedExp& dr);
	void addSubscript(Instruction *def /* , Cfg* cfg */); ///< Add a subscript to all elements
};


/// A class to store connections in a graph, e.g. for interferences of types or live ranges, or the phi_unite relation
/// that phi statements imply
/// If a is connected to b, then b is automatically connected to a
// This is implemented in a std::multimap, even though Appel suggests a bitmap (e.g. std::vector<bool> does this in a
// space efficient manner), but then you still need maps from expression to bit number. So here a standard map is used,
// and when a -> b is inserted, b->a is redundantly inserted.
class ConnectionGraph
{
	std::multimap<SharedExp, SharedExp, lessExpStar> emap;   ///< The map

public:
	typedef std::multimap<SharedExp, SharedExp, lessExpStar>::iterator         iterator;
	typedef std::multimap<SharedExp, SharedExp, lessExpStar>::const_iterator   const_iterator;

	ConnectionGraph() {}

	void add(SharedExp a, SharedExp b); ///< Add pair with check for existing
	void connect(SharedExp a, SharedExp b);

	iterator begin()       { return emap.begin(); }
	iterator end()         { return emap.end(); }
	const_iterator begin() const { return emap.begin(); }
	const_iterator end()   const { return emap.end(); }

	int count(SharedExp a) const;
	bool isConnected(SharedExp a, const Exp& b) const;
	bool allRefsHaveDefs() const;
	void update(SharedExp a, SharedExp b, SharedExp c);
	iterator remove(iterator aa); ///< Remove the mapping at *aa
	void dump() const;            ///< Dump for debugging

private:
	std::vector<SharedExp> allConnected(SharedExp a);
};

QTextStream& operator<<(QTextStream& os, const AssignSet *as);
QTextStream& operator<<(QTextStream& os, const InstructionSet *ss);
QTextStream& operator<<(QTextStream& os, const LocationSet *ls);
