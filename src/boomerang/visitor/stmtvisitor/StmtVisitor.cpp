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


bool StmtVisitor::visit(const RTL *)
{
    // Mostly, don't do anything at the RTL level
    return true;
}

bool StmtVisitor::visit(const Assign *)
{
    return true;
}


bool StmtVisitor::visit(const PhiAssign *)
{
    return true;
}


bool StmtVisitor::visit(const ImplicitAssign *)
{
    return true;
}


bool StmtVisitor::visit(const BoolAssign *)
{
    return true;
}


bool StmtVisitor::visit(const GotoStatement *)
{
    return true;
}


bool StmtVisitor::visit(const BranchStatement *)
{
    return true;
}


bool StmtVisitor::visit(const CaseStatement *)
{
    return true;
}


bool StmtVisitor::visit(const CallStatement *)
{
    return true;
}


bool StmtVisitor::visit(const ReturnStatement *)
{
    return true;
}


bool StmtVisitor::visit(const ImpRefStatement *)
{
    return true;
}
