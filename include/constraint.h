/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       constraint.h
 * OVERVIEW:   Definition of objects related to type constraints
 *============================================================================*/

/*
 * $Revision$
 *
 * 22 Aug 03 - Mike: Created
 */

#include "statement.h"
#include "exp.h"
#include <sstream>

// This class represents fixed constraints (e.g. Ta = <int>, Tb = <alpha2*>),
// but also "tentative" constraints resulting from disjunctions of constraints
class ConstraintMap {
    std::map<Exp*, Exp*, lessExpStar> cmap;
public:
typedef std::map<Exp*, Exp*, lessExpStar>::iterator iterator;

    // Return true if the given expression is in the map
    bool isFound(Exp* e)  {return cmap.find(e) != cmap.end();}
    // Return an iterator to the given left hand side Exp
    iterator find(Exp* e) {return cmap.find(e);}
    // Lookup a given left hand side Exp (e.g. given Tlocal1, return <char*>)
    Exp*& operator[](Exp* e) {return cmap[e];}
    // Return the number of constraints in the map
    int   size() {return cmap.size();}
    // Empty the map
    void  clear() {cmap.clear();}
    // Return iterators for the begin() and end() of the map
    iterator begin() {return cmap.begin();}
    iterator end()   {return cmap.end();}
    // Insert a constraint given two locations (i.e. Tloc1 = Tloc2)
    void constrain(Exp* loc1, Exp* loc2) {
        cmap[new Unary(opTypeOf, loc1)] = new Unary(opTypeOf, loc2);}
    // Insert a constraint given an equality expression
    // e.g. Tlocal1 = <char*>
    void insert(Exp* term);
    // Insert a constraint given left and right hand sides (as type Exps)
    void insert(Exp* lhs, Exp* rhs) {cmap[lhs] = rhs;}
    // Insert a constraint given a location and a Type
    void constrain(Exp* loc, Type* t) {
        cmap[new Unary(opTypeOf, loc)] = new TypeVal(t);}
    // Insert a constraint given two Types (at least one variable)
    void constrain(Type* t1, Type* t2) { // Example: alpha1 = alpha2
        cmap[new TypeVal(t1)] = new TypeVal(t2);}
    // Union with another constraint map
    void makeUnion(ConstraintMap& o) {
        cmap.insert(o.begin(), o.end());}
    // Print to the given stream
    void print(std::ostream& os);
    // Print to the debug buffer, and return that buffer
    char* prints();
    // Substitute the given constraints into this map
    void    substitute(ConstraintMap& other);
    // For this solution, we need to find disjunctions of the form
    // <alphaN> = <type>      or
    // <type>   = <alphaN>
    // and substitute these into each part of the solution
    void    substAlpha();
};

// A class used for fast location of a constraint
// An equation like Ta = Tb is inserted into this class twice (i.e. as
// Ta = Tb and also as Tb = Ta. So to find out if Ta is involved in an
// equate, only have to look up Ta in the map (on the LHS, which is fast)
class EquateMap {
    std::map<Exp*, LocationSet, lessExpStar> emap;
public:
typedef std::map<Exp*, LocationSet, lessExpStar>::iterator iterator;
    iterator begin() {return emap.begin();}
    iterator end()   {return emap.end();}
    void erase(iterator it) {emap.erase(it);}
    int  size() {return emap.size();}
    // Add an equate (both ways)
    void addEquate(Exp* a, Exp* b) {
        emap[a].insert(b); emap[b].insert(a);}
    iterator find(Exp* e) {return emap.find(e);}
    void print(std::ostream& os);
    char* prints();
};

class Constraints {
    LocationSet     conSet;
    std::list<Exp*> disjunctions;
    // Map from location to a fixed type (could be a pointer to a variable
    // type, i.e. an alpha).
    ConstraintMap fixed;
    // EquateMap of locations that are equal
    EquateMap equates;

public:
    Constraints() {}
    ~Constraints();

    LocationSet& getConstraints() {return conSet;}
    void    addConstraints(LocationSet& con) {conSet.makeUnion(con);}
    // Substitute the given constraintMap into the disjuncts
    void    substIntoDisjuncts(ConstraintMap& in);
    // Substitute the given constraintMap into the equates
    void    substIntoEquates(ConstraintMap& in);
    // Perform "alpha substitution". Example:
    //   <fixedtype*> = alphaX* and T[Y] = alphaX*  ->
    //   T[Y] = <fixedtype*>
    void    alphaSubst();

    // Solve the constraints. If they can be solved, return true and put
    // a copy of the solution (in the form of a set of T<location> = <type>)
    // into solns
    bool    solve(std::list<ConstraintMap>& solns);
private:
    bool    doSolve(std::list<Exp*>::iterator it, ConstraintMap& extra,
              std::list<ConstraintMap>& solns);
    bool    unify(Exp* x, Exp* y, ConstraintMap& extra);
};
