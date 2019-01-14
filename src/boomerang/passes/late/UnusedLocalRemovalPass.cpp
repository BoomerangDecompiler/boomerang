#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UnusedLocalRemovalPass.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/util/StatementList.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expvisitor/UsedLocalFinder.h"
#include "boomerang/visitor/stmtexpvisitor/UsedLocsVisitor.h"

#include <QSet>


UnusedLocalRemovalPass::UnusedLocalRemovalPass()
    : IPass("UnusedLocalRemoval", PassID::UnusedLocalRemoval)
{
}


bool UnusedLocalRemovalPass::execute(UserProc *proc)
{
    QSet<QString> usedLocals;
    StatementList stmts;
    proc->getStatements(stmts);

    // First count any uses of the locals
    bool all = false;

    for (Statement *s : stmts) {
        LocationSet locs;
        all |= addUsedLocalsForStmt(s, locs);

        for (SharedExp u : locs) {
            // Must be a real symbol, and not defined in this statement, unless it is a return
            // statement (in which case it is used outside this procedure), or a call statement.
            // Consider local7 = local7+1 and return local7 = local7+1 and local7 = call(local7+1),
            // where in all cases, local7 is not used elsewhere outside this procedure.
            // With the assign, it can be deleted, but with the return or call statements, it can't.
            if ((s->isReturn() || s->isCall() || !s->definesLoc(u))) {
                if (!u->isLocal()) {
                    continue;
                }

                QString name(u->access<Const, 1>()->getStr());
                usedLocals.insert(name);

                if (proc->getProg()->getProject()->getSettings()->debugUnused) {
                    LOG_MSG("Counted local %1 in %2", name, s);
                }
            }
        }

        if (s->isAssignment() && !s->isImplicit() &&
            static_cast<Assignment *>(s)->getLeft()->isLocal()) {
            Assignment *as = static_cast<Assignment *>(s);
            auto c         = as->getLeft()->access<Const, 1>();
            QString name(c->getStr());
            usedLocals.insert(name);

            if (proc->getProg()->getProject()->getSettings()->debugUnused) {
                LOG_MSG("Counted local %1 on left of %2", name, s);
            }
        }
    }

    // Now record the unused ones in set removes

    QSet<QString> removes;

    for (auto it = proc->getLocals().begin(); it != proc->getLocals().end(); ++it) {
        const QString &name(it->first);

        if (all && removes.size()) {
            LOG_VERBOSE("WARNING: defineall seen in procedure %1, so not removing %2 locals", name,
                        removes.size());
        }

        if ((usedLocals.find(name) == usedLocals.end()) && !all) {
            if (proc->getProg()->getProject()->getSettings()->verboseOutput) {
                LOG_VERBOSE("Removed unused local %1", name);
            }

            removes.insert(name);
        }
    }

    // Remove any definitions of the removed locals
    const bool assumeABICompliance = proc->getProg()->getProject()->getSettings()->assumeABI;

    for (Statement *s : stmts) {
        LocationSet ls;
        s->getDefinitions(ls, assumeABICompliance);

        for (const SharedExp &loc : ls) {
            SharedType ty = s->getTypeForExp(loc);
            QString name  = proc->findLocal(loc, ty);

            if (name.isEmpty()) {
                continue;
            }

            if (removes.find(name) != removes.end()) {
                // Remove it. If an assign, delete it; otherwise (call), remove the define
                if (s->isAssignment()) {
                    proc->removeStatement(s);
                    break; // Break to next statement
                }
                else if (s->isCall()) {
                    // Remove just this define. May end up removing several defines from this call.
                    static_cast<CallStatement *>(s)->removeDefine(loc);
                }

                // else if a ReturnStatement, don't attempt to remove it. The definition is used
                // *outside* this proc.
            }
        }
    }

    // Finally, remove them from locals, so they don't get declared
    for (QString localName : removes) {
        proc->getLocals().erase(localName);
    }

    // Also remove them from the symbols, since symbols are a superset of locals at present
    for (UserProc::SymbolMap::iterator sm = proc->getSymbolMap().begin();
         sm != proc->getSymbolMap().end();) {
        SharedExp mapsTo = sm->second;

        if (mapsTo->isLocal()) {
            QString tmpName = mapsTo->access<Const, 1>()->getStr();

            if (removes.find(tmpName) != removes.end()) {
                sm = proc->getSymbolMap().erase(sm);
                continue;
            }
        }

        ++sm;
    }

    proc->getProg()->getProject()->alertDecompileDebugPoint(proc, "After removing unused locals");
    return true;
}


bool UnusedLocalRemovalPass::addUsedLocalsForStmt(Statement *stmt, LocationSet &used)
{
    UsedLocalFinder ulf(used, stmt->getProc());
    UsedLocsVisitor ulv(&ulf, false);

    stmt->accept(&ulv);
    return ulf.wasAllFound();
}
