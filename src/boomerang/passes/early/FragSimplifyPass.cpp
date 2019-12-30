#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "FragSimplifyPass.h"

#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/util/log/Log.h"


FragSimplifyPass::FragSimplifyPass()
    : IPass("FragSimplify", PassID::FragSimplify)
{
}


bool FragSimplifyPass::execute(UserProc *proc)
{
    for (IRFragment *frag : *proc->getCFG()) {
        frag->simplify();
    }

    return simplifyPhis(proc);
}


bool FragSimplifyPass::simplifyPhis(UserProc *proc)
{
    bool change     = false;
    bool thisChange = false;

    do {
        thisChange = false;

        for (IRFragment *frag : *proc->getCFG()) {
            RTL *phiRTL = frag && frag->getRTLs() && !frag->getRTLs()->empty() ?
                frag->getRTLs()->front().get() : nullptr;

            if (!phiRTL || phiRTL->getAddress() != Address::ZERO) {
                // no phis
                continue;
            }

            for (SharedStmt &s : *phiRTL) {
                if (s->isImplicit()) {
                    continue;
                }
                else if (!s->isPhi()) {
                    break; // no more phis
                }

                std::shared_ptr<PhiAssign> phi = s->as<PhiAssign>();
                if (phi->getDefs().empty()) {
                    continue;
                }

                bool allSame        = true;
                SharedStmt firstDef = (*phi->begin())->getDef();

                for (auto &refExp : *phi) {
                    if (refExp->getDef() != firstDef) {
                        allSame = false;
                        break;
                    }
                }

                if (allSame) {
                    LOG_VERBOSE("all the same in %1", phi);
                    proc->replacePhiByAssign(phi, RefExp::get(phi->getLeft(), firstDef));
                    thisChange = true;
                    break;
                }

                bool onlyOneNotThis = true;
                SharedStmt notthis  = STMT_WILD;

                for (const std::shared_ptr<RefExp> &ref : *phi) {
                    SharedStmt def = ref->getDef();
                    if (def == phi) {
                        continue; // ok
                    }
                    else if (notthis == STMT_WILD) {
                        notthis = def;
                    }
                    else {
                        onlyOneNotThis = false;
                        break;
                    }
                }

                if (onlyOneNotThis && (notthis != STMT_WILD)) {
                    LOG_VERBOSE("All but one not this in %1", phi);

                    proc->replacePhiByAssign(phi, RefExp::get(phi->getLeft(), notthis));
                    thisChange = true;
                    break;
                }
            }

            if (thisChange) {
                // already replaced something. Restart from the beginning since all iterators
                // are invalidated.
                break;
            }
        }

        change |= thisChange;
    } while (thisChange);

    return change;
}
