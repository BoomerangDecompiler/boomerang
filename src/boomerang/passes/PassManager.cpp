#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PassManager.h"

#include "boomerang/core/Project.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/passes/call/CallArgumentUpdatePass.h"
#include "boomerang/passes/call/CallDefineUpdatePass.h"
#include "boomerang/passes/dataflow/BlockVarRenamePass.h"
#include "boomerang/passes/dataflow/DominatorPass.h"
#include "boomerang/passes/dataflow/PhiPlacementPass.h"
#include "boomerang/passes/early/FragSimplifyPass.h"
#include "boomerang/passes/early/GlobalConstReplacePass.h"
#include "boomerang/passes/early/StatementInitPass.h"
#include "boomerang/passes/early/StatementPropagationPass.h"
#include "boomerang/passes/late/BranchAnalysisPass.h"
#include "boomerang/passes/late/CallLivenessRemovalPass.h"
#include "boomerang/passes/late/FinalParameterSearchPass.h"
#include "boomerang/passes/late/FromSSAFormPass.h"
#include "boomerang/passes/late/ImplicitPlacementPass.h"
#include "boomerang/passes/late/LocalAndParamMapPass.h"
#include "boomerang/passes/late/LocalTypeAnalysisPass.h"
#include "boomerang/passes/late/UnusedLocalRemovalPass.h"
#include "boomerang/passes/late/UnusedParamRemovalPass.h"
#include "boomerang/passes/late/UnusedStatementRemovalPass.h"
#include "boomerang/passes/middle/AssignRemovalPass.h"
#include "boomerang/passes/middle/CallAndPhiFixPass.h"
#include "boomerang/passes/middle/DuplicateArgsRemovalPass.h"
#include "boomerang/passes/middle/ParameterSymbolMapPass.h"
#include "boomerang/passes/middle/PreservationAnalysisPass.h"
#include "boomerang/passes/middle/SPPreservationPass.h"
#include "boomerang/passes/middle/StrengthReductionReversalPass.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/Log.h"

#include <cassert>


static PassManager g_passManager;


PassManager::PassManager()
{
    m_passes.resize(static_cast<size_t>(PassID::NUM_PASSES));

    registerPass(PassID::Dominators, std::make_unique<DominatorPass>());
    registerPass(PassID::PhiPlacement, std::make_unique<PhiPlacementPass>());
    registerPass(PassID::BlockVarRename, std::make_unique<BlockVarRenamePass>());
    registerPass(PassID::CallDefineUpdate, std::make_unique<CallDefineUpdatePass>());
    registerPass(PassID::CallArgumentUpdate, std::make_unique<CallArgumentUpdatePass>());
    registerPass(PassID::StatementInit, std::make_unique<StatementInitPass>());
    registerPass(PassID::GlobalConstReplace, std::make_unique<GlobalConstReplacePass>());
    registerPass(PassID::StatementPropagation, std::make_unique<StatementPropagationPass>());
    registerPass(PassID::FragSimplify, std::make_unique<FragSimplifyPass>());
    registerPass(PassID::CallAndPhiFix, std::make_unique<CallAndPhiFixPass>());
    registerPass(PassID::SPPreservation, std::make_unique<SPPreservationPass>());
    registerPass(PassID::PreservationAnalysis, std::make_unique<PreservationAnalysisPass>());
    registerPass(PassID::StrengthReductionReversal,
                 std::make_unique<StrengthReductionReversalPass>());
    registerPass(PassID::AssignRemoval, std::make_unique<AssignRemovalPass>());
    registerPass(PassID::DuplicateArgsRemoval, std::make_unique<DuplicateArgsRemovalPass>());
    registerPass(PassID::CallLivenessRemoval, std::make_unique<CallLivenessRemovalPass>());
    registerPass(PassID::LocalTypeAnalysis, std::make_unique<LocalTypeAnalysisPass>());
    registerPass(PassID::BranchAnalysis, std::make_unique<BranchAnalysisPass>());
    registerPass(PassID::FromSSAForm, std::make_unique<FromSSAFormPass>());
    registerPass(PassID::FinalParameterSearch, std::make_unique<FinalParameterSearchPass>());
    registerPass(PassID::UnusedStatementRemoval, std::make_unique<UnusedStatementRemovalPass>());
    registerPass(PassID::ParameterSymbolMap, std::make_unique<ParameterSymbolMapPass>());
    registerPass(PassID::UnusedLocalRemoval, std::make_unique<UnusedLocalRemovalPass>());
    registerPass(PassID::UnusedParamRemoval, std::make_unique<UnusedParamRemovalPass>());
    registerPass(PassID::ImplicitPlacement, std::make_unique<ImplicitPlacementPass>());
    registerPass(PassID::LocalAndParamMap, std::make_unique<LocalAndParamMapPass>());

    for (auto &pass : m_passes) {
        Q_UNUSED(pass);
        assert(pass.get());
    }
}


PassManager::~PassManager()
{
}


PassManager *PassManager::get()
{
    return &g_passManager;
}


bool PassManager::executePass(PassID passID, UserProc *proc)
{
    return executePass(getPass(passID), proc);
}


bool PassManager::executePass(IPass *pass, UserProc *proc)
{
    assert(pass != nullptr);
    LOG_VERBOSE("Executing pass '%1' for '%2'", pass->getName(), proc->getName());

    const bool change = pass->execute(proc);

    if (Log::getOrCreateLog().getLogLevel() >= LogLevel::Verbose1) {
        const QString msg = QString("after executing pass '%1'").arg(pass->getName());
        proc->debugPrintAll(msg);
    }

    return change;
}


void PassManager::registerPass(PassID passID, std::unique_ptr<IPass> pass)
{
    assert(Util::inRange(static_cast<size_t>(passID), static_cast<size_t>(0), m_passes.size()));
    m_passes[static_cast<size_t>(passID)] = std::move(pass);
}


IPass *PassManager::getPass(PassID passID)
{
    if (!Util::inRange(static_cast<size_t>(passID), static_cast<size_t>(0), m_passes.size())) {
        return nullptr;
    }

    return m_passes[static_cast<size_t>(passID)].get();
}
