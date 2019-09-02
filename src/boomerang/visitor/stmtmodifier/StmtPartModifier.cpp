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


void StmtPartModifier::visit(const std::shared_ptr<Assign> &, bool &visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(const std::shared_ptr<PhiAssign> &, bool &visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(const std::shared_ptr<ImplicitAssign> &, bool &visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(const std::shared_ptr<BoolAssign> &, bool &visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(const std::shared_ptr<GotoStatement> &, bool &visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(const std::shared_ptr<BranchStatement> &, bool &visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(const std::shared_ptr<CaseStatement> &, bool &visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(const std::shared_ptr<CallStatement> &, bool &visitChildren)
{
    visitChildren = true;
}


void StmtPartModifier::visit(const std::shared_ptr<ReturnStatement> &, bool &visitChildren)
{
    visitChildren = true;
}
