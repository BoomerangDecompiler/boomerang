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

StmtExpVisitor::StmtExpVisitor(ExpVisitor* v, bool _ignoreCol)
    : ev(v)
    , m_ignoreCol(_ignoreCol)
{
}

bool StmtExpVisitor::visit(Assign*, bool& override)
{
    override = false;
    return true;
}


bool StmtExpVisitor::visit(PhiAssign*, bool& override)
{
    override = false;
    return true;
}


bool StmtExpVisitor::visit(ImplicitAssign*, bool& override)
{
    override = false;
    return true;
}


bool StmtExpVisitor::visit(BoolAssign*, bool& override)
{
    override = false;
    return true;
}


bool StmtExpVisitor::visit(GotoStatement*, bool& override)
{
    override = false;
    return true;
}


bool StmtExpVisitor::visit(BranchStatement*, bool& override)
{
    override = false;
    return true;
}


bool StmtExpVisitor::visit(CaseStatement*, bool& override)
{
    override = false;
    return true;
}


bool StmtExpVisitor::visit(CallStatement*, bool& override)
{
    override = false;
    return true;
}


bool StmtExpVisitor::visit(ReturnStatement*, bool& override)
{
    override = false;
    return true;
}


bool StmtExpVisitor::visit(ImpRefStatement*, bool& override)
{
    override = false;
    return true;
}

