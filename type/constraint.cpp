/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       constraint.cpp
 * OVERVIEW:   Implementation of objects related to type constraints
 *============================================================================*/

/*
 * $Revision$
 *
 * 22 Aug 03 - Mike: Created
 */

#include "constraint.h"
#include "exp.h"

Constraints::~Constraints() {
    LocSetIter cc;
    for (Exp* c = conSet.getFirst(cc); c; c = conSet.getNext(cc)) {
        delete c;
    }
}

bool Constraints::solve(LocationSet& soln) {
    // For now:
    soln = conSet;
    return true;
}

