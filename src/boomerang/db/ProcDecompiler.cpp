#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ProcDecompiler.h"


#include "boomerang/core/Project.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/util/Log.h"


ProcDecompiler::ProcDecompiler(UserProc* proc)
    : m_proc(proc)
{
}


void ProcDecompiler::decompile()
{
    ProcList callStack;
    decompile(&callStack);
}


std::shared_ptr<ProcSet> ProcDecompiler::decompile(ProcList *callStack)
{
    /* Cycle detection logic:
     * *********************
     * cycleGrp is an initially null pointer to a set of procedures, representing the procedures involved in the current
     * recursion group, if any. These procedures have to be analysed together as a group, after individual pre-group
     * analysis.
     * child is a set of procedures, cleared at the top of decompile(), representing the cycles associated with the
     * current procedure and all of its children. If this is empty, the current procedure is not involved in recursion,
     * and can be decompiled up to and including removing unused statements.
     * callStack is an initially empty list of procedures, representing the call stack from the current entry point to the
     * current procedure, inclusive.
     * If (after all children have been processed: important!) the first element in callStack and also cycleGrp is the current
     * procedure, we have the maximal set of distinct cycles, so we can do the recursion group analysis and return an empty
     * set. At the end of the recursion group analysis, the whole group is complete, ready for the global analyses.
     *
     *   cycleSet decompile(ProcList callStack)        // call stack initially empty
     *     child = new ProcSet
     *     push this proc to the call stack
     *     for each child c called by this proc
     *       if c has already been visited but not finished
     *         // have new cycle
     *         if c is in callStack
     *           // this is a completely new cycle
     *           insert every proc from c to the end of callStack into child
     *         else
     *           // this is a new branch of an existing cycle
     *           child = c->cycleGrp
     *           find first element f of callStack that is in cycleGrp
     *           insert every proc after f to the end of callStack into child
     *           for each element e of child
     *         insert e->cycleGrp into child
     *         e->cycleGrp = child
     *       else
     *         // no new cycle
     *         tmp = c->decompile(callStack)
     *         child = union(child, tmp)
     *         set return statement in call to that of c
     *
     *     if (child empty)
     *       earlyDecompile()
     *       child = middleDecompile()
     *       removeUnusedStatments()            // Not involved in recursion
     *     else
     *       // Is involved in recursion
     *       find first element f in callStack that is also in cycleGrp
     *       if (f == this)             // The big test: have we got the complete strongly connected component?
     *         recursionGroupAnalysis() // Yes, we have
     *         child = new ProcSet      // Don't add these processed cycles to the parent
     *     remove last element (= this) from callStack
     *     return child
     */

    LOG_MSG("%1 procedure '%2'", (m_proc->getStatus() >= PROC_VISITED) ? "Re-discovering" : "Discovering", m_proc->getName());
    m_proc->getProg()->getProject()->alertDiscovered(m_proc);

    // Prevent infinite loops when there are cycles in the call graph (should never happen now)
    if (m_proc->getStatus() >= PROC_FINAL) {
        LOG_WARN("Proc %1 already has status PROC_FINAL", m_proc->getName());
        return nullptr; // Already decompiled
    }

    std::shared_ptr<ProcSet> recursionGroup = std::make_shared<ProcSet>();
    if (m_proc->getStatus() < PROC_DECODED) {
        // Can happen e.g. if a callee is visible only after analysing a switch statement
        // Actually decoding for the first time, not REdecoding
        if (!m_proc->getProg()->reDecode(m_proc)) {
            return recursionGroup;
        }
    }

    if (m_proc->getStatus() < PROC_VISITED) {
        m_proc->setStatus(PROC_VISITED); // We have at least visited this proc "on the way down"
    }

    callStack->push_back(m_proc);

    if (m_proc->getProg()->getProject()->getSettings()->decodeChildren) {
        // Recurse to callees first, to perform a depth first search
        for (BasicBlock *bb : *m_proc->getCFG()) {
            if (bb->getType() != BBType::Call) {
                continue;
            }

            // The call Statement will be in the last RTL in this BB
            CallStatement *call = static_cast<CallStatement *>(bb->getRTLs()->back()->getHlStmt());

            if (!call->isCall()) {
                LOG_WARN("BB at address %1 is a CALL but last stmt is not a call: %2", bb->getLowAddr(), call);
                continue;
            }

            assert(call->isCall());
            UserProc *callee = dynamic_cast<UserProc *>(call->getDestProc());

            if (callee == nullptr) { // not an user proc, or missing dest
                continue;
            }

            if (callee->getStatus() == PROC_FINAL) {
                // Already decompiled, but the return statement still needs to be set for this call
                call->setCalleeReturn(callee->getTheReturnStatement());
                continue;
            }

            // if the callee has already been visited but not done (apart from global analyses, i.e. we have a new cycle)
            if ((callee->getStatus() >= PROC_VISITED) && (callee->getStatus() <= PROC_EARLYDONE)) {
                // if callee is in callStack
                ProcList::iterator calleeIt = std::find(callStack->begin(), callStack->end(), callee);

                if (calleeIt != callStack->end()) {
                    // This is a completely new cycle
                    assert(calleeIt != callStack.end());
                    recursionGroup->insert(calleeIt, callStack->end());
                }
                else if (callee->m_recursionGroup) {
                    // This is new branch of an existing cycle
                    recursionGroup = callee->m_recursionGroup;

                    // Find first element func of callStack that is in callee->recursionGroup
                    ProcList::iterator _pi = std::find_if(callStack->begin(), callStack->end(),
                                                          [callee] (UserProc *func) {
                                                              return callee->m_recursionGroup->find(func) != callee->m_recursionGroup->end();
                                                          });

                    // Insert every proc after func to the end of path into child
                    assert(_pi != callStack.end());
                    recursionGroup->insert(std::next(_pi), callStack->end());
                }

                // update the recursion group for each element in the new group;
                // this will union all the recursion groups that are reached along the call path.
                ProcSet oldRecursionGroup = *recursionGroup;
                for (UserProc *proc : oldRecursionGroup) {
                    if (proc->m_recursionGroup) {
                        recursionGroup->insert(proc->m_recursionGroup->begin(), proc->m_recursionGroup->end());
                    }
                }

                // update the recursion group from the old one(s) to the new one
                for (UserProc *proc : *recursionGroup) {
                    proc->m_recursionGroup = recursionGroup;
                }

                m_proc->setStatus(PROC_INCYCLE);
            }
            else {
                // No new cycle
                LOG_VERBOSE("Preparing to decompile callee '%1' of '%2'", callee->getName(), m_proc->getName());

                callee->promoteSignature();
                std::shared_ptr<ProcSet> tmp = ProcDecompiler(callee).decompile(callStack);
                recursionGroup->insert(tmp->begin(), tmp->end());
                // Child has at least done middleDecompile(), possibly more
                call->setCalleeReturn(callee->getTheReturnStatement());

                if (!tmp->empty()) {
                    m_proc->setStatus(PROC_INCYCLE);
                }
            }
        }
    }

    // if no child involved in recursion
    if (recursionGroup->empty()) {
        m_proc->getProg()->getProject()->alertDecompiling(m_proc);
        LOG_MSG("Decompiling procedure '%1'", m_proc->getName());

        m_proc->earlyDecompile();
        recursionGroup = m_proc->middleDecompile(*callStack);

        // If there is a switch statement, middleDecompile could contribute some cycles.
        // If so, we need to test for the recursion logic again
        if (!recursionGroup->empty()) {
            // We've just come back out of decompile(), so we've lost the current proc from the path.
            callStack->push_back(m_proc);
        }
    }

    if (recursionGroup->empty()) {
        m_proc->remUnusedStmtEtc(); // Do the whole works
        m_proc->setStatus(PROC_FINAL);
        m_proc->getProg()->getProject()->alertEndDecompile(m_proc);
    }
    else if (m_proc->m_recursionGroup) {
        // This proc's callees, and hence this proc, is/are involved in recursion.
        // Find first element f in path that is also in our recursion group
        ProcList::iterator f = std::find_if(callStack->begin(), callStack->end(),
            [this] (UserProc *func) {
                return m_proc->m_recursionGroup->find(func) != m_proc->m_recursionGroup->end();
            });

        // The big test: have we found the whole strongly connected component (in the call graph)?
        if (*f == m_proc) {
            // Yes, process these procs as a group
            m_proc->recursionGroupAnalysis(*callStack); // Includes remUnusedStmtEtc on all procs in cycleGrp
            m_proc->setStatus(PROC_FINAL);
            m_proc->getProg()->getProject()->alertEndDecompile(m_proc);
            recursionGroup->clear();
            recursionGroup = std::make_shared<ProcSet>();
        }
    }

    // Remove last element (= this) from path
    // The if should not be neccesary, but nestedswitch needs it
    if (!callStack->empty()) {
        assert(std::find(callStack.begin(), callStack.end(), this) != callStack.end());

        if (callStack->back() != m_proc) {
            LOG_WARN("Last UserProc in UserProc::decompile/path is not this!");
        }

        callStack->remove(m_proc);
    }
    else {
        LOG_WARN("Empty path when trying to remove last proc");
    }

    LOG_VERBOSE("Finished decompile of '%1'", m_proc->getName());
    return recursionGroup;
}
