#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CallGraphDotWriter.h"


#include "boomerang/core/Project.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/util/Log.h"

#include <QSaveFile>

#include <queue>


bool CallGraphDotWriter::writeCallGraph(const Prog *prog, const QString& dstFileName)
{
    LOG_VERBOSE("Writing call graph to '%1'", dstFileName);
    QSaveFile saveFile(prog->getProject()->getSettings()->getOutputDirectory().absoluteFilePath(dstFileName));

    if (!saveFile.open(QFile::WriteOnly)) {
        LOG_ERROR("Cannot open output file '%1' for callgraph output", dstFileName);
        return false;
    }

    QTextStream ost(&saveFile);

    ost << "digraph callgraph\n";
    ost << "{\n";

    std::queue<Function *>           procList;
    std::unordered_set<Function *>   seen;

    for (Function *entry : prog->getEntryProcs()) {
        // We have to explicitly write entry procedures here
        // because not every entry procedure has callees (e.g. hello world main)
        ost << "    " << entry->getName() << ";\n";
        procList.push(entry);
    }

    while (!procList.empty()) {
        Function *currentProc = procList.front();
        procList.pop();

        if (currentProc == reinterpret_cast<Function *>(-1)) {
            continue;
        }
        else if (seen.find(currentProc) != seen.end()) {
            continue; // already processed
        }
        seen.insert(currentProc);

        UserProc *up = static_cast<UserProc *>(currentProc);

        for (Function *callee : up->getCallees()) {
            ost << "    " << up->getName() << " -> " << callee->getName() << ";\n";

            if (seen.find(callee) == seen.end() && !callee->isLib()) {
                procList.push(callee);
            }
        }
    }

    ost << "}\n";

    ost.flush();
    return saveFile.commit();
}
