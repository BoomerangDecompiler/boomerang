/*
 * Copyright (C) 2004, Mike Van Emmerik and Trent Waddington
 */
/*==============================================================================
 * FILE:       visitor.cpp
 * OVERVIEW:   Provides the implementation for the various visitor and modifier
 *             classes.
 *============================================================================*/
/*
 * $Revision$
 *
 * 14 Jun 04 - Mike: Created, from work started by Trent in 2003
 */

#include "visitor.h"
#include "exp.h"
#include "statement.h"


// FixProcVisitor class

bool FixProcVisitor::visit(Location* l) {
    l->setProc(proc);       // Set the proc, but only for Locations
    return true;
}

// GetProcVisitor class

bool GetProcVisitor::visit(Location* l) {
    proc = l->getProc();
    return proc == NULL;        // Continue recursion only if failed so far
}

// GetProcVisitor class

bool SetConscripts::visit(Const* c) {
    if (!bInLocalGlobal)
        c->setConscript(++curConscript);
    bInLocalGlobal = false;
    return true;       // Continue recursion
}

bool SetConscripts::visit(Location* l) {
    OPER op = l->getOper();
    if (op == opLocal || op == opGlobal || op == opRegOf || op == opParam)
        bInLocalGlobal = true;
    return true;       // Continue recursion
}


bool StmtVisitor::visit(RTL* rtl) {
    // Mostly, don't do anything at the RTL level
    return true;
} 

bool StmtSetConscripts::visit(Assign* stmt) {
    SetConscripts sc(curConscript);
    stmt->getLeft()->accept(&sc);
    stmt->getRight()->accept(&sc);
    curConscript = sc.getLast();
    return true;
}

bool StmtSetConscripts::visit(CallStatement* stmt) {
    SetConscripts sc(curConscript);
    std::vector<Exp*> args;
    args = stmt->getArguments();
    int i, n = args.size();
    for (i=0; i < n; i++)
        args[i]->accept(&sc);
    n = stmt->getNumReturns();
    for (i=0; i < n; i++) {
        Exp* r = stmt->getReturnExp(i);
        r->accept(&sc);
    }
    curConscript = sc.getLast();
    return true;
}

bool StmtSetConscripts::visit(CaseStatement* stmt) {
    SetConscripts sc(curConscript);
    SWITCH_INFO* si = stmt->getSwitchInfo();
    if (si) {
        si->pSwitchVar->accept(&sc);
        curConscript = sc.getLast();
    }
    return true;
}

bool StmtSetConscripts::visit(ReturnStatement* stmt) {
    SetConscripts sc(curConscript);
    int n = stmt->getNumReturns();
    for (int i=0; i < n; i++) {
        Exp* r = stmt->getReturnExp(i);
        r->accept(&sc);
    }
    curConscript = sc.getLast();
    return true;
}

bool StmtSetConscripts::visit(BoolStatement* stmt) {
    SetConscripts sc(curConscript);
    stmt->getCondExpr()->accept(&sc);
    stmt->getDest()->accept(&sc);
    curConscript = sc.getLast();
    return true;
}

void StripPhis::visit(Assign* s) {
    del = s->isPhi();
}

Exp* StripRefs::visit(RefExp* e) {
    return e->getSubExp1();     // Do the actual stripping of references!
}
