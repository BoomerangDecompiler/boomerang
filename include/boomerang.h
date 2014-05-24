/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file boomerang.h
 * \brief Interface for the boomerang singleton object.
 */

/** \mainpage Introduction
 *
 * \section Introduction
 *
 * Welcome to the Doxygen generated documentation for the
 * %Boomerang decompiler. Not all classes and functions have been documented
 * yet, but eventually they will. If you have figured out what a function is doing
 * please update the documentation and submit it as a patch.
 *
 * More information on the %Boomerang decompiler can be found at
 * http://boomerang.sourceforge.net.
 *
 */


#ifndef BOOMERANG_H
#define BOOMERANG_H

// Defines to control experimental features
#define USE_DOMINANCE_NUMS 1                // Set true to store a statement number that has dominance properties
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <map>

#include "types.h"
class SeparateLogger;
class Log;
class Prog;
class Proc;
class UserProc;
class HLLCode;
class ObjcModule;

#define LOG Boomerang::get()->log()
#define LOG_SEPARATE(x) Boomerang::get()->separate_log(x)
#define LOG_VERBOSE(x) Boomerang::get()->if_verbose_log(x)
#define LOGTAIL Boomerang::get()->logTail()

#define DEBUG_RANGE_ANALYSIS Boomerang::get()->debugRangeAnalysis

/// Virtual class to monitor the decompilation.
class Watcher {
public:
        Watcher() { }
virtual    ~Watcher() { }                            // Prevent gcc4 warning

virtual void        alert_complete() { }
virtual void        alert_new(Proc *) { }
virtual void        alertRemove(Proc *) { }
virtual void        alert_update_signature(Proc *) { }
virtual void        alert_decode(ADDRESS /*pc*/, int /*nBytes*/) { }
virtual void        alert_baddecode(ADDRESS /*pc*/) { }
virtual void        alert_start_decode(ADDRESS /*start*/, int /*nBytes*/) { }
virtual void        alert_end_decode() { }
virtual void        alert_decode(Proc *, ADDRESS /*pc*/, ADDRESS /*last*/, int /*nBytes*/) { }
virtual void        alert_start_decompile(UserProc *) { }
virtual void        alert_proc_status_change(UserProc *) { }
virtual void        alert_decompile_SSADepth(UserProc *, int /*depth*/) { }
virtual void        alert_decompile_beforePropagate(UserProc *, int /*depth*/) { }
virtual void        alert_decompile_afterPropagate(UserProc *, int /*depth*/) { }
virtual void        alert_decompile_afterRemoveStmts(UserProc *, int /*depth*/) { }
virtual void        alert_end_decompile(UserProc *) { }
virtual void        alert_load(Proc *) { }
virtual void        alert_considering(Proc */*parent*/, Proc *) { }
virtual void        alert_decompiling(UserProc *) { }
virtual void        alert_decompile_debug_point(UserProc *, const char */*description*/) { }
};

/**
 * Controls the loading, decoding, decompilation and code generation for a program.
 * This is the main class of the decompiler.
 */
class Boomerang {
private:
static  Boomerang *         boomerang;
        std::string         progPath;       //!< String with the path to the boomerang executable.
        std::string         outputPath;     //!< The path where all output files are created.
        Log *               logger;         //!< Takes care of the log messages.
        std::set<Watcher*>  watchers;       //!< The watchers which are interested in this decompilation.

        /* Documentation about a function should be at one place only
         * So: Document all functions at the point of implementation (in the .c file)
         */
        void                helpcmd() const;
                            Boomerang();
virtual                     ~Boomerang() {}
public:
        /**
         * \return The global boomerang object. It will be created if it didn't already exist.
         */
static  Boomerang *         get() {
                                if (!boomerang) boomerang = new Boomerang();
                                return boomerang;
                            }

        int                 processCommand(std::vector<std::string> &args);
static  const char *        getVersionStr();
        Log &               log();
        SeparateLogger      separate_log(const char *);
        Log &               if_verbose_log(int verbosity_level);
        void                setLogger(Log *l) { logger = l; }
        bool                setOutputDirectory(const std::string &path);


        HLLCode *           getHLLCode(UserProc *p = nullptr);
        void                setPluginPath(const std::string &p);
        void                setProgPath(const std::string &p);
        const std::string & getProgPath();
                            /// Set the path where the output files are saved.
        void                setOutputPath(const std::string & p) { outputPath = p; }
                            /// Returns the path to where the output files are saved.
        const std::string & getOutputPath() { return outputPath; }

        Prog *              loadAndDecode(const std::string &fname, const char *pname = nullptr);
        int                 decompile(const char *fname, const char *pname = nullptr);
                            /// Add a Watcher to the set of Watchers for this Boomerang object.
        void                addWatcher(Watcher *watcher) { watchers.insert(watcher); }
        void                persistToXML(Prog *prog);
        Prog *              loadFromXML(const char *fname);
        void                objcDecode(std::map<std::string, ObjcModule> &modules, Prog *prog);

        /// Alert the watchers that decompilation has completed.
        void                alert_complete() {
                                for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                                    (*it)->alert_complete();
                            }
        /// Alert the watchers we have found a new %Proc.
        void                alert_new(Proc *p) {
                                for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                                    (*it)->alert_new(p);
                            }
        /// Alert the watchers we have removed a %Proc.
        void                alertRemove(Proc *p) {
                                for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                                    (*it)->alertRemove(p);
                            }
        /// Alert the watchers we have updated this Procs signature
        void                alert_update_signature(Proc *p) {
                                for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                                    (*it)->alert_update_signature(p);
                            }
        /// Alert the watchers we are currently decoding \a nBytes bytes at address \a pc.
        void        alert_decode(ADDRESS pc, int nBytes) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_decode(pc, nBytes);
                    }
        /// Alert the watchers of a bad decode of an instruction at \a pc.
        void        alert_baddecode(ADDRESS pc) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_baddecode(pc);
                    }
        /// Alert the watchers we have succesfully decoded this function
        void        alert_decode(Proc *p, ADDRESS pc, ADDRESS last, int nBytes) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_decode(p, pc, last, nBytes);
                    }
        /// Alert the watchers we have loaded the Proc.
        void        alert_load(Proc *p) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_load(p);
            }
        /// Alert the watchers we are starting to decode.
        void        alert_start_decode(ADDRESS start, int nBytes) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_start_decode(start, nBytes);
                    }
        /// Alert the watchers we finished decoding.
        void        alert_end_decode() {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_end_decode();
                    }
virtual    void        alert_start_decompile(UserProc *p) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_start_decompile(p);
                    }
virtual void        alert_proc_status_change(UserProc *p) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_proc_status_change(p);
                    }
virtual    void        alert_decompile_SSADepth(UserProc *p, int depth) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_decompile_SSADepth(p, depth);
            }
virtual    void        alert_decompile_beforePropagate(UserProc *p, int depth) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_decompile_beforePropagate(p, depth);
                    }
virtual void        alert_decompile_afterPropagate(UserProc *p, int depth) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_decompile_afterPropagate(p, depth);
                    }
virtual void        alert_decompile_afterRemoveStmts(UserProc *p, int depth) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_decompile_afterRemoveStmts(p, depth);
                    }
virtual void        alert_end_decompile(UserProc *p) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_end_decompile(p);
                    }
virtual void        alert_considering(Proc *parent, Proc *p) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_considering(parent, p);
                    }
virtual void        alert_decompiling(UserProc *p) {
                        for (std::set<Watcher*>::iterator it = watchers.begin(); it != watchers.end(); it++)
                            (*it)->alert_decompiling(p);
                    }
virtual void        alert_decompile_debug_point(UserProc *p, const char *description);

        void        logTail();

        // Command line flags
        bool        vFlag               = false;
        bool        printRtl            = false;
        bool        noBranchSimplify    = false;
        bool        noRemoveNull        = false;
        bool        noLocals            = false;
        bool        noRemoveLabels      = false;
        bool        noDataflow          = false;
        bool        noDecompile         = false;
        bool        stopBeforeDecompile = false;
        bool        traceDecoder        = false;
        /// The file in which the dotty graph is saved
        std::string dotFile             = "";
        int         numToPropagate      = -1;
        bool        noPromote           = false;
        bool        propOnlyToAll       = false;
        bool        debugGen            = false;
        int         maxMemDepth         = 99;
        bool        debugSwitch         = false;
        bool        noParameterNames    = false;
        bool        debugLiveness       = false;
        bool        stopAtDebugPoints   = false;
        bool        debugTA             = false;
        /// When true, attempt to decode main, all children, and all procs.
        /// \a decodeMain is set when there are no -e or -E switches given
        bool        decodeMain          = true;
        bool        printAST            = false;
        bool        dumpXML             = false;
        bool        noRemoveReturns     = false;
        bool        debugDecoder        = false;
        bool        decodeThruIndCall   = false;
        std::ofstream* ofsIndCallReport = nullptr;
        bool        noDecodeChildren    = false;
        bool        debugProof          = false;
        bool        debugUnused         = false;
        bool        loadBeforeDecompile = false;
        bool        saveBeforeDecompile = false;
        bool        noProve             = false;
        bool        noChangeSignatures  = false;
        bool        conTypeAnalysis     = false;
        bool        dfaTypeAnalysis     = true;
        int         propMaxDepth        = 3;    ///< Max depth of expression that'll be propagated to more than one dest
        bool        generateCallGraph   = false;
        bool        generateSymbols     = false;
        bool        noGlobals           = false;
        bool        assumeABI           = false;///< Assume ABI compliance
        bool        experimental        = false;///< Activate experimental code. Caution!
        bool        debugRangeAnalysis  = false;
        std::vector<ADDRESS> entrypoints;   /// A vector which contains all know entrypoints for the Prog.
        std::vector<std::string> symbolFiles;   /// A vector containing the names off all symbolfiles to load.
        std::map<ADDRESS, std::string> symbols; /// A map to find a name by a given address.
};

#define VERBOSE                (Boomerang::get()->vFlag)
#define DEBUG_TA            (Boomerang::get()->debugTA)
#define DEBUG_PROOF         (Boomerang::get()->debugProof)
#define DEBUG_UNUSED         (Boomerang::get()->debugUnused)
#define DEBUG_LIVENESS         (Boomerang::get()->debugLiveness)
#define DFA_TYPE_ANALYSIS    (Boomerang::get()->dfaTypeAnalysis)
#define CON_TYPE_ANALYSIS    (Boomerang::get()->conTypeAnalysis)
#define ADHOC_TYPE_ANALYSIS    (!Boomerang::get()->dfaTypeAnalysis && !Boomerang::get()->conTypeAnalysis)
#define DEBUG_GEN            (Boomerang::get()->debugGen)
#define DUMP_XML            (Boomerang::get()->dumpXML)
#define DEBUG_SWITCH        (Boomerang::get()->debugSwitch)
#define EXPERIMENTAL        (Boomerang::get()->experimental)



#endif
