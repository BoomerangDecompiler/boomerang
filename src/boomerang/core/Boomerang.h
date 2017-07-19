#pragma once

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

#include "boomerang/core/IBoomerang.h"
#include "boomerang/db/IProject.h"
#include "boomerang/core/Watcher.h"

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
class ICodeGenerator;
class ObjcModule;
class IBinaryImage;
class IBinarySymbolTable;
class Project;

enum LogLevel
{
    LL_Debug   = 0,
    LL_Default = 1,
    LL_Warn    = 2,
    LL_Error   = 3,
};

#define LOG                Boomerang::get()->log()
#define LOG_SEPARATE(x)    Boomerang::get()->separate_log(x)
#define LOG_VERBOSE(x)     Boomerang::get()->if_verbose_log(x)
#define LOG_STREAM         Boomerang::get()->getLogStream


/**
 * Controls the loading, decoding, decompilation and code generation for a program.
 * This is the main class of the decompiler.
 */
class Boomerang : public QObject, public IBoomerang
{
    Q_OBJECT

private:
    static Boomerang *boomerang; ///< the instance

    IBinarySymbolTable *m_symbols = nullptr;
    QString m_workingDirectory;       ///< String with the path to the boomerang executable.
    QString m_outputDirectory;        ///< The path where all output files are created.
    Log *m_logger = nullptr;          ///< Takes care of the log messages.
    std::set<Watcher *> m_watchers;   ///< The watchers which are interested in this decompilation.

    /// Prints help for the interactive mode.
    void helpcmd() const;

    /**
     * Initializes the Boomerang object.
     * The default settings are:
     * - All options disabled
     * - Infinite propagations
     * - A maximum memory depth of 99
     * - The path to the executable is "./"
     * - The output directory is "./output/"
     * - Main log stream is output on stderr
     */
    Boomerang();
    virtual ~Boomerang();

    /// This is a mini command line debugger.  Feel free to expand it.
    void miniDebugger(UserProc *p, const char *description);

public:

    /// \returns The global boomerang object. It will be created if it does not already exist.
    static Boomerang *get();

    IBinaryImage *getImage() override;
    IBinarySymbolTable *getSymbols() override;

    IProject *getProject() override { return m_currentProject; }

    /**
     * Parse and execute a command supplied in interactive mode.
     *
     * \param args        The array of argument strings.
     *
     * \return A value indicating what happened.
     *
     * \retval 0 Success
     * \retval 1 Failure
     * \retval 2 The user exited with \a quit or \a exit
     */
    int processCommand(QStringList& args);
    static const char *getVersionStr();

    /// \returns the Log object associated with the object.
    Log& log();

    SeparateLogger separate_log(const QString&);
    Log& if_verbose_log(int verbosity_level);

    void setLogger(Log *l);

    /**
     * Sets the directory in which Boomerang creates its output files.  The directory will be created if it doesn't exist.
     *
     * \param path        the path to the directory
     *
     * \retval true Success.
     * \retval false The directory could not be created.
     */
    bool setOutputDirectory(const QString& path);

    /**
     * Returns the ICodeGenerator for the given proc.
     * \return The ICodeGenerator for the specified UserProc.
     */
    ICodeGenerator *getCodeGenerator(UserProc *p = nullptr);

    /// Set the path where the %Boomerang executable will search for plugins.
    void setPluginPath(const QString& p);

    /// Set the path to the %Boomerang executable.
    void setWorkingDirectory(const QString& p);

    /// Get the path to the %Boomerang executable.
    const QString& getWorkingDirectory() const { return m_workingDirectory; }

    /// Get the path to the %Boomerang executable.
    QDir getProgDir() const { return QDir(m_workingDirectory); }

    /// Set the path where the output files are saved.
    void setOutputPath(const QString& p) {
        m_outputDirectory = p; }

    /// Returns the path to where the output files are saved.
    const QString& getOutputPath() { return m_outputDirectory; }

    /**
     * Loads the executable file and decodes it.
     *
     * \param fname The name of the file to load.
     * \param pname How the Prog will be named.
     *
     * \returns A Prog object.
     */
    Prog *loadAndDecode(const QString& fname, const char *pname = nullptr);

    /**
     * The program will be subsequently be loaded, decoded, decompiled and written to a source file.
     * After decompilation the elapsed time is printed to LOG_STREAM().
     *
     * \param fname The name of the file to load.
     * \param pname The name that will be given to the Proc.
     *
     * \return Zero on success, nonzero on faillure.
     */
    int decompile(const QString& fname, const char *pname = nullptr);

    /// Add a Watcher to the set of Watchers for this Boomerang object.
    void addWatcher(Watcher *watcher) { m_watchers.insert(watcher); }

    /**
     * Adds information about functions and classes from Objective-C modules to the Prog object.
     *
     * \param modules A map from name to the Objective-C modules.
     * \param prog The Prog object to add the information to.
     */
    void objcDecode(const std::map<QString, ObjcModule>& modules, Prog *prog);

    /// Alert the watchers that decompilation has completed.
    void alert_complete()
    {
        for (Watcher *it : m_watchers) {
            it->alert_complete();
        }
    }

    /// Alert the watchers we have found a new %Proc.
    void alertNew(Function *p)
    {
        for (Watcher *it : m_watchers) {
            it->alertNew(p);
        }
    }

    /// Alert the watchers we have removed a %Proc.
    void alertRemove(Function *p)
    {
        for (Watcher *it : m_watchers) {
            it->alertRemove(p);
        }
    }

    /// Alert the watchers we have updated this Procs signature
    void alertUpdateSignature(Function *p)
    {
        for (Watcher *it : m_watchers) {
            it->alertUpdateSignature(p);
        }
    }

    /// Alert the watchers we are currently decoding \a nBytes bytes at address \a pc.
    void alertDecode(Address pc, int nBytes)
    {
        for (Watcher *it : m_watchers) {
            it->alertDecode(pc, nBytes);
        }
    }

    /// Alert the watchers of a bad decode of an instruction at \a pc.
    void alertBadDecode(Address pc)
    {
        for (Watcher *it : m_watchers) {
            it->alertBadDecode(pc);
        }
    }

    /// Alert the watchers we have succesfully decoded this function
    void alertDecode(Function *p, Address pc, Address last, int nBytes)
    {
        for (Watcher *it : m_watchers) {
            it->alertDecode(p, pc, last, nBytes);
        }
    }

    /// Alert the watchers we have loaded the Proc.
    void alertLoad(Function *p)
    {
        for (Watcher *it : m_watchers) {
            it->alert_load(p);
        }
    }

    /// Alert the watchers we are starting to decode.
    void alertStartDecode(Address start, int nBytes)
    {
        for (Watcher *it : m_watchers) {
            it->alertStartDecode(start, nBytes);
        }
    }

    /// Alert the watchers we finished decoding.
    void alertEndDecode()
    {
        for (Watcher *it : m_watchers) {
            it->alertEndDecode();
        }
    }

    void alertStartDecompile(UserProc *p)
    {
        for (Watcher *it : m_watchers) {
            it->alertStartDecompile(p);
        }
    }

    void alertProcStatusChange(UserProc *p)
    {
        for (Watcher *it : m_watchers) {
            it->alertProcStatusChange(p);
        }
    }

    void alertDecompileSSADepth(UserProc *p, int depth)
    {
        for (Watcher *it : m_watchers) {
            it->alertDecompileSSADepth(p, depth);
        }
    }

    void alertDecompileBeforePropagate(UserProc *p, int depth)
    {
        for (Watcher *it : m_watchers) {
            it->alertDecompileBeforePropagate(p, depth);
        }
    }

    void alertDecompileAfterPropagate(UserProc *p, int depth)
    {
        for (Watcher *it : m_watchers) {
            it->alertDecompileAfterPropagate(p, depth);
        }
    }

    void alertDecompileAfterRemoveStmts(UserProc *p, int depth)
    {
        for (Watcher *it : m_watchers) {
            it->alertDecompileAfterRemoveStmts(p, depth);
        }
    }

    void alertEndDecompile(UserProc *p)
    {
        for (Watcher *it : m_watchers) {
            it->alertEndDecompile(p);
        }
    }

    void alertConsidering(Function *parent, Function *p)
    {
        for (Watcher *it : m_watchers) {
            it->alertConsidering(parent, p);
        }
    }

    void alertDecompiling(UserProc *p)
    {
        for (Watcher *it : m_watchers) {
            it->alertDecompiling(p);
        }
    }

    void alertDecompileDebugPoint(UserProc *p, const char *description);

    /// Return TextStream to which given \a level of messages shoudl be directed
    /// \param level - describes the message level TODO: describe message levels
    QTextStream& getLogStream(int level = LL_Default); ///< Return overall logging target

    QString filename() const;

public:
    // Command line flags
    bool vFlag               = false;
    bool debugSwitch         = false;
    bool debugLiveness       = false;
    bool debugTA             = false;
    bool debugDecoder        = false;
    bool debugProof          = false;
    bool debugUnused         = false;
    bool debugRangeAnalysis  = false;
    bool printRtl            = false;
    bool noBranchSimplify    = false;
    bool noRemoveNull        = false;
    bool noLocals            = false;
    bool noRemoveLabels      = false;
    bool noDataflow          = false;
    bool noDecompile         = false;
    bool stopBeforeDecompile = false;
    bool traceDecoder        = false;

    /// The file in which the dotty graph is saved
    QString dotFile;
    int numToPropagate     = -1;
    bool noPromote         = false;
    bool propOnlyToAll     = false;
    bool debugGen          = false;
    int maxMemDepth        = 99;
    bool noParameterNames  = false;
    bool stopAtDebugPoints = false;

    /// When true, attempt to decode main, all children, and all procs.
    /// \a decodeMain is set when there are no -e or -E switches given
    bool decodeMain          = true;
    bool printAST            = false;
    bool dumpXML             = false;
    bool noRemoveReturns     = false;
    bool decodeThruIndCall   = false;
    bool noDecodeChildren    = false;
    bool loadBeforeDecompile = false;
    bool saveBeforeDecompile = false;
    bool noProve             = false;
    bool noChangeSignatures  = false;
    bool conTypeAnalysis     = false;
    bool dfaTypeAnalysis     = true;
    int propMaxDepth         = 3; ///< Max depth of expression that'll be propagated to more than one dest
    bool generateCallGraph   = false;
    bool generateSymbols     = false;
    bool noGlobals           = false;
    bool assumeABI           = false; ///< Assume ABI compliance
    bool experimental        = false; ///< Activate experimental code. Caution!

    QTextStream LogStream;
    QTextStream ErrStream;

    std::vector<Address> m_entryPoints; ///< A vector which contains all know entrypoints for the Prog.
    std::vector<QString> m_symbolFiles; ///< A vector containing the names off all symbolfiles to load.
    std::map<Address, QString> symbols; ///< A map to find a name by a given address.
    IProject *m_currentProject;
};

#define VERBOSE                 (Boomerang::get()->vFlag)
#define DEBUG_TA                (Boomerang::get()->debugTA)
#define DEBUG_PROOF             (Boomerang::get()->debugProof)
#define DEBUG_UNUSED            (Boomerang::get()->debugUnused)
#define DEBUG_LIVENESS          (Boomerang::get()->debugLiveness)
#define DEBUG_RANGE_ANALYSIS    (Boomerang::get()->debugRangeAnalysis)
#define DEBUG_SWITCH            (Boomerang::get()->debugSwitch)
#define DEBUG_GEN               (Boomerang::get()->debugGen)

#define DFA_TYPE_ANALYSIS       (Boomerang::get()->dfaTypeAnalysis)
#define CON_TYPE_ANALYSIS       (Boomerang::get()->conTypeAnalysis)
#define ADHOC_TYPE_ANALYSIS     (!Boomerang::get()->dfaTypeAnalysis && !Boomerang::get()->conTypeAnalysis)
#define DUMP_XML                (Boomerang::get()->dumpXML)
#define EXPERIMENTAL            (Boomerang::get()->experimental)
