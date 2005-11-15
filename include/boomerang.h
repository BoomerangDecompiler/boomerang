/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file	boomerang.h
 * \brief	Interface for the boomerang singleton object
 *
 * $Revision$	// 1.61.2.2
 * 04 Dec 2002: Trent: Created
 */

/** \mainpage Boomerang source documentation
 *
 * \section intro_sec Introduction
 *
 * Welcome to the Doxygen generated documentation for the source of the
 * Boomerang decompiler. If you have spent some time trying to figure out
 * the properties of an undocumented function or variable, why not document
 * it?
 *
 */


#ifndef BOOMERANG_H
#define BOOMERANG_H

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <map>

#include "types.h"

class Log;
class Prog;
class Proc;
class UserProc;
class HLLCode;
class ObjcModule;

#define LOG Boomerang::get()->log()
#define LOGTAIL Boomerang::get()->logTail()

class Watcher {
public:
		Watcher() { }
virtual	~Watcher() { };							// Prevent gcc4 warning

virtual void		alert_complete() { }
virtual void		alert_new(Proc *p) { }
virtual void		alert_update_signature(Proc *p) { }
virtual void		alert_decode(ADDRESS pc, int nBytes) { }
virtual void		alert_baddecode(ADDRESS pc) { }
virtual void		alert_start_decode(ADDRESS start, int nBytes) { }
virtual void		alert_end_decode() { }
virtual void		alert_decode(Proc *p, ADDRESS pc, ADDRESS last, int nBytes) { }
virtual void		alert_start_decompile(UserProc *p) { }
virtual void		alert_decompile_SSADepth(UserProc *p, int depth) { }
virtual void		alert_decompile_beforePropagate(UserProc *p, int depth) { }
virtual void		alert_decompile_afterPropagate(UserProc *p, int depth) { }
virtual void		alert_decompile_afterRemoveStmts(UserProc *p, int depth) { }
virtual void		alert_end_decompile(UserProc *p) { }
virtual void		alert_load(Proc *p) { }
};

/// The main class of the decompiler.
/**
 * Controls the loading, decoding, decompilation and code generation for a program.
 */
class Boomerang {
private:
static Boomerang *boomerang;
		/// String with the path to the boomerang executable.
		std::string	progPath;	// String with the path to this exec
		/// The path where all output files are created.
		std::string	outputPath;
		/// Takes care of the log messages.
		Log			*logger;
		/// The watchers which are interested in this decompilation.
		std::set<Watcher*> watchers;

		/// Print how to use this program.
		void		usage();
		/// Print switch help.
		void		help();
		/// Print help for the command (-k) mode.
		void		helpcmd();
		int			splitLine(char *line, char ***pargv);
		int			parseCmd(int argc, const char **argv);
		int			cmdLine();


		/// Simple constructor.
				Boomerang();
		/// The destructor is virtual to force this object to be created on the heap (with \em new).
virtual			~Boomerang() {}
public:
		/// Returns the global boomerang object.
static Boomerang *get() { 
				if (!boomerang) boomerang = new Boomerang(); 
				return boomerang;
			}

		/// Gets a reference to the Log object.
		Log			&log();
		/// Specify the logger to be used for this decompilation.
		void		setLogger(Log *l) { logger = l; }
		/// Set the output directory to the specified path.
		bool		setOutputDirectory(const char *path);

		/// Get the HLLCode for the specified UserProc.
		HLLCode		*getHLLCode(UserProc *p = NULL);

		/// Parse the command line for options.
		int			commandLine(int argc, const char **argv);
		/// Set the programs path to the given one.
		void		setProgPath(const char* p) { progPath = p; }
		/// Get the path to the %Boomerang executable.
		const std::string& getProgPath() { return progPath; }
		/// Set the path where the output files are saved.
		void		setOutputPath(const char* p) { outputPath = p; }
		/// Get the path to where the output files are saved.
		const std::string& getOutputPath() { return outputPath; }
		/// Load a program or library and decode it.
		Prog		*loadAndDecode(const char *fname, const char *pname = NULL);
		/// Decompile the specified program or library
		int			decompile(const char *fname, const char *pname = NULL);
		/// Add a Watcher to the set of %Watchers for this Boomerang object.
		void		addWatcher(Watcher *watcher) { watchers.insert(watcher); }
		/// Save the current state of this Prog to a XML file.
		void		persistToXML(Prog *prog);
		/// Load a Prog from a XML file.
		Prog		*loadFromXML(const char *fname);

		/// Special decoder for Objective-C.
		void		objcDecode(std::map<std::string, ObjcModule> &modules, Prog *prog);

		/// Alert the watchers that decompilation has completed.
		void		alert_complete() {
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_complete();
					}
		/// Alert the watchers we have found a new proc.
		void		alert_new(Proc *p) {
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_new(p);
					}
		/// Alert the watchers we have updated this Procs signature
		void		alert_update_signature(Proc *p) { 
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_update_signature(p);
					}
		/// Alert the watchers we are currently decoding \a nBytes bytes at \a address pc.
		void		alert_decode(ADDRESS pc, int nBytes) {
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_decode(pc, nBytes);
					}
		/// Alert the watchers of a bad decode of an instruction at \a pc.
		void		alert_baddecode(ADDRESS pc) {
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_baddecode(pc);
					}
		/// Alert the watchers we have succesfully decoded this function
		void		alert_decode(Proc *p, ADDRESS pc, ADDRESS last, int nBytes) {
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_decode(p, pc, last, nBytes);
					}
		/// Alert the watchers we have loaded the Proc.
		void		alert_load(Proc *p) {
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_load(p);
			}
		void		alert_start_decode(ADDRESS start, int nBytes) { 
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_start_decode(start, nBytes);
					}
		void		alert_end_decode() { 
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_end_decode();
					}
virtual	void		alert_start_decompile(UserProc *p) { 
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_start_decompile(p);
					}
virtual	void		alert_decompile_SSADepth(UserProc *p, int depth) {
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_decompile_SSADepth(p, depth);
			}
virtual	void		alert_decompile_beforePropagate(UserProc *p, int depth) {
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_decompile_beforePropagate(p, depth);
					}
virtual void		alert_decompile_afterPropagate(UserProc *p, int depth) {
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_decompile_afterPropagate(p, depth);
					}
virtual void		alert_decompile_afterRemoveStmts(UserProc *p, int depth) {
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_decompile_afterRemoveStmts(p, depth);
					}
virtual void		alert_end_decompile(UserProc *p) { 
						for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
							(*it)->alert_end_decompile(p);
			}

		// List the last few lines of the LOG to standard error
		void		logTail();

		// Command line flags
		bool		vFlag;
		bool		printRtl;
		bool		noBranchSimplify;
		bool		noRemoveNull;
		bool		noLocals;
		bool		noRemoveLabels;
		bool		noDataflow;
		bool		noDecompile;
		bool		stopBeforeDecompile;
		bool		traceDecoder;
		const char	*dotFile;
		int			numToPropagate;
		bool		noPromote;
		bool		propOnlyToAll;
		bool		debugGen;
		int			maxMemDepth;
		bool		debugSwitch;
		bool		noParameterNames;
		bool		debugLiveness;
		bool		debugTA;
		/// A vector which contains all know entrypoints for the Prog.
		std::vector<ADDRESS> entrypoints;
		/// A vector containing the names off all symbolfiles to load.
		std::vector<std::string> symbolFiles;
		/// A map to find a name by a given address.
		std::map<ADDRESS, std::string> symbols;
		// decodeMain is set when there are no -e or -E switches given
		bool		decodeMain;					// When true, attempt to decode main, all children, and all procs
		bool		printAST;
		bool		dumpXML;
		bool		noRemoveReturns;
		bool		debugDecoder;
		bool		decodeThruIndCall;
		std::ofstream* ofsIndCallReport;
		bool		noDecodeChildren;
		bool		debugProof;
		bool		debugUnused;
		bool		loadBeforeDecompile;
		bool		saveBeforeDecompile;
		bool		overlapped;
		bool		noProve;
		bool		noChangeSignatures;
		bool		conTypeAnalysis;
		bool		dfaTypeAnalysis;
		int			propMaxDepth;		// Max depth of expression that will be propagated to more than one dest
		bool		generateCallGraph;
		bool		generateSymbols;
		bool		noGlobals;
		bool		assumeABI;			// Assume ABI compliance
		bool		performCSE;			// Perform CSE
};

#define VERBOSE				(Boomerang::get()->vFlag)
#define DEBUG_TA			(Boomerang::get()->debugTA)
#define DEBUG_PROOF 		(Boomerang::get()->debugProof)
#define DEBUG_UNUSED 		(Boomerang::get()->debugUnused)
#define DEBUG_LIVENESS 		(Boomerang::get()->debugLiveness)
#define DFA_TYPE_ANALYSIS	(Boomerang::get()->dfaTypeAnalysis)
#define CON_TYPE_ANALYSIS	(Boomerang::get()->conTypeAnalysis)
#define ADHOC_TYPE_ANALYSIS	(!Boomerang::get()->dfaTypeAnalysis && !Boomerang::get()->conTypeAnalysis)
#define DEBUG_GEN			(Boomerang::get()->debugGen)
#define DUMP_XML			(Boomerang::get()->dumpXML)
#define DEBUG_SWITCH		(Boomerang::get()->debugSwitch)



#endif
