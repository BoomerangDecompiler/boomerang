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


StmtModifier::StmtModifier(ExpModifier* em, bool ic)
    : m_mod(em)
    , m_ignoreCol(ic)
{
}


void StmtModifier::visit(Assign*, bool& recur)
{
    recur = true;
}


void StmtModifier::visit(PhiAssign*, bool& recur)
{
    recur = true;
}


void StmtModifier::visit(ImplicitAssign*, bool& recur)
{
    recur = true;
}


void StmtModifier::visit(BoolAssign*, bool& recur)
{
    recur = true;
}


void StmtModifier::visit(GotoStatement*, bool& recur)
{
    recur = true;
}


void StmtModifier::visit(BranchStatement*, bool& recur)
{
    recur = true;
}


void StmtModifier::visit(CaseStatement*, bool& recur)
{
    recur = true;
}


void StmtModifier::visit(CallStatement*, bool& recur)
{
    recur = true;
}


void StmtModifier::visit(ReturnStatement*, bool& recur)
{
    recur = true;
}


void StmtModifier::visit(ImpRefStatement*, bool& recur)
{
    recur = true;
}

