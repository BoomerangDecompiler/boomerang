#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtModifier.h"


#include "boomerang/db/visitor/ExpModifier.h"


StmtModifier::StmtModifier(ExpModifier* em, bool ignnoreCol)
    : m_mod(em)
    , m_ignoreCol(ignnoreCol)
{
}


void StmtModifier::visit(Assign *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtModifier::visit(PhiAssign *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtModifier::visit(ImplicitAssign *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtModifier::visit(BoolAssign *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtModifier::visit(GotoStatement *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtModifier::visit(BranchStatement *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtModifier::visit(CaseStatement *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtModifier::visit(CallStatement *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtModifier::visit(ReturnStatement *, bool& visitChildren)
{
    visitChildren = true;
}


void StmtModifier::visit(ImpRefStatement *, bool& visitChildren)
{
    visitChildren = true;
}

