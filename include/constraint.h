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

#include <list>

#include "exp.h"



class Constraints {
    std::list<Exp*> conList;

public:
typedef std::list<Exp*>::iterator handle;

    Constraints() {}
    ~Constraints();

    std::list<Exp*>& getConstraints() {return conList;}
    handle  getPos();
    void    printSince(handle);
    // Solve the constraints. If they can be solved, return true and put
    // a copy of the solution (in the form of a list of T<location> = <type>)
    // into soln
    bool    solve(std::list<Exp*>& soln);
};
