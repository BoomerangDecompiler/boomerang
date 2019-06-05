#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementPropagationPass.h"

#include "boomerang/core/Project.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expvisitor/ExpDestCounter.h"
#include "boomerang/visitor/stmtexpvisitor/StmtDestCounter.h"


StatementPropagationPass::StatementPropagationPass()
    : IPass("StatementPropagation", PassID::StatementPropagation)
{
}


bool StatementPropagationPass::execute(UserProc *proc)
{
    StatementList stmts;
    proc->getStatements(stmts);

    // Find the locations that are used by a live, dominating phi-function
    LocationSet usedByDomPhi;
    if (!findLiveAtDomPhi(proc, usedByDomPhi)) {
        return false;
    }

    // Next pass: count the number of times each assignment LHS would be propagated somewhere
    std::map<SharedExp, int, lessExpStar> destCounts;

    // Also maintain a set of locations which are used by phi statements
    for (Statement *s : stmts) {
        ExpDestCounter edc(destCounts);
        StmtDestCounter sdc(&edc);
        s->accept(&sdc);
    }

    // A fourth pass to propagate only the flags (these must be propagated even if it results in
    // extra locals)
    bool change = false;

    Settings *settings = proc->getProg()->getProject()->getSettings();
    for (Statement *s : stmts) {
        if (!s->isPhi()) {
            change |= s->propagateFlagsTo(settings);
        }
    }

    // Finally the actual propagation
    for (Statement *s : stmts) {
        if (!s->isPhi()) {
            change |= s->propagateTo(settings, &destCounts, &usedByDomPhi);
        }
    }

    propagateToCollector(&proc->getUseCollector());

    return change;
}


bool StatementPropagationPass::findLiveAtDomPhi(UserProc *proc, LocationSet &usedByDomPhi)
{
    LocationSet usedByDomPhi0;
    std::map<SharedExp, PhiAssign *, lessExpStar> defdByPhi;

    if (!proc->getDataFlow()->findLiveAtDomPhi(usedByDomPhi, usedByDomPhi0, defdByPhi)) {
        return false;
    }

    // Note that the above is not the complete algorithm; it has found the dead phi-functions
    // in the defdAtPhi
    for (auto &def : defdByPhi) {
        // For each phi parameter, remove from the final usedByDomPhi set
        for (const std::shared_ptr<RefExp> &v : *def.second) {
            assert(v->getSubExp1());
            std::shared_ptr<RefExp> wrappedParam = RefExp::get(v->getSubExp1(), v->getDef());
            usedByDomPhi.remove(wrappedParam);
        }

        // Ick - some problem with return statements not using their returns
        // until more analysis is done
        // removeStatement(it->second);
    }

    return true;
}


void StatementPropagationPass::propagateToCollector(UseCollector *collector)
{
    // TODO propagateToCollector(proc->getUseCollector());
    for (auto it = collector->begin(); it != collector->end();) {
        if (!(*it)->isMemOf()) {
            ++it;
            continue;
        }

        auto addr = (*it)->getSubExp1();
        LocationSet used;
        addr->addUsedLocs(used);

        for (const SharedExp &v : used) {
            if (!v->isSubscript()) {
                continue;
            }

            auto r = v->access<RefExp>();
            if (!r->getDef() || !r->getDef()->isAssign()) {
                continue;
            }

            Assign *as = static_cast<Assign *>(r->getDef());

            bool ch;
            auto res = addr->clone()->searchReplaceAll(*r, as->getRight(), ch);

            if (!ch) {
                continue; // No change
            }

            auto memOfRes = Location::memOf(res)->simplify();

            // First check to see if memOfRes is already in the set
            if (collector->exists(memOfRes)) {
                // Take care not to use an iterator to the newly erased element.
                /* it = */
                collector->remove(it++); // Already exists; just remove the old one
                continue;
            }
            else {
                LOG_VERBOSE("Propagating %1 to %2 in collector; result %3", r, as->getRight(),
                            memOfRes);
                (*it)->setSubExp1(res); // Change the child of the memof
            }
        }

        ++it;
    }
}
