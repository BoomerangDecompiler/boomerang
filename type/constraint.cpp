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

Constraints::~Constraints() {
    handle ii;
    for (ii = conList.begin(); ii != conList.end(); ii++) {
        delete *ii;
    }
}

// Return an iterator to the current position in the list (or NULL)
Constraints::handle Constraints::getPos() {
    if (conList.size() == 0) return NULL;
    return --conList.end();
}

// Print all new constraints since h was end() to std::cerr
void Constraints::printSince(handle h) {
    if (h == NULL) h = conList.begin();
    else h++;
    for (handle ii = h; ii != conList.end(); ii++) {
        std::cerr << *ii << "\n";
    }
    std::cerr << "\n";
}

bool Constraints::solve(std::list<Exp*>& soln) {
    // For now:
    soln = conList;
    return true;
}

