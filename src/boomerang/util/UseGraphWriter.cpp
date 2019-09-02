#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UseGraphWriter.h"

#include "boomerang/core/Project.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/util/StatementList.h"
#include "boomerang/util/log/Log.h"

#include <QFile>
#include <QString>


void UseGraphWriter::writeUseGraph(const UserProc *proc, const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QFile::Text | QFile::WriteOnly)) {
        LOG_ERROR("Can't write to file %1", file.fileName());
        return;
    }

    OStream out(&file);
    out << "digraph " << proc->getName() << " {\n";
    StatementList stmts;
    proc->getStatements(stmts);

    for (SharedStmt s : stmts) {
        if (s->isPhi()) {
            out << s->getNumber() << " [shape=diamond];\n";
        }

        LocationSet refs;
        s->addUsedLocs(refs);

        for (SharedExp rr : refs) {
            if (rr->isSubscript()) {
                auto r = rr->access<RefExp>();

                if (r->getDef()) {
                    out << r->getDef()->getNumber() << " -> " << s->getNumber() << ";\n";
                }
            }
        }
    }

    out << "}\n";
}
