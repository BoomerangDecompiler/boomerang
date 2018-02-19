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

#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"


#include <cassert>


static PassManager g_passManager;


PassManager::PassManager()
{
    m_passes.resize(static_cast<size_t>(PassID::NUM_PASSES));

    m_passes[static_cast<size_t>(PassID::Dominators)].reset(new DominatorPass());
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


bool PassManager::executePass(IPass* pass, UserProc* proc)
{
    assert(pass != nullptr);

    LOG_VERBOSE("Executing pass '%1' for '%2'", pass->getName(), proc->getName());
    bool changed = pass->execute(proc);

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
