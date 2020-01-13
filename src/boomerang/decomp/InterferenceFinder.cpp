#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "InterferenceFinder.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/IRFragment.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/util/log/Log.h"


InterferenceFinder::InterferenceFinder(ProcCFG *cfg)
    : m_cfg(cfg)
{
}


void InterferenceFinder::findInterferences(ConnectionGraph &ig)
{
    if (m_cfg->getNumFragments() == 0) {
        return;
    }

    std::list<IRFragment *> workList; // List of fragments still to be processed
    std::set<IRFragment *> workSet;   // Set of the same; used for quick membership test
    appendFrags(workList, workSet);

    int count            = 0;
    const bool debugLive = m_cfg->getProc()->getProg()->getProject()->getSettings()->debugLiveness;

    while (!workList.empty() && count++ < 1E5) {
        IRFragment *currFrag = workList.back();
        workList.erase(--workList.end());
        workSet.erase(currFrag);

        // Calculate live locations and interferences
        assert(currFrag->getProc() != nullptr);
        bool change = m_livenessAna.calcLiveness(currFrag, ig, currFrag->getProc());

        if (!change) {
            continue;
        }

        if (debugLive) {
            SharedStmt last = currFrag->getLastStmt();

            LOG_MSG("Revisiting BB ending with stmt %1 due to change",
                    last ? QString::number(last->getNumber(), 10) : "<none>");
        }

        updateWorkListRev(currFrag, workList, workSet);
    }
}


void InterferenceFinder::updateWorkListRev(IRFragment *currFrag, std::list<IRFragment *> &workList,
                                           std::set<IRFragment *> &workSet)
{
    // Insert inedges of currFrag into the worklist, unless already there
    for (IRFragment *pred : currFrag->getPredecessors()) {
        if (workSet.find(pred) == workSet.end()) {
            workList.push_front(pred);
            workSet.insert(pred);
        }
    }
}


void InterferenceFinder::appendFrags(std::list<IRFragment *> &workList,
                                     std::set<IRFragment *> &workSet)
{
    workList.insert(workList.end(), m_cfg->begin(), m_cfg->end());

    std::copy(m_cfg->begin(), m_cfg->end(), std::inserter(workSet, workSet.end()));
}
