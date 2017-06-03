/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */
#pragma once

/***************************************************************************/ /**
 * \file       constraint.h
 * OVERVIEW:   Definition of objects related to type constraints
 ******************************************************************************/


#include "db/exphelp.h"
#include "include/type.h"
#include "include/managed.h"

#include <sstream>

class Exp;
class Instruction;

// This class represents fixed constraints (e.g. Ta = <int>, Tb = <alpha2*>),
// but also "tentative" constraints resulting from disjunctions of constraints
class ConstraintMap
{
	std::map<SharedExp, SharedExp, lessExpStar> cmap;

public:
	typedef std::map<SharedExp, SharedExp, lessExpStar>::iterator iterator;

	/// Return true if the given expression is in the map
	bool isFound(SharedExp e) { return cmap.find(e) != cmap.end(); }
	/// Return an iterator to the given left hand side Exp
	iterator find(SharedExp e) { return cmap.find(e); }
	/// Lookup a given left hand side Exp (e.g. given Tlocal1, return <char*>)
	SharedExp& operator[](SharedExp e) { return cmap[e]; }
	/// Return the number of constraints in the map
	size_t size() { return cmap.size(); }
	/// Empty the map
	void clear() { cmap.clear(); }
	/// Return iterators for the begin() and end() of the map
	iterator begin() { return cmap.begin(); }
	iterator end() { return cmap.end(); }
	void constrain(SharedExp loc1, SharedExp loc2);
	void constrain(SharedExp loc, SharedType t);
	void constrain(SharedType t1, SharedType t2);

	/// Insert a constraint given an equality expression
	/// e.g. Tlocal1 = <char*>
	void insert(SharedExp term);

	/// Insert a constraint given left and right hand sides (as type Exps)
	void insert(SharedExp lhs, SharedExp rhs) { cmap[lhs] = rhs; }
	/// Union with another constraint map
	void makeUnion(ConstraintMap& o);

	/// Print to the given stream
	void print(QTextStream& os);

	/// Print to the debug buffer, and return that buffer
	char *prints();

	/// Substitute the given constraints into this map
	void substitute(ConstraintMap& other);

	/// For this solution, we need to find disjunctions of the form
	/// \code
	/// <alphaN> = <type>      or
	/// <type>    = <alphaN>
	/// \endcode
	/// and substitute these into each part of the solution
	void substAlpha();
};

/// A class used for fast location of a constraint
///
/// An equation like Ta = Tb is inserted into this class twice (i.e. as
/// Ta = Tb and also as Tb = Ta. So to find out if Ta is involved in an
/// equate, only have to look up Ta in the map (on the LHS, which is fast)
class EquateMap
{
	std::map<SharedExp, LocationSet, lessExpStar> emap;

public:
	typedef std::map<SharedExp, LocationSet, lessExpStar>::iterator iterator;
	iterator begin() { return emap.begin(); }
	iterator end() { return emap.end(); }
	void erase(iterator it) { emap.erase(it); }
	size_t size() { return emap.size(); }
	// Add an equate (both ways)
	void addEquate(SharedExp a, SharedExp b)
	{
		emap[a].insert(b);
		emap[b].insert(a);
	}

	iterator find(SharedExp e) { return emap.find(e); }
	void print(QTextStream& os);
	char *prints();
}; // class EquateMap

class Constraints
{
	LocationSet conSet;
	std::list<SharedExp> disjunctions;
	/// Map from location to a fixed type (could be a pointer to a variable type, i.e. an alpha).
	ConstraintMap fixed;
	/// EquateMap of locations that are equal
	EquateMap equates;

public:
	Constraints() {}
	~Constraints();

	void print(QTextStream& os);
	char *prints();

	LocationSet& getConstraints() { return conSet; }
	void addConstraints(LocationSet& con) { conSet.makeUnion(con); }
	/// Substitute the given constraintMap into the disjuncts
	void substIntoDisjuncts(ConstraintMap& in);

	/// Substitute the given constraintMap into the equates
	void substIntoEquates(ConstraintMap& in);
	void alphaSubst();

	/// Solve the constraints. If they can be solved, return true and put
	/// a copy of the solution (in the form of a set of T\<location\> = \<type\>)
	/// into solns
	bool solve(std::list<ConstraintMap>& solns);

private:
	bool doSolve(std::list<SharedExp>::iterator it, ConstraintMap& extra, std::list<ConstraintMap>& solns);

	/// Test for compatibility of these types. Sometimes, they are compatible
	/// with an extra constraint (e.g. alpha3* is compatible with alpha4* with
	/// the extra constraint that alpha3 == alpha4)
	bool unify(SharedExp x, SharedExp y, ConstraintMap& extra);
};
