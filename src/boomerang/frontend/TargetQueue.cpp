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


#include "boomerang/db/CFG.h"
#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Log.h"


void TargetQueue::visit(Cfg *cfg, Address newAddr, BasicBlock *& newBB)
{
    // Find out if we've already parsed the destination
    const bool alreadyParsed = cfg->label(newAddr, newBB);

    // Add this address to the back of the local queue,
    // if not already processed
    if (!alreadyParsed) {
        m_targets.push(newAddr);

        if (SETTING(traceDecoder)) {
            LOG_MSG(">%1", newAddr);
        }
    }
}


void TargetQueue::initial(Address uAddr)
{
    m_targets.push(uAddr);
}


Address TargetQueue::getNextAddress(const Cfg& cfg)
{
    while (!m_targets.empty()) {
        Address address = m_targets.front();
        m_targets.pop();

        if (SETTING(traceDecoder)) {
            LOG_MSG("<%1", address);
        }

        // If no label there at all, or if there is a BB, it's incomplete, then we can parse this address next
        if (!cfg.existsBB(address) || cfg.isIncomplete(address)) {
            return address;
        }
    }

    return Address::INVALID;
}


void TargetQueue::dump()
{
    std::queue<Address> copy(m_targets);

    while (!copy.empty()) {
        Address addr = copy.front();
        copy.pop();
        LOG_MSG("  %1,", addr);
    }
}
