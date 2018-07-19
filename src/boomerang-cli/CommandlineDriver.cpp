#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CommandlineDriver.h"


#include "boomerang/db/Prog.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/CFGDotWriter.h"

#include <QCoreApplication>
#include <iostream>


Q_DECLARE_METATYPE(Address)


CommandlineDriver::CommandlineDriver(QObject *_parent)
    : QObject(_parent)
    , m_project(new Project())
    , m_debugger(new MiniDebugger())
    , m_kill_timer(this)
{
    this->connect(&m_kill_timer, &QTimer::timeout, this, &CommandlineDriver::onCompilationTimeout);
    m_project->addWatcher(m_debugger.get());
}


/**
 * Prints help about the command line switches.
 */
static void help()
{
    std::cout <<
        "Symbols\n"
        "  -s <addr> <name> : Define a symbol\n"
        "  -sf <filename>   : Read a symbol/signature file\n"
        "Decoding/decompilation options\n"
        "  -e <addr>        : Decode the procedure beginning at addr, and callees\n"
        "  -E <addr>        : Decode the procedure at addr, no callees\n"
        "                     Use -e and -E repeatedly for multiple entry points\n"
        "  -ic              : Decode through type 0 Indirect Calls\n"
        "  -S <min>         : Stop decompilation after specified number of minutes\n"
        "  -t               : Trace (print address of) every instruction decoded\n"
        "  -Tc              : Use old constraint-based type analysis\n"
        "  -Td              : Use data-flow-based type analysis\n"
        "  -a               : Assume ABI compliance\n"
        "Output\n"
        "  -v               : Verbose\n"
        "  -h               : This help\n"
        "  -o <output path> : Where to generate output (defaults to ./output/)\n"
        "  -r               : Print RTL for each proc to log before code generation\n"
        "  -gd <dot file>   : Generate a dotty graph of the program's CFG\n"
        "  -gc              : Generate a call graph to callgraph.dot\n"
        "  -gs              : Generate a symbol file (symbols.h)\n"
        "  -iw              : Write indirect call report to output/indirect.txt\n"
        "Misc.\n"
        "  -i [<file>]      : Interactive mode; execute commands from <file>, if present\n"
        "  -k               : Same as -i, deprecated\n"
        "  -P <path>        : Path to Boomerang files, defaults to where you run\n"
        "                     Boomerang from\n"
        "  -X               : activate eXperimental code; errors likely\n"
        "  --               : No effect (used for testing)\n"
        "Debug\n"
        "  -dc              : Debug switch (Case) analysis\n"
        "  -dd              : Debug decoder to stdout\n"
        "  -dg              : Debug code Generation\n"
        "  -dl              : Debug liveness (from SSA) code\n"
        "  -dp              : Debug proof engine\n"
        "  -ds              : Stop at debug points for keypress\n"
        "  -dt              : Debug type analysis\n"
        "  -du              : Debug removal of unused statements etc\n"
        "Restrictions\n"
        "  -nc              : No decode children in the call graph (callees)\n"
        "  -nd              : No (reduced) dataflow analysis\n"
        "  -nl              : No creation of local variables\n"
        "  -ng              : No replacement of expressions with Globals\n"
        "  -nn              : No removal of nullptr and unused statements\n"
        "  -np              : No replacement of expressions with Parameter names\n"
        "  -nP              : No promotion of signatures (other than main/WinMain/\n"
        "                     DriverMain)\n"
        "  -nr              : No removal of unneeded labels\n"
        "  -nR              : No removal of unused Returns\n"
        "  -l <depth>       : Limit multi-propagations to expressions with depth <depth>\n"
        "  -p <num>         : Only do num propagations\n"
        "  -m <num>         : Max memory depth\n";
}


/**
 * Prints a short usage statement.
 */
static void usage()
{
    std::cout <<
        "Usage: boomerang [ switches ] <program>\n"
        "boomerang -h for switch help\n";
}


int CommandlineDriver::applyCommandline(const QStringList& args)
{
    bool interactiveMode = false;

    if (args.size() < 2) {
        usage();
        return 1;
    }

    if ((args.size() == 2) && (args[1].compare("-h") == 0)) {
        help();
        return 1;
    }

    for (int i = 1; i < args.size(); ++i) {
        QString arg = args[i];

        if (arg[0] != '-') {
            if (i == args.size() - 1) {
                break;
            }

            // every argument but last must begin with '-'
            usage();
            return 1;
        }

        switch (arg[1].toLatin1())
        {
        case 'E':
            m_project->getSettings()->decodeChildren = false;
        // Fall through

        case 'e':
            {
                Address addr;
                m_project->getSettings()->decodeMain = false;

                if (++i == args.size()) {
                    usage();
                    return 1;
                }

                bool converted = false;
                addr = Address(args[i].toLongLong(&converted, 0));

                if (!converted) {
                    LOG_ERROR("Bad address: %1", args[i]);
                    return 2;
                }

                m_project->getSettings()->m_entryPoints.push_back(addr);
            }
            break;

        case 'h':
            help();
            break;

        case 'v':
            m_project->getSettings()->verboseOutput = true;
            break;

        case 'X':
            m_project->getSettings()->experimental = true;
            LOG_WARN("Activating experimental code!");
            break;

        case 'r':
            m_project->getSettings()->printRTLs = true;
            break;

        case 't':
            m_project->getSettings()->traceDecoder = true;
            break;

        case 'T':
            if (arg[2] == 'c') {
                LOG_WARN("Constraint-based type analysis is no longer supported. "
                        "Falling back to Data-Flow based type analysis.");
                m_project->getSettings()->dfaTypeAnalysis = true;
            }
            else if (arg[2] == 'd') {
                m_project->getSettings()->dfaTypeAnalysis = true;
            }

            break;

        case 'g':

            if (arg[2] == 'd') {
                m_project->getSettings()->dotFile = args[++i];
            }
            else if (arg[2] == 'c') {
                m_project->getSettings()->generateCallGraph = true;
            }
            else if (arg[2] == 's') {
                m_project->getSettings()->generateSymbols     = true;
                m_project->getSettings()->stopBeforeDecompile = true;
            }

            break;

        case 'o':
            {
                QString o_path = args[++i];

                if (!o_path.endsWith('/') && !o_path.endsWith('\\')) {
                    o_path += '/'; // Maintain the convention of a trailing slash
                }

                m_project->getSettings()->setOutputDirectory(o_path);
                break;
            }

        case '-':
            break; // No effect: ignored

        case 'i':

            if (arg[2] == 'c') {
                m_project->getSettings()->decodeThruIndCall = true; // -ic;
                break;
            }
            else if (arg.size() > 2) {
                // unknown command
                break;
            }

        /* fallthrough */

        case 'k':
            {
                interactiveMode = true;

                if ((i + 1 < args.size()) && !args[i + 1].startsWith("-")) {
                    m_project->getSettings()->replayFile = args[++i];
                }
            }
            break;

        case 'P':
            {
                QDir wd(args[++i] + "/");

                if (!wd.exists()) {
                    LOG_WARN("Working directory '%1' does not exist!", wd.path());
                }
                else {
                    LOG_MSG("Working directory now '%1'", wd.path());
                }

                m_project->getSettings()->setWorkingDirectory(wd.path());
                m_project->getSettings()->setDataDirectory(wd.path() + "/../share/boomerang/");
                m_project->getSettings()->setPluginDirectory(wd.path() + "/../lib/boomerang/plugins/");
                m_project->getSettings()->setOutputDirectory(wd.path() + "/./output/");
            }
            break;

        case 'n':
            switch (arg[2].toLatin1())
            {
            case 'c':
                m_project->getSettings()->decodeChildren = false;
                break;

            case 'd':
                m_project->getSettings()->useDataflow = false;
                break;

            case 'l':
                m_project->getSettings()->useLocals = false;
                break;

            case 'n':
                m_project->getSettings()->removeNull = false;
                break;

            case 'P':
                m_project->getSettings()->usePromotion = false;
                break;

            case 'p':
                m_project->getSettings()->nameParameters = false;
                break;

            case 'r':
                m_project->getSettings()->removeLabels = false;
                break;

            case 'R':
                m_project->getSettings()->removeReturns = false;
                break;

            case 'g':
                m_project->getSettings()->useGlobals = false;
                break;

            default:
                help();
            }

            break;

        case 'p':
            if (arg[2] == 'a') {
                m_project->getSettings()->propOnlyToAll = true;
                LOG_WARN(" * * Warning! -pa is not implemented yet!");
            }
            else {
                if (++i == args.size()) {
                    usage();
                    return 1;
                }

                m_project->getSettings()->numToPropagate = args[i].toInt();
            }

            break;

        case 's':
            {
                if (arg[2] == 'f') {
                    m_project->getSettings()->m_symbolFiles.push_back(args[i + 1]);
                    i++;
                    break;
                }

                Address addr;

                if (++i == args.size()) {
                    usage();
                    return 1;
                }

                bool converted = false;
                addr = Address(args[i].toLongLong(&converted, 0));

                if (!converted) {
                    LOG_FATAL("Bad address: %1", args[i + 1]);
                }

                m_project->getSettings()->m_symbolMap[addr] = args[++i];
            }
            break;

        case 'd':

            switch (arg[2].toLatin1())
            {
            case 'c':
                m_project->getSettings()->debugSwitch = true;
                break;

            case 'd':
                m_project->getSettings()->debugDecoder = true;
                break;

            case 'g':
                m_project->getSettings()->debugGen = true;
                break;

            case 'l':
                m_project->getSettings()->debugLiveness = true;
                break;

            case 'p':
                m_project->getSettings()->debugProof = true;
                break;

            case 's':
                m_project->getSettings()->stopAtDebugPoints = true;
                break;

            case 't': // debug type analysis
                m_project->getSettings()->debugTA = true;
                break;

            case 'u': // debug unused locations (including returns and parameters now)
                m_project->getSettings()->debugUnused = true;
                break;

            default:
                help();
            }

            break;

        case 'a':
            m_project->getSettings()->assumeABI = true;
            break;

        case 'l':

            if (++i == args.size()) {
                usage();
                return 1;
            }

            m_project->getSettings()->propMaxDepth = args[i].toInt();
            break;

        case 'S':
            minsToStopAfter = args[++i].toInt();
            break;

        default:
            help();
        }
    }

    if (interactiveMode) {
        return interactiveMain();
    }

    if (minsToStopAfter > 0) {
        LOG_MSG("Stopping decompile after %1 minutes", minsToStopAfter);
        m_kill_timer.setSingleShot(true);
        m_kill_timer.start(1000 * 60 * minsToStopAfter);
    }

    m_pathToBinary = args.last();
    return 0;
}


int CommandlineDriver::interactiveMain()
{
    m_project->loadPlugins();
    m_console.reset(new Console(m_project.get()));

    CommandStatus status = m_console->replayFile(m_project->getSettings()->replayFile);

    if (status == CommandStatus::ExitProgram) {
        return 2;
    }

    // now handle user commands
    QTextStream strm(stdin);
    QString     line;

    while (true) {
        std::cout << "boomerang: ";
        std::cout.flush();

        if (strm.atEnd()) {
            return 0;
        }

        line   = strm.readLine();
        status = m_console->handleCommand(line);

        if (status == CommandStatus::ExitProgram) {
            return 2;
        }
    }
}


int CommandlineDriver::decompile()
{
    Log::getOrCreateLog().addDefaultLogSinks(m_project->getSettings()->getOutputDirectory().absolutePath());
    m_project->loadPlugins();

    QDir       wd = m_project->getSettings()->getWorkingDirectory();
    QFileInfo inf = QFileInfo(wd.absoluteFilePath(m_pathToBinary));

    return decompile(inf.absoluteFilePath(), inf.baseName());
}


void CommandlineDriver::onCompilationTimeout()
{
    LOG_WARN("Compilation timed out, Boomerang will now exit");
    exit(1);
}


bool CommandlineDriver::loadAndDecode(const QString& fname, const QString& pname)
{
    assert(m_project);

    const bool ok = m_project->loadBinaryFile(fname);
    if (!ok) {
        LOG_ERROR("Loading '%1' failed.", fname);
        return false;
    }

    Prog *prog = m_project->getProg();
    assert(prog);

    prog->setName(pname);
    return m_project->decodeBinaryFile();
}


int CommandlineDriver::decompile(const QString& fname, const QString& pname)
{
    time_t start;
    time(&start);

    if (!loadAndDecode(fname, pname)) {
        return 1;
    }


    if (m_project->getSettings()->stopBeforeDecompile) {
        return 0;
    }

    LOG_MSG("Decompiling...");
    m_project->decompileBinaryFile();

    if (!m_project->getSettings()->dotFile.isEmpty()) {
        CfgDotWriter().writeCFG(m_project->getProg(), m_project->getSettings()->dotFile);
    }

    m_project->generateCode();

    QDir outDir = m_project->getSettings()->getOutputDirectory();
    LOG_MSG("Output written to '%1'", outDir.absolutePath());

    time_t end;
    time(&end);
    int hours = static_cast<int>((end - start) / 60 / 60);
    int mins  = static_cast<int>((end - start) / 60 - hours * 60);
    int secs  = static_cast<int>((end - start) - (hours * 60 * 60) - (mins * 60));

    LOG_MSG("Completed in %1 hours %2 minutes %3 seconds.", hours, mins, secs);
    return 0;
}
