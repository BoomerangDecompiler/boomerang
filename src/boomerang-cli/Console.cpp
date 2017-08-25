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
    m_commandTypes["print"]     = CT_print;
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

    while (!ist.atEnd()) {
        line = ist.readLine();
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
    int lastSeparator = i; // position of last space ' ' not within quotation marks
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

        const bool argIsQuoted  = (command[lastSeparator+1] == '\"'); // Were we in a quotation before?
        const int posBegin = lastSeparator + (argIsQuoted ? 2 : 1);
        const int posEnd   = i             - (argIsQuoted ? 2 : 1);
        const QString arg = command.mid(posBegin, posEnd - posBegin +1);

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
    case CT_replay:    return handleReplay(args);
    case CT_move:      return handleMove(args);
    case CT_add:       return handleAdd(args);
    case CT_delete:    return handleDelete(args);
    case CT_rename:    return handleRename(args);
    case CT_info:      return handleInfo(args);
    case CT_print:     return handlePrint(args);
    case CT_exit:      return handleExit(args);
    case CT_help:      return handleHelp(args);

    default:
        std::cerr << "Unrecognized command '" << command.toStdString() << "', try 'help'" << std::endl;
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

    Boomerang::get()->getOrCreateProject();
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


CommandStatus Console::handleReplay(const QStringList& args)
{
    if (args.size() != 1) {
        std::cerr << "Wrong number of arguments for command; Expected 1, got " << args.size() << "." << std::endl;
        return CommandStatus::ParseError;
    }

    return replayFile(args[0]);
}


CommandStatus Console::handleMove(const QStringList& args)
{
    if (args.size() < 2) {
        std::cerr << "Not enough arguments for cmd." << std::endl;
        return CommandStatus::ParseError;
    }
    else if (prog == nullptr) {
        std::cerr << "No valid Prog object!" << std::endl;
        return CommandStatus::Failure;
    }

    if (args[0] == "proc") {
        if (args.size() < 3) {
            std::cerr << "Not enough arguments for cmd" << std::endl;
            return CommandStatus::ParseError;
        }

        Function *proc = prog->findProc(args[1]);

        if (proc == nullptr) {
            std::cerr << "Cannot find proc " << args[1].toStdString() << std::endl;
            return CommandStatus::Failure;
        }

        Module *module = prog->findModule(args[2]);

        if (module == nullptr) {
            std::cerr << "Cannot find module " << args[2].toStdString() << std::endl;
            return CommandStatus::Failure;
        }

        proc->setParent(module);
    }
    else if (args[0] == "module") {
        if (args.size() < 3) {
            std::cerr << "Not enough arguments for cmd" << std::endl;
            return CommandStatus::ParseError;
        }

        Module *module = prog->findModule(args[1]);

        if (module == nullptr) {
            std::cerr << "Cannot find module " << args[1].toStdString() << std::endl;
            return CommandStatus::Failure;
        }

        Module *parentModule = prog->findModule(args[2]);

        if (parentModule == nullptr) {
            std::cerr << "Cannot find module " << args[2].toStdString() << std::endl;
            return CommandStatus::Failure;
        }

        parentModule->addChild(module);
    }
    else {
        std::cerr << "Unknown argument " << args[0].toStdString() << " for command 'move'." << std::endl;
        return CommandStatus::ParseError;
    }

    return CommandStatus::Success;
}


CommandStatus Console::handleAdd(const QStringList& args)
{
    if (args.empty()) {
        std::cerr << "Not enough arguments for command." << std::endl;
        return CommandStatus::ParseError;
    }
    else if (prog == nullptr) {
        std::cerr << "No valid Prog object!" << std::endl;
        return CommandStatus::Failure;
    }

    if (args[0] == "module") {
        if (args.size() < 2) {
            std::cerr << "Not enough arguments for command." << std::endl;
            return CommandStatus::ParseError;
        }

        Module* parent = (args.size() > 2) ? prog->findModule(args[2]) : prog->getRootModule();
        if (!parent) {
            std::cerr << "Cannot find parent module." << std::endl;
            return CommandStatus::Failure;
        }
        else {
            for (size_t i = 0; i < parent->getNumChildren(); i++) {
                Module* existingChild = parent->getChild(i);
                assert(existingChild);

                if (existingChild->getName() == args[1]) {
                    // new module would be a sibling of an existing module with the same name
                    std::cerr << "Cannot create module: A module of the same name already exists." << std::endl;
                    return CommandStatus::Failure;
                }
            }
        }

        Module* module = prog->createModule(args[1], parent);

        if (module == nullptr) {
            std::cerr << "Cannot create module " << args[1].toStdString() << std::endl;
            return CommandStatus::Failure;
        }

        return CommandStatus::Success;
    }
    else {
        std::cerr << "Unknown argument " << args[0].toStdString() << " for command 'add'" << std::endl;
        return CommandStatus::ParseError;
    }
}


CommandStatus Console::handleDelete(const QStringList& args)
{
    if (args.empty()) {
        std::cerr << "Not enough arguments for cmd\n";
        return CommandStatus::ParseError;
    }
    else if (prog == nullptr) {
        std::cerr << "no valid Prog object!" << std::endl;
    }

    if (args[0] == "module") {
        if (args.size() < 2) {
            std::cerr << "Not enough arguments for cmd" << std::endl;
            return CommandStatus::ParseError;
        }

        for (int i = 1; i < args.size(); i++) {
            Module *module = prog->findModule(args[i]);

            if (module == nullptr) {
                std::cerr << "Cannot find module " << args[i].toStdString() << std::endl;
                return CommandStatus::Failure;
            }
            else if (module == prog->getRootModule()) {
                std::cerr << "Cannot remove root module." << std::endl;
                return CommandStatus::Failure;
            }
            else if (module->hasChildren() || prog->isModuleUsed(module)) {
                std::cerr << "Cannot remove module: Module is not empty." << std::endl;
                return CommandStatus::Failure;
            }

            QFile::remove(module->getOutPath("xml"));
            QFile::remove(module->getOutPath("c"));
            assert(module->getUpstream());
            module->getUpstream()->removeChild(module);
        }
        return CommandStatus::Success;
    }
    else {
        std::cerr << "Unknown argument for command 'delete'" << std::endl;
        return CommandStatus::ParseError;
    }
}


CommandStatus Console::handleRename(const QStringList& args)
{
    if (args.empty()) {
        std::cerr << "Not enough arguments for cmd" << std::endl;
        return CommandStatus::ParseError;
    }
    else if (prog == nullptr) {
        std::cerr << "No valid Prog object!" << std::endl;
        return CommandStatus::Failure;
    }

    if (args[0] == "proc") {
        if (args.size() < 3) {
            std::cerr  << "Not enough arguments for cmd" << std::endl;
            return CommandStatus::Failure;
        }

        Function *proc = prog->findProc(args[1]);

        if (proc == nullptr) {
            std::cerr << "Cannot find proc " << args[1].toStdString() << std::endl;
            return CommandStatus::Failure;
        }

        Function *nproc = prog->findProc(args[2]);

        if (nproc != nullptr) {
            std::cerr << "Proc " << args[2].toStdString() << " already exists" << std::endl;
            return CommandStatus::Failure;
        }

        proc->setName(args[2]);
        return CommandStatus::Success;
    }
    else if (args[0] == "module") {
        if (args.size() < 3) {
            std::cerr << "Not enough arguments for cmd" << std::endl;
            return CommandStatus::ParseError;
        }

        Module *module = prog->findModule(args[1]);

        if (module == nullptr) {
            std::cerr << "Cannot find module " << args[1].toStdString() << std::endl;
            return CommandStatus::Failure;
        }

        Module *newModule = prog->findModule(args[2]);

        if (newModule != nullptr) {
            std::cerr << "Module " << args[2].toStdString() << "already exists" << std::endl;
            return CommandStatus::Failure;
        }

        module->setName(args[2]);
        return CommandStatus::Success;
    }
    else {
        std::cerr << "Unknown argument '" << args[0].toStdString() << "' for command 'rename'" << std::endl;
        return CommandStatus::ParseError;
    }
}


CommandStatus Console::handleInfo(const QStringList& args)
{
    if (args.empty()) {
        std::cerr << "Not enough arguments for cmd!" << std::endl;
        return CommandStatus::ParseError;
    }
    else if (prog == nullptr) {
        std::cerr << "No valid Prog object!" << std::endl;
        return CommandStatus::Failure;
    }

    if (args[0] == "prog") {
        QTextStream ost(stdout);
        ost << "Program " << prog->getName() << ":\n";
        ost << "\tModules:\n";
        prog->getRootModule()->printTree(ost);


        std::list<const Function*> libFunctions;
        std::list<const Function*> userFunctions;

        const Prog::ModuleList& modules = prog->getModuleList();
        for (const Module* module : modules) {
            for (const Function* function : *module) {
                if (function->isLib()) {
                    libFunctions.push_back(function);
                }
                else {
                    userFunctions.push_back(function);
                }
            }
        }

        ost << "\n\tLibrary functions:\n";
        for (const Function* function : libFunctions) {
            ost << "\t\t" << function->getParent()->getName() << "::" << function->getName() << "\n";
        }

        ost << "\n\tUser functions:\n";
        for (const Function* function : userFunctions) {
            ost <<  "\t\t" << function->getParent()->getName() << "::" << function->getName() << "\n";
        }

        ost << "\n";
        ost.flush();
        return CommandStatus::Success;
    }
    else if (args[0] == "module") {
        if (args.size() < 2) {
            std::cerr << "Not enough arguments for cmd";
            return CommandStatus::Failure;
        }

        Module *module = prog->findModule(args[1]);
        if (module == nullptr) {
            std::cerr << "Cannot find module " << args[1].toStdString() << std::endl;
            return CommandStatus::Failure;
        }

        QTextStream outStream(stdout);
        outStream << "module " << module->getName() << ":\n";

        if (module->getUpstream()) {
            outStream << "\tparent = " << module->getUpstream()->getName() << "\n";
        }
        else {
            outStream << "\troot module.\n";
        }

        outStream << "\tprocs:\n";

        for (Function *f : *module) {
            outStream << "\t\t" << f->getName() << "\n";
        }

        outStream << "\n";

        return CommandStatus::Success;
    }
    else if (args[0] == "proc") {
        if (args.size() < 2) {
            std::cerr << "Not enough arguments for cmd" << std::endl;
            return CommandStatus::ParseError;
        }

        Function *proc = prog->findProc(args[1]);
        if (proc == nullptr) {
            std::cerr << "Cannot find proc " << args[1].toStdString() << std::endl;
            return CommandStatus::Failure;
        }

        QTextStream outStream(stdout);
        outStream << "proc " << proc->getName() << ":\n";
        outStream << "\tbelongs to module " << proc->getParent()->getName() << "\n";
        outStream << "\tnative address " << proc->getEntryAddress() << "\n";

        if (proc->isLib()) {
            outStream << "\tis a library proc.\n";
        }
        else {
            outStream << "\tis a user proc.\n";
            UserProc *p = (UserProc *)proc;

            if (p->isDecoded()) {
                outStream << "\thas been decoded.\n";
            }

            // if (p->isAnalysed())
            //    out_stream << "\thas been analysed.\n";
        }

        outStream << "\n";

        return CommandStatus::Success;
    }
    else {
        std::cerr << "Unknown argument " << args[0].toStdString() << " for command 'info'" << std::endl;
        return CommandStatus::Failure;
    }
}


CommandStatus Console::handlePrint(const QStringList& args)
{
    if (args.empty()) {
        std::cerr << "Not enough arguments for cmd" << std::endl;
        return CommandStatus::ParseError;
    }
    else if (prog == nullptr) {
        std::cerr << "No valid Prog object!" << std::endl;
        return CommandStatus::Failure;
    }

    if (args[0] == "rtl") {
        if (args.size() < 2) {
            std::cerr << "Too few arguments for command 'print rtl'" << std::endl;
            return CommandStatus::Failure;
        }

        for (int i = 1; i < args.size(); i++) {
            Function *proc = prog->findProc(args[i]);

            if (proc == nullptr) {
                std::cerr << "Cannot find procedure " << args[i].toStdString() << std::endl;
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
    else if (args[0] == "callgraph") {
        if (args.size() > 1) {
            std::cerr << "Wrong number of arguments for command; Expected 1, got " << args.size() << "." << std::endl;
            return CommandStatus::ParseError;
        }

        prog->printCallGraph();
        return CommandStatus::Success;
    }
    else if (args[0] == "cfg") {
        if (args.size() == 1) {
            prog->generateDotFile();
            return CommandStatus::Success;
        }
        else {
            ProcSet procs;
            for (int i = 1; i < args.size(); i++) {
                Function* proc = prog->findProc(args[i]);
                if (!proc) {
                    std::cerr << "Procedure '" << args[i].toStdString() << "' not found.";
                    return CommandStatus::Failure;
                }
                else if (proc->isLib()) {
                    std::cerr << "Cannot print library procedure '" << args[i].toStdString() << "'.";
                    return CommandStatus::Failure;
                }

                UserProc* userProc = dynamic_cast<UserProc*>(proc);
                assert(userProc);
                procs.insert(userProc);
            }

            QFile outFile(QString("cfg.dot"));
            outFile.open(QFile::WriteOnly | QFile::Text);
            QTextStream textStream(&outFile);
            textStream << "digraph cfg {\n";

            for (UserProc* userProc : procs) {
                textStream << "subgraph " << userProc->getName() << " {\n";
                userProc->getCFG()->generateDotFile(textStream);
            }
            textStream << "}";

            return CommandStatus::Success;
        }
    }
    else {
        std::cerr << "Unknown argument " << args[1].toStdString() << " for command 'print'" << std::endl;
        return CommandStatus::ParseError;
    }
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
        "  info prog                          : Print info about the program.\n"
        "  info module <module>               : Print info about a module.\n"
        "  info proc <proc>                   : Print info about a proc.\n"
        "  move proc <proc> <module>          : Moves the specified proc to the specified\n"
        "                                       module.\n"
        "  move module <module> <parent>      : Moves the specified module to the\n"
        "                                       specified parent module.\n"
        "  add module <module> [<parent>]     : Adds a new module to the root/specified\n"
        "                                       module.\n"
        "  delete module <module> [...]       : Deletes empty modules.\n"
        "  rename proc <proc> <newname>       : Renames the specified proc.\n"
        "  rename module <module> <newname>   : Renames the specified module.\n"
        "  print callgraph                    : prints the call graph of the program.\n"
        "  print cfg [<proc1> [proc2 [...]]]  : prints the Control Flow Graph of the program\n"
        "                                       or a set of procedures.\n"
        "  print rtl [<proc1> [proc2 [...]]]  : Print the RTL for a proc.\n"
        "  replay <file>                      : Reads file and executes commands line by line.\n"

        "  help                               : This help.\n"
        "  exit                               : Quit Boomerang.\n";
    std::cout.flush();
    return CommandStatus::Success;
}
