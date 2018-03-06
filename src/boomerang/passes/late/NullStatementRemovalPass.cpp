#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "NullStatementRemovalPass.h"


#include "boomerang/db/proc/UserProc.h"
#include "boomerang/util/StatementList.h"
#include "boomerang/util/Log.h"


NullStatementRemovalPass::NullStatementRemovalPass()
    : IPass("NullStatementRemoval", PassID::NullStatementRemoval)
{
}


bool NullStatementRemovalPass::execute(UserProc *proc)
{
    bool          change = false;
    StatementList stmts;
    proc->getStatements(stmts);

    // remove null code
    for (Statement *s : stmts) {
        if (s->isNullStatement()) {
            // A statement of the form x := x
            LOG_VERBOSE("Removing null statement: %1 %2", s->getNumber(), s);

            proc->removeStatement(s);
            change = true;
        }
    }

    return change;
}
