#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtDestCounter.h"

#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/visitor/expvisitor/ExpDestCounter.h"


StmtDestCounter::StmtDestCounter(ExpDestCounter *edc)
    : StmtExpVisitor(edc)
{
}


bool StmtDestCounter::visit(const std::shared_ptr<PhiAssign> &, bool &visitChildren)
{
    visitChildren = true;
    return true;
}
