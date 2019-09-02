#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DFGWriter.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/util/log/Log.h"

#include <QFile>
#include <QString>


void DFGWriter::printDFG(const UserProc *proc, const QString &fname)
{
    LOG_MSG("Outputing DFG to '%1'", fname);
    QFile file(fname);

    if (!file.open(QFile::WriteOnly)) {
        LOG_WARN("Can't open DFG '%1'", fname);
        return;
    }

    OStream out(&file);
    out << "digraph " << proc->getName() << " {\n";
    proc->numberStatements();
    StatementList stmts;
    proc->getStatements(stmts);

    for (SharedStmt s : stmts) {
        if (s->isPhi()) {
            out << s->getNumber() << " [shape=\"triangle\"];\n";
        }

        if (s->isCall()) {
            out << s->getNumber() << " [shape=\"box\"];\n";
        }

        if (s->isBranch()) {
            out << s->getNumber() << " [shape=\"diamond\"];\n";
        }

        LocationSet refs;
        s->addUsedLocs(refs);

        for (SharedExp rr : refs) {
            auto r = std::dynamic_pointer_cast<RefExp>(rr);

            if (r) {
                if (r->getDef()) {
                    out << r->getDef()->getNumber();
                }
                else {
                    out << "input";
                }

                out << " -> ";

                if (s->isReturn()) {
                    out << "output";
                }
                else {
                    out << s->getNumber();
                }

                out << ";\n";
            }
        }
    }

    out << "}\n";
}
