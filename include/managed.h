/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*===============================================================================================
 * FILE:	   managed.h
 * OVERVIEW:   Definition of "managed" classes such as StatementSet, which feature makeUnion etc
 * CLASSES:		StatementSet
 *				AssignSet
 *				StatementList
 *				StatementVec
 *				LocationSet
 *				//LocationList
 *==============================================================================================*/

/*
 * $Revision$	// 1.11.2.15
 *
 * 26/Aug/03 - Mike: Split off from statement.h
 */

#ifndef __MANAGED_H__
#define __MANAGED_H__

#include <list>
#include <set>
#include <vector>

#include "exphelp.h"		// For lessExpStar

class Statement;
class Assign;
class Exp;
class RefExp;
class Cfg;
class LocationSet;

// A class to implement sets of statements
class StatementSet {
		std::set<Statement*> sset;							// For now, use use standard sets

public:
typedef std::set<Statement*>::iterator iterator;

virtual				~StatementSet() {}
		void		makeUnion(StatementSet& other);		// Set union
		void		makeDiff (StatementSet& other);		// Set difference
		void		makeIsect(StatementSet& other);		// Set intersection
		bool		isSubSetOf(StatementSet& other);	// Subset relation

		unsigned	size() {return sset.size();}		// Number of elements
		iterator	begin()	{return sset.begin();}
		iterator	end()	{return sset.end();}
		
		void		insert(Statement* s) {sset.insert(s);}	// Insertion
		bool		remove(Statement* s);					// Removal; rets false if not found
		bool		removeIfDefines(Exp* given);			// Remove if given exp is defined
		bool		removeIfDefines(StatementSet& given);	// Remove if any given is def'd
		bool		exists(Statement* s);					// Search; returns false if !found
		bool		definesLoc(Exp* loc);					// Search; returns true if any
															// statement defines loc
		void		clear() {sset.clear();}					// Clear the set
		bool		operator==(const StatementSet& o) const	// Compare if equal
						{ return sset == o.sset;}
		bool		operator<(const StatementSet& o) const;	// Compare if less
		void		print(std::ostream& os);				// Print to os
		void		printNums(std::ostream& os);			// Print statements as numbers
		char*		prints();								// Print to string (for debug)
		void		dump();									// Print to standard error for debugging
};		// class StatementSet

// As above, but the Statements are known to be Assigns, and are sorted sensibly
class AssignSet {
		std::set<Assign*, lessAssign> aset;			// For now, use use standard sets

public:
typedef std::set<Assign*, lessAssign>::iterator iterator;
typedef std::set<Assign*, lessAssign>::const_iterator const_iterator;

virtual				~AssignSet() {}
		void		makeUnion(AssignSet& other);		// Set union
		void		makeDiff (AssignSet& other);		// Set difference
		void		makeIsect(AssignSet& other);		// Set intersection
		bool		isSubSetOf(AssignSet& other);		// Subset relation

		unsigned	size() {return aset.size();}		// Number of elements
		//Statement* getFirst(StmtSetIter& it);	  		// Get the first Statement
		//Statement* getNext (StmtSetIter& it);	  		// Get next
		iterator	begin()	{return aset.begin();}
		iterator	end()	{return aset.end();}
		
		void		insert(Assign* a) {aset.insert(a);}		// Insertion
		bool		remove(Assign* a);						// Removal; rets false if not found
		bool		removeIfDefines(Exp* given);			// Remove if given exp is defined
		bool		removeIfDefines(AssignSet& given);		// Remove if any given is def'd
		bool		exists(Assign* s);						// Search; returns false if !found
		bool		definesLoc(Exp* loc);					// Search; returns true if any assignment defines loc
		Assign*		lookupLoc(Exp* loc);					// Search for loc on LHS, return ptr to Assign if found

		void		clear() {aset.clear();}					// Clear the set
		bool		operator==(const AssignSet& o) const	// Compare if equal
						{ return aset == o.aset;}
		bool		operator<(const AssignSet& o) const;	// Compare if less
		void		print(std::ostream& os);				// Print to os
		void		printNums(std::ostream& os);			// Print statements as numbers
		char*		prints();								// Print to string (for debug)
		void		dump();									// Print to standard error for debugging
		//bool	isLast(StmtSetIter& it);					// returns true if it is at end
};		// class AssignSet

class StatementList {
		std::list<Statement*> slist;		  				// For now, use use standard list

public:
typedef std::list<Statement*>::iterator iterator;
typedef std::list<Statement*>::reverse_iterator reverse_iterator;
virtual				~StatementList() {}
		unsigned	size() {return slist.size();}		 	// Number of elements
		iterator	begin()  {return slist.begin();}
		iterator	end()	  {return slist.end();}
		reverse_iterator rbegin() {return slist.rbegin();}
		reverse_iterator rend()	  {return slist.rend();}

		// A special intersection operator; this becomes the intersection of StatementList a (assumed to be a list of
		// Assignment*s) with the LocationSet b.
		// Used for calculating returns for a CallStatement
		void		makeIsect(StatementList& a, LocationSet& b);
		
		void		append(Statement* s) {slist.push_back(s);} // Insert at end
		void		append(StatementList& sl);			// Append whole StatementList
		void		append(StatementSet& sl);			// Append whole StatementSet
		bool		remove(Statement* s);				// Removal; rets false if not found
		void		removeDefOf(Exp* loc);				// Remove definitions of loc
		// This one is needed where you remove in the middle of a loop
		// Use like this: it = mystatementlist.erase(it);
		iterator	erase(iterator it) {return slist.erase(it);}
		iterator	erase(iterator first, iterator last) {return slist.erase(first, last);}
		iterator	insert(iterator it, Statement* s) {return slist.insert(it, s);}
		bool		exists(Statement* s);				// Search; returns false if not found
		char*		prints();							// Print to string (for debugging)
		void		dump();								// Print to standard error for debugging
		void		clear() { slist.clear(); }
		void		makeCloneOf(StatementList& o);		// Make this a clone of o
		bool		existsOnLeft(Exp* loc);				// True if loc exists on the LHS of any Assignment in this list
		Assignment*	findOnLeft(Exp* loc);				// Return the first stmt with loc on the LHS
};		// class StatementList

class StatementVec {
		std::vector<Statement*> svec;			// For now, use use standard vector

public:
typedef std::vector<Statement*>::iterator iterator;
typedef std::vector<Statement*>::reverse_iterator reverse_iterator;
		unsigned	size() {return svec.size();}		 	// Number of elements
		iterator	begin() { return svec.begin();}
		iterator	end()	 { return svec.end();}
		reverse_iterator rbegin() { return svec.rbegin();}
		reverse_iterator rend()	  { return svec.rend();}
		// Get/put at position idx (0 based)
		Statement* operator[](int idx) {return svec[idx];}
		void		putAt(int idx, Statement* s);
		iterator	remove(iterator it);
		char*		prints();								// Print to string (for debugging)
		void		dump();									// Print to standard error for debugging
		void		printNums(std::ostream& os);
		void		clear() { svec.clear(); }
		bool		operator==(const StatementVec& o) const	// Compare if equal
						{ return svec == o.svec;}
		bool		operator<(const StatementVec& o) const		// Compare if less
						{ return svec < o.svec;}
		void		append(Statement* s) {svec.push_back(s);}
		void		erase(iterator it) {svec.erase(it);}
};	// class StatementVec

// For various purposes, we need sets of locations (registers or memory)
class LocationSet {
		// We use a standard set, but with a special "less than" operator so that the sets are ordered
		// by expression value. If this is not done, then two expressions with the same value (say r[10])
		// but that happen to have different addresses (because they came from different statements)
		// would both be stored in the set (instead of the required set behaviour, where only one is stored)
		std::set<Exp*, lessExpStar> lset; 
public:
typedef std::set<Exp*, lessExpStar>::iterator iterator;
					LocationSet() {}						// Default constructor
virtual				~LocationSet() {}						// virtual destructor kills warning
					LocationSet(const LocationSet& o);		// Copy constructor
					LocationSet& operator=(const LocationSet& o); // Assignment
		void		makeUnion(LocationSet& other);			// Set union
		void		makeDiff (LocationSet& other);			// Set difference
		void		clear() {lset.clear();}					// Clear the set
		iterator	begin() {return lset.begin();}
		iterator	end()	 {return lset.end();}
		void		insert(Exp* loc) {lset.insert(loc);}	// Insert the given location
		void		remove(Exp* loc);						// Remove the given location
		void		remove(iterator ll) {lset.erase(ll);}	// Remove location, given iterator
		void		removeIfDefines(StatementSet& given);	// Remove locs defined in given
		unsigned	size() const {return lset.size();}		// Number of elements
		bool		operator==(const LocationSet& o) const; // Compare
		void		substitute(Assign& a);					// Substitute the given assignment to all
		void		print(std::ostream& os);				// Print to os
		char*		prints();								// Print to string for debugging
		void		dump();
		void		diff(LocationSet* o);					// Diff 2 location sets to std::cerr
		bool		exists(Exp* e); 						// Return true if the location exists in the set
		Exp*		findNS(Exp* e);							// Find location e (no subscripts); NULL if not found
		// Return an iterator to the found item (or end() if not). Only really makes sense if e has a wildcard
		iterator	find(Exp* e) {return lset.find(e); }
		// Find a location with a different def, but same expression. For example, pass r28{10},
		// return true if r28{20} in the set. If return true, dr points to the first different ref
		bool		findDifferentRef(RefExp* e, Exp *&dr);
		void		addSubscript(Statement* def /* , Cfg* cfg */);		// Add a subscript to all elements
};	// class LocationSet

#if 0
class LocationList {
		std::list<Exp*> llist; 
public:
typedef std::list<Exp*>::iterator iterator;
					LocationList() {}						// Default constructor
virtual				~LocationList() {}						// virtual destructor kills warning
					LocationList(const LocationList& o);		// Copy constructor
					LocationList& operator=(const LocationList& o); // Assignment
		//void		makeUnion(LocationSet& other);			// Set union
		//void		makeDiff (LocationSet& other);			// Set difference
		void		clear() {llist.clear();}					// Clear the set
		iterator	begin() {return llist.begin();}
		iterator	end()	 {return llist.end();}
		//void		insert(Exp* loc) {llist.insert(loc);}	// Insert the given location
		//void		remove(Exp* loc);						// Remove the given location
		//void		remove(iterator ll);					// Remove location, given iterator
		//void		removeIfDefines(StatementSet& given);	// Remove locs defined in given
		unsigned	size() const {return llist.size();}		// Number of elements
		bool		operator==(const LocationSet& o) const; // Compare
		void		print(std::ostream& os);				// Print to os
		char*		prints();								// Print to string for logging and debugging
		void		dump();									// Print to standard error for debugging
};	// class LocationList
#endif

class Range {
protected:
	int stride, lowerBound, upperBound;
	Exp *base;

public:
	Range();
	Range(int stride, int lowerBound, int upperBound, Exp *base);

	Exp *getBase() { return base; }
	int getStride() { return stride; }
	int getLowerBound() { return lowerBound; }
	int getUpperBound() { return upperBound; }
	void unionWith(Range &r);
	void widenWith(Range &r);
	void print(std::ostream &os);
	bool operator==(Range &other);
	
static const int MAX = 2147483647;
static const int MIN = -2147483647;
};

class RangeMap {
protected:
	std::map<Exp*, Range, lessExpStar> ranges;

public:
	RangeMap() { }
	void addRange(Exp *loc, Range &r) { ranges[loc] = r; }
	bool hasRange(Exp *loc) { return ranges.find(loc) != ranges.end(); }
	Range &getRange(Exp *loc);
	void unionwith(RangeMap &other);
	void widenwith(RangeMap &other);
	void print(std::ostream &os);
	Exp *substInto(Exp *e);
	void killAllMemOfs();
	void clear() { ranges.clear(); }
	bool isSubset(RangeMap &other);
};

#endif	// #ifdef __MANAGED_H__
