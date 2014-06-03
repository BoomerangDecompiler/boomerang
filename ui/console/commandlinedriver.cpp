#include <QtCore>
#include <iostream>
#include <fstream>
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
    std::cout << "Symbols\n";
    std::cout << "  -s <addr> <name> : Define a symbol\n";
    std::cout << "  -sf <filename>   : Read a symbol/signature file\n";
    std::cout << "Decoding/decompilation options\n";
    std::cout << "  -e <addr>        : Decode the procedure beginning at addr, and callees\n";
    std::cout << "  -E <addr>        : Decode the procedure at addr, no callees\n";
    std::cout << "                     Use -e and -E repeatedly for multiple entry points\n";
    std::cout << "  -ic              : Decode through type 0 Indirect Calls\n";
    std::cout << "  -S <min>         : Stop decompilation after specified number of minutes\n";
    std::cout << "  -t               : Trace (print address of) every instruction decoded\n";
    std::cout << "  -Tc              : Use old constraint-based type analysis\n";
    std::cout << "  -Td              : Use data-flow-based type analysis\n";
    std::cout << "  -LD              : Load before decompile (<program> becomes xml input file)\n";
    std::cout << "  -SD              : Save before decompile\n";
    std::cout << "  -a               : Assume ABI compliance\n";
    std::cout << "  -W               : Windows specific decompilation mode (requires pdb information)\n";
    //    std::cout << "  -pa              : only propagate if can propagate to all\n";
    std::cout << "Output\n";
    std::cout << "  -v               : Verbose\n";
    std::cout << "  -h               : This help\n";
    std::cout << "  -o <output path> : Where to generate output (defaults to ./output/)\n";
    std::cout << "  -x               : Dump XML files\n";
    std::cout << "  -r               : Print RTL for each proc to log before code generation\n";
    std::cout << "  -gd <dot file>   : Generate a dotty graph of the program's CFG and DFG\n";
    std::cout << "  -gc              : Generate a call graph (callgraph.out and callgraph.dot)\n";
    std::cout << "  -gs              : Generate a symbol file (symbols.h)\n";
    std::cout << "  -iw              : Write indirect call report to output/indirect.txt\n";
    std::cout << "Misc.\n";
    std::cout << "  -k               : Command mode, for available commands see -h cmd\n";
    std::cout << "  -P <path>        : Path to Boomerang files, defaults to where you run\n";
    std::cout << "                     Boomerang from\n";
    std::cout << "  -X               : activate eXperimental code; errors likely\n";
    std::cout << "  --               : No effect (used for testing)\n";
    std::cout << "Debug\n";
    std::cout << "  -da              : Print AST before code generation\n";
    std::cout << "  -dc              : Debug switch (Case) analysis\n";
    std::cout << "  -dd              : Debug decoder to stdout\n";
    std::cout << "  -dg              : Debug code Generation\n";
    std::cout << "  -dl              : Debug liveness (from SSA) code\n";
    std::cout << "  -dp              : Debug proof engine\n";
    std::cout << "  -ds              : Stop at debug points for keypress\n";
    std::cout << "  -dt              : Debug type analysis\n";
    std::cout << "  -du              : Debug removing unused statements etc\n";
    std::cout << "Restrictions\n";
    std::cout << "  -nb              : No simplifications for branches\n";
    std::cout << "  -nc              : No decode children in the call graph (callees)\n";
    std::cout << "  -nd              : No (reduced) dataflow analysis\n";
    std::cout << "  -nD              : No decompilation (at all!)\n";
    std::cout << "  -nl              : No creation of local variables\n";
    //    std::cout << "  -nm              : No decoding of the 'main' procedure\n";
    std::cout << "  -ng              : No replacement of expressions with Globals\n";
#ifdef HAVE_LIBGC
    std::cout << "  -nG              : No garbage collection\n";
#endif
    std::cout << "  -nn              : No removal of nullptr and unused statements\n";
    std::cout << "  -np              : No replacement of expressions with Parameter names\n";
    std::cout << "  -nP              : No promotion of signatures (other than main/WinMain/\n";
    std::cout << "                     DriverMain)\n";
    std::cout << "  -nr              : No removal of unneeded labels\n";
    std::cout << "  -nR              : No removal of unused Returns\n";
    std::cout << "  -l <depth>       : Limit multi-propagations to expressions with depth <depth>\n";
    std::cout << "  -p <num>         : Only do num propagations\n";
    std::cout << "  -m <num>         : Max memory depth\n";
}
/**
 * Prints a short usage statement.
 */
static void usage() {
    std::cout << "Usage: boomerang [ switches ] <program>" << std::endl;
    std::cout << "boomerang -h for switch help" << std::endl;
}
int CommandlineDriver::applyCommandline() {
    printf("Boomerang %s\n", VERSION); // Display a version and date (mainly for release versions)
    QStringList args(qApp->arguments());
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
            if (not converted) {
                std::cerr << "bad address: " << args[i].toStdString() << std::endl;
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
            std::cout << "Warning: experimental code active!\n";
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
                boom.dotFile = args[++i].toStdString();
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
            if (arg[2] == 'w')                 // -iw
                if (boom.ofsIndCallReport) {
                    QString fname = boom.getOutputPath() + "indirect.txt";
                    boom.ofsIndCallReport = new std::ofstream(fname.toStdString());
                }
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
                std::cerr << " * * Warning! -pa is not implemented yet!\n";
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
                boom.symbolFiles.push_back(args[i + 1].toStdString());
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
            if (not converted) {
                std::cerr << "bad address: " << args[i + 1].toStdString() << std::endl;
                exit(1);
            }
            boom.symbols[addr] = args[++i].toStdString();
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
        std::cout << "stopping decompile after " << minsToStopAfter << " minutes.\n";
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
    return 0;
}
void CommandlineDriver::onCompilationTimeout() {
    std::cerr << "Compilation timed out";
    exit(1);
}

void DecompilationThread::run() {
    Boomerang &boom(*Boomerang::get());
    int res = boom.decompile(m_decompiled);
    QCoreApplication::exit(res == 0 ? 0 : -1);
}
