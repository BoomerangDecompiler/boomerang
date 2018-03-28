#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Boomerang.h"


#include "boomerang/codegen/CCodeGenerator.h"
#include "boomerang/core/Project.h"
#include "boomerang/db/CFGCompressor.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/frontend/Frontend.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/CFGDotWriter.h"

#include <ctime>


static Boomerang *g_boomerang;


Boomerang::Boomerang()
    : m_settings(new Settings)
{
}


void Boomerang::miniDebugger(UserProc *proc, const char *description)
{
    QTextStream q_cout(stdout);
    QTextStream q_cin(stdin);

    q_cout << "decompiling " << proc->getName() << ": " << description << "\n";
    QString stopAt;
    static std::set<Statement *> watches;

    if (stopAt.isEmpty() || !proc->getName().compare(stopAt)) {
        // This is a mini command line debugger.  Feel free to expand it.
        for (auto const& watche : watches) {
            (watche)->print(q_cout);
            q_cout << "\n";
        }

        q_cout << " <press enter to continue> \n";
        QString line;

        while (1) {
            line.clear();
            q_cin >> line;

            if (line.startsWith("print")) {
                proc->print(q_cout);
            }
            else if (line.startsWith("fprint")) {
                QFile tgt("out.proc");

                if (tgt.open(QFile::WriteOnly)) {
                    QTextStream of(&tgt);
                    proc->print(of);
                }
            }
            else if (line.startsWith("run ")) {
                QStringList parts = line.trimmed().split(" ", QString::SkipEmptyParts);

                if (parts.size() > 1) {
                    stopAt = parts[1];
                }

                break;
            }
            else if (line.startsWith("watch ")) {
                QStringList parts = line.trimmed().split(" ", QString::SkipEmptyParts);

                if (parts.size() > 1) {
                    int           n = parts[1].toInt();
                    StatementList stmts;
                    proc->getStatements(stmts);
                    StatementList::iterator it;

                    for (it = stmts.begin(); it != stmts.end(); ++it) {
                        if ((*it)->getNumber() == n) {
                            watches.insert(*it);
                            q_cout << "watching " << *it << "\n";
                        }
                    }
                }
            }
            else {
                break;
            }
        }
    }
}


Boomerang *Boomerang::get()
{
    if (!g_boomerang) {
        g_boomerang = new Boomerang();
    }

    return g_boomerang;
}


void Boomerang::destroy()
{
    delete g_boomerang;
    g_boomerang = nullptr;
}


void Boomerang::alertDecompileDebugPoint(UserProc *p, const char *description)
{
    if (SETTING(stopAtDebugPoints)) {
        miniDebugger(p, description);
    }

    for (IWatcher *elem : m_watchers) {
        elem->alertDecompileDebugPoint(p, description);
    }
}


const char *Boomerang::getVersionStr()
{
    return BOOMERANG_VERSION;
}


void Boomerang::addWatcher(IWatcher* watcher)
{
    m_watchers.insert(watcher);
}


void Boomerang::alertDecompileComplete()
{
    for (IWatcher *it : m_watchers) {
        it->alert_complete();
    }
}


void Boomerang::alertNew(Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->alertNew(function);
    }
}


void Boomerang::alertRemove(Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->alertRemove(function);
    }
}


void Boomerang::alertUpdateSignature(Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->alertUpdateSignature(function);
    }
}


void Boomerang::alertDecode(Address pc, int numBytes)
{
    for (IWatcher *it : m_watchers) {
        it->alertDecode(pc, numBytes);
    }
}


void Boomerang::alertBadDecode(Address pc)
{
    for (IWatcher *it : m_watchers) {
        it->alertBadDecode(pc);
    }
}


void Boomerang::alertDecode(Function* p, Address pc, Address last, int numBytes)
{
    for (IWatcher *it : m_watchers) {
        it->alertDecode(p, pc, last, numBytes);
    }
}


void Boomerang::alertLoad(Function* p)
{
    for (IWatcher *it : m_watchers) {
        it->alert_load(p);
    }
}


void Boomerang::alertStartDecode(Address start, int numBytes)
{
    for (IWatcher *it : m_watchers) {
        it->alertStartDecode(start, numBytes);
    }
}


void Boomerang::alertEndDecode()
{
    for (IWatcher *it : m_watchers) {
        it->alertEndDecode();
    }
}


void Boomerang::alertStartDecompile(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->alertStartDecompile(proc);
    }
}


void Boomerang::alertProcStatusChange(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->alertProcStatusChange(proc);
    }
}


void Boomerang::alertDecompileSSADepth(UserProc* proc, int depth)
{
    for (IWatcher *it : m_watchers) {
        it->alertDecompileSSADepth(proc, depth);
    }
}


void Boomerang::alertDecompileBeforePropagate(UserProc* proc, int depth)
{
    for (IWatcher *it : m_watchers) {
        it->alertDecompileBeforePropagate(proc, depth);
    }
}


void Boomerang::alertDecompileAfterPropagate(UserProc* proc, int depth)
{
    for (IWatcher *it : m_watchers) {
        it->alertDecompileAfterPropagate(proc, depth);
    }
}


void Boomerang::alertDecompileAfterRemoveStmts(UserProc* proc, int depth)
{
    for (IWatcher *it : m_watchers) {
        it->alertDecompileAfterRemoveStmts(proc, depth);
    }
}


void Boomerang::alertEndDecompile(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->alertEndDecompile(proc);
    }
}


void Boomerang::alertDiscovered(Function* caller, Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->alertDiscovered(caller, function);
    }
}


void Boomerang::alertDecompiling(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->alertDecompiling(proc);
    }
}
