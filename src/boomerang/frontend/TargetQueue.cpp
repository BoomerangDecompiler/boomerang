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

#include "boomerang/db/LowLevelCFG.h"
#include "boomerang/util/log/Log.h"


TargetQueue::TargetQueue(bool traceDecoder)
    : m_traceDecoder(traceDecoder)
{
}


void TargetQueue::pushAddress(LowLevelCFG *cfg, Address newAddr, BasicBlock *&newBB)
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

    if (m_traceDecoder) {
        LOG_MSG(">%1", addr);
    }
}


Address TargetQueue::popAddress(const LowLevelCFG &cfg)
{
    while (!m_targets.empty()) {
        Address address = m_targets.front();
        m_targets.pop();

        if (m_traceDecoder) {
            LOG_MSG("<%1", address);
        }

        // Don't return start adresses of already decoded Basic Blocks
        if (!cfg.isStartOfCompleteBB(address)) {
            return address;
        }
    }

    return Address::INVALID;
}
