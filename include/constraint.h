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



class Constraints {
    LocationSet     conSet;
    std::list<Exp*> disjunctions;
    LocationSet     constants;
    std::map<Exp*, Exp*, lessExpStar> equates1, equates2;

public:
    Constraints() {}
    ~Constraints();

    LocationSet& getConstraints() {return conSet;}
    void    addConstraints(LocationSet& con) {conSet.makeUnion(con);}
    // Solve the constraints. If they can be solved, return true and put
    // a copy of the solution (in the form of a set of T<location> = <type>)
    // into soln
    void    solve(std::list<LocationSet>& solns);
private:
    bool    doSolve(std::list<Exp*>::iterator it, LocationSet& extra,
              std::list<LocationSet>& solns);
    bool    unify(Exp* x, Exp* y, LocationSet& extra);
    bool    unifyTerm(Exp* x, Exp* y, LocationSet& extra);
};
