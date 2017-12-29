#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtVisitor.h"



bool StmtVisitor::visit(RTL * /*rtl*/)
{
    // Mostly, don't do anything at the RTL level
    return true;
}

bool StmtVisitor::visit(Assign* /*stmt*/)
{
    return true;
}


bool StmtVisitor::visit(PhiAssign* /*stmt*/)
{
    return true;
}


bool StmtVisitor::visit(ImplicitAssign* /*stmt*/)
{
    return true;
}


bool StmtVisitor::visit(BoolAssign* /*stmt*/)
{
    return true;
}


bool StmtVisitor::visit(GotoStatement* /*stmt*/)
{
    return true;
}


bool StmtVisitor::visit(BranchStatement* /*stmt*/)
{
    return true;
}


bool StmtVisitor::visit(CaseStatement* /*stmt*/)
{
    return true;
}


bool StmtVisitor::visit(CallStatement* /*stmt*/)
{
    return true;
}


bool StmtVisitor::visit(ReturnStatement* /*stmt*/)
{
    return true;
}


bool StmtVisitor::visit(ImpRefStatement* /*stmt*/)
{
    return true;
}
