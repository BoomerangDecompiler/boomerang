#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "TargetQueue.h"

#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/util/log/Log.h"


TargetQueue::TargetQueue(bool traceDecoder)
    : m_traceDecoder(traceDecoder)
{
}


void TargetQueue::pushAddress(ProcCFG *cfg, Address newAddr, BasicBlock *&newBB)
{
    if (cfg->isStartOfBB(newAddr)) {
        // BB is already complete or the start address is already in the queue.
        // Don't visit it twice.
        return;
    }

    // Find out if we've already parsed the destination
    const bool alreadyParsed = cfg->ensureBBExists(newAddr, newBB);

    // Add this address to the back of the local queue,
    // if not already processed
    if (!alreadyParsed) {
        m_targets.push(newAddr);

        if (m_traceDecoder) {
            LOG_MSG(">%1", newAddr);
        }
    }
}


void TargetQueue::initial(Address addr)
{
    m_targets.push(addr);
}


Address TargetQueue::popAddress(const ProcCFG &cfg)
{
    while (!m_targets.empty()) {
        Address address = m_targets.front();
        m_targets.pop();

        if (m_traceDecoder) {
            LOG_MSG("<%1", address);
        }

        // If no label there at all, or if there is a BB, it's incomplete, then we can parse this
        // address next
        if (!cfg.isStartOfBB(address) || cfg.isStartOfIncompleteBB(address)) {
            return address;
        }
    }

    return Address::INVALID;
}
