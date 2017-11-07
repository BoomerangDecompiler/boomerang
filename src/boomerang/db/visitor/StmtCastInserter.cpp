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


#include "boomerang/db/exp/Exp.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/visitor/ExpCastInserter.h"


bool StmtCastInserter::visit(Assign *stmt)
{
    return common(stmt);
}


bool StmtCastInserter::visit(PhiAssign *stmt)
{
    return common(stmt);
}


bool StmtCastInserter::visit(ImplicitAssign *stmt)
{
    return common(stmt);
}


bool StmtCastInserter::visit(BoolAssign *stmt)
{
    return common(stmt);
}


bool StmtCastInserter::common(Assignment *stmt)
{
    SharedExp lhs = stmt->getLeft();

    if (lhs->isMemOf()) {
        SharedType memofType = stmt->getType();
        ExpCastInserter::checkMemofType(lhs, memofType);
    }

    return true;
}
