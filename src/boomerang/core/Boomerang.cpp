/*
 * Copyright (C) 2002-2006, Mike Van Emmerik and Trent Waddington
 */

/***************************************************************************/ /**
 * \file    Boomerang.cpp
 * \brief   Command line processing for the Boomerang decompiler
 ******************************************************************************/

#include "Boomerang.h"

#include "boomerang/codegen/CCodeGenerator.h"

#include "boomerang/db/BinaryImage.h"
#include "boomerang/db/SymTab.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/core/Project.h"
#include "boomerang/db/Signature.h"
#include "boomerang/util/Log.h"

#include "boomerang/frontend/Frontend.h"

// TODO: Move the Objective-C loader code to a more suitable place
#include "../../boomerang-loaders/machO/MachOBinaryLoader.h" // For Objective-C stuff

#include <ctime>


static Boomerang* boomerang = nullptr;

Boomerang::Boomerang()
    : m_settings(new Settings)
    , m_currentProject(nullptr)
    , m_symbols(new SymTab)
    , m_codeGenerator(new CCodeGenerator)
{
    m_settings->setWorkingDirectory(".");
    m_settings->setDataDirectory("../lib/boomerang/");
}


Boomerang::~Boomerang()
{
    delete m_codeGenerator;
    delete m_currentProject;
}


ICodeGenerator *Boomerang::getCodeGenerator()
{
    return m_codeGenerator;
}


void Boomerang::objcDecode(const std::map<QString, ObjcModule>& modules, Prog *prog)
{
    LOG_MSG("Adding Objective-C information to Prog.");
    Module *root = prog->getRootModule();

    for (auto& modules_it : modules) {
        const ObjcModule& mod     = (modules_it).second;
        Module            *module = prog->getOrInsertModule(mod.name);
        root->addChild(module);
        LOG_VERBOSE("\tModule: %1", mod.name);
        ClassModFactory class_fact;

        for (auto& elem : mod.classes) {
            const ObjcClass& c   = (elem).second;
            Module           *cl = prog->getOrInsertModule(mod.name, class_fact);
            root->addChild(cl);
            LOG_VERBOSE("\t\tClass: %1", c.name);

            for (auto& _it2 : c.methods) {
                const ObjcMethod& m = (_it2).second;
                // TODO: parse :'s in names
                QString  method_name = m.name + "_" + m.types;
                Function *existing   = prog->findProc(method_name);

                if (existing) {
                    assert(!"Name clash in objc processor ?");
                    continue;
                }

                Function *p = cl->getOrInsertFunction(method_name, m.addr);
                p->setSignature(Signature::instantiate(prog->getFrontEndId(), CallConv::C, method_name));
                // TODO: decode types in m.types
                LOG_VERBOSE("\t\t\tMethod: ", m.name);
            }
        }
    }

    LOG_VERBOSE("");
}


Prog *Boomerang::loadAndDecode(const QString& fname, const char *pname)
{
    LOG_MSG("Loading...");
    Prog      *prog = new Prog(fname);
    IFrontEnd *fe   = IFrontEnd::create(fname, prog, this->getOrCreateProject());

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

        fe->decode(prog, SETTING(decodeMain), pname);

        if (!SETTING(noDecodeChildren)) {
            // this causes any undecoded userprocs to be decoded
            LOG_MSG("Decoding anything undecoded...");
            fe->decode(prog, Address::INVALID);
        }
    }

    LOG_MSG("Finishing decode...");
    prog->finishDecode();

    Boomerang::get()->alertEndDecode();

    LOG_MSG("Found %1 procs", prog->getNumProcs());

    // GK: The analysis which was performed was not exactly very "analysing", and so it has been moved to
    // prog::finishDecode, UserProc::assignProcsToCalls and UserProc::finalSimplify
    // std::cout << "analysing...\n";
    // prog->analyse();

    if (SETTING(generateSymbols)) {
        prog->printSymbolsToFile();
    }

    if (SETTING(generateCallGraph)) {
        prog->printCallGraph();
        prog->printCallGraphXML();
    }

    return prog;
}


int Boomerang::decompile(const QString& fname, const char *pname)
{
    Prog   *prog = nullptr;
    time_t start;

    time(&start);

    if (SETTING(loadBeforeDecompile)) {
        LOG_ERROR("Loading persisted state is not implemented.");
    }
    else {
        prog = loadAndDecode(fname, pname);

        if (prog == nullptr) {
            return 1;
        }
    }

    if (SETTING(saveBeforeDecompile)) {
        LOG_ERROR("Saving persisted state is not implemented.");
    }

    if (SETTING(stopBeforeDecompile)) {
        return 0;
    }

    LOG_MSG("Decompiling...");
    prog->decompile();

    if (!SETTING(dotFile).isEmpty()) {
        prog->generateDotFile();
    }

    if (SETTING(printAST)) {
        LOG_MSG("Printing AST...");

        for (const Module *module : prog->getModuleList()) {
            for (Function *func : *module) {
                if (!func->isLib()) {
                    UserProc *u = (UserProc *)func;
                    u->getCFG()->compressCfg();
                    u->printAST();
                }
            }
        }
    }

    LOG_MSG("Generating code...");
    Boomerang::get()->getCodeGenerator()->generateCode(prog);

    LOG_VERBOSE("Output written to '%1'", Boomerang::get()->getSettings()->getOutputDirectory().absoluteFilePath(prog->getRootModule()->getName()));

    time_t end;
    time(&end);
    int hours = (int)((end - start) / 60 / 60);
    int mins  = (int)((end - start) / 60 - hours * 60);
    int secs  = (int)((end - start) - (hours * 60 * 60) - (mins * 60));

    LOG_MSG("Completed in %1 hours %2 minutes %3 seconds.", hours, mins, secs);

    delete prog;
    return 0;
}


void Boomerang::miniDebugger(UserProc *p, const char *description)
{
    QTextStream q_cout(stdout);
    QTextStream q_cin(stdin);

    q_cout << "decompiling " << p->getName() << ": " << description << "\n";
    QString stopAt;
    static std::set<Statement *> watches;

    if (stopAt.isEmpty() || !p->getName().compare(stopAt)) {
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
                p->print(q_cout);
            }
            else if (line.startsWith("fprint")) {
                QFile tgt("out.proc");

                if (tgt.open(QFile::WriteOnly)) {
                    QTextStream of(&tgt);
                    p->print(of);
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
                    p->getStatements(stmts);
                    StatementList::iterator it;

                    for (it = stmts.begin(); it != stmts.end(); it++) {
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
    if (!boomerang) {
        boomerang = new Boomerang();
    }

    return boomerang;
}


IBinaryImage *Boomerang::getImage()
{
    return getOrCreateProject()->getImage();
}


IBinarySymbolTable *Boomerang::getSymbols()
{
    return m_symbols;
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


IProject* Boomerang::getOrCreateProject()
{
    if (!m_currentProject) {
        m_currentProject = new Project;
    }
    return m_currentProject;
}

