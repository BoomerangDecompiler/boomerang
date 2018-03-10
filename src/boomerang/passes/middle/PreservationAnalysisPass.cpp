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


#include "boomerang/db/proc/UserProc.h"
#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Log.h"



PreservationAnalysisPass::PreservationAnalysisPass()
    : IPass("PreservationAnalysis", PassID::PreservationAnalysis)
{
}


bool PreservationAnalysisPass::execute(UserProc *proc)
{
    std::set<SharedExp> removes;

    if (proc->getTheReturnStatement() == nullptr) {
        if (DEBUG_PROOF) {
            LOG_MSG("Can't find preservations as there is no return statement!");
        }

        return false;
    }

    // prove preservation for all modifieds in the return statement
    StatementList& modifieds = proc->getTheReturnStatement()->getModifieds();

    for (ReturnStatement::iterator mm = modifieds.begin(); mm != modifieds.end(); ++mm) {
        SharedExp lhs      = static_cast<Assignment *>(*mm)->getLeft();
        auto      equation = Binary::get(opEquals, lhs, lhs);

        if (DEBUG_PROOF) {
            LOG_MSG("attempting to prove %1 is preserved by %2", equation, getName());
        }

        if (proc->prove(equation)) {
            removes.insert(equation);
        }
    }

    if (DEBUG_PROOF) {
        LOG_MSG("### proven true for procedure %1:", getName());

        for (auto& elem : proc->getProvenTrue()) {
            LOG_MSG("  %1 = %2", elem.first, elem.second);
        }

        LOG_MSG("### End proven true for procedure %1", getName());
    }

    // Remove the preserved locations from the modifieds and the returns
    for (auto pp = proc->getProvenTrue().begin(); pp != proc->getProvenTrue().end(); ++pp) {
        SharedExp lhs = pp->first;
        SharedExp rhs = pp->second;

        // Has to be of the form loc = loc, not say loc+4, otherwise the bypass logic won't see the add of 4
        if (!(*lhs == *rhs)) {
            continue;
        }

        proc->getTheReturnStatement()->removeModified(lhs);
    }

    return true;
}
