#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtExpVisitor.h"


StmtExpVisitor::StmtExpVisitor(ExpVisitor *v, bool ignoreCol)
    : ev(v)
    , m_ignoreCol(ignoreCol)
{
}

bool StmtExpVisitor::visit(Assign *, bool& dontVisitChildren)
{
    dontVisitChildren = false;
    return true;
}


bool StmtExpVisitor::visit(PhiAssign *, bool& dontVisitChildren)
{
    dontVisitChildren = false;
    return true;
}


bool StmtExpVisitor::visit(ImplicitAssign *, bool& dontVisitChildren)
{
    dontVisitChildren = false;
    return true;
}


bool StmtExpVisitor::visit(BoolAssign *, bool& dontVisitChildren)
{
    dontVisitChildren = false;
    return true;
}


bool StmtExpVisitor::visit(GotoStatement *, bool& dontVisitChildren)
{
    dontVisitChildren = false;
    return true;
}


bool StmtExpVisitor::visit(BranchStatement *, bool& dontVisitChildren)
{
    dontVisitChildren = false;
    return true;
}


bool StmtExpVisitor::visit(CaseStatement *, bool& dontVisitChildren)
{
    dontVisitChildren = false;
    return true;
}


bool StmtExpVisitor::visit(CallStatement *, bool& dontVisitChildren)
{
    dontVisitChildren = false;
    return true;
}


bool StmtExpVisitor::visit(ReturnStatement *, bool& dontVisitChildren)
{
    dontVisitChildren = false;
    return true;
}


bool StmtExpVisitor::visit(ImpRefStatement *, bool& dontVisitChildren)
{
    dontVisitChildren = false;
    return true;
}

