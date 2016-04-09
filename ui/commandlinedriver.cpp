#include <QtCore>
#include <cstdio>

#include "config.h"
#include "boomerang.h"
#include "commandlinedriver.h"

#ifdef HAVE_LIBGC
#include "gc.h"
#else
#define NO_GARBAGE_COLLECTOR
#endif

CommandlineDriver::CommandlineDriver(QObject *parent) : QObject(parent), m_kill_timer(this) {
    this->connect(&m_kill_timer, &QTimer::timeout, this, &CommandlineDriver::onCompilationTimeout);
    QCoreApplication::instance()->connect(&m_thread, &DecompilationThread::finished,
                                          []() { QCoreApplication::instance()->quit(); });
}
/**
 * Prints help about the command line switches.
 */
static void help() {
    QTextStream q_cout(stdout);
    q_cout << "Symbols\n";
    q_cout << "  -s <addr> <name> : Define a symbol\n";
    q_cout << "  -sf <filename>   : Read a symbol/signature file\n";
    q_cout << "Decoding/decompilation options\n";
    q_cout << "  -e <addr>        : Decode the procedure beginning at addr, and callees\n";
    q_cout << "  -E <addr>        : Decode the procedure at addr, no callees\n";
    q_cout << "                     Use -e and -E repeatedly for multiple entry points\n";
    q_cout << "  -ic              : Decode through type 0 Indirect Calls\n";
    q_cout << "  -S <min>         : Stop decompilation after specified number of minutes\n";
    q_cout << "  -t               : Trace (print address of) every instruction decoded\n";
    q_cout << "  -Tc              : Use old constraint-based type analysis\n";
    q_cout << "  -Td              : Use data-flow-based type analysis\n";
    q_cout << "  -LD              : Load before decompile (<program> becomes xml input file)\n";
    q_cout << "  -SD              : Save before decompile\n";
    q_cout << "  -a               : Assume ABI compliance\n";
    q_cout << "  -W               : Windows specific decompilation mode (requires pdb information)\n";
    //    q_cout << "  -pa              : only propagate if can propagate to all\n";
    q_cout << "Output\n";
    q_cout << "  -v               : Verbose\n";
    q_cout << "  -h               : This help\n";
    q_cout << "  -o <output path> : Where to generate output (defaults to ./output/)\n";
    q_cout << "  -x               : Dump XML files\n";
    q_cout << "  -r               : Print RTL for each proc to log before code generation\n";
    q_cout << "  -gd <dot file>   : Generate a dotty graph of the program's CFG and DFG\n";
    q_cout << "  -gc              : Generate a call graph (callgraph.out and callgraph.dot)\n";
    q_cout << "  -gs              : Generate a symbol file (symbols.h)\n";
    q_cout << "  -iw              : Write indirect call report to output/indirect.txt\n";
    q_cout << "Misc.\n";
    q_cout << "  -k               : Command mode, for available commands see -h cmd\n";
    q_cout << "  -P <path>        : Path to Boomerang files, defaults to where you run\n";
    q_cout << "                     Boomerang from\n";
    q_cout << "  -X               : activate eXperimental code; errors likely\n";
    q_cout << "  --               : No effect (used for testing)\n";
    q_cout << "Debug\n";
    q_cout << "  -da              : Print AST before code generation\n";
    q_cout << "  -dc              : Debug switch (Case) analysis\n";
    q_cout << "  -dd              : Debug decoder to stdout\n";
    q_cout << "  -dg              : Debug code Generation\n";
    q_cout << "  -dl              : Debug liveness (from SSA) code\n";
    q_cout << "  -dp              : Debug proof engine\n";
    q_cout << "  -ds              : Stop at debug points for keypress\n";
    q_cout << "  -dt              : Debug type analysis\n";
    q_cout << "  -du              : Debug removing unused statements etc\n";
    q_cout << "Restrictions\n";
    q_cout << "  -nb              : No simplifications for branches\n";
    q_cout << "  -nc              : No decode children in the call graph (callees)\n";
    q_cout << "  -nd              : No (reduced) dataflow analysis\n";
    q_cout << "  -nD              : No decompilation (at all!)\n";
    q_cout << "  -nl              : No creation of local variables\n";
    //    q_cout << "  -nm              : No decoding of the 'main' procedure\n";
    q_cout << "  -ng              : No replacement of expressions with Globals\n";
#ifdef HAVE_LIBGC
    q_cout << "  -nG              : No garbage collection\n";
#endif
    q_cout << "  -nn              : No removal of nullptr and unused statements\n";
    q_cout << "  -np              : No replacement of expressions with Parameter names\n";
    q_cout << "  -nP              : No promotion of signatures (other than main/WinMain/\n";
    q_cout << "                     DriverMain)\n";
    q_cout << "  -nr              : No removal of unneeded labels\n";
    q_cout << "  -nR              : No removal of unused Returns\n";
    q_cout << "  -l <depth>       : Limit multi-propagations to expressions with depth <depth>\n";
    q_cout << "  -p <num>         : Only do num propagations\n";
    q_cout << "  -m <num>         : Max memory depth\n";
}
/**
 * Prints a short usage statement.
 */
static void usage() {
    QTextStream q_cout(stdout);
    q_cout << "Usage: boomerang [ switches ] <program>" << '\n';
    q_cout << "boomerang -h for switch help" << '\n';
}
int CommandlineDriver::applyCommandline(const QStringList &args) {
    printf("Boomerang %s\n", VERSION); // Display a version and date (mainly for release versions)
    int kmd = 0;
    if (args.size() < 2) {
        usage();
        return 1;
    }
    if ((args.size() == 2) && (args[1].compare("-h") == 0)) {
        help();
        return 1;
    }
    Boomerang &boom(*Boomerang::get());
    boom.setProgPath(QFileInfo(args[0]).absolutePath());
    boom.setPluginPath(QFileInfo(args[0]).absolutePath());
    for (int i = 1; i < args.size(); ++i) {
        QString arg = args[i];
        if (arg[0] != '-') {
            if (i == args.size() - 1)
                break;
            // every argument but last must begin with '-'
            usage();
            return 1;
        }
        switch (arg[1].toLatin1()) {
        case 'E':
            boom.noDecodeChildren = true;
        // Fall through
        case 'e': {
            ADDRESS addr;
            boom.decodeMain = false;
            if (++i == args.size()) {
                usage();
                return 1;
            }
            bool converted = false;
            addr.m_value = args[i].toLongLong(&converted, 0);
            if (!converted) {
                LOG_STREAM() << "bad address: " << args[i] << '\n';
                exit(1);
            }
            boom.entrypoints.push_back(addr);
        } break;
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
            LOG_STREAM(LL_Warn) << "Warning: experimental code active!\n";
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
            } else if (arg[2] == 'd')
                boom.dfaTypeAnalysis = true; // -Td: use data-flow-based type analysis (now default)
            break;
        case 'g':
            if (arg[2] == 'd')
                boom.dotFile = args[++i];
            else if (arg[2] == 'c')
                boom.generateCallGraph = true;
            else if (arg[2] == 's') {
                boom.generateSymbols = true;
                boom.stopBeforeDecompile = true;
            }
            break;
        case 'o': {
            QString o_path = args[++i];
            if(!o_path.endsWith('/') && !o_path.endsWith('\\'))
                o_path += '/'; // Maintain the convention of a trailing slash
            boom.setOutputDirectory(o_path);
            break;
        }
        case 'i':
            if (arg[2] == 'c')
                boom.decodeThruIndCall = true; // -ic;
            break;
        case '-':
            break; // No effect: ignored
        case 'L':
            if (arg[2] == 'D')
                boom.loadBeforeDecompile = true;
            break;
        case 'k':
            kmd = 1;
            break;
        case 'P': {
            QString qstr(args[++i] + "/");
            QFileInfo qfi(qstr);
            boom.setProgPath(qfi.path());
        } break;

        case 'n':
            switch (arg[2].toLatin1()) {
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
            case 'G':
#ifndef NO_GARBAGE_COLLECTOR
                GC_disable();
#endif
                break;
            default:
                help();
            }
            break;

        case 'p':
            if (arg[2] == 'a') {
                boom.propOnlyToAll = true;
                LOG_STREAM() << " * * Warning! -pa is not implemented yet!\n";
            } else {
                if (++i == args.size()) {
                    usage();
                    return 1;
                }
                boom.numToPropagate = args[i].toInt();
            }
            break;
        case 's': {
            if (arg[2] == 'f') {
                boom.symbolFiles.push_back(args[i + 1]);
                i++;
                break;
            }
            ADDRESS addr;
            if (++i == args.size()) {
                usage();
                return 1;
            }
            bool converted = false;
            addr.m_value = args[i].toLongLong(&converted, 0);
            if (!converted) {
                LOG_STREAM() << "bad address: " << args[i + 1] << '\n';
                exit(1);
            }
            boom.symbols[addr] = args[++i];
        } break;
        case 'd':
            switch (arg[2].toLatin1()) {
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
            if (arg[2] == 'D')
                boom.saveBeforeDecompile = true;
            else {
                minsToStopAfter = args[++i].toInt();
            }
            break;
        default:
            help();
        }
    }
    if (kmd)
        return console();

    if (minsToStopAfter) {
        LOG_STREAM(LL_Error) << "stopping decompile after " << minsToStopAfter << " minutes.\n";
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
int CommandlineDriver::console() {
    Boomerang &boom(*Boomerang::get());
    QTextStream strm(stdin);
    printf("boomerang: ");
    fflush(stdout);
    QString line;
    do {
        line = strm.readLine();
        if (line.isNull())
            break;
        QStringList sl = line.split(" \r\n");
        if (boom.processCommand(sl) == 2)
            return 2;
        printf("boomerang: ");
        fflush(stdout);
    } while (!line.isNull());
    return 0;
}
int CommandlineDriver::decompile() {
    m_thread.start();
    m_thread.wait(-1);
    return m_thread.resCode();
}
void CommandlineDriver::onCompilationTimeout() {
    LOG_STREAM() << "Compilation timed out";
    exit(1);
}

void DecompilationThread::run() {
    Boomerang &boom(*Boomerang::get());
    Result = boom.decompile(m_decompiled);
    boom.getLogStream().flush();
    boom.getLogStream(LL_Error).flush();
}
