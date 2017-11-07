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


bool StmtCastInserter::visit(Assign *s)
{
    return common(s);
}


bool StmtCastInserter::visit(PhiAssign *s)
{
    return common(s);
}


bool StmtCastInserter::visit(ImplicitAssign *s)
{
    return common(s);
}


bool StmtCastInserter::visit(BoolAssign *s)
{
    return common(s);
}


bool StmtCastInserter::common(Assignment *s)
{
    SharedExp lhs = s->getLeft();

    if (lhs->isMemOf()) {
        SharedType memofType = s->getType();
        ExpCastInserter::checkMemofType(lhs, memofType);
    }

    return true;
}
