/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:		boomerang.h
 * OVERVIEW:	interface for the boomerang singleton object
 *============================================================================*/
/*
 * $Revision$
 * 04 Dec 2002: Trent: Created
 */

#ifndef BOOMERANG_H
#define BOOMERANG_H

#include <iostream>
#include <string>
#include <set>
#include "cfg.h"
#include "proc.h"
#include "hllcode.h"
#include "log.h"
#include "BinaryFile.h"

#define LOG Boomerang::get()->log()

class Watcher {
public:
		Watcher() { }

		virtual void alert_complete() { }
		virtual void alert_new(Proc *p) { }
		virtual void alert_update_signature(Proc *p) { }
		virtual void alert_decode(ADDRESS pc, int nBytes) { }
		virtual void alert_baddecode(ADDRESS pc) { }
		virtual void alert_start_decode(ADDRESS start, int nBytes) { }
		virtual void alert_end_decode() { }
		virtual void alert_decode(Proc *p, ADDRESS pc, ADDRESS last, int nBytes) { }
		virtual void alert_start_decompile(UserProc *p) { }
		virtual void alert_decompile_SSADepth(UserProc *p, int depth) { }
		virtual void alert_decompile_beforePropagate(UserProc *p, int depth) { }
		virtual void alert_decompile_afterPropagate(UserProc *p, int depth) { }
		virtual void alert_decompile_afterRemoveStmts(UserProc *p, int depth) { }
		virtual void alert_end_decompile(UserProc *p) { }
		virtual void alert_load(Proc *p) { }
};

class Boomerang {
private:
static Boomerang *boomerang;
	std::string progPath;	// String with the path to this exec
	std::string outputPath;
	Log		*logger;
	std::set<Watcher*> watchers;

	void	usage();
	void	help();
	void	helpcmd();
	int		splitLine(char *line, char ***pargv);
	int		parseCmd(int argc, const char **argv);
	int		cmdLine();


	Boomerang();
virtual ~Boomerang() {}
public:
static Boomerang *get() { 
			if (!boomerang) boomerang = new Boomerang(); 
			return boomerang;
		}

	Log		&log();
	void	setLogger(Log *l) { logger = l; }
	bool	setOutputDirectory(const char *path);

	HLLCode *getHLLCode(UserProc *p = NULL);

	// performs command line operation
	int		commandLine(int argc, const char **argv);
	void	setProgPath(const char* p) { progPath = p; }
	const	std::string& getProgPath() { return progPath; }
	void	setOutputPath(const char* p) { outputPath = p; }
	const	std::string& getOutputPath() { return outputPath; }
	Prog	*loadAndDecode(const char *fname, const char *pname = NULL);
	int		decompile(const char *fname, const char *pname = NULL);
	void	addWatcher(Watcher *watcher) { watchers.insert(watcher); }
	void	persistToXML(Prog *prog);
	Prog	*loadFromXML(const char *fname);

    // special decoder for Objective-C
    void    objcDecode(std::map<std::string, ObjcModule> &modules, Prog *prog);

	// call the watchers
	void alert_complete() {
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_complete();
	}
	void alert_new(Proc *p) {
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_new(p);
	}
	void alert_update_signature(Proc *p) { 
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_update_signature(p);
	}
	void alert_decode(ADDRESS pc, int nBytes) {
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_decode(pc, nBytes);
	}
	void alert_baddecode(ADDRESS pc) {
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_baddecode(pc);
	}
	void alert_decode(Proc *p, ADDRESS pc, ADDRESS last, int nBytes) {
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_decode(p, pc, last, nBytes);
	}
	void alert_load(Proc *p) {
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_load(p);
	}
	void alert_start_decode(ADDRESS start, int nBytes) { 
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_start_decode(start, nBytes);
	}
	void alert_end_decode() { 
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_end_decode();
	}
	virtual void alert_start_decompile(UserProc *p) { 
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_start_decompile(p);
	}
	virtual void alert_decompile_SSADepth(UserProc *p, int depth) {
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_decompile_SSADepth(p, depth);
	}
	virtual void alert_decompile_beforePropagate(UserProc *p, int depth) {
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_decompile_beforePropagate(p, depth);
	}
	virtual void alert_decompile_afterPropagate(UserProc *p, int depth) {
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_decompile_afterPropagate(p, depth);
	}
	virtual void alert_decompile_afterRemoveStmts(UserProc *p, int depth) {
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_decompile_afterRemoveStmts(p, depth);
	}
	virtual void alert_end_decompile(UserProc *p) { 
		for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
			(*it)->alert_end_decompile(p);
	}

	// Command line flags
	bool vFlag;
	bool printRtl;
	bool noBranchSimplify;
	bool noRemoveNull;
	bool noLocals;
	bool noRemoveLabels;
	bool noDataflow;
	bool noDecompile;
	bool stopBeforeDecompile;
    bool traceDecoder;
    const char *dotFile;
    int numToPropagate;
    bool noPromote;
    bool propOnlyToAll;
    bool debugGen;
    int maxMemDepth;
    bool debugSwitch;
    bool noParameterNames;
    bool debugLiveness;
    bool debugUnusedRetsAndParams;
    bool debugTA;
    std::vector<ADDRESS> entrypoints;
    std::vector<std::string> symbolFiles;
    std::map<ADDRESS, std::string> symbols;
    bool decodeMain;
    bool printAST;
    bool dumpXML;
    bool noRemoveReturns;
    bool debugDecoder;
    bool decodeThruIndCall;
    bool noDecodeChildren;
    bool debugProof;
    bool debugUnusedStmt;
    bool loadBeforeDecompile;
    bool saveBeforeDecompile;
    bool overlapped;
	bool noProve;
	bool noChangeSignatures;
	bool conTypeAnalysis;
	bool dfaTypeAnalysis;
	bool noLimitPropagations;
    bool fastx86;
    bool generateCallGraph;
	bool generateSymbols;
};

#define VERBOSE				(Boomerang::get()->vFlag)
#define DEBUG_TA			(Boomerang::get()->debugTA)
#define DEBUG_PROOF 		(Boomerang::get()->debugProof)
#define DEBUG_UNUSED_STMT 	(Boomerang::get()->debugUnusedStmt)
#define DEBUG_LIVENESS 		(Boomerang::get()->debugLiveness)
#define DFA_TYPE_ANALYSIS	(Boomerang::get()->dfaTypeAnalysis)
#define CON_TYPE_ANALYSIS	(Boomerang::get()->conTypeAnalysis)
#define ADHOC_TYPE_ANALYSIS	(!Boomerang::get()->dfaTypeAnalysis && !Boomerang::get()->conTypeAnalysis)
#define DEBUG_GEN			(Boomerang::get()->debugGen)
#define DUMP_XML			(Boomerang::get()->dumpXML)
#define DEBUG_SWITCH		(Boomerang::get()->debugSwitch)
#define DEBUG_UNUSED_RETS_PARAMS (Boomerang::get()->debugUnusedRetsAndParams)





#endif
