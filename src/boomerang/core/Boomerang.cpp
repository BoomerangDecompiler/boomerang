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
#include "boomerang/db/BinaryImage.h"
#include "boomerang/db/SymTab.h"
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
    , m_symbols(new SymTab)
    , m_codeGenerator(new CCodeGenerator)
{
}


ICodeGenerator *Boomerang::getCodeGenerator()
{
    return m_codeGenerator.get();
}


std::unique_ptr<Prog> Boomerang::loadAndDecode(const QString& fname, const char *pname)
{
    LOG_MSG("Loading...");
    std::unique_ptr<Prog> prog(new Prog(QFileInfo(fname).baseName()));
    IFrontEnd *fe   = IFrontEnd::create(fname, prog.get(), this->getOrCreateProject());

    if (fe == nullptr) {
        LOG_ERROR("Loading '%1' failed.", fname);
        return nullptr;
    }

    prog->setFrontEnd(fe);

    // Add symbols from -s switch(es)
    for (const std::pair<Address, QString>& elem : m_symbolMap) {
        fe->addSymbol(elem.first, elem.second);
    }

    fe->readLibraryCatalog(); // Needed before readSymbolFile()

    for (auto& elem : m_symbolFiles) {
        LOG_MSG("Reading symbol file '%1'", elem);
        prog->readSymbolFile(elem);
    }

    // Entry points from -e (and -E) switch(es)
    for (auto& elem : m_entryPoints) {
        LOG_MSG("Decoding specified entrypoint at address %1", elem);
        prog->decodeEntryPoint(elem);
    }

    if (m_entryPoints.size() == 0) { // no -e or -E given
        if (SETTING(decodeMain)) {
            LOG_MSG("Decoding entry point...");
        }

        if (!fe->decode(prog.get(), SETTING(decodeMain), pname)) {
            LOG_ERROR("Aborting load due to decode failure");
            return nullptr;
        }

        bool gotMain = false;
        Address mainAddr = fe->getMainEntryPoint(gotMain);
        if (gotMain) {
            prog->addEntryPoint(mainAddr);
        }

        if (SETTING(decodeChildren)) {
            // this causes any undecoded userprocs to be decoded
            LOG_MSG("Decoding anything undecoded...");
            if (!fe->decode(prog.get(), Address::INVALID)) {
                LOG_ERROR("Aborting load due to decode failure");
                return nullptr;
            }
        }
    }

    LOG_MSG("Finishing decode...");
    prog->finishDecode();

    Boomerang::get()->alertEndDecode();

    LOG_MSG("Found %1 procs", prog->getNumFunctions());

    if (SETTING(generateSymbols)) {
        prog->printSymbolsToFile();
    }

    if (SETTING(generateCallGraph)) {
        prog->printCallGraph();
    }

    return prog;
}


int Boomerang::decompile(const QString& fname, const char *pname)
{
    std::unique_ptr<Prog> prog = nullptr;
    time_t start;

    time(&start);

    prog = loadAndDecode(fname, pname);

    if (prog == nullptr) {
        return 1;
    }

    if (SETTING(stopBeforeDecompile)) {
        return 0;
    }

    LOG_MSG("Decompiling...");
    prog->decompile();

    if (!SETTING(dotFile).isEmpty()) {
        CfgDotWriter().writeCFG(prog.get(), SETTING(dotFile));
    }

    if (SETTING(printAST)) {
        LOG_MSG("Printing AST...");

        for (const auto& module : prog->getModuleList()) {
            for (Function *func : *module) {
                if (!func->isLib()) {
                    UserProc *proc = static_cast<UserProc *>(func);
                    CFGCompressor().compressCFG(proc->getCFG());
                    proc->printAST();
                }
            }
        }
    }

    LOG_MSG("Generating code...");
    Boomerang::get()->getCodeGenerator()->generateCode(prog.get());

    LOG_VERBOSE("Output written to '%1'", Boomerang::get()->getSettings()->getOutputDirectory().absoluteFilePath(prog->getRootModule()->getName()));

    time_t end;
    time(&end);
    int hours = static_cast<int>((end - start) / 60 / 60);
    int mins  = static_cast<int>((end - start) / 60 - hours * 60);
    int secs  = static_cast<int>((end - start) - (hours * 60 * 60) - (mins * 60));

    LOG_MSG("Completed in %1 hours %2 minutes %3 seconds.", hours, mins, secs);

    return 0;
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


IBinaryImage *Boomerang::getImage()
{
    return getOrCreateProject()->getImage();
}


IBinarySymbolTable *Boomerang::getSymbols()
{
    return m_symbols.get();
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


IProject *Boomerang::getOrCreateProject()
{
    if (!m_currentProject) {
        m_currentProject.reset(new Project);
    }

    return m_currentProject.get();
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
