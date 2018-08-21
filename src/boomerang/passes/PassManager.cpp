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
#include "boomerang/passes/early/BBSimplifyPass.h"
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

    registerPass(PassID::Dominators, Util::makeUnique<DominatorPass>());
    registerPass(PassID::PhiPlacement, Util::makeUnique<PhiPlacementPass>());
    registerPass(PassID::BlockVarRename, Util::makeUnique<BlockVarRenamePass>());
    registerPass(PassID::CallDefineUpdate, Util::makeUnique<CallDefineUpdatePass>());
    registerPass(PassID::CallArgumentUpdate, Util::makeUnique<CallArgumentUpdatePass>());
    registerPass(PassID::StatementInit, Util::makeUnique<StatementInitPass>());
    registerPass(PassID::GlobalConstReplace, Util::makeUnique<GlobalConstReplacePass>());
    registerPass(PassID::StatementPropagation, Util::makeUnique<StatementPropagationPass>());
    registerPass(PassID::BBSimplify, Util::makeUnique<BBSimplifyPass>());
    registerPass(PassID::CallAndPhiFix, Util::makeUnique<CallAndPhiFixPass>());
    registerPass(PassID::SPPreservation, Util::makeUnique<SPPreservationPass>());
    registerPass(PassID::PreservationAnalysis, Util::makeUnique<PreservationAnalysisPass>());
    registerPass(PassID::StrengthReductionReversal,
                 Util::makeUnique<StrengthReductionReversalPass>());
    registerPass(PassID::AssignRemoval, Util::makeUnique<AssignRemovalPass>());
    registerPass(PassID::DuplicateArgsRemoval, Util::makeUnique<DuplicateArgsRemovalPass>());
    registerPass(PassID::CallLivenessRemoval, Util::makeUnique<CallLivenessRemovalPass>());
    registerPass(PassID::LocalTypeAnalysis, Util::makeUnique<LocalTypeAnalysisPass>());
    registerPass(PassID::BranchAnalysis, Util::makeUnique<BranchAnalysisPass>());
    registerPass(PassID::FromSSAForm, Util::makeUnique<FromSSAFormPass>());
    registerPass(PassID::FinalParameterSearch, Util::makeUnique<FinalParameterSearchPass>());
    registerPass(PassID::UnusedStatementRemoval, Util::makeUnique<UnusedStatementRemovalPass>());
    registerPass(PassID::ParameterSymbolMap, Util::makeUnique<ParameterSymbolMapPass>());
    registerPass(PassID::UnusedLocalRemoval, Util::makeUnique<UnusedLocalRemovalPass>());
    registerPass(PassID::UnusedParamRemoval, Util::makeUnique<UnusedParamRemovalPass>());
    registerPass(PassID::ImplicitPlacement, Util::makeUnique<ImplicitPlacementPass>());
    registerPass(PassID::LocalAndParamMap, Util::makeUnique<LocalAndParamMapPass>());

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


bool PassManager::createPassGroup(const QString &name, const std::initializer_list<IPass *> &passes)
{
    auto it = m_passGroups.find(name);
    if (it != m_passGroups.end()) {
        LOG_WARN("Cannot create pass group with name '%1': "
                 "A group of the same name already exists",
                 name);
        return false;
    }

    m_passGroups.insert(name, PassGroup(name, passes));
    return true;
}


bool PassManager::executePass(PassID passID, UserProc *proc)
{
    return executePass(getPass(passID), proc);
}


bool PassManager::executePass(IPass *pass, UserProc *proc)
{
    assert(pass != nullptr);
    LOG_VERBOSE("Executing pass '%1' for '%2'", pass->getName(), proc->getName());

    const bool changed = pass->execute(proc);

    QString msg = QString("after executing pass '%1'").arg(pass->getName());
    proc->debugPrintAll(qPrintable(msg));
    proc->getProg()->getProject()->alertDecompileDebugPoint(proc, qPrintable(msg));

    return changed;
}


bool PassManager::executePassGroup(const QString &name, UserProc *proc)
{
    auto it = m_passGroups.find(name);
    if (it == m_passGroups.end()) {
        throw std::invalid_argument(
            QString("Pass group '%1' does not exist").arg(name).toStdString());
    }

    const PassGroup &group = it.value();
    bool changed           = false;

    LOG_VERBOSE("Executing pass group '%1' for '%2'", name, proc->getName());
    for (IPass *pass : group) {
        changed |= executePass(pass, proc);
    }

    return changed;
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
