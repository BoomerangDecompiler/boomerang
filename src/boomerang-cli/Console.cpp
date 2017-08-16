#include "Console.h"

#include <QFile>
#include <QString>
#include <QStringRef>
#include <QTextStream>
#include <iostream>


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/codegen/ICodeGenerator.h"

static Prog* prog;


Console::Console()
{
    m_commandTypes["decode"]    = CT_decode;
    m_commandTypes["decompile"] = CT_decompile;
    m_commandTypes["codegen"]   = CT_codegen;
    m_commandTypes["move"]      = CT_move;
    m_commandTypes["add"]       = CT_add;
    m_commandTypes["delete"]    = CT_delete;
    m_commandTypes["rename"]    = CT_rename;
    m_commandTypes["info"]      = CT_info;
    m_commandTypes["exit"]      = CT_exit;
    m_commandTypes["quit"]      = CT_exit;
    m_commandTypes["help"]      = CT_help;
    m_commandTypes["replay"]    = CT_replay;

    m_commandTypes["print-callgraph"] = CT_callgraph;
    m_commandTypes["print-cfg"] = CT_printCFG;
    m_commandTypes["print-rtl"] = CT_print;
}


CommandStatus Console::handleCommand(const QString& commandWithArgs)
{
    QStringList args;
    QString command;

    if (!commandSucceeded(splitCommand(commandWithArgs, command, args))) {
        return CommandStatus::Failure;
    }

    return processCommand(command, args);
}


CommandStatus Console::replayFile(const QString& replayFile)
{
    if (replayFile == QString::null) {
        // nothing to execute
        return CommandStatus::Success;
    }

    // replay commands in file
    QFile file(replayFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Cannot open replay file!\n";
        return CommandStatus::Failure;
    }

    // execute commands until the first failure
    QString line;
    QTextStream ist(&file);
    CommandStatus lastResult = CommandStatus::Success;

    while (ist.readLineInto(&line)) {
        std::cout << "boomerang: " << line.toStdString() << std::endl;
        lastResult = this->handleCommand(line);

        if (!commandSucceeded(lastResult)) {
            if (lastResult != CommandStatus::ExitProgram) {
                std::cerr << "Stopping replay due to command failure." << std::endl;
            }
            break;
        }
    }

    return lastResult;
}


CommandStatus Console::splitCommand(const QString& commandWithArgs, QString& mainCommand, QStringList& args)
{
    // remove unnecessary whitespace
    const QString command = commandWithArgs.simplified();

    // cannot use QString::split since we have to take care of quotation marks in arguments
    int i = 0;

    // find first whitespace
    while (i < command.size() && command[i] != ' ') {
        if (command[i] == '\"') { return CommandStatus::ParseError; } // quotation marks in commands are not allowed
        i++;
    }

    mainCommand = QStringRef(&command, 0, i).toString();

    /// extract arguments
    int lastSeparator = i; // position of last ' ' not within quotation marks
    bool isInQuotation = false;

    while (i < command.size()) {
        while (++i < command.size()) {
            if (command[i] == '\"' && command[i-1] != '\\') {
                isInQuotation = !isInQuotation;
            }
            else if (command[i] == ' ') {
                if (!isInQuotation) {
                    break; // found argument
                }
            }
        }

        if (isInQuotation) { return CommandStatus::ParseError; } // missing closing "

        bool argIsQuoted  = (command[lastSeparator+1] == '\"'); // Were we in a quotation before?

        QString arg = command.mid(
            lastSeparator + (argIsQuoted ? 2 : 1),
            i             - (argIsQuoted ? 2 : 1));
        args.push_back(arg);
        lastSeparator = i;
    }

    return CommandStatus::Success;
}


CommandStatus Console::processCommand(const QString& command, const QStringList& args)
{
    switch (commandNameToType(command))
    {
    case CT_decode:    return handleDecode(args);
    case CT_decompile: return handleDecompile(args);
    case CT_codegen:   return handleCodegen(args);
    case CT_callgraph: return handleCallgraph(args);
    case CT_printCFG:  return handleDot(args);
    case CT_replay:    return handleReplay(args);

/*
    case CT_move:

        if (prog == nullptr) {
            err_stream << "no valid Prog object !\n";
            return 1;
        }

        if (args.size() <= 1) {
            err_stream << "not enough arguments for cmd\n";
            return 1;
        }

        if (args[1] == "proc") {
            if (args.size() < 4) {
                err_stream << "not enough arguments for cmd\n";
                return 1;
            }

            Function *proc = prog->findProc(args[2]);

            if (proc == nullptr) {
                err_stream << "cannot find proc " << args[2] << "\n";
                return 1;
            }

            Module *cluster = prog->findModule(args[3]);

            if (cluster == nullptr) {
                err_stream << "cannot find cluster " << args[3] << "\n";
                return 1;
            }

            proc->setParent(cluster);
        }
        else if (!args[1].compare("cluster")) {
            if (args.size() <= 3) {
                err_stream << "not enough arguments for cmd\n";
                return 1;
            }

            Module *cluster = prog->findModule(args[2]);

            if (cluster == nullptr) {
                err_stream << "cannot find cluster " << args[2] << "\n";
                return 1;
            }

            Module *parent = prog->findModule(args[3]);

            if (parent == nullptr) {
                err_stream << "cannot find cluster " << args[3] << "\n";
                return 1;
            }

            parent->addChild(cluster);
        }
        else {
            err_stream << "don't know how to move a " << args[1] << "\n";
            return 1;
        }

        break;

    case CT_add:

        if (prog == nullptr) {
            err_stream << "no valid Prog object !\n";
            return 1;
        }

        if (args.size() <= 1) {
            err_stream << "not enough arguments for cmd\n";
            return 1;
        }

        if (args[1] == "cluster") {
            if (args.size() <= 2) {
                err_stream << "not enough arguments for cmd\n";
                return 1;
            }

            Module *cluster = new Module(args[2], prog, prog->getFrontEnd());

            if (cluster == nullptr) {
                err_stream << "cannot create cluster " << args[2] << "\n";
                return 1;
            }

            Module *parent = prog->getRootCluster();

            if (args.size() > 3) {
                parent = prog->findModule(args[3]);

                if (cluster == nullptr) {
                    err_stream << "cannot find cluster " << args[3] << "\n";
                    return 1;
                }
            }

            parent->addChild(cluster);
        }
        else {
            err_stream << "don't know how to add a " << args[1] << "\n";
            return 1;
        }

        break;

    case CT_delete:

        if (prog == nullptr) {
            err_stream << "no valid Prog object !\n";
            return 1;
        }

        if (args.size() <= 1) {
            err_stream << "not enough arguments for cmd\n";
            return 1;
        }

        if (!args[1].compare("cluster")) {
            if (args.size() <= 2) {
                err_stream << "not enough arguments for cmd\n";
                return 1;
            }

            Module *cluster = prog->findModule(args[2]);

            if (cluster == nullptr) {
                err_stream << "cannot find cluster " << args[2] << "\n";
                return 1;
            }

            if (cluster->hasChildren() || (cluster == prog->getRootCluster())) {
                err_stream << "cluster " << args[2] << " is not empty\n";
                return 1;
            }

            if (prog->isModuleUsed(cluster)) {
                err_stream << "cluster " << args[2] << " is not empty\n";
                return 1;
            }

            QFile::remove(cluster->getOutPath("xml"));
            QFile::remove(cluster->getOutPath("c"));
            assert(cluster->getUpstream());
            cluster->getUpstream()->removeChild(cluster);
        }
        else {
            err_stream << "don't know how to delete a " << args[1] << "\n";
            return 1;
        }

        break;

    case CT_rename:

        if (prog == nullptr) {
            err_stream << "no valid Prog object !\n";
            return 1;
        }

        if (args.size() <= 1) {
            err_stream << "not enough arguments for cmd\n";
            return 1;
        }

        if (args[1] == "proc") {
            if (args.size() <= 3) {
                err_stream << "not enough arguments for cmd\n";
                return 1;
            }

            Function *proc = prog->findProc(args[2]);

            if (proc == nullptr) {
                err_stream << "cannot find proc " << args[2] << "\n";
                return 1;
            }

            Function *nproc = prog->findProc(args[3]);

            if (nproc != nullptr) {
                err_stream << "proc " << args[3] << " already exists\n";
                return 1;
            }

            proc->setName(args[3]);
        }
        else if (args[1] == "cluster") {
            if (args.size() <= 3) {
                err_stream << "not enough arguments for cmd\n";
                return 1;
            }

            Module *cluster = prog->findModule(args[2]);

            if (cluster == nullptr) {
                err_stream << "cannot find cluster " << args[2] << "\n";
                return 1;
            }

            Module *ncluster = prog->findModule(args[3]);

            if (ncluster == nullptr) {
                err_stream << "cluster " << args[3] << " already exists\n";
                return 1;
            }

            cluster->setName(args[3]);
        }
        else {
            err_stream << "don't know how to rename a " << args[1] << "\n";
            return 1;
        }

        break;

    case CT_info:

        if (prog == nullptr) {
            err_stream << "no valid Prog object !\n";
            return 1;
        }

        if (args.size() <= 1) {
            err_stream << "not enough arguments for cmd\n";
            return 1;
        }

        if (args[1] == "prog") {
            out_stream << "prog " << prog->getName() << ":\n";
            out_stream << "\tclusters:\n";
            prog->getRootCluster()->printTree(out_stream);
            out_stream << "\n\tlibprocs:\n";

            // TODO: print module name before function's ?
            for (const Module *module : *prog) {
                for (Function *func : *module) {
                    if (func->isLib()) {
                        out_stream << "\t\t" << func->getName() << "\n";
                    }
                }
            }

            out_stream << "\n\tuserprocs:\n";

            for (const Module *module : *prog) {
                for (Function *func : *module) {
                    if (!func->isLib()) {
                        out_stream << "\t\t" << func->getName() << "\n";
                    }
                }
            }

            out_stream << "\n";

            return 0;
        }
        else if (args[1] == "cluster") {
            if (args.size() <= 2) {
                err_stream << "not enough arguments for cmd\n";
                return 1;
            }

            Module *cluster = prog->findModule(args[2]);

            if (cluster == nullptr) {
                err_stream << "cannot find cluster " << args[2] << "\n";
                return 1;
            }

            out_stream << "cluster " << cluster->getName() << ":\n";

            if (cluster->getUpstream()) {
                out_stream << "\tparent = " << cluster->getUpstream()->getName() << "\n";
            }
            else {
                out_stream << "\troot cluster.\n";
            }

            out_stream << "\tprocs:\n";

            for (Function *f : *cluster) {
                out_stream << "\t\t" << f->getName() << "\n";
            }

            out_stream << "\n";

            return 0;
        }
        else if (args[1] == "proc") {
            if (args.size() <= 2) {
                err_stream << "not enough arguments for cmd\n";
                return 1;
            }

            Function *proc = prog->findProc(args[2]);

            if (proc == nullptr) {
                err_stream << "cannot find proc " << args[2] << "\n";
                return 1;
            }

            out_stream << "proc " << proc->getName() << ":\n";
            out_stream << "\tbelongs to cluster " << proc->getParent()->getName() << "\n";
            out_stream << "\tnative address " << proc->getEntryAddress() << "\n";

            if (proc->isLib()) {
                out_stream << "\tis a library proc.\n";
            }
            else {
                out_stream << "\tis a user proc.\n";
                UserProc *p = (UserProc *)proc;

                if (p->isDecoded()) {
                    out_stream << "\thas been decoded.\n";
                }

                // if (p->isAnalysed())
                //    out_stream << "\thas been analysed.\n";
            }

            out_stream << "\n";

            return 0;
        }
        else {
            err_stream << "don't know how to print info about a " << args[1] << "\n";
            return 1;
        }

    // no break needed all branches return
    case CT_print:
        {

        }
*/
    case CT_print: return handlePrint(args);
    case CT_exit: return handleExit(args);
    case CT_help: return handleHelp(args);

    default:
        std::cout << "Unrecognized command '" << command.toStdString() << "', try 'help'" << std::endl;
        return CommandStatus::ParseError;
    }
}


CommandType Console::commandNameToType(const QString& command)
{
    QMap<QString, CommandType>::iterator it = m_commandTypes.find(command);
    return (it != m_commandTypes.end()) ? *it : CT_unknown;
}


CommandStatus Console::handleDecode(const QStringList& args)
{
    if (args.size() != 1) {
        std::cerr << "Wrong number of arguments for command; Expected 1, got " << args.size() << "." << std::endl;
        return CommandStatus::ParseError;
    }
    else if (prog) {
        std::cerr << "Cannot decode program: A program is already loaded." << std::endl;
        return CommandStatus::Failure;
    }

    prog = Boomerang::get()->loadAndDecode(args[0]);

    if (prog) {
        std::cout << "Loaded '" << args[0].toStdString() << "'." << std::endl;
        return CommandStatus::Success;
    }
    else {
        std::cout << "Failed to load '" << args[0].toStdString() << "'." << std::endl;
        return CommandStatus::Failure;
    }
}


CommandStatus Console::handleDecompile(const QStringList& args)
{
    if (prog == nullptr) {
        std::cerr << "Cannot decompile: Need to 'decode' a program first.\n";
        return CommandStatus::Failure;
    }

    if (args.empty()) {
        prog->decompile();
        return CommandStatus::Success;
    }
    else {
        // decompile all specified procedures
        ProcSet procSet;

        for (const QString& procName : args) {
            Function *proc = prog->findProc(procName);

            if (proc == nullptr) {
                std::cerr << "Cannot find function '" << procName.toStdString() << "'\n";
                return CommandStatus::Failure;
            }
            else if (proc->isLib()) {
                std::cerr << "Cannot decompile library function '" << procName.toStdString() << "'\n";
                return CommandStatus::Failure;
            }

            UserProc* userProc = dynamic_cast<UserProc*>(proc);
            assert(userProc != nullptr);

            procSet.insert(userProc);
        }

        for (UserProc* userProc : procSet) {
            int indent = 0;
            userProc->decompile(new ProcList, indent);
        }

        return CommandStatus::Success;
    }
}


CommandStatus Console::handleCodegen(const QStringList& args)
{
    if (prog == nullptr) {
        std::cerr << "Cannot generate code: need to 'decompile' first.\n";
        return CommandStatus::Failure;
    }

    if (args.empty()) {
        Boomerang::get()->getCodeGenerator()->generateCode(prog);
    }
    else {
        std::set<Module*> modules;

        for (QString name : args) {
            Module* module = prog->findModule(name);
            if (!module) {
                std::cerr << "Cannot find module '" << name.toStdString() << "'\n";
                return CommandStatus::Failure;
            }

            modules.insert(module);
        }

        for (Module* mod : modules) {
            Boomerang::get()->getCodeGenerator()->generateCode(prog, mod);
        }
    }

    std::cout << "Code generated." << std::endl;
    return CommandStatus::Success;
}


CommandStatus Console::handleCallgraph(const QStringList& args)
{
    if (!args.empty()) {
        std::cerr << "Wrong number of arguments for command; Expected 0, got " << args.size() << "." << std::endl;
        return CommandStatus::ParseError;
    }
    else if (prog == nullptr) {
        std::cerr << "Cannot print call graph: No program loaded.\n";
        return CommandStatus::Failure;
    }


    prog->printCallGraph();
    return CommandStatus::Success;
}


CommandStatus Console::handleDot(const QStringList& args)
{
    if (!args.empty()) {
        std::cerr << "Wrong number of arguments for command; Expected 0, got " << args.size() << "." << std::endl;
        return CommandStatus::ParseError;
    }
    else if (prog == nullptr) {
        std::cerr << "Cannot print call graph: No program loaded.\n";
        return CommandStatus::Failure;
    }

    prog->generateDotFile();
    return CommandStatus::Success;
}


CommandStatus Console::handleReplay(const QStringList& args)
{
    if (args.size() != 1) {
        std::cerr << "Wrong number of arguments for command; Expected 1, got " << args.size() << "." << std::endl;
        return CommandStatus::ParseError;
    }

    return replayFile(args[0]);
}


CommandStatus Console::handlePrint(const QStringList& args)
{
    if (args.empty()) {
        std::cerr << "not enough arguments for cmd" << std::endl;
        return CommandStatus::ParseError;
    }
    else if (prog == nullptr) {
        std::cerr << "No valid Prog object!" << std::endl;
        return CommandStatus::Failure;
    }

    for (QString procName : args) {
        Function *proc = prog->findProc(procName);

        if (proc == nullptr) {
            std::cerr << "Cannot find procedure " << procName.toStdString() << std::endl;
            return CommandStatus::Failure;
        }
        else if (proc->isLib()) {
            std::cerr << "Cannot print a library procedure." << std::endl;
            return CommandStatus::Failure;
        }

        UserProc* userProc = dynamic_cast<UserProc*>(proc);
        assert(userProc != nullptr);

        QTextStream outStream(stdout);
        userProc->print(outStream);
        outStream << "\n";
        outStream.flush();
    }

    return CommandStatus::Success;
}


CommandStatus Console::handleExit(const QStringList& args)
{
    if (args.size() != 0) {
        std::cerr << "Wrong number of arguments for command; Expected 0, got " << args.size() << "." << std::endl;
        return CommandStatus::ParseError;
    }

    return CommandStatus::ExitProgram;
}


CommandStatus Console::handleHelp(const QStringList& args)
{
    if (args.size() != 0) {
        std::cerr << "Wrong number of arguments for command; Expected 0, got " << args.size() << "." << std::endl;
        return CommandStatus::ParseError;
    }

    // Column 98 of this source file is column 80 of output (don't use tabs)
    //   ____.____1____.____2____.____3____.____4____.____5____.____6____.____7____.____8
    std::cout <<
        "Available commands:\n"
        "  decode <file>                      : Loads and decodes the specified binary.\n"
        "  decompile [proc1 [proc2 [...]]]    : Decompiles the program or specified function(s).\n"
        "  codegen [module1 [module2 [...]]]  : Generates code for the program or a\n"
        "                                       specified module.\n"
        "  print-callgraph                    : prints the call graph of the program.\n"
        "  print-cfg                          : prints the cfg of the program.\n"
        "  print-rtl <proc>                   : Print the RTL for a proc.\n"
        "  replay <file>                      : Reads file and executes commands line by line.\n"
//        "  move proc <proc> <cluster>         : Moves the specified proc to the specified\n"
//        "                                       cluster.\n"
//        "  move cluster <cluster> <parent>    : Moves the specified cluster to the\n"
//        "                                       specified parent cluster.\n"
//         "  add cluster <cluster> [parent]     : Adds a new cluster to the root/specified\n"
//         "                                       cluster.\n"
//         "  delete cluster <cluster>           : Deletes an empty cluster.\n"
//         "  rename proc <proc> <newname>       : Renames the specified proc.\n"
//         "  rename cluster <cluster> <newname> : Renames the specified cluster.\n"
//         "  info prog                          : Print info about the program.\n"
//         "  info cluster <cluster>             : Print info about a cluster.\n"
//         "  info proc <proc>                   : Print info about a proc.\n"
        "  help                               : This help.\n"
        "  exit                               : Quit Boomerang.\n";
    std::cout.flush();
    return CommandStatus::Success;
}
