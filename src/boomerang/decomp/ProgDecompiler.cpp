#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ProgDecompiler.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Global.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/decomp/CFGCompressor.h"
#include "boomerang/decomp/UnusedReturnRemover.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/util/log/Log.h"


ProgDecompiler::ProgDecompiler(Prog *prog)
    : m_prog(prog)
{
}


void ProgDecompiler::decompile()
{
    assert(!m_prog->getModuleList().empty());
    LOG_VERBOSE("%1 procedures", m_prog->getNumFunctions(false));

    // Start decompiling each entry point
    for (UserProc *up : m_prog->getEntryProcs()) {
        LOG_MSG("Decompiling entry point '%1'", up->getName());
        up->decompileRecursive();
    }

    // Just in case there are any Procs not in the call graph.

    if (m_prog->getProject()->getSettings()->decodeMain &&
        m_prog->getProject()->getSettings()->decodeChildren) {
        bool foundone = true;

        while (foundone) {
            foundone = false;

            for (const auto &module : m_prog->getModuleList()) {
                for (Function *pp : *module) {
                    if (pp->isLib()) {
                        continue;
                    }

                    UserProc *proc = static_cast<UserProc *>(pp);

                    if (proc->isDecompiled()) {
                        continue;
                    }
                    proc->decompileRecursive();
                    foundone = true;
                }
            }
        }
    }

    globalTypeAnalysis();

    if (m_prog->getProject()->getSettings()->removeReturns) {
        // Repeat until no change. Not 100% sure if needed.
        while (removeUnusedParamsAndReturns()) {
            for (auto &module : m_prog->getModuleList()) {
                for (Function *proc : *module) {
                    if (proc->isLib()) {
                        continue;
                    }

                    PassManager::get()->executePass(PassID::BranchAnalysis,
                                                    static_cast<UserProc *>(proc));
                }
            }
        }
    }

    globalTypeAnalysis();

    // Now it is OK to transform out of SSA form
    fromSSAForm();
    removeUnusedGlobals();

    LOG_MSG("Compressing CFG...");

    for (const auto &module : m_prog->getModuleList()) {
        for (Function *func : *module) {
            if (!func->isLib()) {
                CFGCompressor().compressCFG(static_cast<UserProc *>(func)->getCFG());
            }
        }
    }

    LOG_MSG("Decompilation finished.");
}


void ProgDecompiler::globalTypeAnalysis()
{
    LOG_MSG("Performing global type analysis...");

    if (m_prog->getProject()->getSettings()->debugTA) {
        LOG_VERBOSE("### Start global data-flow-based type analysis ###");
    }

    for (const auto &module : m_prog->getModuleList()) {
        for (Function *pp : *module) {
            UserProc *proc = dynamic_cast<UserProc *>(pp);

            if (!proc || !proc->isDecoded()) {
                continue;
            }

            // FIXME: this just does local TA again. Need to meet types for all parameter/arguments,
            // and return/results! This will require a repeat until no change loop
            LOG_VERBOSE("Global type analysis for '%1'", proc->getName());
            PassManager::get()->executePass(PassID::LocalTypeAnalysis, proc);
        }
    }

    if (m_prog->getProject()->getSettings()->debugTA) {
        LOG_VERBOSE("### End type analysis ###");
    }
}


void ProgDecompiler::removeUnusedGlobals()
{
    LOG_MSG("Removing unused global variables...");

    // seach for used globals
    std::list<SharedExp> usedGlobals;

    for (const auto &module : m_prog->getModuleList()) {
        for (Function *func : *module) {
            if (func->isLib()) {
                continue;
            }

            UserProc *proc = static_cast<UserProc *>(func);
            Location search(opGlobal, Terminal::get(opWild), proc);
            // Search each statement in u, excepting implicit assignments (their uses don't count,
            // since they don't really exist in the program representation)
            StatementList stmts;
            StatementList::iterator ss;
            proc->getStatements(stmts);

            for (SharedStmt s : stmts) {
                if (s->isImplicit()) {
                    continue; // Ignore the uses in ImplicitAssigns
                }

                bool found = s->searchAll(search, usedGlobals);

                if (found && m_prog->getProject()->getSettings()->debugUnused) {
                    LOG_VERBOSE("A global is used by stmt %1", s->getNumber());
                }
            }
        }
    }

    // make a map to find a global by its name (could be a global var too)
    QMap<QString, std::shared_ptr<Global>> namedGlobals;

    for (auto &g : m_prog->getGlobals()) {
        namedGlobals[g->getName()] = g;
    }

    // Rebuild the globals vector. Delete the unused globals only after re-inserting them
    Prog::GlobalSet oldGlobals = m_prog->getGlobals();
    m_prog->getGlobals().clear();

    for (const SharedExp &e : usedGlobals) {
        if (m_prog->getProject()->getSettings()->debugUnused) {
            LOG_MSG(" %1 is used", e);
        }

        QString name(e->access<Const, 1>()->getStr());
        auto &usedGlobal = namedGlobals[name];

        if (usedGlobal) {
            m_prog->getGlobals().insert(usedGlobal);
        }
        else {
            LOG_WARN("An expression refers to a nonexistent global");
        }
    }
}


bool ProgDecompiler::removeUnusedParamsAndReturns()
{
    LOG_MSG("Removing unused returns...");
    return UnusedReturnRemover(m_prog).removeUnusedReturns();
}


void ProgDecompiler::fromSSAForm()
{
    LOG_MSG("Transforming from SSA form...");

    for (const auto &module : m_prog->getModuleList()) {
        for (Function *pp : *module) {
            if (pp->isLib()) {
                continue;
            }

            UserProc *proc = static_cast<UserProc *>(pp);
            proc->numberStatements();
            PassManager::get()->executePass(PassID::FromSSAForm, proc);
        }
    }
}
