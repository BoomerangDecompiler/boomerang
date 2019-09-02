#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UnusedStatementRemovalPass.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/util/StatementSet.h"
#include "boomerang/util/log/Log.h"


UnusedStatementRemovalPass::UnusedStatementRemovalPass()
    : IPass("UnusedStatementRemoval", PassID::UnusedStatementRemoval)
{
}


bool UnusedStatementRemovalPass::execute(UserProc *proc)
{
    // Only remove unused statements after decompiling as much as possible of the proc
    // Remove unused statements
    RefCounter refCounts; // The map
    // Count the references first
    updateRefCounts(proc, refCounts);

    // Now remove any that have no used
    if (proc->getProg()->getProject()->getSettings()->removeNull) {
        remUnusedStmtEtc(proc, refCounts);
        removeNullStatements(proc);
        proc->debugPrintAll("after removing unused and null statements pass 1");
    }

    return true;
}


void UnusedStatementRemovalPass::updateRefCounts(UserProc *proc, RefCounter &refCounts)
{
    StatementList stmts;
    proc->getStatements(stmts);

    for (SharedStmt s : stmts) {
        // Don't count uses in implicit statements. There is no RHS of course,
        // but you can still have x from m[x] on the LHS and so on, but these are not real uses
        if (s->isImplicit()) {
            continue;
        }

        if (proc->getProg()->getProject()->getSettings()->debugUnused) {
            LOG_MSG("Counting references in %1", s);
        }

        LocationSet refs;
        s->addUsedLocs(refs, false); // Ignore uses in collectors

        for (const SharedExp &rr : refs) {
            if (rr->isSubscript()) {
                SharedStmt def = rr->access<RefExp>()->getDef();

                // Used to not count implicit refs here (def->getNumber() == 0), meaning that
                // implicit definitions get removed as dead code! But these are the ideal place to
                // read off final parameters, and it is guaranteed now that implicit statements are
                // sorted out for us by now (for dfa type analysis)
                if (def /* && def->getNumber() */) {
                    refCounts[def]++;

                    if (proc->getProg()->getProject()->getSettings()->debugUnused) {
                        LOG_MSG("counted ref to %1", rr);
                    }
                }
            }
        }
    }

    if (proc->getProg()->getProject()->getSettings()->debugUnused) {
        LOG_MSG("### Reference counts for %1:", proc->getName());

        for (RefCounter::iterator rr = refCounts.begin(); rr != refCounts.end(); ++rr) {
            LOG_MSG("  %1: %2", rr->first->getNumber(), rr->second);
        }

        LOG_MSG("### End reference counts");
    }
}


void UnusedStatementRemovalPass::remUnusedStmtEtc(UserProc *proc, RefCounter &refCounts)
{
    StatementList stmts;
    proc->getStatements(stmts);
    bool change;

    do { // FIXME: check if this is ever needed
        change                     = false;
        StatementList::iterator ll = stmts.begin();

        while (ll != stmts.end()) {
            SharedStmt s = *ll;

            if (!s->isAssignment()) {
                // Never delete a statement other than an assignment (e.g. nothing "uses" a Jcond)
                ++ll;
                continue;
            }

            const std::shared_ptr<Assignment> as  = s->as<Assignment>();
            SharedConstExp asLeft = as->getLeft();

            if (asLeft && (asLeft->getOper() == opGlobal)) {
                // assignments to globals must always be kept
                ++ll;
                continue;
            }

            // If it's a memof and renameable it can still be deleted
            if (asLeft->isMemOf() && !proc->canRename(asLeft)) {
                // Assignments to memof-anything-but-local must always be kept.
                ++ll;
                continue;
            }

            if (asLeft->isMemberOf() || asLeft->isArrayIndex()) {
                // can't say with these; conservatively never remove them
                ++ll;
                continue;
            }

            // Care not to insert unnecessarily
            if ((refCounts.find(s) == refCounts.end()) || (refCounts[s] == 0)) {
                // First adjust the counts, due to statements only referenced by statements that are
                // themselves unused. Need to be careful not to count two refs to the same def as
                // two; refCounts is a count of the number of statements that use a definition, not
                // the total number of refs
                StatementSet stmtsRefdByUnused;
                LocationSet components;
                // Second parameter false to ignore uses in collectors
                s->addUsedLocs(components, false);

                for (const SharedExp &component : components) {
                    if (component->isSubscript() && component->access<RefExp>()->getDef()) {
                        stmtsRefdByUnused.insert(component->access<RefExp>()->getDef());
                    }
                }

                for (SharedStmt refd : stmtsRefdByUnused) {
                    if (refd == nullptr) {
                        continue;
                    }

                    if (proc->getProg()->getProject()->getSettings()->debugUnused) {
                        LOG_MSG("Decrementing ref count of %1 because %2 is unused",
                                refd->getNumber(), s->getNumber());
                    }

                    refCounts[refd]--;
                }

                if (proc->getProg()->getProject()->getSettings()->debugUnused) {
                    LOG_MSG("Removing unused statement %1 %2", s->getNumber(), s);
                }

                proc->removeStatement(s);
                ll     = stmts.erase(ll); // So we don't try to re-remove it
                change = true;
                continue; // Don't call getNext this time
            }

            ++ll;
        }
    } while (change);

    // Recalulate at least the livenesses. Example: first call to printf in test/pentium/fromssa2,
    // eax used only in a removed statement, so liveness in the call needs to be removed
    PassManager::get()->executePass(PassID::CallLivenessRemoval, proc);
    PassManager::get()->executePass(PassID::BlockVarRename, proc);

    // Now fully decompiled (apart from one final pass, and transforming out of SSA form)
    proc->setStatus(ProcStatus::FinalDone);
}


bool UnusedStatementRemovalPass::removeNullStatements(UserProc *proc)
{
    bool change = false;
    StatementList stmts;
    proc->getStatements(stmts);

    // remove null code
    for (SharedStmt s : stmts) {
        if (s->isNullStatement()) {
            // A statement of the form x := x
            LOG_VERBOSE("Removing null statement: %1 %2", s->getNumber(), s);

            proc->removeStatement(s);
            change = true;
        }
    }

    return change;
}
