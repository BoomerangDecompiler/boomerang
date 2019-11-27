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
#include "boomerang/db/LowLevelCFG.h"
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
    of << "digraph ProgCFG {\n";

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
            of << "  subgraph cluster_" << p->getName() << " {\n";
            of << "    color=gray;\n";
            of << "    label=" << p->getName() << ";\n";
            of << "\n";

            of << "    subgraph cluster_llcfg {\n";
            writeCFG(p, of);
            of << "    }\n";
            of << "\n";

            // Generate dotty CFG for this proc
            of << "    subgraph cluster_hlcfg {\n";
            writeCFG(p->getCFG(), of);
            of << "    }\n";
            of << "  }\n";
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


void CFGDotWriter::writeCFG(const UserProc *proc, OStream &of)
{
    const LowLevelCFG *cfg = proc->getProg()->getCFG();

    for (const LowLevelCFG::BBStart &b : *cfg) {
        const BasicBlock *bb    = b.bb;
        const BasicBlock *delay = b.delay;

        if (bb && bb->getFunction() == proc) {
            of << "      bb" << bb->getLowAddr() << "[shape=rectangle, label=\"";

            for (const MachineInstruction &insn : bb->getInsns()) {
                of << insn.m_addr << "  " << insn.m_mnem.data() << " " << insn.m_opstr.data()
                   << "\\l";
            }

            of << "\"];\n";
        }

        if (delay && delay->getFunction() == proc) {
            of << "      bb" << delay->getLowAddr() << "_d[shape=rectangle, label=\"";

            for (const MachineInstruction &insn : delay->getInsns()) {
                of << insn.m_addr << "  " << insn.m_mnem.data() << " " << insn.m_opstr.data()
                   << "\\l";
            }

            of << "\"];\n";
        }
    }

    of << "\n";

    // edges
    for (const LowLevelCFG::BBStart &b : *cfg) {
        const BasicBlock *srcBB    = b.bb;
        const BasicBlock *srcDelay = b.delay;

        if (srcBB && srcBB->getFunction() == proc) {
            for (int j = 0; j < srcBB->getNumSuccessors(); j++) {
                const BasicBlock *dstBB = srcBB->getSuccessor(j);

                of << "      bb" << srcBB->getLowAddr() << " -> bb" << dstBB->getLowAddr();

                if (dstBB->isType(BBType::DelaySlot)) {
                    of << "_d";
                }

                if (srcBB->isType(BBType::Twoway)) {
                    if (j == 0) {
                        of << " [color=\"green\"];\n"; // cond == true
                    }
                    else {
                        of << " [color=\"red\"];\n"; // cond == false
                    }
                }
                else {
                    of << " [color=\"black\"];\n"; // normal connection
                }
            }
        }

        if (srcDelay && srcDelay->getFunction() == proc) {
            for (int j = 0; j < srcDelay->getNumSuccessors(); j++) {
                const BasicBlock *dstBB = srcDelay->getSuccessor(j);

                of << "      bb" << srcDelay->getLowAddr() << "_d -> bb" << dstBB->getLowAddr();

                if (dstBB->isType(BBType::DelaySlot)) {
                    of << "_d";
                }

                if (srcDelay->isType(BBType::Twoway)) {
                    if (j == 0) {
                        of << " [color=\"green\"];\n"; // cond == true
                    }
                    else {
                        of << " [color=\"red\"];\n"; // cond == false
                    }
                }
                else {
                    of << " [color=\"black\"];\n"; // normal connection
                }
            }
        }
    }

    of << "\n";
}


void CFGDotWriter::writeCFG(const ProcCFG *cfg, OStream &of)
{
    if (cfg->getNumFragments() > 0) {
        cfg->getProc()->numberStatements();
    }

    // The nodes
    for (IRFragment *frag : *cfg) {
        of << "      frag" << frag->getLowAddr() << "[shape=rectangle, label=\"";

        IRFragment::RTLIterator rit;
        StatementList::iterator sit;

        for (SharedStmt stmt = frag->getFirstStmt(rit, sit); stmt;
             stmt            = frag->getNextStmt(rit, sit)) {
            QString str;
            OStream temp(&str);
            stmt->print(temp);
            str.replace('\n', "\\l");
            str.replace('%', "\\%");
            str.replace('\"', "\\\"");
            of << str << "\\l";
        }

        of << "\"];\n";
    }

    of << "\n";

    // Now the edges
    for (IRFragment *srcFrag : *cfg) {
        for (int j = 0; j < srcFrag->getNumSuccessors(); j++) {
            IRFragment *dstFrag = srcFrag->getSuccessor(j);

            of << "      frag" << srcFrag->getLowAddr() << " -> frag" << dstFrag->getLowAddr();

            if (srcFrag->isType(FragType::Twoway)) {
                if (j == 0) {
                    of << " [color=\"green\"];\n"; // cond == true
                }
                else {
                    of << " [color=\"red\"];\n"; // cond == false
                }
            }
            else {
                of << " [color=\"black\"];\n"; // normal connection
            }
        }
    }
}
