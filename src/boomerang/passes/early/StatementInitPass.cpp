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


#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/statements/CallStatement.h"


StatementInitPass::StatementInitPass()
    : IPass("StatementInit", PassID::StatementInit)
{
}


bool StatementInitPass::execute(UserProc* proc)
{
    BasicBlock::RTLIterator rit;
    StatementList::iterator sit;

    for (BasicBlock *bb : *proc->getCFG()) {
        for (Statement *stmt = bb->getFirstStmt(rit, sit); stmt != nullptr; stmt = bb->getNextStmt(rit, sit)) {
            stmt->setProc(proc);
            stmt->setBB(bb);
            CallStatement *call = dynamic_cast<CallStatement *>(stmt);

            if (call) {
                call->setSigArguments();

                // Remove out edges of BBs of noreturn calls (e.g. call BBs to abort())
                if (call->getDestProc() && call->getDestProc()->isNoReturn() && (bb->getNumSuccessors() == 1)) {
                    BasicBlock *nextBB = bb->getSuccessor(0);

                    if ((nextBB != proc->getCFG()->getExitBB()) || (proc->getCFG()->getExitBB()->getNumPredecessors() != 1)) {
                        nextBB->removePredecessor(bb);
                        bb->removeAllSuccessors();
                    }
                }
            }
        }
    }

    return true;
}

