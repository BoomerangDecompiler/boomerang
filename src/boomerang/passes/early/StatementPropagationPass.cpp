#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementPropagationPass.h"


#include "boomerang/core/Project.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/visitor/expvisitor/ExpDestCounter.h"
#include "boomerang/visitor/stmtexpvisitor/StmtDestCounter.h"
#include "boomerang/passes/PassManager.h"


StatementPropagationPass::StatementPropagationPass()
    : IPass("StatementPropagation", PassID::StatementPropagation)
{
}


bool StatementPropagationPass::execute(UserProc *proc)
{
    StatementList stmts;
    proc->getStatements(stmts);

    // Find the locations that are used by a live, dominating phi-function
    LocationSet usedByDomPhi;
    findLiveAtDomPhi(proc, usedByDomPhi);

    // Next pass: count the number of times each assignment LHS would be propagated somewhere
    std::map<SharedExp, int, lessExpStar> destCounts;

    // Also maintain a set of locations which are used by phi statements
    for (Statement *s : stmts) {
        ExpDestCounter  edc(destCounts);
        StmtDestCounter sdc(&edc);
        s->accept(&sdc);
    }

    // A fourth pass to propagate only the flags (these must be propagated even if it results in extra locals)
    bool change = false;

    Settings *settings = proc->getProg()->getProject()->getSettings();
    for (Statement *s : stmts) {
        if (!s->isPhi()) {
            change |= s->propagateFlagsTo(settings);
        }
    }

    // Finally the actual propagation
    bool convert = false;

    for (Statement *s : stmts) {
        if (!s->isPhi()) {
            change |= s->propagateTo(convert, settings, &destCounts, &usedByDomPhi);
        }
    }

    PassManager::get()->executePass(PassID::BBSimplify, proc);
    // TODO propagateToCollector(proc->getUseCollector());
    proc->propagateToCollector();

    return change || convert;
}


void StatementPropagationPass::findLiveAtDomPhi(UserProc *proc, LocationSet& usedByDomPhi)
{
    LocationSet usedByDomPhi0;
    std::map<SharedExp, PhiAssign *, lessExpStar> defdByPhi;

    proc->getDataFlow()->findLiveAtDomPhi(usedByDomPhi, usedByDomPhi0, defdByPhi);

    // Note that the above is not the complete algorithm; it has found the dead phi-functions in the defdAtPhi
    for (auto it = defdByPhi.begin(); it != defdByPhi.end(); ++it) {
        // For each phi parameter, remove from the final usedByDomPhi set
        for (RefExp& v : *it->second) {
            assert(v.getSubExp1());
            auto wrappedParam = RefExp::get(v.getSubExp1(), v.getDef());
            usedByDomPhi.remove(wrappedParam);
        }

        // Now remove the actual phi-function (a PhiAssign Statement)
        // Ick - some problem with return statements not using their returns until more analysis is done
        // removeStatement(it->second);
    }
}
