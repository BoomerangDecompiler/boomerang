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
#include "managed.h"
#include "exp.h"

Constraints::~Constraints() {
    LocationSet::iterator cc;
    for (cc = conSet.begin(); cc != conSet.end(); cc++) {
        delete *cc;
    }
}

bool Constraints::solve(LocationSet& soln) {
std::cerr << conSet.size() << " constraints:";
conSet.print(std::cerr);
    // Replace Ta[loc] = ptr(alpha) with
    //         Tloc = alpha
    LocationSet::iterator cc;
    for (cc = conSet.begin(); cc != conSet.end(); cc++) {
        Exp* c = *cc;
        if (!c->isEquality()) continue;
        Exp* left  = ((Binary*)c)->getSubExp1();
        if (!left->isTypeOf()) continue;
        Exp* leftSub = ((Unary*)left)->getSubExp1();
        if (!leftSub->isAddrOf()) continue;
        Exp* right = ((Binary*)c)->getSubExp2();
        if (!right->isTypeVal()) continue;
        Type* t = ((TypeVal*)right)->getType();
        if (!t->isPointer()) continue;
        // Don't modify a key in a map
        Exp* clone = c->clone();
        // left is typeof(addressof(something)) -> typeof(something)
        left  = ((Binary*)clone)->getSubExp1();
        leftSub = ((Unary*)left)->getSubExp1();
        Exp* something = ((Unary*)leftSub)->getSubExp1();
        ((Unary*)left)->setSubExp1ND(something);
        ((Unary*)leftSub)->setSubExp1ND(NULL);
        delete leftSub;
        // right is <alpha*> -> <alpha>
        right = ((Binary*)clone)->getSubExp2();
        t = ((TypeVal*)right)->getType();
        ((TypeVal*)right)->setType(((PointerType*)t)->getPointsTo()->clone());
        delete t;
        conSet.remove(c);
        conSet.insert(clone);
        delete c;
    }

    //doSolve(new Ternary(opTrue), conSet, conSet.begin());


    // For now:
    soln = conSet;
    return true;
}

