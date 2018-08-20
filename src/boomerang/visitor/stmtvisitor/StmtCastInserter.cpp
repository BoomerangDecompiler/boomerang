#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtCastInserter.h"

#include "boomerang/ssl/exp/Exp.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/visitor/expmodifier/ExpCastInserter.h"


bool StmtCastInserter::visit(const Assign *stmt)
{
    return common(stmt);
}


bool StmtCastInserter::visit(const PhiAssign *stmt)
{
    return common(stmt);
}


bool StmtCastInserter::visit(const ImplicitAssign *stmt)
{
    return common(stmt);
}


bool StmtCastInserter::visit(const BoolAssign *stmt)
{
    return common(stmt);
}


bool StmtCastInserter::common(const Assignment *stmt)
{
    SharedExp lhs = stmt->getLeft();

    if (lhs->isMemOf()) {
        SharedType memofType = stmt->getType();
        ExpCastInserter::checkMemofType(lhs, memofType);
    }

    return true;
}
