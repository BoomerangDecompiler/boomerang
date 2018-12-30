#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "AssignRemovalPass.h"

#include "boomerang/core/Project.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/exp/Unary.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/util/StatementList.h"
#include "boomerang/util/log/Log.h"


AssignRemovalPass::AssignRemovalPass()
    : IPass("AssignRemoval", PassID::AssignRemoval)
{
}


bool AssignRemovalPass::execute(UserProc *proc)
{
    bool change = false;
    change |= removeSpAssigns(proc);
    change |= removeMatchingAssigns(proc, Unary::get(opTemp, Terminal::get(opWildStrConst)));
    change |= removeMatchingAssigns(proc, Terminal::get(opPC));
    return change;
}


bool AssignRemovalPass::removeSpAssigns(UserProc *proc)
{
    // if there are no uses of sp other than sp{-} in the whole procedure,
    // we can safely remove all assignments to sp, this will make the output
    // more readable for human eyes.

    SharedExp sp  = Location::regOf(Util::getStackRegisterIndex(proc->getProg()));
    bool foundone = false;

    StatementList stmts;
    proc->getStatements(stmts);

    for (Statement *stmt : stmts) {
        if (stmt->isAssign() && (*static_cast<Assign *>(stmt)->getLeft() == *sp)) {
            foundone = true;
        }

        LocationSet refs;
        stmt->addUsedLocs(refs);

        for (const SharedExp &rr : refs) {
            if (rr->isSubscript() && (*rr->getSubExp1() == *sp)) {
                Statement *def = rr->access<RefExp>()->getDef();

                if (def && (def->getProc() == proc)) {
                    return false;
                }
            }
        }
    }

    if (!foundone) {
        return false;
    }

    proc->getProg()->getProject()->alertDecompileDebugPoint(
        proc, "Before removing stack pointer assigns.");

    for (auto &stmt : stmts) {
        if (stmt->isAssign()) {
            Assign *a = static_cast<Assign *>(stmt);

            if (*a->getLeft() == *sp) {
                proc->removeStatement(a);
            }
        }
    }

    proc->getProg()->getProject()->alertDecompileDebugPoint(
        proc, "After removing stack pointer assigns.");
    return true;
}


bool AssignRemovalPass::removeMatchingAssigns(UserProc *proc, SharedExp e)
{
    // if there are no uses of %flags in the whole procedure,
    // we can safely remove all assignments to %flags, this will make the output
    // more readable for human eyes and makes short circuit analysis easier.

    bool foundone = false;

    StatementList stmts;
    proc->getStatements(stmts);

    for (auto stmt : stmts) {
        if (stmt->isAssign() && (*static_cast<const Assign *>(stmt)->getLeft() == *e)) {
            foundone = true;
        }

        if (stmt->isPhi()) {
            if (*static_cast<const PhiAssign *>(stmt)->getLeft() == *e) {
                foundone = true;
            }

            continue;
        }

        LocationSet refs;
        stmt->addUsedLocs(refs);

        for (const SharedExp &rr : refs) {
            if (rr->isSubscript() && (*rr->getSubExp1() == *e)) {
                Statement *def = rr->access<RefExp>()->getDef();

                if (def && (def->getProc() == proc)) {
                    return false;
                }
            }
        }
    }

    if (!foundone) {
        return false;
    }

    QString msg;
    OStream str(&msg);
    str << "Before removing matching assigns (" << e << ").";

    proc->getProg()->getProject()->alertDecompileDebugPoint(proc, qPrintable(msg));

    for (auto &stmt : stmts) {
        if ((stmt)->isAssign()) {
            Assign *a = static_cast<Assign *>(stmt);

            if (*a->getLeft() == *e) {
                proc->removeStatement(a);
            }
        }
        else if ((stmt)->isPhi()) {
            PhiAssign *a = static_cast<PhiAssign *>(stmt);

            if (*a->getLeft() == *e) {
                proc->removeStatement(a);
            }
        }
    }

    msg.clear();
    str << "After removing matching assigns (" << e << ").";
    proc->getProg()->getProject()->alertDecompileDebugPoint(proc, qPrintable(msg));
    LOG_VERBOSE(msg);

    return true;
}
