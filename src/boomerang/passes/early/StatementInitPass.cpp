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

StatementInitPass::StatementInitPass()
    : IPass("StatementInit", PassID::StatementInit)
{
}


bool StatementInitPass::execute(UserProc *proc)
{
    proc->getCFG()->clear();
    if (!proc->getProg()->getFrontEnd()->liftProc(proc)) {
        return false;
    }

    IRFragment::RTLIterator rit;
    StatementList::iterator sit;

    for (IRFragment *bb : *proc->getCFG()) {
        for (SharedStmt stmt = bb->getFirstStmt(rit, sit); stmt != nullptr;
             stmt            = bb->getNextStmt(rit, sit)) {
            assert(stmt->getProc() == nullptr || stmt->getProc() == proc);

            // Remove out edges of BBs of noreturn calls (e.g. call BBs to abort())
            if (!stmt->isCall()) {
                continue;
            }

            std::shared_ptr<CallStatement> call = stmt->as<CallStatement>();
            call->setSigArguments();

            if ((bb->getNumSuccessors() != 1)) {
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


            IRFragment *nextBB = bb->getSuccessor(0);

            // Do not remove the only predecessor of a return fragment
            if ((nextBB == proc->getCFG()->getExitFragment()) &&
                proc->getCFG()->getExitFragment()->getNumPredecessors() == 1) {
                continue;
            }

            nextBB->removePredecessor(bb);
            bb->removeAllSuccessors();
        }
    }

    // Removing out edges of noreturn calls might sever paths between
    // the entry BB and other (now orphaned) BBs. We have to remove these BBs
    // since all BBs must be reachable from the entry BB for data-flow analysis
    // to work.
    CFGCompressor().compressCFG(proc->getCFG());
    return true;
}
