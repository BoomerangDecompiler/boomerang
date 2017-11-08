#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "RangeAnalysis.h"


#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/statements/JunctionStatement.h"
#include "boomerang/passes/range/RangePrivateData.h"
#include "boomerang/passes/range/RangeVisitor.h"
#include "boomerang/util/Log.h"

#include <cassert>


RangeAnalysisPass::RangeAnalysisPass()
    : m_rangeData(new RangePrivateData)
{
}


void RangeAnalysisPass::addJunctionStatements(Cfg& cfg)
{
    for (BasicBlock *pbb : cfg) {
        if ((pbb->getNumPredecessors() > 1) && ((pbb->getFirstStmt() == nullptr) || !pbb->getFirstStmt()->isJunction())) {
            assert(pbb->getRTLs());
            JunctionStatement *j = new JunctionStatement();
            j->setBB(pbb);
            pbb->getRTLs()->front()->push_front(j);
        }
    }
}


void RangeAnalysisPass::clearRanges()
{
    m_rangeData->clearRanges();
}


bool RangeAnalysisPass::runOnFunction(Function& F)
{
    if (F.isLib()) {
        return false;
    }

    LOG_VERBOSE("Performing range analysis on %1", F.getName());

    UserProc& UF((UserProc&)F);
    assert(UF.getCFG());

    // this helps
    UF.getCFG()->sortByAddress();

    addJunctionStatements(*UF.getCFG());
    UF.getCFG()->establishDFTOrder();

    clearRanges();

    UF.debugPrintAll("Before performing range analysis");

    std::list<Statement *> execution_paths;
    std::list<Statement *> junctions;

    assert(UF.getCFG()->getEntryBB());
    assert(UF.getCFG()->getEntryBB()->getFirstStmt());
    execution_paths.push_back(UF.getCFG()->getEntryBB()->getFirstStmt());

    int          watchdog = 0;
    RangeVisitor rv(m_rangeData, execution_paths);

    while (!execution_paths.empty()) {
        while (!execution_paths.empty()) {
            Statement *stmt = execution_paths.front();
            execution_paths.pop_front();

            if (stmt == nullptr) {
                continue; // ??
            }

            if (stmt->isJunction()) {
                junctions.push_back(stmt);
            }
            else {
                stmt->accept(&rv);
            }
        }

        if (watchdog > 45) {
            LOG_MSG("Processing execution paths resulted in %1 junctions to process", junctions.size());
        }

        while (!junctions.empty()) {
            Statement *junction = junctions.front();
            junctions.pop_front();

            if (watchdog > 45) {
                LOG_MSG("Processing junction %1", junction);
            }

            assert(junction->isJunction());
            junction->accept(&rv);
        }

        watchdog++;

        if (watchdog > 10) {
            LOG_MSG("  watchdog %1", watchdog);

            if (watchdog > 45) {
                LOG_MSG("%1 execution paths remaining.", execution_paths.size());
                LOG_SEPARATE(UF.getName(), "=== After range analysis watchdog %1 ===", watchdog, UF.getName());
                LOG_SEPARATE(UF.getName(), "%1", UF);
                LOG_SEPARATE(UF.getName(), "=== End after range analysis watchdog %1 for %2 ===", watchdog, UF.getName());
            }
        }

        if (watchdog > 50) {
            LOG_MSG("  watchdog expired");
            break;
        }
    }

    UF.debugPrintAll("After range analysis");

    UF.getCFG()->removeJunctionStatements();
    logSuspectMemoryDefs(UF);
    return true;
}


void RangeAnalysisPass::logSuspectMemoryDefs(UserProc& UF)
{
    StatementList stmts;

    UF.getStatements(stmts);

    for (Statement *st : stmts) {
        if (!st->isAssign()) {
            continue;
        }

        Assign *a = (Assign *)st;

        if (!a->getLeft()->isMemOf()) {
            continue;
        }

        RangeMap& rm = m_rangeData->getRanges(st);
        SharedExp p  = rm.substInto(a->getLeft()->getSubExp1()->clone());

        if (rm.hasRange(p)) {
            Range& r = rm.getRange(p);
            LOG_VERBOSE("Got p %1 with range %2", p, r.toString());

            if ((r.getBase()->getOper() == opInitValueOf) && r.getBase()->getSubExp1()->isRegOfK() &&
                (r.getBase()->access<Const, 1, 1>()->getInt() == 28)) {
                RTL *rtl = a->getBB()->getRTLWithStatement(a);
                LOG_VERBOSE("Interesting stack reference at address %1: %2", rtl->getAddress(), a);
            }
        }
    }
}


