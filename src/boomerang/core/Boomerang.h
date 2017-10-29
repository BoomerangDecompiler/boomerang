#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/core/IBoomerang.h"
#include "boomerang/core/IProject.h"
#include "boomerang/core/Watcher.h"
#include "boomerang/util/Log.h"
#include "boomerang/core/Settings.h"

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


/**
 * Controls the loading, decoding, decompilation and code generation for a program.
 * This is the main class of the decompiler.
 */
class Boomerang : public IBoomerang
{
private:
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
    virtual ~Boomerang() override;

public:
    /// \returns The global boomerang object. It will be created if it does not already exist.
    static Boomerang *get();
    static void destroy();

    IBinaryImage *getImage() override;
    IBinarySymbolTable *getSymbols() override;

    IProject *getOrCreateProject() override;

    static const char *getVersionStr();

    /**
     * Returns the ICodeGenerator for the given proc.
     * \return The ICodeGenerator for the specified UserProc.
     */
    ICodeGenerator *getCodeGenerator();


    Settings *getSettings() { return m_settings.get(); }
    const Settings *getSettings() const { return m_settings.get(); }

    /**
     * Loads the executable file and decodes it.
     * \param fname The name of the file to load.
     * \param pname How the Prog will be named.
     */
    std::unique_ptr<Prog> loadAndDecode(const QString& fname, const char *pname = nullptr);

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
    void addWatcher(IWatcher *watcher) { m_watchers.insert(watcher); }

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
        for (IWatcher *it : m_watchers) {
            it->alert_complete();
        }
    }

    /// Alert the watchers we have found a new %Proc.
    void alertNew(Function *p)
    {
        for (IWatcher *it : m_watchers) {
            it->alertNew(p);
        }
    }

    /// Alert the watchers we have removed a %Proc.
    void alertRemove(Function *p)
    {
        for (IWatcher *it : m_watchers) {
            it->alertRemove(p);
        }
    }

    /// Alert the watchers we have updated this Procs signature
    void alertUpdateSignature(Function *p)
    {
        for (IWatcher *it : m_watchers) {
            it->alertUpdateSignature(p);
        }
    }

    /// Alert the watchers we are currently decoding \a nBytes bytes at address \a pc.
    void alertDecode(Address pc, int nBytes)
    {
        for (IWatcher *it : m_watchers) {
            it->alertDecode(pc, nBytes);
        }
    }

    /// Alert the watchers of a bad decode of an instruction at \a pc.
    void alertBadDecode(Address pc)
    {
        for (IWatcher *it : m_watchers) {
            it->alertBadDecode(pc);
        }
    }

    /// Alert the watchers we have succesfully decoded this function
    void alertDecode(Function *p, Address pc, Address last, int nBytes)
    {
        for (IWatcher *it : m_watchers) {
            it->alertDecode(p, pc, last, nBytes);
        }
    }

    /// Alert the watchers we have loaded the Proc.
    void alertLoad(Function *p)
    {
        for (IWatcher *it : m_watchers) {
            it->alert_load(p);
        }
    }

    /// Alert the watchers we are starting to decode.
    void alertStartDecode(Address start, int nBytes)
    {
        for (IWatcher *it : m_watchers) {
            it->alertStartDecode(start, nBytes);
        }
    }

    /// Alert the watchers we finished decoding.
    void alertEndDecode()
    {
        for (IWatcher *it : m_watchers) {
            it->alertEndDecode();
        }
    }

    void alertStartDecompile(UserProc *p)
    {
        for (IWatcher *it : m_watchers) {
            it->alertStartDecompile(p);
        }
    }

    void alertProcStatusChange(UserProc *p)
    {
        for (IWatcher *it : m_watchers) {
            it->alertProcStatusChange(p);
        }
    }

    void alertDecompileSSADepth(UserProc *p, int depth)
    {
        for (IWatcher *it : m_watchers) {
            it->alertDecompileSSADepth(p, depth);
        }
    }

    void alertDecompileBeforePropagate(UserProc *p, int depth)
    {
        for (IWatcher *it : m_watchers) {
            it->alertDecompileBeforePropagate(p, depth);
        }
    }

    void alertDecompileAfterPropagate(UserProc *p, int depth)
    {
        for (IWatcher *it : m_watchers) {
            it->alertDecompileAfterPropagate(p, depth);
        }
    }

    void alertDecompileAfterRemoveStmts(UserProc *p, int depth)
    {
        for (IWatcher *it : m_watchers) {
            it->alertDecompileAfterRemoveStmts(p, depth);
        }
    }

    void alertEndDecompile(UserProc *p)
    {
        for (IWatcher *it : m_watchers) {
            it->alertEndDecompile(p);
        }
    }

    void alertConsidering(Function *_parent, Function *p)
    {
        for (IWatcher *it : m_watchers) {
            it->alertConsidering(_parent, p);
        }
    }

    void alertDecompiling(UserProc *p)
    {
        for (IWatcher *it : m_watchers) {
            it->alertDecompiling(p);
        }
    }

    void alertDecompileDebugPoint(UserProc *p, const char *description);

public:
    std::unique_ptr<Settings> m_settings;
    std::unique_ptr<IProject> m_currentProject;
    std::unique_ptr<IBinarySymbolTable> m_symbols;
    std::unique_ptr<ICodeGenerator> m_codeGenerator;

    std::set<IWatcher *> m_watchers;        ///< The watchers which are interested in this decompilation.
    std::vector<Address> m_entryPoints;     ///< A vector which contains all know entrypoints for the Prog.
    std::vector<QString> m_symbolFiles;     ///< A vector containing the names off all symbolfiles to load.
    std::map<Address, QString> m_symbolMap; ///< A map to find a name by a given address.

private:
    /// Prints help for the interactive mode.
    void helpcmd() const;

    /// This is a mini command line debugger.  Feel free to expand it.
    void miniDebugger(UserProc *p, const char *description);
};

/**
 * Global settings
 */

#define SETTING(var)    (Boomerang::get()->getSettings()->var)

#define VERBOSE                 (Boomerang::get()->getSettings()->vFlag)
#define DEBUG_TA                (Boomerang::get()->getSettings()->debugTA)
#define DEBUG_PROOF             (Boomerang::get()->getSettings()->debugProof)
#define DEBUG_UNUSED            (Boomerang::get()->getSettings()->debugUnused)
#define DEBUG_LIVENESS          (Boomerang::get()->getSettings()->debugLiveness)
#define DEBUG_RANGE_ANALYSIS    (Boomerang::get()->getSettings()->debugRangeAnalysis)
#define DEBUG_SWITCH            (Boomerang::get()->getSettings()->debugSwitch)
#define DEBUG_GEN               (Boomerang::get()->getSettings()->debugGen)
#define DEBUG_DECODER           (Boomerang::get()->getSettings()->debugDecoder)
#define DEBUG_LIVENESS          (Boomerang::get()->getSettings()->debugLiveness)
#define DFA_TYPE_ANALYSIS       (Boomerang::get()->getSettings()->dfaTypeAnalysis)
#define DUMP_XML                (Boomerang::get()->getSettings()->dumpXML)
#define EXPERIMENTAL            (Boomerang::get()->getSettings()->experimental)
