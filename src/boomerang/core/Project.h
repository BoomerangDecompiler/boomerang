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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/core/plugin/PluginManager.h"
#include "boomerang/ifc/IFileLoader.h"
#include "boomerang/util/Address.h"

#include <memory>
#include <set>
#include <vector>


class BinaryFile;
class Function;
class ICodeGenerator;
class IFrontEnd;
class ITypeRecovery;
class IWatcher;
class Module;
class Prog;
class Settings;
class UserProc;

class QString;


class BOOMERANG_API Project
{
public:
    Project();
    Project(const Project &other) = delete;
    Project(Project &&other)      = delete;

    virtual ~Project();

    Project &operator=(const Project &other) = delete;
    Project &operator=(Project &&other) = delete;

public:
    Settings *getSettings();
    const Settings *getSettings() const;

    BinaryFile *getLoadedBinaryFile();
    const BinaryFile *getLoadedBinaryFile() const;

    Prog *getProg();
    const Prog *getProg() const;

    /// \returns the type recovery engine
    ITypeRecovery *getTypeRecoveryEngine();
    const ITypeRecovery *getTypeRecoveryEngine() const;

    PluginManager *getPluginManager();
    const PluginManager *getPluginManager() const;

public:
    /// \returns the library version string
    const char *getVersionStr() const;

    /// Load all plugins from the plugin directory.
    void loadPlugins();

    /**
     * Import a binary file from \p filePath.
     * Loads the binary file and decodes it.
     * If a binary file is already loaded, it is unloaded first (all unsaved data is lost).
     * \returns true iff loading was successful.
     */
    bool loadBinaryFile(const QString &filePath);

    /**
     * Load a saved file from \p filePath.
     * If a binary file is already loaded, it is unloaded first (all unsaved data is lost).
     * \note Not yet implemented.
     * \returns true iff loading was successful.
     */
    bool loadSaveFile(const QString &filePath);

    /**
     * Save data to the save file at \p filePath.
     * If the file already exists, it is overwritten.
     * \note Not yet implemented.
     * \returns true iff saving was successful.
     */
    bool writeSaveFile(const QString &filePath);

    /**
     * Check if the project contains a loaded binary.
     */
    bool isBinaryLoaded() const;

    /**
     * Unload the loaded binary file, discarding all unsaved data.
     * If there is no loaded binary, nothing happens.
     */
    void unloadBinaryFile();

    /**
     * Decode the loaded binary file.
     * \returns true on success, false if no binary is loaded or an error occurred.
     */
    bool decodeBinaryFile();

    /**
     * Decompile the decoded binary file.
     * \returns true on success, false if no binary is decoded or an error occurred.
     */
    bool decompileBinaryFile();

    /**
     * Generate code for \p module, or all modules if \p module is nullptr.
     * \returns true on success, false if no binary is decompiled or an error occurred.
     */
    bool generateCode(Module *module = nullptr);

public:
    /// Register a watcher to receive events about the decompilation.
    /// Does NOT take ownership of the pointer.
    void addWatcher(IWatcher *watcher);

    /// Called once after a function was created.
    void alertFunctionCreated(Function *function);

    /// Called once after a function was removed.
    void alertFunctionRemoved(Function *function);

    /// Called once after the function signature was updated.
    void alertSignatureUpdated(Function *function);

    /// Called once on decode start.
    void alertStartDecode(Address start, int numBytes);

    /// Called every time an instruction is decoded.
    /// \param numBytes size of the instruction
    void alertInstructionDecoded(Address pc, int numBytes);

    /// Called every time an invalid or unrecognized instruction is encountered.
    void alertBadDecode(Address pc);

    /// Called every time a function was decoded completely.
    void alertFunctionDecoded(Function *function, Address pc, Address last, int numBytes);

    /// Called once on decode end.
    void alertEndDecode();

    /// Called once for every function on decompilation start (before earlyDecompile)
    void alertStartDecompile(UserProc *proc);

    /// Called every time the status of \p proc has changed.
    void alertProcStatusChanged(UserProc *proc);

    /// Called once for every completely decompiled proc \p proc.
    void alertEndDecompile(UserProc *proc);

    /// Called every time before middleDecompile is executed for \p function
    void alertDiscovered(Function *function);

    /// Called during the decompilation process when resuming decompilation of this proc.
    void alertDecompiling(UserProc *proc);

    void alertDecompileDebugPoint(UserProc *p, const char *description);

    /// Called once on decompilation end.
    void alertDecompilationEnd();

private:
    /// Get the best loader that is able to load the file at \p filePath
    IFileLoader *getBestLoader(const QString &filePath) const;

    /**
     * Create a Prog from a loaded binary file. Returns nullptr on failure.
     */
    Prog *createProg(BinaryFile *file, const QString &name);

    /**
     * Create a FrontEnd object. Will fail if one is already created
     * or no program is created.
     *
     * \param binaryFile Loaded binary file
     * \param prog       program being decoded
     *
     * \returns the newly created frontend on success, or nullptr on failure.
     */
    IFrontEnd *createFrontEnd();

    /**
     * Define symbols from symbol files and command line switches ("-s")
     */
    void loadSymbols();

    /**
     * Disassemble the whole binary file.
     * \returns false iff an error occurred.
     */
    bool decodeAll();

private:
    std::unique_ptr<Settings> m_settings;

    /// The watchers which are interested in this decompilation.
    std::set<IWatcher *> m_watchers;

    std::unique_ptr<PluginManager> m_pluginManager;

    std::unique_ptr<BinaryFile> m_loadedBinary;
    std::unique_ptr<Prog> m_prog;

    IFrontEnd *m_fe = nullptr;
};
