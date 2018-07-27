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


#include "boomerang/core/Settings.h"
#include "boomerang/db/binary/BinaryFile.h"
#include "boomerang/db/Prog.h"
#include "boomerang/frontend/Frontend.h"
#include "boomerang/ifc/ICodeGenerator.h"
#include "boomerang/ifc/IFileLoader.h"
#include "boomerang/ifc/ITypeRecovery.h"
#include "boomerang/util/Address.h"

#include <set>
#include <memory>


class BinaryImage;
class IFrontEnd;
class ICodeGenerator;
class Module;
class IWatcher;
class UserProc;
class QString;


class Project
{
public:
    Project();
    Project(const Project& other) = delete;
    Project(Project&& other) = default;

    virtual ~Project();

    Project& operator=(const Project& other) = delete;
    Project& operator=(Project&& other) = default;

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
    bool loadBinaryFile(const QString& filePath);

    /**
     * Load a saved file from \p filePath.
     * If a binary file is already loaded, it is unloaded first (all unsaved data is lost).
     * \note Not yet implemented.
     * \returns true iff loading was successful.
     */
    bool loadSaveFile(const QString& filePath);

    /**
     * Save data to the save file at \p filePath.
     * If the file already exists, it is overwritten.
     * \note Not yet implemented.
     * \returns true iff saving was successful.
     */
    bool writeSaveFile(const QString& filePath);

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

public:
    Settings *getSettings()             { return m_settings.get(); }
    const Settings *getSettings() const { return m_settings.get(); }

    BinaryFile *getLoadedBinaryFile() { return m_loadedBinary.get(); }
    const BinaryFile *getLoadedBinaryFile() const { return m_loadedBinary.get(); }

    Prog *getProg() { return m_prog.get(); }
    const Prog *getProg() const { return m_prog.get(); }

    /// \returns the type recovery engine
    ITypeRecovery *getTypeRecoveryEngine() { return m_typeRecovery.get(); }
    const ITypeRecovery *getTypeRecoveryEngine() const { return m_typeRecovery.get(); }

private:
    /// Get the best loader that is able to load the file at \p filePath
    IFileLoader *getBestLoader(const QString& filePath) const;

    /**
     * Create a Prog from a loaded binary file. Returns nullptr on failure.
     */
    Prog *createProg(BinaryFile *file, const QString& name);

    /**
     * Define symbols from symbol files and command line switches ("-s")
     */
    void loadSymbols();

    bool readSymbolFile(const QString& fname);

    /**
     * Disassemble the whole binary file.
     * \returns false iff an error occurred.
     */
    bool decodeAll();

private:
    std::unique_ptr<Settings> m_settings;

    /// The watchers which are interested in this decompilation.
    std::set<IWatcher *> m_watchers;

    // Plugins
    std::vector<std::unique_ptr<LoaderPlugin> > m_loaderPlugins;

    std::unique_ptr<BinaryFile> m_loadedBinary;
    std::unique_ptr<Prog> m_prog;

    std::unique_ptr<IFrontEnd> m_fe;                 ///< front end
    std::unique_ptr<ITypeRecovery> m_typeRecovery;   ///< middle end
    std::unique_ptr<ICodeGenerator> m_codeGenerator; ///< back end
};
