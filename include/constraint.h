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

    bool isFound(Exp* e)  {return cmap.find(e) != cmap.end();}
    iterator find(Exp* e) {return cmap.find(e);}
    Exp*& operator[](Exp* e) {return cmap[e];}
    int   size() {return cmap.size();}
    void  clear() {cmap.clear();}
    iterator begin() {return cmap.begin();}
    iterator end()   {return cmap.end();}
    void constrain(Exp* loc1, Exp* loc2) {
        cmap[new Unary(opTypeOf, loc1)] = new Unary(opTypeOf, loc2);}
    void insert(Exp* term);
    void insert(Exp* lhs, Exp* rhs) {cmap[lhs] = rhs;}
    void constrain(Exp* loc, Type* t) {
        cmap[new Unary(opTypeOf, loc)] = new TypeVal(t);}
    void constrain(Type* t1, Type* t2) { // Example: alpha1 = alpha2
        cmap[new TypeVal(t1)] = new TypeVal(t2);}
    void makeUnion(ConstraintMap& o) {
        cmap.insert(o.begin(), o.end());}
    void print(std::ostream& os);
    char* prints();
};

class EquateMap {
    std::map<Exp*, LocationSet, lessExpStar> emap;
public:
typedef std::map<Exp*, LocationSet, lessExpStar>::iterator iterator;
    iterator begin() {return emap.begin();}
    iterator end()   {return emap.end();}
    void erase(iterator it) {emap.erase(it);}
    int  size() {return emap.size();}
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
    // Solve the constraints. If they can be solved, return true and put
    // a copy of the solution (in the form of a set of T<location> = <type>)
    // into soln
    bool    solve(std::list<ConstraintMap>& solns);
private:
    bool    doSolve(std::list<Exp*>::iterator it, ConstraintMap& extra,
              std::list<ConstraintMap>& solns);
    bool    unify(Exp* x, Exp* y, ConstraintMap& extra);
};
