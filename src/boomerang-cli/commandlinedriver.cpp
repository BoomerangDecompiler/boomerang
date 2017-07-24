#include <QtCore>
#include <cstdio>

#include "commandlinedriver.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Log.h"


CommandlineDriver::CommandlineDriver(QObject *parent)
    : QObject(parent)
    , m_kill_timer(this)
{
    this->connect(&m_kill_timer, &QTimer::timeout, this, &CommandlineDriver::onCompilationTimeout);
    QCoreApplication::instance()->connect(&m_thread, &DecompilationThread::finished,
                                          []() {
        QCoreApplication::instance()->quit();
    });
}


/**
 * Prints help about the command line switches.
 */
static void help()
{
    QTextStream q_cout(stdout);

    q_cout <<
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
        "  -LD              : Load before decompile (<program> becomes xml input file)\n"
        "  -SD              : Save before decompile\n"
        "  -a               : Assume ABI compliance\n"
        "  -W               : Windows specific decompilation mode (requires pdb information)\n"
//        "  -pa              : only propagate if can propagate to all\n"
        "Output\n"
        "  -v               : Verbose\n"
        "  -h               : This help\n"
        "  -o <output path> : Where to generate output (defaults to ./output/)\n"
        "  -x               : Dump XML files\n"
        "  -r               : Print RTL for each proc to log before code generation\n"
        "  -gd <dot file>   : Generate a dotty graph of the program's CFG and DFG\n"
        "  -gc              : Generate a call graph (callgraph.out and callgraph.dot)\n"
        "  -gs              : Generate a symbol file (symbols.h)\n"
        "  -iw              : Write indirect call report to output/indirect.txt\n"
        "Misc.\n"
        "  -k               : Command mode, for available commands see -h cmd\n"
        "  -P <path>        : Path to Boomerang files, defaults to where you run\n"
        "                     Boomerang from\n"
        "  -X               : activate eXperimental code; errors likely\n"
        "  --               : No effect (used for testing)\n"
        "Debug\n"
        "  -da              : Print AST before code generation\n"
        "  -dc              : Debug switch (Case) analysis\n"
        "  -dd              : Debug decoder to stdout\n"
        "  -dg              : Debug code Generation\n"
        "  -dl              : Debug liveness (from SSA) code\n"
        "  -dp              : Debug proof engine\n"
        "  -ds              : Stop at debug points for keypress\n"
        "  -dt              : Debug type analysis\n"
        "  -du              : Debug removing unused statements etc\n"
        "Restrictions\n"
        "  -nb              : No simplifications for branches\n"
        "  -nc              : No decode children in the call graph (callees)\n"
        "  -nd              : No (reduced) dataflow analysis\n"
        "  -nD              : No decompilation (at all!)\n"
        "  -nl              : No creation of local variables\n"
//        "  -nm              : No decoding of the 'main' procedure\n"
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
    QTextStream q_cout(stdout);

    q_cout << "Usage: boomerang [ switches ] <program>" << '\n';
    q_cout << "boomerang -h for switch help" << '\n';
}


int CommandlineDriver::applyCommandline(const QStringList& args)
{
    printf("Boomerang %s\n", BOOMERANG_VERSION); // Display a version and date (mainly for release versions)
    int kmd = 0;

    if (args.size() < 2) {
        usage();
        return 1;
    }

    if ((args.size() == 2) && (args[1].compare("-h") == 0)) {
        help();
        return 1;
    }

    Boomerang& boom(*Boomerang::get());
    boom.setWorkingDirectory(QFileInfo(args[0]).absolutePath());
    boom.setDataDirectory(qApp->applicationDirPath() + "/../lib/boomerang/");

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
            boom.noDecodeChildren = true;
            // Fall through

        case 'e':
            {
                Address addr;
                boom.decodeMain = false;

                if (++i == args.size()) {
                    usage();
                    return 1;
                }

                bool converted = false;
                addr = Address(args[i].toLongLong(&converted, 0));

                if (!converted) {
                    LOG_STREAM() << "bad address: " << args[i] << '\n';
                    exit(1);
                }

                boom.m_entryPoints.push_back(addr);
            }
            break;

        case 'h':
            help();
            break;

        case 'v':
            boom.vFlag = true;
            break;

        case 'x':
            boom.dumpXML = true;
            break;

        case 'X':
            boom.experimental = true;
            LOG_STREAM(LogLevel::LL_Warn) << "Warning: experimental code active!\n";
            break;

        case 'r':
            boom.printRtl = true;
            break;

        case 't':
            boom.traceDecoder = true;
            break;

        case 'T':

            if (arg[2] == 'c') {
                boom.conTypeAnalysis = true; // -Tc: use old constraint-based type analysis
                boom.dfaTypeAnalysis = false;
            }
            else if (arg[2] == 'd') {
                boom.dfaTypeAnalysis = true; // -Td: use data-flow-based type analysis (now default)
            }

            break;

        case 'g':

            if (arg[2] == 'd') {
                boom.dotFile = args[++i];
            }
            else if (arg[2] == 'c') {
                boom.generateCallGraph = true;
            }
            else if (arg[2] == 's') {
                boom.generateSymbols     = true;
                boom.stopBeforeDecompile = true;
            }

            break;

        case 'o':
            {
                QString o_path = args[++i];

                if (!o_path.endsWith('/') && !o_path.endsWith('\\')) {
                    o_path += '/'; // Maintain the convention of a trailing slash
                }

                boom.setOutputDirectory(o_path);
                break;
            }

        case 'i':

            if (arg[2] == 'c') {
                boom.decodeThruIndCall = true; // -ic;
            }

            break;

        case '-':
            break; // No effect: ignored

        case 'L':

            if (arg[2] == 'D') {
                boom.loadBeforeDecompile = true;
            }

            break;

        case 'k':
            kmd = 1;
            break;

        case 'P':
            {
                QString   qstr(args[++i] + "/");
                QFileInfo qfi(qstr);
                boom.setWorkingDirectory(qfi.path());
            }
            break;

        case 'n':

            switch (arg[2].toLatin1())
            {
            case 'b':
                boom.noBranchSimplify = true;
                break;

            case 'c':
                boom.noDecodeChildren = true;
                break;

            case 'd':
                boom.noDataflow = true;
                break;

            case 'D':
                boom.noDecompile = true;
                break;

            case 'l':
                boom.noLocals = true;
                break;

            case 'n':
                boom.noRemoveNull = true;
                break;

            case 'P':
                boom.noPromote = true;
                break;

            case 'p':
                boom.noParameterNames = true;
                break;

            case 'r':
                boom.noRemoveLabels = true;
                break;

            case 'R':
                boom.noRemoveReturns = true;
                break;

            case 'g':
                boom.noGlobals = true;
                break;

            default:
                help();
            }

            break;

        case 'p':

            if (arg[2] == 'a') {
                boom.propOnlyToAll = true;
                LOG_STREAM() << " * * Warning! -pa is not implemented yet!\n";
            }
            else {
                if (++i == args.size()) {
                    usage();
                    return 1;
                }

                boom.numToPropagate = args[i].toInt();
            }

            break;

        case 's':
            {
                if (arg[2] == 'f') {
                    boom.m_symbolFiles.push_back(args[i + 1]);
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
                    LOG_STREAM() << "bad address: " << args[i + 1] << '\n';
                    exit(1);
                }

                boom.symbols[addr] = args[++i];
            }
            break;

        case 'd':

            switch (arg[2].toLatin1())
            {
            case 'a':
                boom.printAST = true;
                break;

            case 'c':
                boom.debugSwitch = true;
                break;

            case 'd':
                boom.debugDecoder = true;
                break;

            case 'g':
                boom.debugGen = true;
                break;

            case 'l':
                boom.debugLiveness = true;
                break;

            case 'p':
                boom.debugProof = true;
                break;

            case 's':
                boom.stopAtDebugPoints = true;
                break;

            case 't': // debug type analysis
                boom.debugTA = true;
                break;

            case 'u': // debug unused locations (including returns and parameters now)
                boom.debugUnused = true;
                break;

            default:
                help();
            }

            break;

        case 'm':

            if (++i == args.size()) {
                usage();
                return 1;
            }

            boom.maxMemDepth = args[i].toInt();
            break;

        case 'a':
            boom.assumeABI = true;
            break;

        case 'l':

            if (++i == args.size()) {
                usage();
                return 1;
            }

            boom.propMaxDepth = args[i].toInt();
            break;

        case 'S':

            if (arg[2] == 'D') {
                boom.saveBeforeDecompile = true;
            }
            else {
                minsToStopAfter = args[++i].toInt();
            }

            break;

        default:
            help();
        }
    }

    if (kmd) {
        return console();
    }

    if (minsToStopAfter > 0) {
        LOG_STREAM(LogLevel::LL_Default) << "stopping decompile after " << minsToStopAfter << " minutes.\n";
        m_kill_timer.setSingleShot(true);
        m_kill_timer.start(1000 * 60 * minsToStopAfter);
    }

    m_thread.setDecompiled(args.last());
    return 0;
}


/**
 * Displays a command line and processes the commands entered.
 *
 * \retval 0 stdin was closed.
 * \retval 2 The user typed exit or quit.
 */
int CommandlineDriver::console()
{
    Boomerang&  boom(*Boomerang::get());
    QTextStream strm(stdin);

    printf("boomerang: ");
    fflush(stdout);
    QString line;

    do {
        line = strm.readLine();

        if (line.isNull()) {
            break;
        }

        QStringList sl = line.split(" \r\n");

        if (boom.processCommand(sl) == 2) {
            return 2;
        }

        printf("boomerang: ");
        fflush(stdout);
    } while (!line.isNull());

    return 0;
}


int CommandlineDriver::decompile()
{
    m_thread.start();
    m_thread.wait(-1);
    return m_thread.resCode();
}


void CommandlineDriver::onCompilationTimeout()
{
    LOG_STREAM() << "Compilation timed out";
    exit(1);
}


void DecompilationThread::run()
{
    Boomerang& boom(*Boomerang::get());

    m_result = boom.decompile(m_decompiled);
    boom.getLogStream().flush();
    boom.getLogStream(LogLevel::LL_Default).flush();
}
