/*
 * Copyright (C) 2002-2005, Mike Van Emmerik and Trent Waddington
 */
/*==============================================================================
 * FILE:	   boomerang.cpp
 * OVERVIEW:   Command line processing for the Boomerang decompiler
 *============================================================================*/
/*
 * $Revision$
 * 28 Jan 05 - G. Krol: Separated -h output into sections and neatened
*/

#include <iostream>
#include <fstream>
#include <time.h>
#ifdef WIN32
#include <direct.h>			// mkdir under Windows
#else
#include <sys/stat.h>		// For mkdir
#include <unistd.h>			// For unlink
#endif
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "hllcode.h"
#include "codegen/chllcode.h"
#include "transformer.h"
#include "boomerang.h"
#include "xmlprogparser.h"

Boomerang *Boomerang::boomerang = NULL;

Boomerang::Boomerang() : logger(NULL), vFlag(false), printRtl(false), 
    noBranchSimplify(false), noRemoveNull(false), noLocals(false),
    noRemoveLabels(false), noDataflow(false), noDecompile(false), stopBeforeDecompile(false),
    traceDecoder(false), dotFile(NULL), numToPropagate(-1),
    noPromote(false), propOnlyToAll(false), debugGen(false),
    maxMemDepth(99), debugSwitch(false),
    noParameterNames(false), debugLiveness(false), debugUnusedRetsAndParams(false),
    debugTA(false), decodeMain(true), printAST(false), dumpXML(false),
    noRemoveReturns(false), debugDecoder(false), decodeThruIndCall(false),
    noDecodeChildren(false), debugProof(false), debugUnusedStmt(false),
    loadBeforeDecompile(false), saveBeforeDecompile(false), overlapped(false),
	noProve(false), noChangeSignatures(false), conTypeAnalysis(false), dfaTypeAnalysis(false),
	noLimitPropagations(false), fastx86(false)
{
	progPath = "./";
	outputPath = "./output/";
}


Log &Boomerang::log() {
	return *logger;
}

FileLogger::FileLogger() : out((Boomerang::get()->getOutputPath() + "log").c_str()) {
}

HLLCode *Boomerang::getHLLCode(UserProc *p) {
	return new CHLLCode(p);
}

void Boomerang::usage() {
	std::cout << "Usage: boomerang [ switches ] <program>" << std::endl;
	std::cout << "boomerang -h for switch help" << std::endl;
	exit(1);
}

void Boomerang::helpcmd() {
	std::cout << "Available commands (for use with -k):\n";
	std::cout << "  decode                             : Loads and decodes the specified binary.\n";
	std::cout << "  decompile [proc]                   : Decompiles the program or specified proc.\n";
	std::cout << "  codegen [cluster]                  : Generates code for the program or a\n";
	std::cout << "                                       specified cluster.\n";
	std::cout << "  move proc <proc> <cluster>         : Moves the specified proc to the specified\n";
	std::cout << "                                       cluster.\n";
	std::cout << "  move cluster <cluster> <parent>    : Moves the specified cluster to the\n";
	std::cout << "                                       specified parent cluster.\n";
	std::cout << "  add cluster <cluster> [parent]     : Adds a new cluster to the root/specified\n";
	std::cout << "                                       cluster.\n";
	std::cout << "  delete cluster <cluster>           : Deletes an empty cluster.\n";
	std::cout << "  rename proc <proc> <newname>       : Renames the specified proc.\n";
	std::cout << "  rename cluster <cluster> <newname> : Renames the specified cluster.\n";
	std::cout << "  info prog                          : Print info about the program.\n";
	std::cout << "  info cluster <cluster>             : Print info about a cluster.\n";
	std::cout << "  info proc <proc>                   : Print info about a proc.\n";
	std::cout << "  print <proc>                       : Print the RTL for a proc.\n";
	std::cout << "  help                               : This help.\n";
	std::cout << "  exit                               : Quit the shell.\n";
}

void Boomerang::help() {
	std::cout << "Symbols\n";
	std::cout << "  -s <addr> <name> : Define a symbol\n";
	std::cout << "  -sf <filename>   : Read a symbol/signature file\n";
	std::cout << "Decoding/decompilation options\n";
	std::cout << "  -e <addr>        : Decode the procedure beginning at addr\n";
	std::cout << "  -E <addr>        : Decode ONLY the procedure at addr\n";
	std::cout << "  -ic              : Decode through type 0 Indirect Calls\n";
	std::cout << "  -t               : Trace (print address of) every instruction decoded\n";
	std::cout << "  -Tc              : Use old constraint-based type analysis\n";
	std::cout << "  -Td              : Use data-flow-based type analysis\n";
    std::cout << "  -f               : Fast x86 specific decompilation\n";
	std::cout << "  -O               : Handle Overlapped registers (for X86 only)\n";
	std::cout << "  -LD              : Load before decompile (<program> becomes xml input file)\n";
	std::cout << "  -SD              : Save before decompile\n";
//	std::cout << "  -pa              : only propagate if can propagate to all\n";
	std::cout << "Output\n";
	std::cout << "  -v               : Verbose\n";
	std::cout << "  -h               : This help\n";
	std::cout << "  -o <output path> : Where to generate output (defaults to ./output/)\n";
	std::cout << "  -x               : Dump XML files\n";
	std::cout << "  -r               : Print RTL for each proc to log before code generation\n";
	std::cout << "  -g <dot file>    : Generate a dotty graph of the program's CFG\n";
	std::cout << "Misc.\n";
	std::cout << "  -k               : Command mode, for available commands see -h cmd\n";
	std::cout << "  -P <path>        : Path to Boomerang files, defaults to where you run\n";
	std::cout << "                     Boomerang from\n";
	std::cout << "  --               : No effect (used for testing)\n";
	std::cout << "Debug\n";
	std::cout << "  -da              : Print AST before code generation\n";
	std::cout << "  -dc              : Debug switch (Case) analysis\n";
	std::cout << "  -dd              : Debug decoder to stdout\n";
	std::cout << "  -dg              : Debug code Generation\n";
	std::cout << "  -dl              : Debug liveness (from SSA) code\n";
	std::cout << "  -dp              : Debug proof engine\n";
	std::cout << "  -dr              : Debug removing unused returns and parameters\n";
	std::cout << "  -dt              : Debug type analysis\n";
	std::cout << "  -du              : Debug removing unused statements\n";
	std::cout << "Restrictions\n";
	std::cout << "  -nb              : No simplifications for branches\n";
	std::cout << "  -nd              : No (reduced) dataflow analysis\n";
	std::cout << "  -nD              : No decompilation (at all!)\n";
	std::cout << "  -nl              : No creation of local variables\n";
	std::cout << "  -nL              : No limiting of propagations using the self-referencing\n";
	std::cout << "                     heuristic\n";
	std::cout << "  -nm              : No decoding of the 'main' procedure\n";
	std::cout << "  -nn              : No removal of NULL and unused statements\n";
	std::cout << "  -np              : No replacement of expressions with Parameter names\n";
	std::cout << "  -nP              : No promotion of signatures (other than main/WinMain/\n";
	std::cout << "                     DriverMain)\n";
	std::cout << "  -nr              : No removal of unneeded labels\n";
	std::cout << "  -nR              : No removal of unused Returns\n";
	std::cout << "  -p <num>         : Only do num propagations\n";
	std::cout << "  -m <num>         : Max memory depth\n";
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
		mkdir(path.c_str(), 0777);				// Doesn't matter if already exists
#endif
			}
	// Now try to create a test file
	path += remainder;
#ifdef WIN32
	mkdir(path.c_str());					// Make the last dir if needed
#else
	mkdir(path.c_str(), 0777);				// Make the last dir if needed
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

void Cluster::printTree(std::ostream &out)
{
	out << "\t\t" << name << "\n";
	for (unsigned i = 0; i < children.size(); i++)
	children[i]->printTree(out);
}

typedef char *crazy_vc_bug;

int Boomerang::splitLine(char *line, char ***pargv)
{
	int argc = 0;
	*pargv = new crazy_vc_bug[100];
	const char *p = strtok(line, " \r\n");
	while(p) {
	(*pargv)[argc++] = (char*)p;
	p = strtok(NULL, " \r\n");
	}
	return argc;
}

int Boomerang::parseCmd(int argc, const char **argv)
{
	static Prog *prog = NULL;
	if (!strcmp(argv[0], "decode")) {
		if (argc <= 1) {
			std::cerr << "not enough arguments for cmd\n";
			return 1;
		}
		const char *fname = argv[1];
		Prog *p = loadAndDecode(fname);
			if (p == NULL) {
				std::cerr << "failed to load " << fname << "\n";
				return 1;
			}
		prog = p;
	} else if (!strcmp(argv[0], "load")) {
        if (argc <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        const char *fname = argv[1];
        XMLProgParser *p = new XMLProgParser();
        Prog *pr = p->parse(fname);
        if (pr == NULL) {
            // try guessing
            pr = p->parse((outputPath + "/" + fname + "/" + fname + ".xml").c_str());
            if (pr == NULL) {
            std::cerr << "failed to read xml " << fname << "\n";
            return 1;
            }
        }
        prog = pr;
	} else if (!strcmp(argv[0], "save")) {
        if (prog == NULL) {
            std::cerr << "need to load or decode before save!\n";
            return 1;
        }
        XMLProgParser *p = new XMLProgParser();
        p->persistToXML(prog);
	} else if (!strcmp(argv[0], "decompile")) {
        if (argc > 1) {
            Proc *proc = prog->findProc(argv[1]);
            if (proc == NULL) {
                std::cerr << "cannot find proc " << argv[1] << "\n";
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
	} else if (!strcmp(argv[0], "codegen")) {
        if (argc > 1 ) {
            Cluster *cluster = prog->findCluster(argv[1]);
            if (cluster == NULL) {
            std::cerr << "cannot find cluster " << argv[1] << "\n";
            return 1;
            }
            prog->generateCode(cluster);
        } else {
            prog->generateCode();
        }
	} else if (!strcmp(argv[0], "move")) {
        if (argc <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        if (!strcmp(argv[1], "proc")) {
            if (argc <= 3) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Proc *proc = prog->findProc(argv[2]);
            if (proc == NULL) {
                std::cerr << "cannot find proc " << argv[2] << "\n";
                return 1;
            }

            Cluster *cluster = prog->findCluster(argv[3]);
            if (cluster == NULL) {
                std::cerr << "cannot find cluster " << argv[3] << "\n";
                return 1;
            }
            proc->setCluster(cluster);
        } else if (!strcmp(argv[1], "cluster")) {
            if (argc <= 3) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Cluster *cluster = prog->findCluster(argv[2]);
            if (cluster == NULL) {
                std::cerr << "cannot find cluster " << argv[2] << "\n";
                return 1;
            }

            Cluster *parent = prog->findCluster(argv[3]);
            if (parent == NULL) {
                std::cerr << "cannot find cluster " << argv[3] << "\n";
                return 1;
            }

            parent->addChild(cluster);
        } else {
            std::cerr << "don't know how to move a " << argv[1] << "\n";
            return 1;
        }
	} else if (!strcmp(argv[0], "add")) {
        if (argc <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        if (!strcmp(argv[1], "cluster")) {
            if (argc <= 2) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Cluster *cluster = new Cluster(argv[2]);
            if (cluster == NULL) {
                std::cerr << "cannot create cluster " << argv[2] << "\n";
                return 1;
            }

            Cluster *parent = prog->getRootCluster();
            if (argc > 3) {
                parent = prog->findCluster(argv[3]);
                if (cluster == NULL) {
                    std::cerr << "cannot find cluster " << argv[3] << "\n";
                    return 1;
                }
            }

            parent->addChild(cluster);
        } else {
            std::cerr << "don't know how to add a " << argv[1] << "\n";
            return 1;
        }
	} else if (!strcmp(argv[0], "delete")) {
        if (argc <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        if (!strcmp(argv[1], "cluster")) {
            if (argc <= 2) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Cluster *cluster = prog->findCluster(argv[2]);
            if (cluster == NULL) {
                std::cerr << "cannot find cluster " << argv[2] << "\n";
                return 1;
            }

            if (cluster->hasChildren() || cluster == prog->getRootCluster()) {
                std::cerr << "cluster " << argv[2] << " is not empty\n";
                return 1;
            }

            if (prog->clusterUsed(cluster)) {
                std::cerr << "cluster " << argv[2] << " is not empty\n";
                return 1;
            }

            unlink(cluster->getOutPath("xml"));
            unlink(cluster->getOutPath("c"));
            assert(cluster->getParent());
            cluster->getParent()->removeChild(cluster);
        } else {
            std::cerr << "don't know how to delete a " << argv[1] << "\n";
            return 1;
        }
	} else if (!strcmp(argv[0], "rename")) {
        if (argc <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        if (!strcmp(argv[1], "proc")) {
            if (argc <= 3) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Proc *proc = prog->findProc(argv[2]);
            if (proc == NULL) {
                std::cerr << "cannot find proc " << argv[2] << "\n";
                return 1;
            }

            Proc *nproc = prog->findProc(argv[3]);
            if (nproc != NULL) {
                std::cerr << "proc " << argv[3] << " already exists\n";
                return 1;
            }

            proc->setName(argv[3]);
        } else if (!strcmp(argv[1], "cluster")) {
            if (argc <= 3) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Cluster *cluster = prog->findCluster(argv[2]);
            if (cluster == NULL) {
                std::cerr << "cannot find cluster " << argv[2] << "\n";
                return 1;
            }

            Cluster *ncluster = prog->findCluster(argv[3]);
            if (ncluster == NULL) {
                std::cerr << "cluster " << argv[3] << " already exists\n";
                return 1;
            }

            cluster->setName(argv[3]);
        } else {
            std::cerr << "don't know how to rename a " << argv[1] << "\n";
            return 1;
        }
	} else if (!strcmp(argv[0], "info")) {
        if (argc <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }
        if (!strcmp(argv[1], "prog")) {

            std::cout << "prog " << prog->getName() << ":\n";
            std::cout << "\tclusters:\n";
            prog->getRootCluster()->printTree(std::cout);
            std::cout << "\n\tlibprocs:\n";
            PROGMAP::const_iterator it;
            for (Proc *p = prog->getFirstProc(it); p; p = prog->getNextProc(it))
            if (p->isLib())
                std::cout << "\t\t" << p->getName() << "\n";
            std::cout << "\n\tuserprocs:\n";
            for (Proc *p = prog->getFirstProc(it); p; p = prog->getNextProc(it))
            if (!p->isLib())
                std::cout << "\t\t" << p->getName() << "\n";
            std::cout << "\n";
            
            return 0;
        } else if (!strcmp(argv[1], "cluster")) {
            if (argc <= 2) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Cluster *cluster = prog->findCluster(argv[2]);
            if (cluster == NULL) {
                std::cerr << "cannot find cluster " << argv[2] << "\n";
                return 1;
            }

            std::cout << "cluster " << cluster->getName() << ":\n";
            if (cluster->getParent())
                std::cout << "\tparent = " << cluster->getParent()->getName() << "\n";
            else
                std::cout << "\troot cluster.\n";
            std::cout << "\tprocs:\n";
            PROGMAP::const_iterator it;
            for (Proc *p = prog->getFirstProc(it); p; p = prog->getNextProc(it))
                if (p->getCluster() == cluster)
                    std::cout << "\t\t" << p->getName() << "\n";
            std::cout << "\n";
            
            return 0;
        } else if (!strcmp(argv[1], "proc")) {
            if (argc <= 2) {
                std::cerr << "not enough arguments for cmd\n";
                return 1;
            }

            Proc *proc = prog->findProc(argv[2]);
            if (proc == NULL) {
                std::cerr << "cannot find proc " << argv[2] << "\n";
                return 1;
            }

            std::cout << "proc " << proc->getName() << ":\n";
            std::cout << "\tbelongs to cluster " << proc->getCluster()->getName() << "\n";
            std::cout << "\tnative address " << std::hex << proc->getNativeAddress() << std::dec << "\n";
            if (proc->isLib())
                std::cout << "\tis a library proc.\n";
            else {
                std::cout << "\tis a user proc.\n";
                UserProc *p = (UserProc*)proc;
                if (p->isDecoded())
                    std::cout << "\thas been decoded.\n";
                if (p->isAnalysed())
                    std::cout << "\thas been analysed.\n";
            }
            std::cout << "\n";

            return 0;
        } else {
            std::cerr << "don't know how to print info about a " << argv[1] << "\n";
            return 1;
        }
	} else if (!strcmp(argv[0], "print")) {
        if (argc <= 1) {
            std::cerr << "not enough arguments for cmd\n";
            return 1;
        }

        Proc *proc = prog->findProc(argv[1]);
        if (proc == NULL) {
            std::cerr << "cannot find proc " << argv[1] << "\n";
            return 1;
        }
        if (proc->isLib()) {
            std::cerr << "cannot print a libproc.\n";
            return 1;
        }

        ((UserProc*)proc)->print(std::cout);
        std::cout << "\n";
        return 0;
	} else if (!strcmp(argv[0], "exit")) {
	    return 2;
	} else if (!strcmp(argv[0], "quit")) {
	    return 2;
	} else if (!strcmp(argv[0], "help")) {
    	helpcmd();
	    return 0;
	} else {
    	std::cerr << "unknown cmd " << argv[0] << ".\n";
	    return 1;
	}

	return 0;
}

int Boomerang::cmdLine()
{
	char line[1024];
	printf("boomerang: ");
	fflush(stdout);
	while (fgets(line, sizeof(line), stdin)) {
		char **argv;
		int argc = splitLine(line, &argv);
		if (parseCmd(argc, (const char **)argv) == 2) 
			return 2;
		printf("boomerang: ");
		fflush(stdout);
	}
	return 0;
}

int Boomerang::commandLine(int argc, const char **argv) 
{
	if (argc < 2) usage();
#ifdef _MSC_VER				// For the console mode version; Windows GUI will override in windows.cpp
#ifndef MAX_PATH
#define MAX_PATH 132
#endif
	char filename[MAX_PATH];
	getcwd(filename, MAX_PATH-10);
	strcat(filename, "\\");
	progPath = filename;
	strcat(filename, "output");
	outputPath = filename;
#else
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
		progPath = "./";			// Just assume the current directory
	}
#endif
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

	for (int i=1; i < argc; i++) {
		if (argv[i][0] != '-' && i == argc - 1)
			break;
		if (argv[i][0] != '-')
			usage();
		switch (argv[i][1]) {
			case '-': break;		// No effect: ignored
			case 'h': help(); break;
			case 'v': vFlag = true; break;
			case 'x': dumpXML = true; break;
			case 'r': printRtl = true; break;
			case 't': traceDecoder = true; break;
			case 'T':
				if (argv[i][2] == 'c')
					conTypeAnalysis = true;		// -Tc: use old constraint-based type analysis
				else if (argv[i][2] == 'd')
					dfaTypeAnalysis = true;		// -Td: use data-flow-based type analysis
				break;
			case 'g': 
				dotFile = argv[++i];
				break;
			case 'o':
				outputPath = argv[++i];
				if (outputPath[outputPath.size()-1] != '/')
					outputPath += '/';
				break;
			case 'O': overlapped = true; break;
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
					case 'd':
						noDataflow = true;
						break;
					case 'D':
						noDecompile = true;
						break;
					case 'l':
						noLocals = true;
						break;
					case 'L':
						noLimitPropagations = true;
						break;
					case 'n':
						noRemoveNull = true;
						break;
					case 'm':
						decodeMain = false;
						break;
					case 'P':
						noPromote = true;
						break;
					case 'p':
						noParameterNames = true;
						break;
					case 'r':
						noRemoveLabels = true;
						break;
					case 'R':
						noRemoveReturns = true;
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
					case 'r':		// debug counting unused Returns
						debugUnusedRetsAndParams = true;
						break;
					case 't':		// debug type analysis
						debugTA = true;
						break;
					case 'u':		// debug unused locations (incl unused rets)
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
					decodeThruIndCall = true;		// -ic;
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
				kmd = 1;
				break;
			case 'P':
				progPath = argv[++i];
				if (progPath[progPath.length()-1] != '\\')
					progPath += "\\";
				break;
            case 'f':
                fastx86 = true;
                break;
			default:
				help();
		}
	}

	setOutputDirectory(outputPath.c_str());
	
	if (kmd)
		return cmdLine();

	return decompile(argv[argc-1]);	   
}

bool Boomerang::setOutputDirectory(const char *path)
{
	outputPath = path;
	// Create the output directory, if needed
	if (!createDirectory(outputPath)) {
		std::cerr << "Warning! Could not create path " <<
		  outputPath << "!\n";
		return false;
	}
	if (logger == NULL)
		setLogger(new FileLogger());
	return true;
}

void Boomerang::objcDecode(std::map<std::string, ObjcModule> &modules, Prog *prog)
{
    if (VERBOSE)
        LOG << "Adding Objective-C information to Prog.\n";
    Cluster *root = prog->getRootCluster();
    for (std::map<std::string, ObjcModule>::iterator it = modules.begin(); it != modules.end(); it++) {
        ObjcModule &mod = (*it).second;
        Module *module = new Module(mod.name.c_str());
        root->addChild(module);
        if (VERBOSE)
            LOG << "\tModule: " << mod.name.c_str() << "\n";
        for (std::map<std::string, ObjcClass>::iterator it1 = mod.classes.begin(); it1 != mod.classes.end(); it1++) {
            ObjcClass &c = (*it1).second;
            Class *cl = new Class(c.name.c_str());
            root->addChild(cl);
            if (VERBOSE)
                LOG << "\t\tClass: " << c.name.c_str() << "\n";
            for (std::map<std::string, ObjcMethod>::iterator it2 = c.methods.begin(); it2 != c.methods.end(); it2++) {
                ObjcMethod &m = (*it2).second;
                // TODO: parse :'s in names
                Proc *p = prog->newProc(m.name.c_str(), m.addr);
                p->setCluster(cl);
                // TODO: decode types in m.types
                if (VERBOSE)
                    LOG << "\t\t\tMethod: " << m.name.c_str() << "\n";
            }
        }
    }
    if (VERBOSE)
        LOG << "\n";
}

Prog *Boomerang::loadAndDecode(const char *fname, const char *pname)
{
	std::cerr << "loading...\n";
	FrontEnd *fe = FrontEnd::Load(fname);
	if (fe == NULL) {
		std::cerr << "failed.\n";
		return NULL;
	}
	Prog *prog = new Prog(fe->getBinaryFile(), fe);

	// Add symbols from -s switch(es)
	for (std::map<ADDRESS, std::string>::iterator it = symbols.begin();
		 it != symbols.end(); it++) {
		fe->AddSymbol((*it).first, (*it).second.c_str());
	}
	fe->readLibraryCatalog();		// Needed before readSymbolFile()

	for (unsigned i = 0; i < symbolFiles.size(); i++) {
		std::cerr << "reading symbol file " << symbolFiles[i].c_str() << "\n";
		prog->readSymbolFile(symbolFiles[i].c_str());
	}

    std::map<std::string, ObjcModule> &objcmodules = fe->getBinaryFile()->getObjcModules();
    if (objcmodules.size())
        objcDecode(objcmodules, prog);

	prog->setNextIsEntry();			// The next proc created will be designated the "entry point"
	
	if (decodeMain)
		std::cerr << "decoding...\n";
	fe->decode(prog, decodeMain, pname);

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

	Boomerang::get()->alert_end_decode();

	std::cerr << "found " << std::dec << prog->getNumUserProcs() << " procs\n";

	std::cerr << "analysing...\n";
	prog->analyse();

	prog->printCallGraph();
	prog->printCallGraphXML();
	return prog;
}

int Boomerang::decompile(const char *fname, const char *pname)
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
		prog = loadAndDecode(fname, pname);
		if (prog == NULL)
			return 1;
	}

	if (saveBeforeDecompile) {
		std::cerr << "saving persistable state...\n";
		XMLProgParser *p = new XMLProgParser();
		p->persistToXML(prog);
	}

	if (stopBeforeDecompile)
		return 0;

	std::cerr << "decompiling...\n";
    if (fastx86)
        prog->fastx86decompile();
	else
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

    std::cerr << "output written to " << outputPath;
#ifdef WIN32
    std::cerr << "\\";
#else
    std::cerr << "/";
#endif
    std::cerr << prog->getRootCluster()->getName() << "\n";

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

void Boomerang::persistToXML(Prog *prog)
{
	LOG << "saving persistable state...\n";
	XMLProgParser *p = new XMLProgParser();
	p->persistToXML(prog);
}

Prog *Boomerang::loadFromXML(const char *fname)
{
	LOG << "loading persistable state...\n";
	XMLProgParser *p = new XMLProgParser();
	return p->parse(fname);
}
