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

class Boomerang {
private:
    static Boomerang *boomerang;
    std::string progPath;   // String with the path to this exec

    void usage();
    void help();

    Boomerang();
public:
    static Boomerang *get() { 
        if (!boomerang) boomerang = new Boomerang(); 
	return boomerang;
    }

    static HLLCode *getHLLCode(UserProc *p = NULL);

    // performs command line operation
    int commandLine(int argc, const char **argv);
    void setProgPath(const char* p) { progPath = p; }
    const std::string& getProgPath() { return progPath; }

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
    bool debugDataflow;
    bool debugPrintReach;
    bool debugPrintSSA;
    int maxMemDepth;
    bool debugSwitch;
    bool noParameterNames;
    bool debugLiveness;
    bool debugUnusedRets;
};

#define VERBOSE Boomerang::get()->vFlag


#endif
