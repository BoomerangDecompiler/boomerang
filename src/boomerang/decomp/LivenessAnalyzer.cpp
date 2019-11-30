#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "LivenessAnalyzer.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/IRFragment.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/util/ConnectionGraph.h"
#include "boomerang/util/log/Log.h"

#include <deque>


/**
 * Check for overlap of liveness between the currently live locations (liveLocs) and the set of
 * locations in \p ls.
 * Also check for type conflicts when using DFA type analysis
 * This is a helper function.
 */
void checkForOverlap(LocationSet &liveLocs, LocationSet &ls, ConnectionGraph &ig, UserProc *proc)
{
    // For each location to be considered
    for (SharedExp exp : ls) {
        if (!exp->isSubscript()) {
            continue; // Only interested in subscripted vars
        }

        assert(std::dynamic_pointer_cast<RefExp>(exp) != nullptr);
        auto refexp = exp->access<RefExp>();

        // Interference if we can find a live variable which differs only in the reference
        SharedExp dr;

        if (liveLocs.findDifferentRef(refexp, dr)) {
            assert(dr->access<RefExp>()->getDef() != nullptr);
            assert(exp->access<RefExp>()->getDef() != nullptr);
            // We have an interference between r and dr. Record it
            ig.connect(refexp, dr);

            if (proc->getProg()->getProject()->getSettings()->debugLiveness) {
                LOG_MSG("Interference of %1 with %2", dr, refexp);
            }
        }

        // Add the uses one at a time. Note: don't use makeUnion, because then we don't discover
        // interferences from the same statement, e.g.  blah := r24{2} + r24{3}
        liveLocs.insert(exp);
    }
}


bool LivenessAnalyzer::calcLiveness(IRFragment *frag, ConnectionGraph &ig, UserProc *myProc)
{
    // Start with the liveness at the bottom of the fragment
    LocationSet liveLocs, phiLocs;
    getLiveOut(frag, liveLocs, phiLocs);

    // Do the livenesses that result from phi statements at successors first.
    // FIXME: document why this is necessary
    checkForOverlap(liveLocs, phiLocs, ig, myProc);

    const bool assumeABICompliance = myProc->getProg()->getProject()->getSettings()->assumeABI;
    const bool debugLiveness       = myProc->getProg()->getProject()->getSettings()->debugLiveness;

    if (frag->getRTLs()) {
        // For all statements in this fragment in reverse order
        IRFragment::RTLRIterator rit;
        StatementList::reverse_iterator sit;

        for (SharedStmt s = frag->getLastStmt(rit, sit); s; s = frag->getPrevStmt(rit, sit)) {
            LocationSet defs;
            s->getDefinitions(defs, assumeABICompliance);

            // The definitions don't have refs yet
            defs.addSubscript(s);

            // Definitions kill uses. Now we are moving to the "top" of statement s
            liveLocs.makeDiff(defs);

            // Phi functions are a special case. The operands of phi functions are uses,
            // but they don't interfere with each other (since they come via different fragments).
            // However, we don't want to put these uses into liveLocs, because then the
            // livenesses will flow to all predecessors. Only the appropriate livenesses from
            // the appropriate phi parameter should flow to the predecessor.
            // This is done in getLiveOut()
            if (s->isPhi()) {
                continue;
            }

            // Check for livenesses that overlap
            LocationSet uses;
            s->addUsedLocs(uses);
            checkForOverlap(liveLocs, uses, ig, myProc);

            if (debugLiveness) {
                LOG_MSG(" ## liveness: at top of %1, liveLocs is %2", s, liveLocs.toString());
            }
        }
    }

    // liveIn is what we calculated last time
    if (!(liveLocs == m_liveIn[frag])) {
        m_liveIn[frag] = liveLocs;
        return true; // A change
    }

    // No change
    return false;
}


void LivenessAnalyzer::getLiveOut(IRFragment *frag, LocationSet &liveout, LocationSet &phiLocs)
{
    ProcCFG *cfg         = frag->getProc()->getCFG();
    const bool debugLive = cfg->getProc()->getProg()->getProject()->getSettings()->debugLiveness;

    liveout.clear();

    for (IRFragment *currFrag : frag->getSuccessors()) {
        // First add the non-phi liveness
        liveout.makeUnion(m_liveIn[currFrag]); // add successor liveIn to this liveout set.

        // The first RTL will have the phi functions, if any
        if (!currFrag->getRTLs() || currFrag->getRTLs()->empty()) {
            continue;
        }

        RTL *phiRTL = currFrag->getRTLs()->front().get();
        assert(phiRTL);

        for (SharedStmt st : *phiRTL) {
            // Only interested in phi assignments. Note that it is possible that some phi
            // assignments have been converted to ordinary assignments. So the below is a continue,
            // not a break.
            if (!st->isPhi()) {
                continue;
            }

            std::shared_ptr<PhiAssign> pa = st->as<PhiAssign>();

            for (const auto &v : pa->getDefs()) {
                if (!cfg->hasFragment(v.first)) {
                    LOG_WARN("Someone removed the fragment that defined the Phi! "
                             "Need to update PhiAssign defs");
                }
            }

            // Get the jth operand to the phi function; it has a use from fragment *this
            // assert(j>=0);
            SharedStmt def = pa->getStmtAt(frag);

            if (!def) {
                std::set<IRFragment *> tried{ frag };
                std::deque<IRFragment *> to_visit(frag->getPredecessors().begin(),
                                                  frag->getPredecessors().end());

                // TODO: this looks like a hack ?  but sometimes PhiAssign has value which is
                // defined in parent of 'this'
                //  frag1 1  - defines r20
                //  frag2 33 - transfers control to frag3
                //  frag3 40 - r10 = phi { 1 }
                while (!to_visit.empty()) {
                    IRFragment *theFrag = to_visit.back();

                    if (tried.find(theFrag) != tried.end()) {
                        to_visit.pop_back();
                        continue;
                    }

                    def = pa->getStmtAt(theFrag);

                    if (def) {
                        break;
                    }

                    tried.insert(theFrag);
                    to_visit.pop_back();

                    for (IRFragment *pred : theFrag->getPredecessors()) {
                        if (tried.find(pred) != tried.end()) { // already tried
                            continue;
                        }

                        to_visit.push_back(pred);
                    }
                }
            }

            if (!def) {
                // This is not defined anywhere, so it is an initial parameter.
                def = cfg->findOrCreateImplicitAssign(pa->getLeft());
            }

            assert(def);
            SharedExp ref = RefExp::get(pa->getLeft()->clone(), def);
            liveout.insert(ref);
            phiLocs.insert(ref);

            if (debugLive) {
                LOG_MSG(" ## Liveness: adding %1 due due to ref to phi %2 in fragment at %3", ref,
                        st, frag->getLowAddr());
            }
        }
    }
}
