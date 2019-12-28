#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementInitPass.h"

#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/decomp/CFGCompressor.h"
#include "boomerang/ifc/IFrontEnd.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/util/CFGDotWriter.h"


StatementInitPass::StatementInitPass()
    : IPass("StatementInit", PassID::StatementInit)
{
}


bool StatementInitPass::execute(UserProc *proc)
{
    proc->getCFG()->clear();
    proc->removeRetStmt();

    if (!proc->getProg()->getFrontEnd()->liftProc(proc)) {
        return false;
    }

    for (IRFragment *frag : *proc->getCFG()) {
        IRFragment::RTLIterator rit;
        StatementList::iterator sit;

        for (SharedStmt stmt = frag->getFirstStmt(rit, sit); stmt != nullptr;
             stmt            = frag->getNextStmt(rit, sit)) {
            assert(stmt->getProc() == nullptr || stmt->getProc() == proc);

            // Remove out edges of fragments of noreturn calls (e.g. call fragments to abort())
            if (!stmt->isCall()) {
                continue;
            }

            std::shared_ptr<CallStatement> call = stmt->as<CallStatement>();
            call->setSigArguments();

            if ((frag->getNumSuccessors() != 1)) {
                continue;
            }

            Function *destProc = call->getDestProc();
            if (!destProc) {
                continue;
            }

            if (!destProc->isLib() &&
                static_cast<UserProc *>(destProc)->getStatus() != ProcStatus::Visited) {
                continue; // Proc was not visited yet - We cannot know if it will return
            }

            if (!destProc->isNoReturn()) {
                continue;
            }


            IRFragment *nextFrag = frag->getSuccessor(0);

            // Do not remove the only predecessor of a return fragment
            if ((nextFrag == proc->getCFG()->getExitFragment()) &&
                proc->getCFG()->getExitFragment()->getNumPredecessors() == 1) {
                continue;
            }

            nextFrag->removePredecessor(frag);
            frag->removeAllSuccessors();
        }
    }

    // Removing out edges of noreturn calls might sever paths between the entry fragment
    // and other (now orphaned) fragments. We have to remove these fragments
    // since all fragments must be reachable from the entry fragment for data-flow analysis
    // to work.
    CFGCompressor().compressCFG(proc->getCFG());
    return true;
}
