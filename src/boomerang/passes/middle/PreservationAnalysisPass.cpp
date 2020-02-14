#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PreservationAnalysisPass.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/util/log/Log.h"


PreservationAnalysisPass::PreservationAnalysisPass()
    : IPass("PreservationAnalysis", PassID::PreservationAnalysis)
{
}


bool PreservationAnalysisPass::execute(UserProc *proc)
{
    std::set<SharedExp> removes;

    if (proc->getRetStmt() == nullptr) {
        if (proc->getProg()->getProject()->getSettings()->debugProof) {
            LOG_MSG("Can't find preservations as there is no return statement!");
        }

        return false;
    }

    // prove preservation for all modifieds in the return statement
    for (SharedStmt mod : proc->getRetStmt()->getModifieds()) {
        SharedExp lhs = mod->as<Assignment>()->getLeft();
        auto equation = Binary::get(opEquals, lhs, lhs);

        if (proc->getProg()->getProject()->getSettings()->debugProof) {
            LOG_MSG("Attempting to prove %1 is preserved by %2", equation, proc->getName());
        }

        if (proc->preservesExp(lhs)) {
            removes.insert(equation);
        }
    }

    if (proc->getProg()->getProject()->getSettings()->debugProof) {
        LOG_MSG("### Proven true for procedure %1:", proc->getName());

        for (auto &elem : proc->getProvenTrue()) {
            LOG_MSG("  %1 = %2", elem.first, elem.second);
        }

        LOG_MSG("### End proven true for procedure %1", proc->getName());
    }

    // Remove the preserved locations from the modifieds and the returns
    for (auto pp = proc->getProvenTrue().begin(); pp != proc->getProvenTrue().end(); ++pp) {
        SharedExp lhs = pp->first;
        SharedExp rhs = pp->second;

        // Has to be of the form loc = loc, not say loc+4, otherwise the bypass logic won't see the
        // add of 4
        if (!(*lhs == *rhs)) {
            continue;
        }

        proc->getRetStmt()->removeFromModifiedsAndReturns(lhs);
    }

    return true;
}
