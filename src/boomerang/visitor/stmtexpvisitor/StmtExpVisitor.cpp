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


bool StmtExpVisitor::visit(const std::shared_ptr<Assign> &, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(const std::shared_ptr<PhiAssign> &, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(const std::shared_ptr<ImplicitAssign> &, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(const std::shared_ptr<BoolAssign> &, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(const std::shared_ptr<GotoStatement> &, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(const std::shared_ptr<BranchStatement> &, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(const std::shared_ptr<CaseStatement> &, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(const std::shared_ptr<CallStatement> &, bool &visitChildren)
{
    visitChildren = true;
    return true;
}


bool StmtExpVisitor::visit(const std::shared_ptr<ReturnStatement> &, bool &visitChildren)
{
    visitChildren = true;
    return true;
}
