#include <iostream>
#include <fstream>
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "hllcode.h"
#include "codegen/chllcode.h"
#include "boomerang.h"
// For some reason, MSVC 5.00 complains about use of undefined type RTL a lot
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"		// For MSVC 5.00
#include "rtl.h"
#endif

Boomerang *Boomerang::boomerang = NULL;

Boomerang::Boomerang() : logger(NULL), vFlag(false), printRtl(false), 
    noBranchSimplify(false), noRemoveNull(false), noLocals(false),
    noRemoveLabels(false), noDataflow(false), noDecompile(false),
    traceDecoder(false), dotFile(NULL), numToPropagate(-1),
    noPromote(false), propOnlyToAll(false), debugGen(false),
    maxMemDepth(99), debugSwitch(false),
    noParameterNames(false), debugLiveness(false), debugUnusedRets(false),
    debugTA(false), decodeMain(true), printAST(false), dumpXML(false),
    noRemoveReturns(false), debugDecoder(false)
{
}

class FileLogger : public Log {
public:
    FileLogger() : Log(), out((Boomerang::get()->getOutputPath() + "log").c_str()) { }
    virtual Log &operator<<(const char *str) { 
        out << str << std::flush;  
        return *this; 
    }
    virtual ~FileLogger() {};
protected:
    std::ofstream out;
};

Log &Boomerang::log() {
    return *logger;
}

HLLCode *Boomerang::getHLLCode(UserProc *p) {
    return new CHLLCode(p);
}

void Boomerang::usage() {
    std::cerr << "usage: boomerang [ switches ] <program>" << std::endl;
    std::cerr << "boomerang -h for switch help" << std::endl;
    exit(1);
}

void Boomerang::help() {
    std::cerr << "-dc: debug - debug switch (case) analysis\n";
    std::cerr << "-dd: debug - debug decoder to stderr\n";
    std::cerr << "-dg: debug - debug code generation\n";
    std::cerr << "-dl: debug - debug liveness (from SSA) code\n";
    std::cerr << "-dr: debug - debug unused Returns\n";
    std::cerr << "-dt: debug - debug type analysis\n";
    std::cerr << "-da: debug - print AST before code generation\n";
    std::cerr << "-e <addr>: decode the procedure beginning at addr\n";
    std::cerr << "-g <dot file>: generate a dotty graph of the program's CFG\n";
    std::cerr << "-o <output path>: where to generate output (defaults to .)\n";
    std::cerr << "-h: this help\n";
    std::cerr << "-m <num>: max memory depth\n";
    std::cerr << "-nb: no simplifications for branches\n";
    std::cerr << "-nn: no removal of null and unused statements\n";
    std::cerr << "-nl: no creation of local variables\n";
    std::cerr << "-np: no replacement of expressions with parameter names\n";
    std::cerr << "-nr: no removal of unnedded labels\n";
    std::cerr << "-nR: no removal of unused returns\n";
    std::cerr << "-nd: no (reduced) dataflow analysis\n";
    std::cerr << "-nD: no decompilation (at all!)\n";
    std::cerr << "-nP: no promotion of signatures (at all!)\n";
    std::cerr << "-nm: don't decode the 'main' procedure\n";
    std::cerr << "-p <num>: only do num propogations\n";
//  std::cerr << "-pa: only propagate if can propagate to all\n";
    std::cerr << "-r: print rtl for each proc to stderr before code generation"
                    "\n";
    std::cerr << "-s <addr> <name>: define a symbol\n";
    std::cerr << "-sf <filename>: read a symbol/signature file\n";
    std::cerr << "-t: trace every instruction decoded\n";
    std::cerr << "-x: dump xml files\n";
    std::cerr << "-v: verbose\n";
    exit(1);
}
        
int Boomerang::commandLine(int argc, const char **argv) {
    if (argc < 2) usage();
    progPath = argv[0];
    // Chop off after the last slash
    size_t j = progPath.rfind("/");
    if (j == (size_t)-1) 
        j = progPath.rfind("\\");
    if (j != (size_t)-1)
    {
        // Do the chop; keep the trailing slash
        progPath = progPath.substr(0, j+1);
    }
    else {
        std::cerr << "? No slash in argv[0]!" << std::endl;
        return 1;
    }
    outputPath = "./";

    // Parse switches on command line
    if ((argc == 2) && (strcmp(argv[1], "-h") == 0)) {
        help();
        return 1;
    }
    for (int i=1; i < argc-1; i++) {
        if (argv[i][0] != '-')
            usage();
        switch (argv[i][1]) {
            case 'h': help(); break;
            case 'v': vFlag = true; break;
            case 'x': dumpXML = true; break;
            case 'r': printRtl = true; break;
            case 't': traceDecoder = true; break;
            case 'g': 
                dotFile = argv[++i];
                break;
            case 'o':
                outputPath = argv[++i];
                if (outputPath[outputPath.size()-1] != '/')
                    outputPath += '/';
                break;
            case 'p':
                if (argv[i][2] == 'a') {
                    propOnlyToAll = true;
                    std::cerr << " * * Warning! -pa is not implemented yet!\n";
                }
                else
                    sscanf(argv[++i], "%i", &numToPropagate);
                break;
            case 'n':
                switch(argv[i][2]) {
                    case 'b':
                        noBranchSimplify = true;
                        break;
                    case 'n':
                        noRemoveNull = true;
                        break;
                    case 'l':
                        noLocals = true;
                        break;
                    case 'r':
                        noRemoveLabels = true;
                        break;
                    case 'R':
                        noRemoveReturns = true;
                        break;
                    case 'd':
                        noDataflow = true;
                        break;
                    case 'D':
                        noDecompile = true;
                        break;
                    case 'P':
                        noPromote = true;
                        break;
                    case 'p':
                        noParameterNames = true;
                        break;
                    case 'm':
                        decodeMain = false;
                        break;
                    default:
                        help();
                }
                break;
            case 'e':
                {
                    ADDRESS addr;
                    int n;
                    if (argv[i+1][0] == '0' && argv[i+1][1] == 'x') {
                        n = sscanf(argv[i+1], "0x%x", &addr);
                    } else {
                        n = sscanf(argv[i+1], "%i", &addr);
                    }
                    i++;
                    if (n != 1) {
                        std::cerr << "bad address: " << argv[i+1] << std::endl;
                        exit(1);
                    }
                    entrypoints.push_back(addr);
                }
                break;
            case 's':
                {
                    if (argv[i][2] == 'f') {
                        symbolFiles.push_back(argv[i+1]);
                        i++;
                        break;
                    }
                    ADDRESS addr;
                    int n;
                    if (argv[i+1][0] == '0' && argv[i+1][1] == 'x') {
                        n = sscanf(argv[i+1], "0x%x", &addr);
                    } else {
                        n = sscanf(argv[i+1], "%i", &addr);
                    }
                    i++;
                    if (n != 1) {
                        std::cerr << "bad address: " << argv[i+1] << std::endl;
                        exit(1);
                    }
                    const char *nam = argv[++i];
                    symbols[addr] = nam;
                }
                break;
            case 'd':
                switch(argv[i][2]) {
                    case 'c':
                        debugSwitch = true;
                        break;
                    case 'd':
                        debugDecoder = true;
                        break;
                    case 'g':
                        debugGen = true;
                        break;
                    case 'l':
                        debugLiveness = true;
                        break;
                    case 'r':       // debug counting unused Returns
                        debugUnusedRets = true;
                        break;
                    case 't':       // debug type analysis
                        debugTA = true;
                        break;
                    case 'a':
                        printAST = true;
                        break;
                }
                break;
            case 'm':
                sscanf(argv[++i], "%i", &maxMemDepth);
                break;
            default:
                help();
        }
    }
    setLogger(new FileLogger());
    
    return decompile(argv[argc-1]);    
}

int Boomerang::decompile(const char *fname)
{
    std::cerr << "loading...\n";
    FrontEnd *fe = FrontEnd::Load(fname);
    if (fe == NULL) {
        std::cerr << "failed.\n";
        return 1;
    }

    for (std::map<ADDRESS, std::string>::iterator it = symbols.begin();
         it != symbols.end(); it++) {
        fe->AddSymbol((*it).first, (*it).second.c_str());
    }

    std::cerr << "decoding...\n";
    Prog *prog = fe->decode(decodeMain);

    for (unsigned i = 0; i < symbolFiles.size(); i++) {
        std::cerr << "reading symbol file " << symbolFiles[i].c_str() << "\n";
        prog->readSymbolFile(symbolFiles[i].c_str());
    }
    
    // this causes any undecoded userprocs to be decoded
    std::cerr << "decoding anything undecoded...\n";
    fe->decode(prog, NO_ADDRESS);

    if (entrypoints.size()) {
        for (unsigned i = 0; i < entrypoints.size(); i++) {
            std::cerr<< "decoding extra entrypoint " << std::hex <<
              entrypoints[i] << "\n";
            prog->decodeFunction(entrypoints[i]);
        }
    }

    std::cerr << "analysing...\n";
    prog->analyse();

    prog->printCallGraph();
    prog->printCallGraphXML();

    std::cerr << "decompiling...\n";
    prog->decompile();

    if (dotFile)
        prog->generateDotFile();

    if (printAST) {
        std::cerr << "printing AST...\n";
        PROGMAP::const_iterator it;
        for (Proc *p = prog->getFirstProc(it); p; p = prog->getNextProc(it))
            if (!p->isLib()) {
                UserProc *u = (UserProc*)p;
                u->getCFG()->compressCfg();
                u->printAST();
            }
    }

    std::cerr << "generating code...\n";
    std::ofstream out((getOutputPath() + "code").c_str());
    prog->generateCode(out);
    out.close();

    return 0;
}
