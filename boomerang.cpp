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
    noLocals(false), noRemoveLabels(false), traceDecoder(false), dotFile(NULL)
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
    std::cerr << "-nl: no creation of local variables\n";
    std::cerr << "-nr: no removal of unnedded labels\n";
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
            case 'n':
                switch(argv[i][2]) {
                    case 'b':
                        noBranchSimplify = true;
                        break;
                    case 'i':
                        noRemoveInternal = true;
                        break;
                    case 'l':
                        noLocals = true;
                        break;
                    case 'r':
                        noRemoveLabels = true;
                        break;
                    default:
                        help();
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
    std::cerr << "analysing..." << std::endl;
    prog->analyse();
    std::cerr << "decompiling..." << std::endl;
    prog->decompile();
    if (dotFile) {
        std::cerr << "generating dot file..." << std::endl;
        prog->generateDotFile();
    }
    std::cerr << "generating code..." << std::endl;
    prog->generateCode(std::cout);

    return 0;
}

