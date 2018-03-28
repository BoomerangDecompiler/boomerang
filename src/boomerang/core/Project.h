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


#include "boomerang/core/Project.h"
#include "boomerang/loader/IFileLoader.h"
#include "boomerang/type/TypeRecovery.h"
#include "boomerang/db/binary/BinaryFile.h"

#include <QByteArray>
#include <memory>
#include <vector>


class BinaryImage;
class IFrontEnd;
class ICodeGenerator;
class Module;


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
    BinaryFile *getLoadedBinaryFile() { return m_loadedBinary.get(); }
    const BinaryFile *getLoadedBinaryFile() const { return m_loadedBinary.get(); }

    Prog *getProg() { return m_prog.get(); }
    const Prog *getProg() const { return m_prog.get(); }

    /// \returns the type recovery engine
    ITypeRecovery *getTypeRecoveryEngine() { return m_typeRecovery.get(); }
    const ITypeRecovery *getTypeRecoveryEngine() const { return m_typeRecovery.get(); }

private:
    /// Load all plugins from the plugin directory.
    void loadPlugins();

    /// Get the best loader that is able to load the file at \p filePath
    IFileLoader *getBestLoader(const QString& filePath) const;

    /**
     * Create a Prog from a loaded binary file. Returns nullptr on failure.
     */
    Prog *createProg(BinaryFile *file, const QString& name = "");

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
    // Plugins
    std::vector<std::unique_ptr<LoaderPlugin> > m_loaderPlugins;

    std::unique_ptr<BinaryFile> m_loadedBinary;
    std::unique_ptr<Prog> m_prog;

    std::unique_ptr<IFrontEnd> m_fe;                 ///< front end
    std::unique_ptr<ITypeRecovery> m_typeRecovery;   ///< middle end
    std::unique_ptr<ICodeGenerator> m_codeGenerator; ///< back end
};
