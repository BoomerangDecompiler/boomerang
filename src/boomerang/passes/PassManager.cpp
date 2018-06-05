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


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/proc/UserProc.h"

#include "boomerang/passes/dataflow/DominatorPass.h"
#include "boomerang/passes/dataflow/PhiPlacementPass.h"
#include "boomerang/passes/dataflow/BlockVarRenamePass.h"
#include "boomerang/passes/call/CallDefineUpdatePass.h"
#include "boomerang/passes/call/CallArgumentUpdatePass.h"
#include "boomerang/passes/early/StatementInitPass.h"
#include "boomerang/passes/early/GlobalConstReplacePass.h"
#include "boomerang/passes/early/StatementPropagationPass.h"
#include "boomerang/passes/early/BBSimplifyPass.h"
#include "boomerang/passes/middle/CallAndPhiFixPass.h"
#include "boomerang/passes/middle/SPPreservationPass.h"
#include "boomerang/passes/middle/PreservationAnalysisPass.h"
#include "boomerang/passes/middle/StrengthReductionReversalPass.h"
#include "boomerang/passes/middle/AssignRemovalPass.h"
#include "boomerang/passes/middle/DuplicateArgsRemovalPass.h"
#include "boomerang/passes/middle/ParameterSymbolMapPass.h"
#include "boomerang/passes/late/CallLivenessRemovalPass.h"
#include "boomerang/passes/late/LocalTypeAnalysisPass.h"
#include "boomerang/passes/late/BranchAnalysisPass.h"
#include "boomerang/passes/late/FromSSAFormPass.h"
#include "boomerang/passes/late/FinalParameterSearchPass.h"
#include "boomerang/passes/late/UnusedStatementRemovalPass.h"
#include "boomerang/passes/late/UnusedLocalRemovalPass.h"
#include "boomerang/passes/late/ImplicitPlacementPass.h"

#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"

#include <cassert>


static PassManager g_passManager;


PassManager::PassManager()
{
    m_passes.resize(static_cast<size_t>(PassID::NUM_PASSES));

    m_passes[static_cast<size_t>(PassID::Dominators               )].reset(new DominatorPass());
    m_passes[static_cast<size_t>(PassID::PhiPlacement             )].reset(new PhiPlacementPass());
    m_passes[static_cast<size_t>(PassID::BlockVarRename           )].reset(new BlockVarRenamePass());
    m_passes[static_cast<size_t>(PassID::CallDefineUpdate         )].reset(new CallDefineUpdatePass());
    m_passes[static_cast<size_t>(PassID::CallArgumentUpdate       )].reset(new CallArgumentUpdatePass());
    m_passes[static_cast<size_t>(PassID::StatementInit            )].reset(new StatementInitPass());
    m_passes[static_cast<size_t>(PassID::GlobalConstReplace       )].reset(new GlobalConstReplacePass());
    m_passes[static_cast<size_t>(PassID::StatementPropagation     )].reset(new StatementPropagationPass());
    m_passes[static_cast<size_t>(PassID::BBSimplify               )].reset(new BBSimplifyPass());
    m_passes[static_cast<size_t>(PassID::CallAndPhiFix            )].reset(new CallAndPhiFixPass());
    m_passes[static_cast<size_t>(PassID::SPPreservation           )].reset(new SPPreservationPass());
    m_passes[static_cast<size_t>(PassID::PreservationAnalysis     )].reset(new PreservationAnalysisPass());
    m_passes[static_cast<size_t>(PassID::StrengthReductionReversal)].reset(new StrengthReductionReversalPass());
    m_passes[static_cast<size_t>(PassID::AssignRemoval            )].reset(new AssignRemovalPass());
    m_passes[static_cast<size_t>(PassID::DuplicateArgsRemoval     )].reset(new DuplicateArgsRemovalPass());
    m_passes[static_cast<size_t>(PassID::CallLivenessRemoval      )].reset(new CallLivenessRemovalPass());
    m_passes[static_cast<size_t>(PassID::LocalTypeAnalysis        )].reset(new LocalTypeAnalysisPass());
    m_passes[static_cast<size_t>(PassID::BranchAnalysis           )].reset(new BranchAnalysisPass());
    m_passes[static_cast<size_t>(PassID::FromSSAForm              )].reset(new FromSSAFormPass());
    m_passes[static_cast<size_t>(PassID::FinalParameterSearch     )].reset(new FinalParameterSearchPass());
    m_passes[static_cast<size_t>(PassID::UnusedStatementRemoval   )].reset(new UnusedStatementRemovalPass());
    m_passes[static_cast<size_t>(PassID::ParameterSymbolMap       )].reset(new ParameterSymbolMapPass());
    m_passes[static_cast<size_t>(PassID::UnusedLocalRemoval       )].reset(new UnusedLocalRemovalPass());
    m_passes[static_cast<size_t>(PassID::ImplicitPlacement        )].reset(new ImplicitPlacementPass());

    for (auto& pass : m_passes) {
        Q_UNUSED(pass);
        assert(pass.get());
    }
}


PassManager *PassManager::get()
{
    return &g_passManager;
}


bool PassManager::createPassGroup(const QString& name, const std::initializer_list<IPass *>& passes)
{
    auto it = m_passGroups.find(name);
    if (it != m_passGroups.end()) {
        LOG_WARN("Cannot create pass group with name '%1': "
            "A group of the same name already exists", name);
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
    Boomerang::get()->alertDecompileDebugPoint(proc, qPrintable(msg));

    return changed;
}


bool PassManager::executePassGroup(const QString& name, UserProc *proc)
{
    auto it = m_passGroups.find(name);
    if (it == m_passGroups.end()) {
        throw std::invalid_argument(QString("Pass group '%1' does not exist").arg(name).toStdString());
    }

    const PassGroup &group = it.value();
    bool changed = false;

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
