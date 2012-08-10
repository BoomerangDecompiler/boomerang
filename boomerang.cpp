/*
 * Copyright (C) 2002-2006, Mike Van Emmerik and Trent Waddington
 */
/*==============================================================================
 * FILE:	   boomerang.cpp
 * OVERVIEW:   Command line processing for the Boomerang decompiler
 *============================================================================*/
/*
 * $Revision$	// 1.115.2.5
 *
 * 28 Jan 05 - G. Krol: Separated -h output into sections and neatened
 * 02 Sep 06 - Mike: introduced USE_XML to make it easy to disable use of the expat library
*/

#define VERSION "alpha 0.3.1 09/Sep/2006"

#if __CYGWIN__
#define USE_XML 0			// Cygwin has a weird problem that causes libBinaryFile.dll not to load if the expat library
							// is used. Note that other Windows versions require expat.
#else						// For all platforms other than Cygwin:
#define USE_XML 1		// Set to 0 to not use the expat library for XML loading and saving
#endif

#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <fstream>
#include <time.h>
#ifdef _WIN32
#include <direct.h>			// mkdir under Windows
#else
#include <sys/stat.h>		// For mkdir
#include <unistd.h>			// For unlink
#include <signal.h>
#endif
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h>
#endif
#include "prog.h"
#include "proc.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "hllcode.h"
#include "codegen/chllcode.h"
//#include "transformer.h"
#include "boomerang.h"
#include "log.h"
#if USE_XML
#include "xmlprogparser.h"
#endif
#if defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(disable:4996)		// Warnings about e.g. _strdup deprecated in VS 2005
#endif


// For the -nG switch to disable the garbage collector
#include "gc.h"

Boomerang *Boomerang::boomerang = NULL;

/**
 * Initializes the Boomerang object.
 * The default settings are:
 * - All options disabled
 * - Infinite propagations
 * - A maximum memory depth of 99
 * - The path to the executable is "./"
 * - The output directory is "./output/"
 */
Boomerang::Boomerang() : logger(NULL), vFlag(false), printRtl(false), 
	noBranchSimplify(false), noRemoveNull(false), noLocals(false),
	noRemoveLabels(false), noDataflow(false), noDecompile(false), stopBeforeDecompile(false),
	traceDecoder(false), dotFile(NULL), numToPropagate(-1),
	noPromote(false), propOnlyToAll(false), debugGen(false),
	maxMemDepth(99), debugSwitch(false), noParameterNames(false), debugLiveness(false),
	stopAtDebugPoints(false), debugTA(false), decodeMain(true), printAST(false), dumpXML(false),
	noRemoveReturns(false), debugDecoder(false), decodeThruIndCall(false), ofsIndCallReport(NULL),
	noDecodeChildren(false), debugProof(false), debugUnused(false),
	loadBeforeDecompile(false), saveBeforeDecompile(false),
	noProve(false), noChangeSignatures(false), conTypeAnalysis(false), dfaTypeAnalysis(true),
	propMaxDepth(3), generateCallGraph(false), generateSymbols(false), noGlobals(false), assumeABI(false),
	experimental(false), minsToStopAfter(0)
{
	progPath = "./";
	outputPath = "./output/";
}

/**
 * Returns the Log object associated with the object.
 */
Log &Boomerang::log() {
	return *logger;
}

/**
 * Sets the outputfile to be the file "log" in the default output directory.
 */
FileLogger::FileLogger() : out((Boomerang::get()->getOutputPath() + "log").c_str()) {
}

/**
 * Returns the HLLCode for the given proc.
 */
HLLCode *Boomerang::getHLLCode(UserProc *p) {
	return new CHLLCode(p);
}

/**
 * Prints a short usage statement.
 */
void Boomerang::usage() {
	std::cout << "Usage: boomerang [ switches ] <program>" << std::endl;
	std::cout << "boomerang -h for switch help" << std::endl;
	exit(1);
}

/**
 * Prints help for the interactive mode.
 */
void Boomerang::helpcmd() {
	// Column 98 of this source file is column 80 of output (don't use tabs)
	//            ____.____1____.____2____.____3____.____4____.____5____.____6____.____7____.____8
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

/**
 * Prints help about the command line switches.
 */
void Boomerang::help() {
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
#if USE_XML
	std::cout << "  -LD              : Load before decompile (<program> becomes xml input file)\n";
	std::cout << "  -SD              : Save before decompile\n";
#endif
	std::cout << "  -a               : Assume ABI compliance\n";
	std::cout << "  -W               : Windows specific decompilation mode (requires pdb information)\n";
//	std::cout << "  -pa              : only propagate if can propagate to all\n";
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
//	std::cout << "  -nm              : No decoding of the 'main' procedure\n";
	std::cout << "  -ng              : No replacement of expressions with Globals\n";
	std::cout << "  -nG              : No garbage collection\n";
	std::cout << "  -nn              : No removal of NULL and unused statements\n";
	std::cout << "  -np              : No replacement of expressions with Parameter names\n";
	std::cout << "  -nP              : No promotion of signatures (other than main/WinMain/\n";
	std::cout << "                     DriverMain)\n";
	std::cout << "  -nr              : No removal of unneeded labels\n";
	std::cout << "  -nR              : No removal of unused Returns\n";
	std::cout << "  -l <depth>       : Limit multi-propagations to expressions with depth <depth>\n";
	std::cout << "  -p <num>         : Only do num propagations\n";
	std::cout << "  -m <num>         : Max memory depth\n";
	exit(1);
}
		
/**
 * Creates a directory and tests it.
 *
 * \param dir	The name of the directory.
 * 
 * \retval true The directory is valid.
 * \retval false The directory is invalid.
 */
bool createDirectory(std::string dir) {
	std::string remainder(dir);
	std::string path;
	size_t i;
	while ((i = remainder.find('/')) != std::string::npos) {
		path += remainder.substr(0, i+1);
		remainder = remainder.substr(i+1);
#ifdef _WIN32
		mkdir(path.c_str());
#else
		mkdir(path.c_str(), 0777);				// Doesn't matter if already exists
#endif
			}
	// Now try to create a test file
	path += remainder;
#ifdef _WIN32
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

/**
 * Prints a tree graph.
 */
void Cluster::printTree(std::ostream &out)
{
	out << "\t\t" << name << "\n";
	for (unsigned i = 0; i < children.size(); i++)
	children[i]->printTree(out);
}

typedef char *crazy_vc_bug;

/**
 * Splits a string up in different words.
 * use like: argc = splitLine(line, &argv);
 *
 * \param[in] line		the string to parse
 * \param[out] pargc	&argv
 *
 * \return The number of words found (argc).
 */
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

/**
 * Parse and execute a command supplied in interactive mode.
 *
 * \param argc		The number of arguments.
 * \param argv		Pointers to the arguments.
 *
 * \return A value indicating what happened.
 *
 * \retval 0 Success
 * \retval 1 Faillure
 * \retval 2 The user exited with \a quit or \a exit
 */
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
#if USE_XML
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
			pr = p->parse((outputPath + fname + "/" + fname + ".xml").c_str());
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
#endif
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
			int indent = 0;
			((UserProc*)proc)->decompile(new ProcList, indent);
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
				//if (p->isAnalysed())
				//	std::cout << "\thas been analysed.\n";
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

/**
 * Displays a command line and processes the commands entered.
 *
 * \retval 0 stdin was closed.
 * \retval 2 The user typed exit or quit.
 */
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

/**
 * The main function for the command line mode. Parses switches and runs decompile(filename).
 *
 * \return Zero on success, nonzero on faillure.
 */
int Boomerang::commandLine(int argc, const char **argv) 
{
	printf("Boomerang %s\n", VERSION);		// Display a version and date (mainly for release versions)
	if (argc < 2) usage();
	progPath = argv[0];
	size_t j = progPath.rfind('/');			// Chop off after the last slash
	if (j == (size_t)-1) 
		j = progPath.rfind('\\');			// .. or reverse slash
	if (j != (size_t)-1) {
		// Do the chop; keep the trailing slash or reverse slash
		progPath = progPath.substr(0, j+1);
	}
	else {
		progPath = "./";			// Just assume the current directory
	}
#ifdef _MSC_VER						// For the console mode version; Windows GUI will override in windows.cpp
	// As a special case for MSVC testing, make the program path the parent of the dir with the .exe
	j = progPath.find("ebug\\", progPath.length() - (4+1));
	if (j != std::string::npos)
		j--;			// Point to the 'd' or 'D'
	if (j == std::string::npos) {
			j = progPath.rfind("elease\\", progPath.length() - (6+1));
			if (j != std::string::npos)
				j--;			// Point to the 'r' or 'R'
	}
	if (j != std::string::npos)
		progPath = progPath.substr(0, j);			// Chop off "Release\" or "Debug\"
	SetCurrentDirectoryA(progPath.c_str());			// Note: setcwd() doesn't seem to work
#endif
	outputPath = progPath + "output/";				// Default output path (can be overridden with -o below)

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
			case 'X': experimental = true;
				std::cout << "Warning: experimental code active!\n"; break;
			case 'r': printRtl = true; break;
			case 't': traceDecoder = true; break;
			case 'T':
				if (argv[i][2] == 'c') {
					conTypeAnalysis = true;		// -Tc: use old constraint-based type analysis
					dfaTypeAnalysis = false;
				}
				else if (argv[i][2] == 'd')
					dfaTypeAnalysis = true;		// -Td: use data-flow-based type analysis (now default)
				break;
			case 'g': 
				if(argv[i][2]=='d')
					dotFile = argv[++i];
				else if(argv[i][2]=='c')
					generateCallGraph=true;
				else if(argv[i][2]=='s') {
					generateSymbols=true;
					stopBeforeDecompile=true;
				}
				break;
			case 'o': {
				outputPath = argv[++i];
				char lastCh = outputPath[outputPath.size()-1];
				if (lastCh != '/' && lastCh != '\\')
					outputPath += '/';		// Maintain the convention of a trailing slash
				break;
			}
			case 'p':
				if (argv[i][2] == 'a') {
					propOnlyToAll = true;
					std::cerr << " * * Warning! -pa is not implemented yet!\n";
				}
				else {
					if (++i == argc) {
						usage();
						return 1;
					}
					sscanf(argv[i], "%i", &numToPropagate);
				}
				break;
			case 'n':
				switch(argv[i][2]) {
					case 'b':
						noBranchSimplify = true;
						break;
					case 'c':
						noDecodeChildren = true;
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
					case 'n':
						noRemoveNull = true;
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
					case 'g':
						noGlobals = true;
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
			case 'E':
				noDecodeChildren = true;
				// Fall through
			case 'e':
				{
					ADDRESS addr;
					int n;
					decodeMain = false;
					if (++i == argc) {
						usage();
						return 1;
					}
					if (argv[i][0] == '0' && argv[i+1][1] == 'x') {
						n = sscanf(argv[i], "0x%x", &addr);
					} else {
						n = sscanf(argv[i], "%i", &addr);
					}
					if (n != 1) {
						std::cerr << "bad address: " << argv[i] << std::endl;
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
					if (++i == argc) {
						usage();
						return 1;
					}
					if (argv[i][0] == '0' && argv[i+1][1] == 'x') {
						n = sscanf(argv[i], "0x%x", &addr);
					} else {
						n = sscanf(argv[i], "%i", &addr);
					}
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
					case 's':
						stopAtDebugPoints = true;
						break;
					case 't':		// debug type analysis
						debugTA = true;
						break;
					case 'u':		// debug unused locations (including returns and parameters now)
						debugUnused = true;
						break;
					default:
						help();
				}
				break;
			case 'm':
				if (++i == argc) {
					usage();
					return 1;
				}
				sscanf(argv[i], "%i", &maxMemDepth);
				break;
			case 'i':
				if (argv[i][2] == 'c')
					decodeThruIndCall = true;		// -ic;
				if (argv[i][2] == 'w')				// -iw
					if (ofsIndCallReport) {
						std::string fname = getOutputPath() + "indirect.txt";
						ofsIndCallReport = new std::ofstream(fname.c_str());
					}
				break;
			case 'L':
				if (argv[i][2] == 'D')
					#if USE_XML
					loadBeforeDecompile = true;
					#else
					std::cerr << "LD command not enabled since compiled without USE_XML\n";
					#endif
				break;
			case 'S':
				if (argv[i][2] == 'D')
					#if USE_XML
					saveBeforeDecompile = true;
					#else
					std::cerr << "SD command not enabled since compiled without USE_XML\n";
					#endif
				else {
					sscanf(argv[++i], "%i", &minsToStopAfter);					
				}
				break;
			case 'k':
				kmd = 1;
				break;
			case 'P':
				progPath = argv[++i];
				if (progPath[progPath.length()-1] != '\\')
					progPath += "\\";
				break;
			case 'a':
				assumeABI = true;
				break;
			case 'l':
				if (++i == argc) {
					usage();
					return 1;
				}
				sscanf(argv[i], "%i", &propMaxDepth);
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

/**
 * Sets the directory in which Boomerang creates its output files.  The directory will be created if it doesn't exist.
 *
 * \param path		the path to the directory
 *
 * \retval true Success.
 * \retval false The directory could not be created.
 */
bool Boomerang::setOutputDirectory(const char *path)
{
	outputPath = path;
	// Create the output directory, if needed
	if (!createDirectory(outputPath)) {
		std::cerr << "Warning! Could not create path " << outputPath << "!\n";
		return false;
	}
	if (logger == NULL)
		setLogger(new FileLogger());
	return true;
}

/**
 * Adds information about functions and classes from Objective-C modules to the Prog object.
 *
 * \param modules A map from name to the Objective-C modules.
 * \param prog The Prog object to add the information to.
 */
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

/**
 * Loads the executable file and decodes it.
 *
 * \param fname The name of the file to load.
 * \param pname How the Prog will be named.
 *
 * \returns A Prog object.
 */
Prog *Boomerang::loadAndDecode(const char *fname, const char *pname)
{
	std::cout << "loading...\n";
	Prog *prog = new Prog();
	FrontEnd *fe = FrontEnd::Load(fname, prog);
	if (fe == NULL) {
		std::cerr << "failed.\n";
		return NULL;
	}
	prog->setFrontEnd(fe);

	// Add symbols from -s switch(es)
	for (std::map<ADDRESS, std::string>::iterator it = symbols.begin();
		 it != symbols.end(); it++) {
		fe->AddSymbol((*it).first, (*it).second.c_str());
	}
	fe->readLibraryCatalog();		// Needed before readSymbolFile()

	for (unsigned i = 0; i < symbolFiles.size(); i++) {
		std::cout << "reading symbol file " << symbolFiles[i].c_str() << "\n";
		prog->readSymbolFile(symbolFiles[i].c_str());
	}

	std::map<std::string, ObjcModule> &objcmodules = fe->getBinaryFile()->getObjcModules();
	if (objcmodules.size())
		objcDecode(objcmodules, prog);

	// Entry points from -e (and -E) switch(es)
	for (unsigned i = 0; i < entrypoints.size(); i++) {
		std::cout<< "decoding specified entrypoint " << std::hex << entrypoints[i] << "\n";
		prog->decodeEntryPoint(entrypoints[i]);
	}

	if (entrypoints.size() == 0) {		// no -e or -E given
		if (decodeMain)
			std::cout << "decoding entry point...\n";
		fe->decode(prog, decodeMain, pname);

		if (!noDecodeChildren) {
			// this causes any undecoded userprocs to be decoded
			std::cout << "decoding anything undecoded...\n";
			fe->decode(prog, NO_ADDRESS);
		}
	}

	std::cout << "finishing decode...\n";
	prog->finishDecode();

	Boomerang::get()->alert_end_decode();

	std::cout << "found " << std::dec << prog->getNumUserProcs() << " procs\n";

	// GK: The analysis which was performed was not exactly very "analysing", and so it has been moved to
	// prog::finishDecode, UserProc::assignProcsToCalls and UserProc::finalSimplify
	//std::cout << "analysing...\n";
 	//prog->analyse();

	if (generateSymbols) {
		prog->printSymbolsToFile();
	}
	if (generateCallGraph) {
		prog->printCallGraph();
		prog->printCallGraphXML();
	}
	return prog;
}

#if defined(_WIN32) && !defined(__MINGW32__)
DWORD WINAPI stopProcess(
    time_t start
)
{
	int mins = Boomerang::get()->minsToStopAfter;
	while(1) {
		time_t now;
		time(&now);
		if ((now - start) > mins * 60) {
			std::cerr << "\n\n Stopping process, timeout.\n";
			ExitProcess(1);
		}
		Sleep(1000);
	}
}
#else
void stopProcess(int n)
{
	std::cerr << "\n\n Stopping process, timeout.\n";
	exit(1);
}
#endif

/**
 * The program will be subsequently be loaded, decoded, decompiled and written to a source file.
 * After decompilation the elapsed time is printed to std::cerr.
 *
 * \param fname The name of the file to load.
 * \param pname The name that will be given to the Proc.
 *
 * \return Zero on success, nonzero on faillure.
 */
int Boomerang::decompile(const char *fname, const char *pname)
{
	Prog *prog;
	time_t start;
	time(&start);

	if (minsToStopAfter) {
		std::cout << "stopping decompile after " << minsToStopAfter << " minutes.\n";
#if defined(_WIN32) 			// Includes MinGW
		DWORD id;
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)stopProcess, (LPVOID)start, 0, &id);
#else
		signal(SIGALRM, stopProcess);
		alarm(minsToStopAfter * 60);
#endif
	}

//	std::cout << "setting up transformers...\n";
//	ExpTransformer::loadAll();

#if USE_XML
	if (loadBeforeDecompile) {
		std::cout << "loading persisted state...\n";
		XMLProgParser *p = new XMLProgParser();
		prog = p->parse(fname);
	} else
#endif
	{
		prog = loadAndDecode(fname, pname);
		if (prog == NULL)
			return 1;
	}

#if USE_XML
	if (saveBeforeDecompile) {
		std::cout << "saving persistable state...\n";
		XMLProgParser *p = new XMLProgParser();
		p->persistToXML(prog);
	}
#endif

	if (stopBeforeDecompile)
		return 0;

	std::cout << "decompiling...\n";
	prog->decompile();

	if (dotFile)
		prog->generateDotFile();

	if (printAST) {
		std::cout << "printing AST...\n";
		PROGMAP::const_iterator it;
		for (Proc *p = prog->getFirstProc(it); p; p = prog->getNextProc(it))
			if (!p->isLib()) {
				UserProc *u = (UserProc*)p;
				u->getCFG()->compressCfg();
				u->printAST();
			}
	}

	std::cout << "generating code...\n";
	prog->generateCode();

	std::cout << "output written to " << outputPath << prog->getRootCluster()->getName() << "\n";

	if (Boomerang::get()->ofsIndCallReport)
		ofsIndCallReport->close();

	time_t end;
	time(&end);
	int hours = (int)((end-start) / 60 / 60);
	int mins = (int)((end-start) / 60 - hours * 60);
	int secs = (int)((end-start) - hours * 60 * 60 - mins * 60);
	std::cout << "completed in " << std::dec;
	if (hours)
		std::cout << hours << " hours ";
	if (hours || mins)
		std::cout << mins << " mins ";
	std::cout << secs << " sec" << (secs == 1 ? "" : "s") << ".\n";

	return 0;
}

#if USE_XML
/**
 * Saves the state of the Prog object to a XML file.
 * \param prog The Prog object to save.
 */
void Boomerang::persistToXML(Prog *prog)
{
	LOG << "saving persistable state...\n";
	XMLProgParser *p = new XMLProgParser();
	p->persistToXML(prog);
}
/**
 * Loads the state of a Prog object from a XML file.
 * \param fname The name of the XML file.
 * \return The loaded Prog object.
 */
Prog *Boomerang::loadFromXML(const char *fname)
{
	LOG << "loading persistable state...\n";
	XMLProgParser *p = new XMLProgParser();
	return p->parse(fname);
}
#endif

/**
 * Prints the last lines of the log file.
 */
void Boomerang::logTail() {
	logger->tail();
}

void Boomerang::alert_decompile_debug_point(UserProc *p, const char *description) {
	if (stopAtDebugPoints) {
		std::cout << "decompiling " << p->getName() << ": " << description << "\n";
		static char *stopAt = NULL;
		static std::set<Statement*> watches;
		if (stopAt == NULL || !strcmp(p->getName(), stopAt)) {
			// This is a mini command line debugger.  Feel free to expand it.
			for (std::set<Statement*>::iterator it = watches.begin(); it != watches.end(); it++) {
				(*it)->print(std::cout);
				std::cout << "\n";
			}
			std::cout << " <press enter to continue> \n";
			char line[1024];
			while(1) {
				*line = 0;
				fgets(line, 1024, stdin);
				if (!strncmp(line, "print", 5))
					p->print(std::cout);
				else if (!strncmp(line, "fprint", 6)) {
					std::ofstream of("out.proc");
					p->print(of);
					of.close();
				} else if (!strncmp(line, "run ", 4)) {
					stopAt = strdup(line + 4);
					if (strchr(stopAt, '\n'))
						*strchr(stopAt, '\n') = 0;
					if (strchr(stopAt, ' '))
						*strchr(stopAt, ' ') = 0;
					break;
				} else if (!strncmp(line, "watch ", 6)) {
					int n = atoi(line + 6);
					StatementList stmts;
					p->getStatements(stmts);
					StatementList::iterator it;
					for (it = stmts.begin(); it != stmts.end(); it++) 
						if ((*it)->getNumber() == n) {
							watches.insert(*it);
							std::cout << "watching " << *it << "\n";
						}
				} else
					break;
			}
		}
	}
	for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
		(*it)->alert_decompile_debug_point(p, description);
}

char* Boomerang::getVersionStr() {
	return const_cast<char *>(VERSION);
}
