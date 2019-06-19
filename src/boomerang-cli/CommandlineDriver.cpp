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

#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/util/CFGDotWriter.h"
#include "boomerang/util/log/Log.h"

#include <QCoreApplication>
#include <QTextStream>

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
    // clang-format off
    std::cout <<
"Usage:\n"
"  boomerang-cli [ switches ] [ -- ] program\n"
"  boomerang-cli -i [ command_file ]\n"
"  boomerang-cli ( -h | --help | --version )\n"
"\n"
"\n"
"Symbols\n"
"  -s <addr> <name> : Define a symbol\n"
"  -sf <filename>   : Read a symbol/signature file\n"
"\n"
"Decoding/decompilation options\n"
"  --decode-only    : Decode only, do not decompile\n"
"  --ssl <file>     : Use <file> as SSL specification file\n"
"  -e <addr>        : Decode or decompile the procedure beginning at addr, and callees\n"
"  -E <addr>        : Equivalent to -nc -e <addr>\n"
"  -ic              : Decode through type 0 Indirect Calls\n"
"  -S <min>         : Stop decompilation after specified number of minutes\n"
"  -t               : Trace (print address of) every instruction decoded\n"
"  -a               : Assume ABI compliance\n"
"\n"
"Output\n"
"  --version        : Print version information and exit\n"
"  -h, --help       : Show this help and exit\n"
"  -v               : Verbose decompilation output\n"
"  -o <output_path> : Where to generate output (defaults to ./output/)\n"
"  -r               : Print RTL for each proc to log before code generation\n"
"  -gd <dot_file>   : Generate a dotty graph of the program's CFG(s)\n"
"  -gc              : Generate a call graph to callgraph.dot\n"
"  -gs              : Generate a symbol file (symbols.h). Implies --decode-only.\n"
"\n"
"Misc.\n"
"  -i [<file>]      : Interactive mode; execute commands from <file>, if present\n"
"  -P <path>        : Path to Boomerang files, defaults to the path to the Boomerang executable\n"
"  -X               : activate eXperimental code; errors likely\n"
"  --               : Terminates argument processing\n"
"\n"
"Debug\n"
"  -dc              : Debug Switch/Case Analysis\n"
"  -dd              : Debug Instruction Decoder\n"
"  -dg              : Debug Dode Generation\n"
"  -dl              : Debug SSA Liveness Analysis\n"
"  -dp              : Debug Proof Engine\n"
"  -ds              : Stop at debug points for keypress\n"
"  -dt              : Debug Type Analysis\n"
"  -du              : Debug removal of unused statements etc.\n"
"\n"
"Restrictions\n"
"  -nc              : Do not decode callees of functions\n"
"  -nd              : No (reduced) Dataflow Analysis\n"
"  -ng              : Do not create global variables from expressions\n"
"  -nl              : Do not create local variables\n"
"  -nn              : Do not remove unused or tautological statements\n"
"  -np              : Do not replace expressions with Parameter names\n"
"  -nP              : No promotion of signatures (other than main/WinMain/DriverMain)\n"
"  -nr              : Do not remove unneeded labels\n"
"  -nR              : Do not remove unused return values\n"
"  -nT              : No Type Analysis\n"
"  -l <depth>       : Limit multi-propagations to expressions with depth <depth>\n"
"  -p <num>         : Only do <num> propagations\n";
    // clang-format on
}


int CommandlineDriver::applyCommandline(const QStringList &args)
{
    bool interactiveMode = false;

    if (args.size() < 2) {
        help();
        return 1;
    }

    QString binaryPath;

    for (int i = 1; i < args.size(); ++i) {
        const QString arg = args[i];

        if (!arg.startsWith('-') || arg == "-") {
            // every argument but last must begin with '-'
            if (i + 1 == args.size()) {
                binaryPath = arg;
                break;
            }

            help();
            return 1;
        }

        if (arg == "-h" || arg == "--help") {
            help();
            return 2;
        }
        else if (arg == "--version") {
            std::cout << "boomerang-cli " << m_project->getVersionStr() << std::endl;
            return 2;
        }
        else if (arg == "-v") {
            m_project->getSettings()->verboseOutput = true;
            continue;
        }
        else if (arg == "-X") {
            m_project->getSettings()->experimental = true;
            continue;
        }
        else if (arg == "-r") {
            m_project->getSettings()->printRTLs = true;
            continue;
        }
        else if (arg == "-t") {
            m_project->getSettings()->traceDecoder = true;
            continue;
        }
        else if (arg == "-gd") {
            if (++i == args.size()) {
                help();
                return 1;
            }

            m_project->getSettings()->dotFile = args[i];
            continue;
        }
        else if (arg == "-gc") {
            m_project->getSettings()->generateCallGraph = true;
            continue;
        }
        else if (arg == "-gs") {
            m_project->getSettings()->generateSymbols     = true;
            m_project->getSettings()->stopBeforeDecompile = true;
            continue;
        }
        else if (arg == "--decode-only") {
            m_project->getSettings()->stopBeforeDecompile = true;
            continue;
        }
        else if (arg == "--ssl") {
            if (++i == args.size()) {
                help();
                return 1;
            }

            m_project->getSettings()->sslFileName = args[i];
            continue;
        }
        else if (arg == "-o") {
            if (++i == args.size()) {
                help();
                return 1;
            }

            QString outPath = args[i];

            if (!outPath.endsWith('/') && !outPath.endsWith('\\')) {
                outPath += '/'; // Maintain the convention of a trailing slash
            }

            m_project->getSettings()->setOutputDirectory(outPath);
            continue;
        }
        else if (arg == "-P") {
            QDir wd(args[++i] + "/");

            if (!wd.exists()) {
                std::cerr << "Working directory '" << wd.path().toStdString()
                          << "' does not exist!";
            }

            m_project->getSettings()->setWorkingDirectory(wd.path());
            m_project->getSettings()->setDataDirectory(wd.path() + "/../share/boomerang/");
            m_project->getSettings()->setPluginDirectory(wd.path() + "/../lib/boomerang/plugins/");
            m_project->getSettings()->setOutputDirectory(wd.path() + "/./output/");
            continue;
        }
        else if (arg == "-ic") {
            m_project->getSettings()->decodeThruIndCall = true;
            continue;
        }
        else if (arg == "-i") {
            interactiveMode = true;

            if (i + 2 != args.size() && i + 1 != args.size()) {
                help();
                return 1;
            }
            else if (i + 2 == args.size()) {
                m_project->getSettings()->replayFile = args[++i];
            }

            break;
        }
        else if (arg == "-nc") {
            m_project->getSettings()->decodeChildren = false;
            continue;
        }
        else if (arg == "-nd") {
            m_project->getSettings()->useDataflow = false;
            continue;
        }
        else if (arg == "-ng") {
            m_project->getSettings()->useGlobals = false;
            continue;
        }
        else if (arg == "-nl") {
            m_project->getSettings()->useLocals = false;
            continue;
        }
        else if (arg == "-nn") {
            m_project->getSettings()->removeNull = false;
            continue;
        }
        else if (arg == "-np") {
            m_project->getSettings()->nameParameters = false;
            continue;
        }
        else if (arg == "-nP") {
            m_project->getSettings()->usePromotion = false;
            continue;
        }
        else if (arg == "-nr") {
            m_project->getSettings()->removeLabels = false;
            continue;
        }
        else if (arg == "-nR") {
            m_project->getSettings()->removeReturns = false;
            continue;
        }
        else if (arg == "-nT") {
            m_project->getSettings()->useTypeAnalysis = false;
            continue;
        }
        else if (arg == "-pa") {
            m_project->getSettings()->propOnlyToAll = true;
            std::cerr << "Warning! -pa is not implemented yet!" << std::endl;
            continue;
        }
        else if (arg == "-p") {
            if (++i == args.size()) {
                help();
                return 1;
            }

            bool converted                           = false;
            m_project->getSettings()->numToPropagate = args[i].toInt(&converted, 0);
            if (!converted) {
                std::cerr << "'-p': Bad argument '" << args[i].toStdString() << "' (try --help)"
                          << std::endl;
                return 1;
            }

            continue;
        }
        else if (arg == "-sf") {
            if (++i == args.size()) {
                help();
                return 1;
            }

            m_project->getSettings()->m_symbolFiles.push_back(args[i]);
            continue;
        }
        else if (arg == "-s") {
            if (++i == args.size()) {
                help();
                return 1;
            }

            const QString addrStr = args[i];
            if (++i == args.size()) {
                help();
                return 1;
            }

            bool converted     = false;
            const Address addr = Address(addrStr.toLongLong(&converted, 0));

            if (!converted) {
                std::cerr << "'-s': Bad argument '" << addrStr.toStdString() << "' (try --help)"
                          << std::endl;
                return 1;
            }

            m_project->getSettings()->m_symbolMap[addr] = args[i];
            continue;
        }
        else if (arg == "-dc") {
            m_project->getSettings()->debugSwitch = true;
            continue;
        }
        else if (arg == "-dd") {
            m_project->getSettings()->debugDecoder = true;
            continue;
        }
        else if (arg == "-dg") {
            m_project->getSettings()->debugGen = true;
            continue;
        }
        else if (arg == "-dl") {
            m_project->getSettings()->debugLiveness = true;
            continue;
        }
        else if (arg == "-dp") {
            m_project->getSettings()->debugProof = true;
            continue;
        }
        else if (arg == "-ds") {
            m_project->getSettings()->stopAtDebugPoints = true;
            continue;
        }
        else if (arg == "-dt") {
            m_project->getSettings()->debugTA = true;
            continue;
        }
        else if (arg == "-du") {
            m_project->getSettings()->debugUnused = true;
            continue;
        }
        else if (arg == "-a") {
            m_project->getSettings()->assumeABI = true;
            continue;
        }
        else if (arg == "-l") {
            if (++i == args.size()) {
                help();
                return 1;
            }

            bool converted                         = false;
            m_project->getSettings()->propMaxDepth = args[i].toInt(&converted, 0);

            if (!converted) {
                std::cerr << "'-l': Bad argument '" << args[i].toStdString() << "' (try --help)"
                          << std::endl;
                return 1;
            }

            continue;
        }
        else if (arg == "-S") {
            if (++i == args.size()) {
                help();
                return 1;
            }

            bool converted  = false;
            minsToStopAfter = args[i].toInt(&converted, 0);

            if (!converted) {
                std::cerr << "'-S': Bad argument '" << args[i].toStdString() << "' (try --help)"
                          << std::endl;
                return 1;
            }

            continue;
        }
        else if (arg == "--") {
            if (i + 2 != args.size()) {
                help();
                return 1;
            }

            binaryPath = args[i + 1];
            break;
        }
        else if (arg.size() == 2) {
            switch (arg[1].toLatin1()) {
            case 'E':
                m_project->getSettings()->decodeChildren = false;
                // Fall through

            case 'e': {
                Address addr;
                m_project->getSettings()->decodeMain = false;

                if (++i == args.size()) {
                    help();
                    return 1;
                }

                bool converted = false;
                addr           = Address(args[i].toLongLong(&converted, 0));

                if (!converted) {
                    std::cerr << "'" << arg.toStdString()
                              << "' Bad address: " << args[i].toStdString() << " (try --help)";
                    return 1;
                }

                m_project->getSettings()->m_entryPoints.push_back(addr);
                continue;
            }
            default: help(); return 1;
            }
        }


        if (i + 1 != args.size()) {
            // switch not recognized
            help();
            return false;
        }
        else {
            binaryPath = arg;
        }
    }

    if (interactiveMode) {
        return interactiveMain();
    }
    else if (binaryPath == "") {
        help();
        return 1;
    }

    if (minsToStopAfter > 0) {
        LOG_MSG("Stopping decompile after %1 minutes", minsToStopAfter);
        m_kill_timer.setSingleShot(true);
        m_kill_timer.start(1000 * 60 * minsToStopAfter);
    }

    m_pathToBinary = binaryPath;
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
    QString line;

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
    Log::getOrCreateLog().addDefaultLogSinks(
        m_project->getSettings()->getOutputDirectory().absolutePath());
    m_project->loadPlugins();

    QDir wd       = m_project->getSettings()->getWorkingDirectory();
    QFileInfo inf = QFileInfo(wd.absoluteFilePath(m_pathToBinary));

    return decompile(inf.absoluteFilePath(), inf.baseName());
}


void CommandlineDriver::onCompilationTimeout()
{
    LOG_WARN("Compilation timed out, Boomerang will now exit");
    exit(1);
}


bool CommandlineDriver::loadAndDecode(const QString &fname, const QString &pname)
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


int CommandlineDriver::decompile(const QString &fname, const QString &pname)
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
        CFGDotWriter().writeCFG(m_project->getProg(), m_project->getSettings()->dotFile);
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
