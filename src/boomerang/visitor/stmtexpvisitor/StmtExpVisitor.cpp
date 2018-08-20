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
{}


bool StmtExpVisitor::visit(Assign *, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(PhiAssign *, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(ImplicitAssign *, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(BoolAssign *, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(GotoStatement *, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(BranchStatement *, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(CaseStatement *, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(CallStatement *, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(ReturnStatement *, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(ImpRefStatement *, bool &visitChildren)
{
    visitChildren = true;
    return true;
}
