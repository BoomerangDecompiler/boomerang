/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:        boomerang.h
 * OVERVIEW:    interface for the boomerang singleton object
 *============================================================================*/
/*
 * $Revision$
 * 04 Dec 2002: Trent: Created
 */

#ifndef BOOMERANG_H
#define BOOMERANG_H

#include <iostream>
#include <string>
#include "cfg.h"
#include "proc.h"
#include "hllcode.h"
#include "log.h"

#define LOG Boomerang::get()->log()

class Boomerang {
private:
    static Boomerang *boomerang;
    std::string progPath;   // String with the path to this exec
    std::string outputPath;
    Log *logger;

    void usage();
    void help();

    Boomerang();
public:
    static Boomerang *get() { 
        if (!boomerang) boomerang = new Boomerang(); 
	return boomerang;
    }

    Log &log();
    void setLogger(Log *l) { logger = l; }

    HLLCode *getHLLCode(UserProc *p = NULL);

    // performs command line operation
    int commandLine(int argc, const char **argv);
    void setProgPath(const char* p) { progPath = p; }
    const std::string& getProgPath() { return progPath; }
    void setOutputPath(const char* p) { outputPath = p; }
    const std::string& getOutputPath() { return outputPath; }
    int decompile(const char *fname);

    // Command line flags
    bool vFlag;
    bool printRtl;
    bool noBranchSimplify;
    bool noRemoveNull;
    bool noLocals;
    bool noRemoveLabels;
    bool noDataflow;
    bool noDecompile;
    bool noDecompileUp;
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
    bool debugUnusedRets;
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
};

#define VERBOSE  (Boomerang::get()->vFlag)
#define DEBUG_TA (Boomerang::get()->debugTA)


#endif
