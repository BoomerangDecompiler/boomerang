#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtPartModifier.h"


StmtPartModifier::StmtPartModifier(ExpModifier *em, bool ignoreCol)
    : mod(em)
    , m_ignoreCol(ignoreCol)
{
}


void StmtPartModifier::visit(Assign *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(PhiAssign *, bool& visitChildren)
{
    visitChildren = true;
}

void StmtPartModifier::visit(ImplicitAssign *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(BoolAssign *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(GotoStatement *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(BranchStatement *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(CaseStatement *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(CallStatement *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(ReturnStatement *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(ImpRefStatement *, bool& visitChildren)
{
    visitChildren = true;
}

