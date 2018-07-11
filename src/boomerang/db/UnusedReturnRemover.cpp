#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UnusedReturnRemover.h"


#include "boomerang/core/Project.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/statements/ReturnStatement.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/util/Log.h"
#include "boomerang/visitor/expmodifier/ImplicitConverter.h"


UnusedReturnRemover::UnusedReturnRemover(Prog *prog)
    : m_prog(prog)
{
}


bool UnusedReturnRemover::removeUnusedReturns()
{
    bool change = false;

    for (const auto& module : m_prog->getModuleList()) {
        for (Function *proc : *module) {
            if (proc && !proc->isLib() && static_cast<UserProc *>(proc)->isDecoded()) {
                m_removeRetSet.insert(static_cast<UserProc *>(proc));
            }
            // else e.g. use -sf file to just prototype the proc
        }
    }

    // The workset is processed in arbitrary order. May be able to do better,
    // but note that sometimes changes propagate down the call tree
    // (no caller uses potential returns for child), and sometimes up the call tree
    // (removal of returns and/or dead code removes parameters, which affects all callers).
    while (!m_removeRetSet.empty()) {
        auto it = m_removeRetSet.begin(); // Pick the first element of the set
        const bool removedReturns = removeRedundantReturns(*it);

        if (removedReturns) {
            // Removing returns changes the uses of the callee.
            // So we have to do type analyis to update the use information.
            PassManager::get()->executePass(PassID::LocalTypeAnalysis, *it);
        }
        change |= removedReturns;

        // Note: removing the currently processed item here should prevent
        // unnecessary reprocessing of self recursive procedures
        m_removeRetSet.erase(it);
    }

    return change;
}


bool UnusedReturnRemover::removeRedundantReturns(UserProc *proc)
{
    assert(m_removeRetSet.find(proc) != m_removeRetSet.end());

    m_prog->getProject()->alertDecompiling(proc);
    m_prog->getProject()->alertDecompileDebugPoint(proc, "before removing unused returns");
    // First remove the unused parameters
    bool removedParams = removeRedundantParameters(proc);

    if (proc->getRetStmt() == nullptr) {
        return removedParams;
    }

    if (m_prog->getProject()->getSettings()->debugUnused) {
        LOG_MSG("%%% removing unused returns for %1 %%%", proc->getName());
    }

    if (proc->getSignature()->isForced()) {
        // Respect the forced signature, but use it to remove returns if necessary
        bool removedRets = false;

        for (ReturnStatement::iterator rr = proc->getRetStmt()->begin(); rr != proc->getRetStmt()->end();) {
            Assign    *a  = static_cast<Assign *>(*rr);
            SharedExp lhs = a->getLeft();
            // For each location in the returns, check if in the signature
            bool found = false;

            for (int i = 0; i < proc->getSignature()->getNumReturns(); i++) {
                if (*proc->getSignature()->getReturnExp(i) == *lhs) {
                    found = true;
                    break;
                }
            }

            if (found) {
                ++rr; // Yes, in signature; OK
            }
            else {
                // This return is not in the signature. Remove it
                rr          = proc->getRetStmt()->erase(rr);
                removedRets = true;

                if (m_prog->getProject()->getSettings()->debugUnused) {
                    LOG_MSG("%%%  removing unused return %1 from proc %2 (forced signature)", a, proc->getName());
                }
            }
        }

        if (removedRets) {
            // Still may have effects on calls or now unused statements
            updateForUseChange(proc);
        }

        return removedRets;
    }

    // FIXME: this needs to be more sensible when we don't decompile down from main! Probably should assume just the
    // first return is valid, for example (presently assume none are valid)
    LocationSet unionOfCallerLiveLocs;

    if (proc->getName() == "main") { // Probably not needed: main is forced so handled above
        // Just insert one return for main. Note: at present, the first parameter is still the stack pointer
        if (proc->getSignature()->getNumReturns() <= 1) {
            // handle the case of missing main() signature
            LOG_WARN("main signature definition is missing; assuming void main()");
        }
        else {
            unionOfCallerLiveLocs.insert(proc->getSignature()->getReturnExp(1));
        }
    }
    else {
        // For each caller
        for (CallStatement *cc : proc->getCallers()) {
            #if RECURSION_WIP
            // TODO: prevent function from blocking it's own removals, needs more work
            if (cc->getProc()->doesRecurseTo(this)) {
                continue;
            }
            #endif
            // Union in the set of locations live at this call
            UseCollector *useCol = cc->getUseCollector();
            unionOfCallerLiveLocs.makeUnion(useCol->getLocSet());
        }
    }

    // Intersect with the current returns
    bool removedRets = false;

    for (auto rr = proc->getRetStmt()->begin(); rr != proc->getRetStmt()->end();) {
        Assign *a = static_cast<Assign *>(*rr);

        if (unionOfCallerLiveLocs.contains(a->getLeft())) {
            ++rr;
            continue;
        }

        if (m_prog->getProject()->getSettings()->debugUnused) {
            LOG_MSG("%%%  removing unused return %1 from proc %2", a, proc->getName());
        }

        // If a component of the RHS referenced a call statement, the liveness used to be killed here.
        // This was wrong; you need to notice the liveness changing inside updateForUseChange() to correctly
        // recurse to callee
        rr          = proc->getRetStmt()->erase(rr);
        removedRets = true;
    }

    if (m_prog->getProject()->getSettings()->debugUnused) {
        QString     tgt;
        QTextStream ost(&tgt);
        unionOfCallerLiveLocs.print(ost);
        LOG_MSG("%%%  union of caller live locations for %1: %2", proc->getName(), tgt);
        LOG_MSG("%%%  final returns for %1: %2", proc->getName(), proc->getRetStmt()->getReturns().prints());
    }

    // removing returns might result in params that can be removed, might as well do it now.
    removedParams |= removeRedundantParameters(proc);

    ProcSet updateSet; // Set of procs to update

    if (removedParams || removedRets) {
        // Update the statements that call us

        for (CallStatement *call : proc->getCallers()) {
            PassManager::get()->executePass(PassID::CallArgumentUpdate, proc);
            updateSet.insert(call->getProc());    // Make sure we redo the dataflow
            m_removeRetSet.insert(call->getProc()); // Also schedule caller proc for more analysis
        }

        // Now update myself
        updateForUseChange(proc);

        // Update any other procs that need updating
        updateSet.erase(proc); // Already done this proc

        while (!updateSet.empty()) {
            UserProc *_proc = *updateSet.begin();
            updateSet.erase(_proc);
            updateForUseChange(_proc);
        }
    }

    m_prog->getProject()->alertDecompileDebugPoint(proc, "after removing unused and redundant returns");
    return removedRets || removedParams;
}


void UnusedReturnRemover::updateForUseChange(UserProc *proc)
{
    // We need to remember the parameters, and all the livenesses for all the calls, to see if these are changed
    // by removing returns
    if (m_prog->getProject()->getSettings()->debugUnused) {
        LOG_MSG("%%% updating %1 for changes to uses (returns or arguments)", proc->getName());
        LOG_MSG("%%% updating dataflow:");
    }

    // Save the old parameters and call liveness
    const size_t oldNumParameters = proc->getParameters().size();
    std::map<CallStatement *, UseCollector> callLiveness;

    for (BasicBlock *bb : *proc->getCFG()) {
        BasicBlock::RTLRIterator        rrit;
        StatementList::reverse_iterator srit;
        CallStatement *c = dynamic_cast<CallStatement *>(bb->getLastStmt(rrit, srit));

        // Note: we may have removed some statements, so there may no longer be a last statement!
        if (c == nullptr) {
            continue;
        }

        UserProc *dest = dynamic_cast<UserProc *>(c->getDestProc());

        // Not interested in unanalysed indirect calls (not sure) or calls to lib procs
        if (dest == nullptr) {
            continue;
        }

        callLiveness[c].makeCloneOf(*c->getUseCollector());
    }

    // Have to redo dataflow to get the liveness at the calls correct
    PassManager::get()->executePass(PassID::CallLivenessRemoval, proc); // Want to recompute the call livenesses
    PassManager::get()->executePass(PassID::BlockVarRename, proc);

    proc->remUnusedStmtEtc(); // Also redoes parameters

    // Have the parameters changed? If so, then all callers will need to update their arguments, and do similar
    // analysis to the removal of returns
    // findFinalParameters();
    removeRedundantParameters(proc);

    if (proc->getParameters().size() != oldNumParameters) {
        if (m_prog->getProject()->getSettings()->debugUnused) {
            LOG_MSG("%%%  parameters changed for %1", proc->getName());
        }

        std::set<CallStatement *>& callers = proc->getCallers();
        const bool experimental = m_prog->getProject()->getSettings()->experimental;

        for (CallStatement *cc : callers) {
            cc->updateArguments(experimental);
            // Schedule the callers for analysis
            m_removeRetSet.insert(cc->getProc());
        }
    }

    // Check if the liveness of any calls has changed
    for (auto ll = callLiveness.begin(); ll != callLiveness.end(); ++ll) {
        CallStatement *call             = ll->first;
        const UseCollector& oldLiveness = ll->second;
        const UseCollector& newLiveness = *call->getUseCollector();

        if (newLiveness != oldLiveness) {
            if (m_prog->getProject()->getSettings()->debugUnused) {
                LOG_MSG("%%%  Liveness for call to %1 in %2 changed",
                        call->getDestProc()->getName(), proc->getName());
            }

            m_removeRetSet.insert(static_cast<UserProc *>(call->getDestProc()));
        }
    }
}


bool UnusedReturnRemover::checkForGainfulUse(UserProc *proc, SharedExp bparam, ProcSet& visited)
{
    visited.insert(proc); // Prevent infinite recursion

    StatementList stmts;
    proc->getStatements(stmts);

    for (Statement *s : stmts) {
        // Special checking for recursive calls
        if (s->isCall()) {
            CallStatement *c    = static_cast<CallStatement *>(s);
            UserProc      *dest = dynamic_cast<UserProc *>(c->getDestProc());

            if (dest && dest->doesRecurseTo(proc)) {
                // In the destination expression?
                LocationSet u;
                c->getDest()->addUsedLocs(u);

                if (u.containsImplicit(bparam)) {
                    return true; // Used by the destination expression
                }

                // Else check for arguments of the form lloc := f(bparam{0})
                const StatementList& args = c->getArguments();

                for (StatementList::const_iterator aa = args.begin(); aa != args.end(); ++aa) {
                    const Assign *a = dynamic_cast<const Assign *>(*aa);
                    SharedExp   rhs = a ? a->getRight() : nullptr;
                    if (!rhs) {
                        continue;
                    }

                    LocationSet argUses;
                    rhs->addUsedLocs(argUses);

                    if (argUses.containsImplicit(bparam)) {
                        SharedExp lloc = static_cast<Assign *>(*aa)->getLeft();

                        if ((visited.find(dest) == visited.end()) && checkForGainfulUse(dest, lloc, visited)) {
                            return true;
                        }
                    }
                }

                // If get to here, then none of the arguments is of this form, and we can ignore this call
                continue;
            }
        }
        else if (s->isReturn()) {
            if (proc->getRecursionGroup() && !proc->getRecursionGroup()->empty()) { // If this function is involved in recursion
                continue;                               //  then ignore this return statement
            }
        }
        else if (s->isPhi() && (proc->getRetStmt() != nullptr) && proc->getRecursionGroup() && !proc->getRecursionGroup()->empty()) {
            SharedExp  phiLeft = static_cast<PhiAssign *>(s)->getLeft();
            auto       refPhi  = RefExp::get(phiLeft, s);
            bool       foundPhi = false;

            for (Statement *stmt : *proc->getRetStmt()) {
                SharedExp   rhs = static_cast<Assign *>(stmt)->getRight();
                LocationSet uses;
                rhs->addUsedLocs(uses);

                if (uses.contains(refPhi)) {
                    // s is a phi that defines a component of a recursive return. Ignore it
                    foundPhi = true;
                    break;
                }
            }

            if (foundPhi) {
                continue; // Ignore this phi
            }
        }

        // Otherwise, consider uses in s
        LocationSet uses;
        s->addUsedLocs(uses);

        if (uses.containsImplicit(bparam)) {
            return true; // A gainful use
        }
    }                    // for each statement s

    return false;
}


bool UnusedReturnRemover::removeRedundantParameters(UserProc *proc)
{
    if (proc->getSignature()->isForced()) {
        // Assume that no extra parameters would have been inserted... not sure always valid
        return false;
    }

    bool          ret = false;
    StatementList newParameters;

    m_prog->getProject()->alertDecompileDebugPoint(proc, "Before removing redundant parameters");

    if (m_prog->getProject()->getSettings()->debugUnused) {
        LOG_MSG("%%% removing unused parameters for %1", proc->getName());
    }

    // Note: this would be far more efficient if we had def-use information
    for (StatementList::iterator pp = proc->getParameters().begin(); pp != proc->getParameters().end(); ++pp) {
        SharedExp param = static_cast<Assignment *>(*pp)->getLeft();
        bool      az;
        SharedExp bparam = param->clone()->removeSubscripts(az); // FIXME: why does main have subscripts on parameters?
        // Memory parameters will be of the form m[sp + K]; convert to m[sp{0} + K] as will be found in uses
        bparam = bparam->expSubscriptAllNull();                  // Now m[sp{-}+K]{-}
        ImplicitConverter ic(proc->getCFG());
        bparam = bparam->acceptModifier(&ic);                            // Now m[sp{0}+K]{0}
        assert(bparam->isSubscript());
        bparam = bparam->access<Exp, 1>();                       // now m[sp{0}+K] (bare parameter)

        ProcSet visited;

        if (checkForGainfulUse(proc, bparam, visited)) {
            newParameters.append(*pp); // Keep this parameter
        }
        else {
            // Remove the parameter
            ret = true;

            if (m_prog->getProject()->getSettings()->debugUnused) {
                LOG_MSG(" %%% removing unused parameter %1 in %2", param, proc->getName());
            }

            // Check if it is in the symbol map. If so, delete it; a local will be created later
            UserProc::SymbolMap::iterator ss = proc->getSymbolMap().find(param);

            if (ss != proc->getSymbolMap().end()) {
                proc->getSymbolMap().erase(ss);           // Kill the symbol
            }

            proc->getSignature()->removeParameter(param); // Also remove from the signature
            proc->getCFG()->removeImplicitAssign(param);  // Remove the implicit assignment so it doesn't come back
        }
    }

    proc->getParameters() = newParameters;

    if (m_prog->getProject()->getSettings()->debugUnused) {
        LOG_MSG("%%% end removing unused parameters for %1", proc->getName());
    }

    m_prog->getProject()->alertDecompileDebugPoint(proc, "after removing redundant parameters");

    return ret;
}
