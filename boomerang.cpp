#include <iostream>
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "hllcode.h"
#include "codegen/chllcode.h"
#include "boomerang.h"

Boomerang *Boomerang::boomerang = NULL;

Boomerang::Boomerang() : vFlag(false), printRtl(false), 
    noBranchSimplify(false), noRemoveInternal(false),
    noRemoveNull(false), noLocals(false), noRemoveLabels(false), 
    noDataflow(false), noDecompileUp(false),
    traceDecoder(false), dotFile(NULL), numToPropagate(-1), noPromote(false)
{
}

HLLCode *Boomerang::getHLLCode(UserProc *p)
{
    return new CHLLCode(p);
}

void Boomerang::usage() {
    std::cerr << "usage: boomerang [ switches ] <program>" << std::endl;
    std::cerr << "boomerang -h for switch help" << std::endl;
    exit(1);
}

void Boomerang::help() {
    std::cerr << "-h: this help\n";
    std::cerr << "-v: verbose\n";
    std::cerr << "-g <dot file>: generate a dotty graph of the program\n";
    std::cerr << "-r: print rtl for each proc to stderr before code generation\n";
    std::cerr << "-t: trace every instruction decoded\n";
    std::cerr << "-nb: no simplications for branches\n";
    std::cerr << "-ni: no removal of internal statements\n";
    std::cerr << "-nn: no removal of null and dead statements\n";
    std::cerr << "-nl: no creation of local variables\n";
    std::cerr << "-nr: no removal of unnedded labels\n";
    std::cerr << "-nd: no (reduced) dataflow analysis\n";
    std::cerr << "-nDu: no decompilation when recursing up the call graph\n";
    std::cerr << "-nD: no decompilation (at all!)\n";
    std::cerr << "-nP: no promotion of signatures (at all!)\n";
    std::cerr << "-p num: only do num propogations\n";
    std::cerr << "-e <addr>: decode the procedure beginning at addr\n";
    exit(1);
}
        
int Boomerang::commandLine(int argc, const char **argv) {
    if (argc < 2) usage();
    progPath = argv[0];
    // Chop off after the last slash
    size_t j = progPath.rfind("/");
    if (j != (size_t)-1)
    {
        // Do the chop; keep the trailing slash
        progPath = progPath.substr(0, j+1);
    }
    else {
        std::cerr << "? No slash in argv[0]!" << std::endl;
        return 1;
    }

    // Parse switches on command line
    if ((argc == 2) && (strcmp(argv[1], "-h") == 0)) {
        help();
        return 1;
    }
    std::list<ADDRESS> entrypoints;
    bool noDecompilation = false;
    for (int i=1; i < argc-1; i++) {
        if (argv[i][0] != '-')
            usage();
        switch (argv[i][1]) {
            case 'h': help(); break;
            case 'v': vFlag = true; break;
            case 'r': printRtl = true; break;
            case 't': traceDecoder = true; break;
            case 'g': 
                dotFile = argv[++i];
                break;
            case 'p':
                sscanf(argv[++i], "%i", &numToPropagate);
                break;
            case 'n':
                switch(argv[i][2]) {
                    case 'b':
                        noBranchSimplify = true;
                        break;
                    case 'i':
                        noRemoveInternal = true;
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
                    case 'd':
                        noDataflow = true;
                        break;
                    case 'D':
                        if (argv[i][3] == 'u')
                            noDecompileUp = true;
                        else
                            noDecompilation = true;
                        break;
                    case 'P':
                        noPromote = true;
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
            default:
                help();
        }
    }
    
    std::cerr << "loading..." << std::endl;
    FrontEnd *fe = FrontEnd::Load(argv[argc-1]);
    if (fe == NULL) {
        std::cerr << "failed." << std::endl;
        return 1;
    }
    std::cerr << "decoding..." << std::endl;
    Prog *prog = fe->decode();
    if (entrypoints.size()) {
        for (std::list<ADDRESS>::iterator it = entrypoints.begin();
             it != entrypoints.end(); it++) {
            std::cerr << "decoding extra entrypoint " << *it << std::endl;
            prog->decode(*it);
        }
    }
    std::cerr << "analysing..." << std::endl;
    prog->analyse();
    if (!noDecompilation) {
        std::cerr << "decompiling..." << std::endl;
        prog->decompile();
    }
    if (dotFile) {
        std::cerr << "generating dot file..." << std::endl;
        prog->generateDotFile();
    }
    std::cerr << "generating code..." << std::endl;
    prog->generateCode(std::cout);

    return 0;
}
