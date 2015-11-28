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

#include "config.h"
#include "types.h"
#include "IBoomerang.h"
#include "IProject.h"

#include <QObject>
#include <QDir>
#include <QTextStream>
#include <string>
#include <set>
#include <vector>
#include <map>

class QString;
class SeparateLogger;
class Log;
class Prog;
class Function;
class UserProc;
class HLLCode;
class ObjcModule;
class IBinaryImage;
class IBinarySymbolTable;
class Project;
enum LogLevel {
    LL_Debug = 0,
    LL_Default=1,
    LL_Warn = 2,
    LL_Error= 3,
};
#define LOG Boomerang::get()->log()
#define LOG_SEPARATE(x) Boomerang::get()->separate_log(x)
#define LOG_VERBOSE(x) Boomerang::get()->if_verbose_log(x)
#define LOG_STREAM Boomerang::get()->getLogStream

/// Virtual class to monitor the decompilation.
class Watcher {
public:
    Watcher() {}
    virtual ~Watcher() {} // Prevent gcc4 warning

    virtual void alert_complete() {}
    virtual void alertNew(Function *) {}
    virtual void alertRemove(Function *) {}
    virtual void alertUpdateSignature(Function *) {}
    virtual void alertDecode(ADDRESS /*pc*/, int /*nBytes*/) {}
    virtual void alertBadDecode(ADDRESS /*pc*/) {}
    virtual void alertStartDecode(ADDRESS /*start*/, int /*nBytes*/) {}
    virtual void alertEndDecode() {}
    virtual void alertDecode(Function *, ADDRESS /*pc*/, ADDRESS /*last*/, int /*nBytes*/) {}
    virtual void alertStartDecompile(UserProc *) {}
    virtual void alertProcStatusChange(UserProc *) {}
    virtual void alertDecompileSSADepth(UserProc *, int /*depth*/) {}
    virtual void alertDecompileBeforePropagate(UserProc *, int /*depth*/) {}
    virtual void alertDecompileAfterPropagate(UserProc *, int /*depth*/) {}
    virtual void alertDecompileAfterRemoveStmts(UserProc *, int /*depth*/) {}
    virtual void alertEndDecompile(UserProc *) {}
    virtual void alert_load(Function *) {}
    virtual void alertConsidering(Function * /*parent*/, Function *) {}
    virtual void alertDecompiling(UserProc *) {}
    virtual void alertDecompileDebugPoint(UserProc *, const char * /*description*/) {}
};

/**
 * Controls the loading, decoding, decompilation and code generation for a program.
 * This is the main class of the decompiler.
 */
class Boomerang : public QObject,public IBoomerang {
    Q_OBJECT
private:
    static Boomerang *boomerang;
    IBinaryImage *Image = nullptr;
    IBinarySymbolTable *Symbols = nullptr;
    QString progPath;               //!< String with the path to the boomerang executable.
    QString outputPath;             //!< The path where all output files are created.
    Log *logger = nullptr;          //!< Takes care of the log messages.
    std::set<Watcher *> watchers;   //!< The watchers which are interested in this decompilation.

    /* Documentation about a function should be at one place only
     * So: Document all functions at the point of implementation (in the .c file)
     */
    void helpcmd() const;
    Boomerang();
    virtual ~Boomerang();
    void miniDebugger(UserProc *p, const char *description);

public:
    /**
     * \return The global boomerang object. It will be created if it didn't already exist.
     */
    static Boomerang *get();
    IBinaryImage *getImage() override;
    IBinarySymbolTable *getSymbols() override;
    IProject *project()  override { return currentProject; }
    int processCommand(QStringList &args);
    static const char *getVersionStr();
    Log &log();
    SeparateLogger separate_log(const QString &);
    Log &if_verbose_log(int verbosity_level);
    void setLogger(Log *l);
    bool setOutputDirectory(const QString &path);

    HLLCode *getHLLCode(UserProc *p = nullptr);
    void setPluginPath(const QString &p);
    void setProgPath(const QString &p);
    /// Get the path to the %Boomerang executable.
    const QString &getProgPath() { return progPath; }
    /// Get the path to the %Boomerang executable.
    QDir getProgDir() { return QDir(progPath); }
    /// Set the path where the output files are saved.
    void setOutputPath(const QString &p) { outputPath = p; }
    /// Returns the path to where the output files are saved.
    const QString &getOutputPath() { return outputPath; }
    Prog *loadAndDecode(const QString &fname, const char *pname = nullptr);
    int decompile(const QString &fname, const char *pname = nullptr);
    /// Add a Watcher to the set of Watchers for this Boomerang object.
    void addWatcher(Watcher *watcher) { watchers.insert(watcher); }
    void persistToXML(Prog *prog);
    Prog *loadFromXML(const char *fname);
    void objcDecode(const std::map<QString, ObjcModule> &modules, Prog *prog);

    /// Alert the watchers that decompilation has completed.
    void alert_complete() {
        for (Watcher *it : watchers)
            it->alert_complete();
    }
    /// Alert the watchers we have found a new %Proc.
    void alertNew(Function *p) {
        for (Watcher *it : watchers)
            it->alertNew(p);
    }
    /// Alert the watchers we have removed a %Proc.
    void alertRemove(Function *p) {
        for (Watcher *it : watchers)
            it->alertRemove(p);
    }
    /// Alert the watchers we have updated this Procs signature
    void alertUpdateSignature(Function *p) {
        for (Watcher *it : watchers)
            it->alertUpdateSignature(p);
    }
    /// Alert the watchers we are currently decoding \a nBytes bytes at address \a pc.
    void alertDecode(ADDRESS pc, int nBytes) {
        for (Watcher *it : watchers)
            it->alertDecode(pc, nBytes);
    }
    /// Alert the watchers of a bad decode of an instruction at \a pc.
    void alertBadDecode(ADDRESS pc) {
        for (Watcher *it : watchers)
            it->alertBadDecode(pc);
    }
    /// Alert the watchers we have succesfully decoded this function
    void alertDecode(Function *p, ADDRESS pc, ADDRESS last, int nBytes) {
        for (Watcher *it : watchers)
            it->alertDecode(p, pc, last, nBytes);
    }
    /// Alert the watchers we have loaded the Proc.
    void alertLoad(Function *p) {
        for (Watcher *it : watchers)
            it->alert_load(p);
    }
    /// Alert the watchers we are starting to decode.
    void alertStartDecode(ADDRESS start, int nBytes) {
        for (Watcher *it : watchers)
            it->alertStartDecode(start, nBytes);
    }
    /// Alert the watchers we finished decoding.
    void alertEndDecode() {
        for (Watcher *it : watchers)
            it->alertEndDecode();
    }
    void alertStartDecompile(UserProc *p) {
        for (Watcher *it : watchers)
            it->alertStartDecompile(p);
    }
    void alertProcStatusChange(UserProc *p) {
        for (Watcher *it : watchers)
            it->alertProcStatusChange(p);
    }
    void alertDecompileSSADepth(UserProc *p, int depth) {
        for (Watcher *it : watchers)
            it->alertDecompileSSADepth(p, depth);
    }
    void alertDecompileBeforePropagate(UserProc *p, int depth) {
        for (Watcher *it : watchers)
            it->alertDecompileBeforePropagate(p, depth);
    }
    void alertDecompileAfterPropagate(UserProc *p, int depth) {
        for (Watcher *it : watchers)
            it->alertDecompileAfterPropagate(p, depth);
    }
    void alertDecompileAfterRemoveStmts(UserProc *p, int depth) {
        for (Watcher *it : watchers)
            it->alertDecompileAfterRemoveStmts(p, depth);
    }
    void alertEndDecompile(UserProc *p) {
        for (Watcher *it : watchers)
            it->alertEndDecompile(p);
    }
    void alertConsidering(Function *parent, Function *p) {
        for (Watcher *it : watchers)
            it->alertConsidering(parent, p);
    }
    void alertDecompiling(UserProc *p) {
        for (Watcher *it : watchers)
            it->alertDecompiling(p);
    }
    void alertDecompileDebugPoint(UserProc *p, const char *description);

    QTextStream &getLogStream(int level=LL_Default); //!< Return overall logging target
    QString filename() const;

    // Command line flags
    bool vFlag = false;
    bool debugSwitch = false;
    bool debugLiveness = false;
    bool debugTA = false;
    bool debugDecoder = false;
    bool debugProof = false;
    bool debugUnused = false;
    bool debugRangeAnalysis = false;
    bool printRtl = false;
    bool noBranchSimplify = false;
    bool noRemoveNull = false;
    bool noLocals = false;
    bool noRemoveLabels = false;
    bool noDataflow = false;
    bool noDecompile = false;
    bool stopBeforeDecompile = false;
    bool traceDecoder = false;
    /// The file in which the dotty graph is saved
    QString dotFile;
    int numToPropagate = -1;
    bool noPromote = false;
    bool propOnlyToAll = false;
    bool debugGen = false;
    int maxMemDepth = 99;
    bool noParameterNames = false;
    bool stopAtDebugPoints = false;
    /// When true, attempt to decode main, all children, and all procs.
    /// \a decodeMain is set when there are no -e or -E switches given
    bool decodeMain = true;
    bool printAST = false;
    bool dumpXML = false;
    bool noRemoveReturns = false;
    bool decodeThruIndCall = false;
    bool noDecodeChildren = false;
    bool loadBeforeDecompile = false;
    bool saveBeforeDecompile = false;
    bool noProve = false;
    bool noChangeSignatures = false;
    bool conTypeAnalysis = false;
    bool dfaTypeAnalysis = true;
    int propMaxDepth = 3; ///< Max depth of expression that'll be propagated to more than one dest
    bool generateCallGraph = false;
    bool generateSymbols = false;
    bool noGlobals = false;
    bool assumeABI = false;    ///< Assume ABI compliance
    bool experimental = false; ///< Activate experimental code. Caution!
    QTextStream LogStream;
    QTextStream ErrStream;
    std::vector<ADDRESS> entrypoints;       /// A vector which contains all know entrypoints for the Prog.
    std::vector<QString> symbolFiles;   /// A vector containing the names off all symbolfiles to load.
    std::map<ADDRESS, QString> symbols; /// A map to find a name by a given address.
    IProject *currentProject;
};

#define VERBOSE (Boomerang::get()->vFlag)
#define DEBUG_TA (Boomerang::get()->debugTA)
#define DEBUG_PROOF (Boomerang::get()->debugProof)
#define DEBUG_UNUSED (Boomerang::get()->debugUnused)
#define DEBUG_LIVENESS (Boomerang::get()->debugLiveness)
#define DEBUG_RANGE_ANALYSIS Boomerang::get()->debugRangeAnalysis
#define DFA_TYPE_ANALYSIS (Boomerang::get()->dfaTypeAnalysis)
#define CON_TYPE_ANALYSIS (Boomerang::get()->conTypeAnalysis)
#define ADHOC_TYPE_ANALYSIS (!Boomerang::get()->dfaTypeAnalysis && !Boomerang::get()->conTypeAnalysis)
#define DEBUG_GEN (Boomerang::get()->debugGen)
#define DUMP_XML (Boomerang::get()->dumpXML)
#define DEBUG_SWITCH (Boomerang::get()->debugSwitch)
#define EXPERIMENTAL (Boomerang::get()->experimental)

#endif
