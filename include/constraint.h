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
    LocationSet conSet;

public:
    Constraints() {}
    ~Constraints();

    LocationSet& getConstraints() {return conSet;}
    void    addConstraints(LocationSet& con) {conSet.makeUnion(con);}
    // Solve the constraints. If they can be solved, return true and put
    // a copy of the solution (in the form of a set of T<location> = <type>)
    // into soln
    bool    solve(LocationSet& soln);
};
