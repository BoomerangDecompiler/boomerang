#include <iostream>
#include <fstream>
#include <time.h>
#include <sys/stat.h>     // For mkdir
#include <direct.h>       // mkdir under Windows
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "hllcode.h"
#include "codegen/chllcode.h"
#include "transformer.h"
#include "boomerang.h"
#include "xmlprogparser.h"
// For some reason, MSVC 5.00 complains about use of undefined type RTL a lot
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"                // For MSVC 5.00
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
    noRemoveReturns(false), debugDecoder(false), decodeThruIndCall(false),
    noDecodeChildren(false), debugProof(false), debugUnusedStmt(false),
    loadBeforeDecompile(false), saveBeforeDecompile(false)
{
    outputPath = "./output/";
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

void Boomerang::helpcmd() {
    std::cerr << "Available commands (for use with -k):\n";
    std::cerr << "\tdecode: loads and decodes the specified binary.\n";
    std::cerr << "\tdecompile [proc]: decompiles the program or specified proc.\n";
    std::cerr << "\tcodegen [cluster]: generates code for the program or a specified cluster.\n";
    std::cerr << "\tmove proc <proc> <cluster>: moves the specified proc to the specified cluster.\n";
    std::cerr << "\tmove cluster <cluster> <parent>: moves the specified cluster to the specified parent cluster.\n";
    std::cerr << "\tadd cluster <cluster> [parent]: adds a new cluster to the root/specified cluster.\n";
    std::cerr << "\tdelete cluster <cluster>: deletes an empty cluster.\n";
    std::cerr << "\trename proc <proc> <newname>: renames the specified proc.\n";
    std::cerr << "\trename cluster <cluster> <newname>: renames the specified cluster.\n";
    std::cerr << "\nFor all commands except decode the <program> argument is an XML input file.\n";
    std::cerr << "All commands result in output of XML files to the default/specified output directory.\n";
    exit(1);
}

void Boomerang::help() {
    std::cerr << "-da: debug - print AST before code generation\n";
    std::cerr << "-dc: debug - debug switch (case) analysis\n";
    std::cerr << "-dd: debug - debug decoder to stdout\n";
    std::cerr << "-dg: debug - debug code generation\n";
    std::cerr << "-dl: debug - debug liveness (from SSA) code\n";
    std::cerr << "-dp: debug - debug proof engine\n";
    std::cerr << "-dr: debug - debug removing unused Returns\n";
    std::cerr << "-dt: debug - debug type analysis\n";
    std::cerr << "-du: debug - debug removing unused statements\n";
    std::cerr << "-e <addr>: decode the procedure beginning at addr\n";
    std::cerr << "-E <addr>: decode ONLY the procedure at addr\n";
    std::cerr << "-g <dot file>: generate a dotty graph of the program's CFG\n";
    std::cerr << "-ic: decode through type 0 indirect calls\n";
    std::cerr << "-o <output path>: where to generate output (defaults to "
        "./output/)\n";
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
    std::cerr << "-r: print rtl for each proc to log before code generation\n";
    std::cerr << "-s <addr> <name>: define a symbol\n";
    std::cerr << "-sf <filename>: read a symbol/signature file\n";
    std::cerr << "-t: trace every instruction decoded\n";
    std::cerr << "-x: dump xml files\n";
    std::cerr << "-LD: load before decompile (<program> becomes xml input file)\n";
    std::cerr << "-SD: save before decompile\n";
    std::cerr << "-k <cmd>: command mode, for available commands see -h cmd\n";
    std::cerr << "-v: verbose\n";
    exit(1);
}
        
// Create the directory. Return false if invalid
bool createDirectory(std::string dir) {
    std::string remainder(dir);
    std::string path;
    unsigned i;
    while ((i = remainder.find('/')) != std::string::npos) {
        path += remainder.substr(0, i+1);
        remainder = remainder.substr(i+1);
#ifdef WIN32
	    mkdir(path.c_str());
#else
		mkdir(path.c_str(), 0777);              // Doesn't matter if already exists
#endif
            }
    // Now try to create a test file
    path += remainder;
#ifdef WIN32
    mkdir(path.c_str());	                // Make the last dir if needed
#else
	mkdir(path.c_str(), 0777);              // Make the last dir if needed
#endif
    path += "test.file";
    std::ofstream test;
    test.open(path.c_str(), std::ios::out);
    test << "testing\n";
    bool pathOK = !test.bad();
    test.close();
    if (pathOK)
        remove(path.c_str());
    return pathOK;
}

int Boomerang::parseCmd(int argc, const char **argv, int n)
{
    const char *fname = argv[argc-1];

    Prog *prog;
    if (!strcmp(argv[n], "decode")) {
        prog = loadAndDecode(fname);
        if (prog == NULL) {
            std::cerr << "failed to load " << fname << "\n";
            return 1;
        }
    } else {
	XMLProgParser *p = new XMLProgParser();
	prog = p->parse(fname);
	if (prog == NULL) {
	    std::cerr << "failed to read xml " << fname << "\n";
	    return 1;
	}

	if (!strcmp(argv[n], "decompile")) {
	    if (n < argc-2 && argv[n+1][0] != '-') {
		Proc *proc = prog->findProc(argv[n+1]);
		if (proc == NULL) {
		    std::cerr << "cannot find proc " << argv[n+1] << "\n";
		    return 1;
		}
		if (proc->isLib()) {
		    std::cerr << "cannot decompile a lib proc\n";
		    return 1;
		}
		((UserProc*)proc)->decompile();
	    } else {
		prog->decompile();
	    }
	} else if (!strcmp(argv[n], "codegen")) {
	    if (n < argc-2 && argv[n+1][0] != '-') {
		Cluster *cluster = prog->findCluster(argv[n+1]);
		if (cluster == NULL) {
		    std::cerr << "cannot find cluster " << argv[n+1] << "\n";
		    return 1;
		}
		prog->generateCode(cluster);
	    } else {
		prog->generateCode();
	    }
	} else if (!strcmp(argv[n], "move")) {
	    if (!strcmp(argv[n+1], "proc")) {
		assert(n < argc-3 && argv[n+2][0] != '-');
		assert(n < argc-4 && argv[n+3][0] != '-');

		Proc *proc = prog->findProc(argv[n+2]);
		if (proc == NULL) {
		    std::cerr << "cannot find proc " << argv[n+2] << "\n";
		    return 1;
		}

		Cluster *cluster = prog->findCluster(argv[n+3]);
		if (cluster == NULL) {
		    std::cerr << "cannot find cluster " << argv[n+3] << "\n";
		    return 1;
		}
		proc->setCluster(cluster);
	    } else if (!strcmp(argv[n+1], "cluster")) {
		assert(n < argc-3 && argv[n+2][0] != '-');
		assert(n < argc-4 && argv[n+3][0] != '-');

		Cluster *cluster = prog->findCluster(argv[n+2]);
		if (cluster == NULL) {
		    std::cerr << "cannot find cluster " << argv[n+2] << "\n";
		    return 1;
		}

		Cluster *parent = prog->findCluster(argv[n+3]);
		if (parent == NULL) {
		    std::cerr << "cannot find cluster " << argv[n+3] << "\n";
		    return 1;
		}

		parent->addChild(cluster);
	    } else {
		std::cerr << "don't know how to move a " << argv[n+1] << "\n";
		return 1;
	    }
	} else if (!strcmp(argv[n], "add")) {
	    if (!strcmp(argv[n+1], "cluster")) {
		assert(n < argc-3 && argv[n+2][0] != '-');

		Cluster *cluster = new Cluster(argv[n+2]);
		if (cluster == NULL) {
		    std::cerr << "cannot create cluster " << argv[n+2] << "\n";
		    return 1;
		}

		Cluster *parent = prog->getRootCluster();
		if (n < argc-4 && argv[n+3][0] != '-') {
		    parent = prog->findCluster(argv[n+3]);
		    if (cluster == NULL) {
			std::cerr << "cannot find cluster " << argv[n+3] << "\n";
			return 1;
		    }
		}

		parent->addChild(cluster);
	    } else {
		std::cerr << "don't know how to add a " << argv[n+1] << "\n";
		return 1;
	    }
	} else if (!strcmp(argv[n], "delete")) {
	    if (!strcmp(argv[n+1], "cluster")) {
		assert(n < argc-3 && argv[n+2][0] != '-');

		Cluster *cluster = prog->findCluster(argv[n+2]);
		if (cluster == NULL) {
		    std::cerr << "cannot find cluster " << argv[n+2] << "\n";
		    return 1;
		}

		if (cluster->hasChildren() || cluster == prog->getRootCluster()) {
		    std::cerr << "cluster " << argv[n+2] << " is not empty\n";
		    return 1;
		}

		if (prog->clusterUsed(cluster)) {
		    std::cerr << "cluster " << argv[n+2] << " is not empty\n";
		    return 1;
		}

		unlink(cluster->getOutPath("xml"));
		unlink(cluster->getOutPath("c"));
		assert(cluster->getParent());
		cluster->getParent()->removeChild(cluster);
	    } else {
		std::cerr << "don't know how to delete a " << argv[n+1] << "\n";
		return 1;
	    }
	} else if (!strcmp(argv[n], "rename")) {
	    if (!strcmp(argv[n+1], "proc")) {
		assert(n < argc-3 && argv[n+2][0] != '-');
		assert(n < argc-4 && argv[n+3][0] != '-');

		Proc *proc = prog->findProc(argv[n+2]);
		if (proc == NULL) {
		    std::cerr << "cannot find proc " << argv[n+2] << "\n";
		    return 1;
		}

		Proc *nproc = prog->findProc(argv[n+3]);
		if (nproc != NULL) {
		    std::cerr << "proc " << argv[n+3] << " already exists\n";
		    return 1;
		}

		proc->setName(argv[n+3]);
	    } else if (!strcmp(argv[n+1], "cluster")) {
		assert(n < argc-3 && argv[n+2][0] != '-');
		assert(n < argc-4 && argv[n+3][0] != '-');

		Cluster *cluster = prog->findCluster(argv[n+2]);
		if (cluster == NULL) {
		    std::cerr << "cannot find cluster " << argv[n+2] << "\n";
		    return 1;
		}

		Cluster *ncluster = prog->findCluster(argv[n+3]);
		if (ncluster == NULL) {
		    std::cerr << "cluster " << argv[n+3] << " already exists\n";
		    return 1;
		}

		cluster->setName(argv[n+3]);
	    } else {
		std::cerr << "don't know how to rename a " << argv[n+1] << "\n";
		return 1;
	    }
	} else {
	    std::cerr << "unknown cmd " << argv[n] << ".\n";
	    return 1;
	}
    }

    XMLProgParser *p = new XMLProgParser();
    p->persistToXML(prog);
    return 0;
}

int Boomerang::commandLine(int argc, const char **argv) 
{
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

    // Parse switches on command line
    if ((argc == 2) && (strcmp(argv[1], "-h") == 0)) {
        help();
        return 1;
    }
    if (argc == 3 && !strcmp(argv[1], "-h") && !strcmp(argv[2], "cmd")) {
        helpcmd();
        return 1;
    }

    int kmd = 0;

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
            case 'E':
                noDecodeChildren = true;
                decodeMain = false;
                // Fall through
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
                    case 'a':
                        printAST = true;
                        break;
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
                    case 'p':
                        debugProof = true;
                        break;
                    case 'r':       // debug counting unused Returns
                        debugUnusedRets = true;
                        break;
                    case 't':       // debug type analysis
                        debugTA = true;
                        break;
                    case 'u':       // debug unused locations (incl unused rets)
                        debugUnusedStmt = true;
                        break;
                    default:
                        help();
                }
                break;
            case 'm':
                sscanf(argv[++i], "%i", &maxMemDepth);
                break;
            case 'i':
                if (argv[i][2] == 'c')
                    decodeThruIndCall = true;       // -ic;
                break;
            case 'L':
                if (argv[i][2] == 'D')
                    loadBeforeDecompile = true;
                break;
            case 'S':
                if (argv[i][2] == 'D')
                    saveBeforeDecompile = true;
                break;
            case 'k':
                i++;
                kmd = i;
                for (; i < argc-1 && argv[i][0] != '-'; i++)
                    ;
                i--;
                break;
            default:
                help();
        }
    }

    // Create the output directory, if needed
    if (!createDirectory(outputPath))
        std::cerr << "Warning! Could not create path " <<
          outputPath << "!\n";
    setLogger(new FileLogger());
    
    if (kmd)
        return parseCmd(argc, argv, kmd);

    return decompile(argv[argc-1]);    
}

Prog *Boomerang::loadAndDecode(const char *fname)
{
    Prog *prog;
    std::cerr << "loading...\n";
    FrontEnd *fe = FrontEnd::Load(fname);
    if (fe == NULL) {
        std::cerr << "failed.\n";
        return NULL;
    }

    // Add symbols from -s switch(es)
    for (std::map<ADDRESS, std::string>::iterator it = symbols.begin();
         it != symbols.end(); it++) {
        fe->AddSymbol((*it).first, (*it).second.c_str());
    }

    if (decodeMain)
        std::cerr << "decoding...\n";
    prog = fe->decode(decodeMain);

    // Delay symbol files to now, since need Prog* prog
    // Also, decode() reads the library catalog
    // symbolFiles from -sf switch(es)
    for (unsigned i = 0; i < symbolFiles.size(); i++) {
        std::cerr << "reading symbol file " << symbolFiles[i].c_str() << "\n";
        prog->readSymbolFile(symbolFiles[i].c_str());
    }
    
    if (!noDecodeChildren) {   // MVE: Not sure if this is right...
        // this causes any undecoded userprocs to be decoded
        std::cerr << "decoding anything undecoded...\n";
        fe->decode(prog, NO_ADDRESS);
    }

    // Entry points from -e (and -E) switch(es)
    for (unsigned i = 0; i < entrypoints.size(); i++) {
        std::cerr<< "decoding extra entrypoint " << std::hex <<
          entrypoints[i] << "\n";
        prog->decodeExtraEntrypoint(entrypoints[i]);
    }

    std::cerr << "found " << std::dec << prog->getNumUserProcs() << " procs\n";

    std::cerr << "analysing...\n";
    prog->analyse();

    prog->printCallGraph();
    prog->printCallGraphXML();
    return prog;
}

int Boomerang::decompile(const char *fname)
{
    Prog *prog;
    time_t start;
    time(&start);
    std::cerr << "setting up transformers...\n";
    ExpTransformer::loadAll();

    if (loadBeforeDecompile) {
        std::cerr << "loading persisted state...\n";
        XMLProgParser *p = new XMLProgParser();
        prog = p->parse(fname);
    } else {
        prog = loadAndDecode(fname);
        if (prog == NULL)
            return 1;
    }

    if (saveBeforeDecompile) {
        std::cerr << "saving persistable state...\n";
        XMLProgParser *p = new XMLProgParser();
        p->persistToXML(prog);
    }

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
    prog->generateCode();

    time_t end;
    time(&end);
    int hours = (end-start) / 60 / 60;
    int mins = (end-start) / 60 - hours * 60;
    int secs = (end-start) - hours * 60 * 60 - mins * 60;
    std::cerr << "completed in " << std::dec;
    if (hours)
        std::cerr << hours << " hours ";
    if (hours || mins)
        std::cerr << mins << " mins ";
    std::cerr << secs << " sec" << (secs == 1 ? "" : "s") << ".\n";

    return 0;
}
