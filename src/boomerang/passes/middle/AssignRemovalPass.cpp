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


#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Unary.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/util/StatementList.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/core/Boomerang.h"
#include "boomerang/db/statements/PhiAssign.h"


AssignRemovalPass::AssignRemovalPass()
    : IPass("AssignRemoval", PassID::AssignRemoval)
{
}


bool AssignRemovalPass::execute(UserProc *proc)
{
    bool change = false;

    change |= removeSpAssigns(proc);

    // The problem with removing %flags and %CF is that %CF is a subset of %flags
    // removeMatchingAssignsIfPossible(Terminal::get(opFlags));
    // removeMatchingAssignsIfPossible(Terminal::get(opCF));
    change |= removeMatchingAssigns(proc, Unary::get(opTemp, Terminal::get(opWildStrConst)));
    change |= removeMatchingAssigns(proc, Terminal::get(opPC));
    return change;
}


bool AssignRemovalPass::removeSpAssigns(UserProc* proc)
{
    // if there are no uses of sp other than sp{-} in the whole procedure,
    // we can safely remove all assignments to sp, this will make the output
    // more readable for human eyes.

    auto sp(Location::regOf(proc->getSignature()->getStackRegister(proc->getProg())));
    bool foundone = false;

    StatementList stmts;

    proc->getStatements(stmts);

    for (auto stmt : stmts) {
        if (stmt->isAssign() && (*static_cast<Assign *>(stmt)->getLeft() == *sp)) {
            foundone = true;
        }

        LocationSet refs;
        stmt->addUsedLocs(refs);

        for (const SharedExp& rr : refs) {
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

    Boomerang::get()->alertDecompileDebugPoint(proc, "Before removing stack pointer assigns.");

    for (auto& stmt : stmts) {
        if (stmt->isAssign()) {
            Assign *a = static_cast<Assign *>(stmt);

            if (*a->getLeft() == *sp) {
                proc->removeStatement(a);
            }
        }
    }

    Boomerang::get()->alertDecompileDebugPoint(proc, "After removing stack pointer assigns.");
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

        for (const SharedExp& rr : refs) {
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

    QString     res_str;
    QTextStream str(&res_str);
    str << "Before removing matching assigns (" << e << ").";

    Boomerang::get()->alertDecompileDebugPoint(proc, qPrintable(res_str));
    LOG_VERBOSE(res_str);

    for (auto& stmt : stmts) {
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

    res_str.clear();
    str << "After removing matching assigns (" << e << ").";
    Boomerang::get()->alertDecompileDebugPoint(proc, qPrintable(res_str));
    LOG_VERBOSE(res_str);

    return true;
}
