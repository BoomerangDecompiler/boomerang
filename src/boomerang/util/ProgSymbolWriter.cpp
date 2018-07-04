#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ProgSymbolWriter.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/core/Project.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/util/Log.h"

#include <QSaveFile>


void printProcsRecursive(Function *function, int indent, QTextStream& f, std::set<Function *>& seen)
{
    const bool firsttime = (seen.find(function) == seen.end());

    if (firsttime) {
        seen.insert(function);
    }

    for (int i = 0; i < indent; i++) {
        f << "    ";
    }

    if (!function->isLib() && firsttime) { // seen lib proc
        f << function->getEntryAddress();
        f << " __nodecode __incomplete void " << function->getName() << "();\n";

        UserProc *proc = static_cast<UserProc *>(function);

        for (Function *callee : proc->getCallees()) {
            printProcsRecursive(callee, indent + 1, f, seen);
        }

        for (int i = 0; i < indent; i++) {
            f << "     ";
        }

        f << "// End of " << function->getName() << "\n";
    }
    else {
        f << "// " << function->getName() << "();\n";
    }
}


bool ProgSymbolWriter::writeSymbolsToFile(const Prog *prog, const QString& dstFileName)
{
    LOG_VERBOSE("Writing symbols to '%1'", dstFileName);
    const QString fname = prog->getProject()->getSettings()->getOutputDirectory().absoluteFilePath(dstFileName);
    QSaveFile tgt(fname);

    if (!tgt.open(QFile::WriteOnly)) {
        LOG_ERROR("Cannot open '%1' for writing", fname);
        return false;
    }

    QTextStream f(&tgt);

    /* Print procs */
    f << "/* Functions: */\n";
    std::set<Function *> seen;

    for (UserProc *up : prog->getEntryProcs()) {
        printProcsRecursive(up, 0, f, seen);
    }

    f << "/* Leftovers: */\n";

    for (const auto& m : prog->getModuleList()) {
        for (Function *pp : *m) {
            if (!pp->isLib() && (seen.find(pp) == seen.end())) {
                printProcsRecursive(pp, 0, f, seen);
            }
        }
    }

    f.flush();
    return tgt.commit();
}
