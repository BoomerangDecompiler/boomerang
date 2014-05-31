/*
 * Copyright (C) 2002-2006, Mike Van Emmerik and Trent Waddington
 */
/***************************************************************************/ /**
  * \file       boomerang.cpp
  * \brief   Command line processing for the Boomerang decompiler
  ******************************************************************************/

#include "boomerang.h"

#include "config.h"
#include "prog.h"
#include "proc.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "hllcode.h"
#include "codegen/chllcode.h"
//#include "transformer.h"
#include "util.h"
#include "log.h"
#include "xmlprogparser.h"

// For the -nG switch to disable the garbage collector
#include "config.h"
#ifdef HAVE_LIBGC
#include "gc.h"
#else
#define NO_GARBAGE_COLLECTOR
#endif

#include <inttypes.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <QtCore>

Boomerang *Boomerang::boomerang = nullptr;

/**
 * Initializes the Boomerang object.
 * The default settings are:
 * - All options disabled
 * - Infinite propagations
 * - A maximum memory depth of 99
 * - The path to the executable is "./"
 * - The output directory is "./output/"
 */
Boomerang::Boomerang() : progPath("./"), outputPath("./output/"), logger(nullptr) {}

/**
 * \returns the Log object associated with the object.
 */
Log &Boomerang::log() { return *logger; }

SeparateLogger Boomerang::separate_log(const QString &v) { return SeparateLogger(v); }
Log &Boomerang::if_verbose_log(int verbosity_level) {
    static NullLogger null_log;
    if (verbosity_level == 2)
        return *logger;
    if ((verbosity_level == 1) && VERBOSE)
        return *logger;
    else
        return null_log;
}

/**
 * Sets the outputfile to be the file "log" in the default output directory.
 */
FileLogger::FileLogger() : out((Boomerang::get()->getOutputPath() + "log").toStdString()) {}

SeparateLogger::SeparateLogger(const QString &v) {
    static QMap<QString, int> versions;
    if (!versions.contains(v))
        versions[v] = 0;
    QDir outDir(Boomerang::get()->getOutputPath());
    QString full_path = outDir.absoluteFilePath(QString("%1_%2.log").arg(v).arg(versions[v]++, 2, 10, QChar('0')));
    out = new std::ofstream(full_path.toStdString());
}

/**
 * Returns the HLLCode for the given proc.
 * \return The HLLCode for the specified UserProc.
 */
HLLCode *Boomerang::getHLLCode(UserProc *p) { return new CHLLCode(p); }

/**
 * Prints help for the interactive mode.
 */
void Boomerang::helpcmd() const {
    // Column 98 of this source file is column 80 of output (don't use tabs)
    //            ____.____1____.____2____.____3____.____4____.____5____.____6____.____7____.____8
    std::cout << "Available commands (for use with -k):\n";
    std::cout << "  decode                             : Loads and decodes the specified binary.\n";
    std::cout << "  decompile [proc]                   : Decompiles the program or specified proc.\n";
    std::cout << "  codegen [cluster]                  : Generates code for the program or a\n";
    std::cout << "                                       specified cluster.\n";
    std::cout << "  move proc <proc> <cluster>         : Moves the specified proc to the specified\n";
    std::cout << "                                       cluster.\n";
    std::cout << "  move cluster <cluster> <parent>    : Moves the specified cluster to the\n";
    std::cout << "                                       specified parent cluster.\n";
    std::cout << "  add cluster <cluster> [parent]     : Adds a new cluster to the root/specified\n";
    std::cout << "                                       cluster.\n";
    std::cout << "  delete cluster <cluster>           : Deletes an empty cluster.\n";
    std::cout << "  rename proc <proc> <newname>       : Renames the specified proc.\n";
    std::cout << "  rename cluster <cluster> <newname> : Renames the specified cluster.\n";
    std::cout << "  info prog                          : Print info about the program.\n";
    std::cout << "  info cluster <cluster>             : Print info about a cluster.\n";
    std::cout << "  info proc <proc>                   : Print info about a proc.\n";
    std::cout << "  print <proc>                       : Print the RTL for a proc.\n";
    std::cout << "  help                               : This help.\n";
    std::cout << "  exit                               : Quit the shell.\n";
}

/**
 * Creates a directory and tests it.
 *
 * \param dir The name of the directory.
 *
 * \retval true The directory is valid.
 * \retval false The directory is invalid.
 */
bool createDirectory(const QString &dir) {
    QFileInfo dir_fi(dir);
    QString abspath = dir_fi.absolutePath();
    return QDir::root().mkpath(abspath);
}

/**
 * Prints a tree graph.
 */
void Cluster::printTree(std::ostream &ostr) {
    ostr << "\t\t" << name << "\n";
    for (auto &elem : children)
        elem->printTree(ostr);
}

static int commandNameToID(const std::string &_cmd) {
    const char *cmd = _cmd.c_str();
    if (!strcmp(cmd, "decode"))
        return 1;
    if (!strcmp(cmd, "load"))
        return 2;
    if (!strcmp(cmd, "save"))
        return 3;
    if (!strcmp(cmd, "decompile"))
        return 4;
    if (!strcmp(cmd, "codegen"))
        return 5;
    if (!strcmp(cmd, "move"))
        return 6;
    if (!strcmp(cmd, "add"))
        return 7;
    if (!strcmp(cmd, "delete"))
        return 8;
    if (!strcmp(cmd, "rename"))
        return 9;
    if (!strcmp(cmd, "info"))
        return 10;
    if (!strcmp(cmd, "print"))
        return 11;
    if (!strcmp(cmd, "exit") || !strcmp(cmd, "quit"))
        return 12;
    if (!strcmp(cmd, "help"))
        return 13;
    return -1;
}
/**
 * Parse and execute a command supplied in interactive mode.
 *
 * \param args        The array of argument strings.
 *
 * \return A value indicating what happened.
 *
 * \retval 0 Success
 * \retval 1 Failure
 * \retval 2 The user exited with \a quit or \a exit
 */
int Boomerang::processCommand(std::vector<std::string> &args) {
    static Prog *prog = nullptr;
    if (args.empty()) {
        std::cerr << "not arguments\n";
        return 1;
    }

    int command = commandNameToID(args[0]);
    switch (command) {
    case 1: {
        if (args.size() < 2) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        Prog *p = loadAndDecode(args[1].c_str());
        if (p == nullptr) {
            std::cerr << "failed to load " << args[1] << "\n";
            return 1;
        }
        prog = p;
        break;
    }
    case 2: {
        if (args.size() < 2) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        QString fname = args[1].c_str();
        XMLProgParser *p = new XMLProgParser();
        Prog *pr = p->parse(fname);
        if (pr == nullptr) {
            // try guessing
            pr = p->parse(outputPath + fname + "/" + fname + ".xml");
            if (pr == nullptr) {
                qCritical() << "failed to read xml " << fname << "\n";
                return 1;
            }
        }
        prog = pr;
        break;
    }
    case 3: {
        if (prog == nullptr) {
            std::cerr << "need to load or decode before save!\n";
            return 1;
        }
        XMLProgParser *p = new XMLProgParser();
        p->persistToXML(prog);
        break;
    }
    case 4: {
        if (prog == nullptr) {
            std::cerr << "no valid Prog object !\n";
            return 1;
        }

        if (args.size() > 1) {
            Function *proc = prog->findProc(args[1].c_str());
            if (proc == nullptr) {
                std::cerr << "cannot find proc " << args[1] << "\n";
                return 1;
            }
            if (proc->isLib()) {
                std::cerr << "cannot decompile a lib proc\n";
                return 1;
            }
            int indent = 0;
            assert(dynamic_cast<UserProc *>(proc) != nullptr);
            ((UserProc *)proc)->decompile(new ProcList, indent);
        } else {
            prog->decompile();
        }
    }
    case 5: {
        if (prog == nullptr) {
            std::cerr << "no valid Prog object !\n";
            return 1;
        }
        if (args.size() > 1) {
            Cluster *cluster = prog->findCluster(args[1]);
            if (cluster == nullptr) {
                std::cerr << "cannot find cluster " << args[1] << "\n";
                return 1;
            }
            prog->generateCode(cluster);
        } else {
            prog->generateCode();
        }
    }
    case 6: {
        if (prog == nullptr) {
            std::cerr << "no valid Prog object !\n";
            return 1;
        }
        if (args.size() <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        if (!strcmp(args[1].c_str(), "proc")) {
            if (args.size() < 4) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Function *proc = prog->findProc(args[2].c_str());
            if (proc == nullptr) {
                std::cerr << "cannot find proc " << args[2] << "\n";
                return 1;
            }

            Cluster *cluster = prog->findCluster(args[3]);
            if (cluster == nullptr) {
                std::cerr << "cannot find cluster " << args[3] << "\n";
                return 1;
            }
            proc->setCluster(cluster);
        } else if (!args[1].compare("cluster")) {
            if (args.size() <= 3) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Cluster *cluster = prog->findCluster(args[2]);
            if (cluster == nullptr) {
                std::cerr << "cannot find cluster " << args[2] << "\n";
                return 1;
            }

            Cluster *parent = prog->findCluster(args[3]);
            if (parent == nullptr) {
                std::cerr << "cannot find cluster " << args[3] << "\n";
                return 1;
            }

            parent->addChild(cluster);
        } else {
            std::cerr << "don't know how to move a " << args[1] << "\n";
            return 1;
        }
    }
    case 7: {
        if (prog == nullptr) {
            std::cerr << "no valid Prog object !\n";
            return 1;
        }
        if (args.size() <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        if (args[1] == "cluster") {
            if (args.size() <= 2) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Cluster *cluster = new Cluster(args[2]);
            if (cluster == nullptr) {
                std::cerr << "cannot create cluster " << args[2] << "\n";
                return 1;
            }

            Cluster *parent = prog->getRootCluster();
            if (args.size() > 3) {
                parent = prog->findCluster(args[3]);
                if (cluster == nullptr) {
                    std::cerr << "cannot find cluster " << args[3] << "\n";
                    return 1;
                }
            }

            parent->addChild(cluster);
        } else {
            std::cerr << "don't know how to add a " << args[1] << "\n";
            return 1;
        }
    }
    case 8: {
        if (prog == nullptr) {
            std::cerr << "no valid Prog object !\n";
            return 1;
        }
        if (args.size() <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        if (!args[1].compare("cluster")) {
            if (args.size() <= 2) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Cluster *cluster = prog->findCluster(args[2]);
            if (cluster == nullptr) {
                std::cerr << "cannot find cluster " << args[2] << "\n";
                return 1;
            }

            if (cluster->hasChildren() || cluster == prog->getRootCluster()) {
                std::cerr << "cluster " << args[2] << " is not empty\n";
                return 1;
            }

            if (prog->clusterUsed(cluster)) {
                std::cerr << "cluster " << args[2] << " is not empty\n";
                return 1;
            }
            QFile::remove(cluster->getOutPath("xml"));
            QFile::remove(cluster->getOutPath("c"));
            assert(cluster->getParent());
            cluster->getParent()->removeChild(cluster);
        } else {
            std::cerr << "don't know how to delete a " << args[1] << "\n";
            return 1;
        }
    }
    case 9: {
        if (prog == nullptr) {
            std::cerr << "no valid Prog object !\n";
            return 1;
        }
        if (args.size() <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        if (args[1] == "proc") {
            if (args.size() <= 3) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Function *proc = prog->findProc(args[2].c_str());
            if (proc == nullptr) {
                std::cerr << "cannot find proc " << args[2] << "\n";
                return 1;
            }

            Function *nproc = prog->findProc(args[3].c_str());
            if (nproc != nullptr) {
                std::cerr << "proc " << args[3] << " already exists\n";
                return 1;
            }

            proc->setName(args[3].c_str());
        } else if (args[1] == "cluster") {
            if (args.size() <= 3) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Cluster *cluster = prog->findCluster(args[2]);
            if (cluster == nullptr) {
                std::cerr << "cannot find cluster " << args[2] << "\n";
                return 1;
            }

            Cluster *ncluster = prog->findCluster(args[3]);
            if (ncluster == nullptr) {
                std::cerr << "cluster " << args[3] << " already exists\n";
                return 1;
            }

            cluster->setName(args[3]);
        } else {
            std::cerr << "don't know how to rename a " << args[1] << "\n";
            return 1;
        }
    }
    case 10: {
        if (prog == nullptr) {
            std::cerr << "no valid Prog object !\n";
            return 1;
        }
        if (args.size() <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        if (args[1] == "prog") {

            std::cout << "prog " << prog->getName().toStdString() << ":\n";
            std::cout << "\tclusters:\n";
            prog->getRootCluster()->printTree(std::cout);
            std::cout << "\n\tlibprocs:\n";
            PROGMAP::const_iterator it;
            for (Function *p = prog->getFirstProc(it); p; p = prog->getNextProc(it))
                if (p->isLib())
                    std::cout << "\t\t" << p->getName().toStdString() << "\n";
            std::cout << "\n\tuserprocs:\n";
            for (Function *p = prog->getFirstProc(it); p; p = prog->getNextProc(it))
                if (!p->isLib())
                    std::cout << "\t\t" << p->getName().toStdString() << "\n";
            std::cout << "\n";

            return 0;
        } else if (args[1] == "cluster") {
            if (args.size() <= 2) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Cluster *cluster = prog->findCluster(args[2]);
            if (cluster == nullptr) {
                std::cerr << "cannot find cluster " << args[2] << "\n";
                return 1;
            }

            std::cout << "cluster " << cluster->getName() << ":\n";
            if (cluster->getParent())
                std::cout << "\tparent = " << cluster->getParent()->getName() << "\n";
            else
                std::cout << "\troot cluster.\n";
            std::cout << "\tprocs:\n";
            PROGMAP::const_iterator it;
            for (Function *p = prog->getFirstProc(it); p; p = prog->getNextProc(it))
                if (p->getCluster() == cluster)
                    std::cout << "\t\t" << p->getName().toStdString() << "\n";
            std::cout << "\n";

            return 0;
        } else if (args[1] == "proc") {
            if (args.size() <= 2) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Function *proc = prog->findProc(args[2].c_str());
            if (proc == nullptr) {
                std::cerr << "cannot find proc " << args[2] << "\n";
                return 1;
            }

            std::cout << "proc " << proc->getName().toStdString() << ":\n";
            std::cout << "\tbelongs to cluster " << proc->getCluster()->getName() << "\n";
            std::cout << "\tnative address " << proc->getNativeAddress() << "\n";
            if (proc->isLib())
                std::cout << "\tis a library proc.\n";
            else {
                std::cout << "\tis a user proc.\n";
                UserProc *p = (UserProc *)proc;
                if (p->isDecoded())
                    std::cout << "\thas been decoded.\n";
                // if (p->isAnalysed())
                //    std::cout << "\thas been analysed.\n";
            }
            std::cout << "\n";

            return 0;
        } else {
            std::cerr << "don't know how to print info about a " << args[1] << "\n";
            return 1;
        }
    }
    case 11: {
        if (prog == nullptr) {
            std::cerr << "no valid Prog object !\n";
            return 1;
        }
        if (args.size() < 2) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }

        Function *proc = prog->findProc(args[1].c_str());
        if (proc == nullptr) {
            std::cerr << "cannot find proc " << args[1] << "\n";
            return 1;
        }
        if (proc->isLib()) {
            std::cerr << "cannot print a libproc.\n";
            return 1;
        }

        ((UserProc *)proc)->print(std::cout);
        std::cout << "\n";
        return 0;
    }
    case 12: {
        return 2;
    }
    case 13: {
        helpcmd();
        return 0;
    }
    default:
        std::cerr << "unknown cmd " << args[0] << ".\n";
        return 1;
    }
    return 0;
}

/**
 * Set the path to the %Boomerang executable.
 *
 */
void Boomerang::setProgPath(const QString &p) {
    progPath = p + "/";
    outputPath = progPath + "/output/"; // Default output path (can be overridden with -o below)
}

/**
 * Set the path where the %Boomerang executable will search for plugins.
 *
 */
void Boomerang::setPluginPath(const QString &p) { BinaryFileFactory::setBasePath(p); }

/**
 * Sets the directory in which Boomerang creates its output files.  The directory will be created if it doesn't exist.
 *
 * \param path        the path to the directory
 *
 * \retval true Success.
 * \retval false The directory could not be created.
 */
bool Boomerang::setOutputDirectory(const QString &path) {
    outputPath = path;
    // Create the output directory, if needed
    if (!createDirectory(path)) {
        qWarning() << "Warning! Could not create path " << outputPath << "!\n";
        return false;
    }
    if (logger == nullptr)
        setLogger(new FileLogger());
    return true;
}

/**
 * Adds information about functions and classes from Objective-C modules to the Prog object.
 *
 * \param modules A map from name to the Objective-C modules.
 * \param prog The Prog object to add the information to.
 */
void Boomerang::objcDecode(std::map<std::string, ObjcModule> &modules, Prog *prog) {
    LOG_VERBOSE(1) << "Adding Objective-C information to Prog.\n";
    Cluster *root = prog->getRootCluster();
    for (auto &modules_it : modules) {
        ObjcModule &mod = (modules_it).second;
        Module *module = new Module(mod.name.c_str());
        root->addChild(module);
        LOG_VERBOSE(1) << "\tModule: " << mod.name.c_str() << "\n";
        for (auto &elem : mod.classes) {
            ObjcClass &c = (elem).second;
            Class *cl = new Class(c.name.c_str());
            root->addChild(cl);
            LOG_VERBOSE(1) << "\t\tClass: " << c.name.c_str() << "\n";
            for (auto &_it2 : c.methods) {
                ObjcMethod &m = (_it2).second;
                // TODO: parse :'s in names
                Function *p = prog->newProc(m.name.c_str(), m.addr);
                p->setCluster(cl);
                // TODO: decode types in m.types
                LOG_VERBOSE(1) << "\t\t\tMethod: " << m.name.c_str() << "\n";
            }
        }
    }
    LOG_VERBOSE(1) << "\n";
}

/**
 * Loads the executable file and decodes it.
 *
 * \param fname The name of the file to load.
 * \param pname How the Prog will be named.
 *
 * \returns A Prog object.
 */
Prog *Boomerang::loadAndDecode(const QString &fname, const char *pname) {
    std::cout << "loading...\n";
    Prog *prog = new Prog();
    FrontEnd *fe = FrontEnd::Load(fname, prog);
    if (fe == nullptr) {
        std::cerr << "failed.\n";
        return nullptr;
    }
    prog->setFrontEnd(fe);

    // Add symbols from -s switch(es)
    for (auto &elem : symbols) {
        fe->AddSymbol((elem).first, (elem).second.c_str());
    }
    fe->readLibraryCatalog(); // Needed before readSymbolFile()

    for (auto &elem : symbolFiles) {
        std::cout << "reading symbol file " << elem.c_str() << "\n";
        prog->readSymbolFile(elem.c_str());
    }
    ObjcAccessInterface *objc = qobject_cast<ObjcAccessInterface *>(fe->getBinaryFile());
    if (objc) {
        std::map<std::string, ObjcModule> &objcmodules = objc->getObjcModules();
        if (objcmodules.size())
            objcDecode(objcmodules, prog);
    }
    // Entry points from -e (and -E) switch(es)
    for (auto &elem : entrypoints) {
        std::cout << "decoding specified entrypoint " << elem << "\n";
        prog->decodeEntryPoint(elem);
    }

    if (entrypoints.size() == 0) { // no -e or -E given
        if (decodeMain)
            std::cout << "decoding entry point...\n";
        fe->decode(prog, decodeMain, pname);

        if (!noDecodeChildren) {
            // this causes any undecoded userprocs to be decoded
            std::cout << "decoding anything undecoded...\n";
            fe->decode(prog, NO_ADDRESS);
        }
    }

    std::cout << "finishing decode...\n";
    prog->finishDecode();

    Boomerang::get()->alert_end_decode();

    std::cout << "found " << std::dec << prog->getNumUserProcs() << " procs\n";

    // GK: The analysis which was performed was not exactly very "analysing", and so it has been moved to
    // prog::finishDecode, UserProc::assignProcsToCalls and UserProc::finalSimplify
    // std::cout << "analysing...\n";
    // prog->analyse();

    if (generateSymbols) {
        prog->printSymbolsToFile();
    }
    if (generateCallGraph) {
        prog->printCallGraph();
        prog->printCallGraphXML();
    }
    return prog;
}

/**
 * The program will be subsequently be loaded, decoded, decompiled and written to a source file.
 * After decompilation the elapsed time is printed to std::cerr.
 *
 * \param fname The name of the file to load.
 * \param pname The name that will be given to the Proc.
 *
 * \return Zero on success, nonzero on faillure.
 */
int Boomerang::decompile(const char *fname, const char *pname) {
    Prog *prog;
    time_t start;
    time(&start);
    if (logger == nullptr)
        setLogger(new FileLogger());

//    std::cout << "setting up transformers...\n";
//    ExpTransformer::loadAll();

    if (loadBeforeDecompile) {
        std::cout << "loading persisted state...\n";
        XMLProgParser *p = new XMLProgParser();
        prog = p->parse(fname);
    } else
    {
        prog = loadAndDecode(fname, pname);
        if (prog == nullptr)
            return 1;
    }

    if (saveBeforeDecompile) {
        std::cout << "saving persistable state...\n";
        XMLProgParser *p = new XMLProgParser();
        p->persistToXML(prog);
    }

    if (stopBeforeDecompile)
        return 0;

    std::cout << "decompiling...\n";
    prog->decompile();

    if (!dotFile.empty())
        prog->generateDotFile();

    if (printAST) {
        std::cout << "printing AST...\n";
        PROGMAP::const_iterator it;
        for (Function *p = prog->getFirstProc(it); p; p = prog->getNextProc(it))
            if (!p->isLib()) {
                UserProc *u = (UserProc *)p;
                u->getCFG()->compressCfg();
                u->printAST();
            }
    }

    qDebug() << "generating code...\n";
    prog->generateCode();

    qDebug() << "output written to " << outputPath << prog->getRootCluster()->getName() << "\n";

    if (Boomerang::get()->ofsIndCallReport)
        ofsIndCallReport->close();

    time_t end;
    time(&end);
    int hours = (int)((end - start) / 60 / 60);
    int mins = (int)((end - start) / 60 - hours * 60);
    int secs = (int)((end - start) - (hours * 60 * 60) - (mins * 60));
    std::cout << "completed in " << std::dec;
    if (hours)
        std::cout << hours << " hours ";
    if (hours || mins)
        std::cout << mins << " mins ";
    std::cout << secs << " sec" << (secs == 1 ? "" : "s") << ".\n";

    return 0;
}

/**
 * Saves the state of the Prog object to a XML file.
 * \param prog The Prog object to save.
 */
void Boomerang::persistToXML(Prog *prog) {
    LOG << "saving persistable state...\n";
    XMLProgParser *p = new XMLProgParser();
    p->persistToXML(prog);
}
/**
 * Loads the state of a Prog object from a XML file.
 * \param fname The name of the XML file.
 * \return The loaded Prog object.
 */
Prog *Boomerang::loadFromXML(const char *fname) {
    LOG << "loading persistable state...\n";
    XMLProgParser *p = new XMLProgParser();
    return p->parse(fname);
}

/**
 * Prints the last lines of the log file.
 */
void Boomerang::logTail() { logger->tail(); }

void Boomerang::alert_decompile_debug_point(UserProc *p, const char *description) {
    if (stopAtDebugPoints) {
        std::cout << "decompiling " << p->getName().toStdString() << ": " << description << "\n";
        static char *stopAt = nullptr;
        static std::set<Statement *> watches;
        if (stopAt == nullptr || !p->getName().compare(stopAt)) {
            // This is a mini command line debugger.  Feel free to expand it.
            for (auto const &watche : watches) {
                (watche)->print(std::cout);
                std::cout << "\n";
            }
            std::cout << " <press enter to continue> \n";
            char line[1024];
            while (1) {
                *line = 0;
                fgets(line, 1024, stdin);
                if (!strncmp(line, "print", 5))
                    p->print(std::cout);
                else if (!strncmp(line, "fprint", 6)) {
                    std::ofstream of("out.proc");
                    p->print(of);
                    of.close();
                } else if (!strncmp(line, "run ", 4)) {
                    stopAt = strdup(line + 4);
                    if (strchr(stopAt, '\n'))
                        *strchr(stopAt, '\n') = 0;
                    if (strchr(stopAt, ' '))
                        *strchr(stopAt, ' ') = 0;
                    break;
                } else if (!strncmp(line, "watch ", 6)) {
                    int n = atoi(line + 6);
                    StatementList stmts;
                    p->getStatements(stmts);
                    StatementList::iterator it;
                    for (it = stmts.begin(); it != stmts.end(); it++)
                        if ((*it)->getNumber() == n) {
                            watches.insert(*it);
                            std::cout << "watching " << *it << "\n";
                        }
                } else
                    break;
            }
        }
    }
    for (auto const &elem : watchers)
        (elem)->alert_decompile_debug_point(p, description);
}

const char *Boomerang::getVersionStr() { return VERSION; }
