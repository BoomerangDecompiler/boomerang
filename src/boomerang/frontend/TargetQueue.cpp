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


void TargetQueue::visit(Cfg *pCfg, Address uNewAddr, BasicBlock *& pNewBB)
{
    // Find out if we've already parsed the destination
    bool alreadyParsed = pCfg->label(uNewAddr, pNewBB);

    // Add this address to the back of the local queue,
    // if not already processed
    if (!alreadyParsed) {
        targets.push(uNewAddr);

        if (SETTING(traceDecoder)) {
            LOG_MSG(">%1", uNewAddr);
        }
    }
}


void TargetQueue::initial(Address uAddr)
{
    targets.push(uAddr);
}


Address TargetQueue::nextAddress(const Cfg& cfg)
{
    while (!targets.empty()) {
        Address address = targets.front();
        targets.pop();

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
    std::queue<Address> copy(targets);

    while (!copy.empty()) {
        Address a = copy.front();
        copy.pop();
        LOG_MSG("  %1,", a);
    }
}
