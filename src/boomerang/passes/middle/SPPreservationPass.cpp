#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SPPreservationPass.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/util/Log.h"


SPPreservationPass::SPPreservationPass()
    : IPass("SPPreservation", PassID::SPPreservation)
{
}


bool SPPreservationPass::execute(UserProc *proc)
{
    bool stdsp = false; // FIXME: are these really used?
    // Note: need this non-virtual version most of the time, since nothing proved yet
    int sp = Util::getStackRegisterIndex(proc->getProg());

    for (int n = 0; n < 2; n++) {
        // may need to do multiple times due to dependencies FIXME: efficiency! Needed any more?

        // Special case for 32-bit stack-based machines (e.g. Pentium).
        // RISC machines generally preserve the stack pointer (so no special case required)
        for (int p = 0; !stdsp && p < 8; p++) {
            if (DEBUG_PROOF) {
                LOG_MSG("Attempting to prove sp = sp + %1 for %2", p * 4, getName());
            }

            stdsp = proc->prove(
                Binary::get(opEquals,
                            Location::regOf(sp),
                            Binary::get(opPlus, Location::regOf(sp), Const::get(p * 4))));
        }
    }

    if (DEBUG_PROOF) {
        LOG_MSG("Proven for %1:", getName());

        for (auto& elem : proc->getProvenTrue()) {
            LOG_MSG("    %1 = %2", elem.first, elem.second);
        }
    }

    return true;
}
