#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CFGDotWriter.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Exp.h"
#include "boomerang/util/log/Log.h"


void CFGDotWriter::writeCFG(const Prog *prog, const QString &filename)
{
    QFile tgt(prog->getProject()->getSettings()->getOutputDirectory().absoluteFilePath(filename));

    if (!tgt.open(QFile::WriteOnly | QFile::Text)) {
        return;
    }

    OStream of(&tgt);
    of << "digraph ProcCFG {\n";

    for (const auto &module : prog->getModuleList()) {
        for (Function *func : *module) {
            if (func->isLib()) {
                continue;
            }

            UserProc *p = static_cast<UserProc *>(func);

            if (!p->isDecoded()) {
                continue;
            }

            // Subgraph for the proc name
            of << "\nsubgraph cluster_" << p->getName() << " {\n"
               << "       color=gray;\n    label=" << p->getName() << ";\n";
            // Generate dotty CFG for this proc
            writeCFG(p->getCFG(), of);
        }
    }

    of << "}";
}


void CFGDotWriter::writeCFG(const ProcSet &procs, const QString &filename)
{
    QFile outFile(filename);
    if (!outFile.open(QFile::WriteOnly | QFile::Text)) {
        LOG_ERROR("Could not open '%1' for writing", filename);
        return;
    }

    OStream textStream(&outFile);
    textStream << "digraph ProcCFG {\n";

    for (UserProc *userProc : procs) {
        textStream << "subgraph " << userProc->getName() << " {\n";
        writeCFG(userProc->getCFG(), textStream);
    }

    textStream << "}";
}


void CFGDotWriter::writeCFG(const ProcCFG *cfg, OStream &of)
{
    Address returnAddress = Address::INVALID;

    // The nodes
    for (BasicBlock *bb : *cfg) {
        of << "       "
           << "bb" << bb->getIR()->getLowAddr() << " ["
           << "label=\"" << bb->getIR()->getLowAddr() << " ";

        switch (bb->getType()) {
        case BBType::Oneway: of << "oneway"; break;

        case BBType::Twoway:
            if (bb->getIR()->getCond()) {
                of << "\\n";
                bb->getIR()->getCond()->print(of);
                of << "\" shape=diamond];\n";
                continue;
            }
            else {
                of << "twoway";
            }
            break;

        case BBType::Nway: {
            of << "nway";
            SharedExp de = bb->getIR()->getDest();

            if (de) {
                of << "\\n";
                of << de;
            }

            of << "\" shape=trapezium];\n";
            continue;
        }

        case BBType::Call: {
            of << "call";
            Function *dest = bb->getIR()->getCallDestProc();

            if (dest) {
                of << "\\n" << dest->getName();
            }

            break;
        }

        case BBType::Ret:
            of << "ret\" shape=triangle];\n";
            // Remember the (unique) return BB's address
            returnAddress = bb->getIR()->getLowAddr();
            continue;

        case BBType::Fall: of << "fall"; break;
        case BBType::CompJump: of << "compjump"; break;
        case BBType::CompCall: of << "compcall"; break;
        case BBType::Invalid: of << "invalid"; break;
        }

        of << "\"];\n";
    }

    // Force the one return node to be at the bottom (max rank).
    // Otherwise, with all its in-edges, it will end up in the middle
    if (!returnAddress.isZero()) {
        of << "{rank=max; bb" << returnAddress << "}\n";
    }

    // Close the subgraph
    of << "}\n";

    // Now the edges
    for (BasicBlock *srcBB : *cfg) {
        for (int j = 0; j < srcBB->getNumSuccessors(); j++) {
            BasicBlock *dstBB = srcBB->getSuccessor(j);

            of << "       bb" << srcBB->getIR()->getLowAddr() << " -> ";
            of << "bb" << dstBB->getIR()->getLowAddr();

            if (srcBB->getType() == BBType::Twoway) {
                if (j == 0) {
                    of << " [color=\"green\"]"; // cond == true
                }
                else {
                    of << " [color=\"red\"]"; // cond == false
                }
            }
            else {
                of << " [color=\"black\"];\n"; // normal connection
            }
        }
    }
}
