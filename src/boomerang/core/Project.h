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


#include "boomerang/core/IProject.h"
#include "boomerang/loader/IFileLoader.h"
#include "boomerang/type/TypeRecovery.h"
#include "boomerang/db/binary/BinaryFile.h"

#include <QByteArray>
#include <memory>
#include <vector>


class BinaryImage;
class IFrontEnd;


class Project : public IProject
{
public:
    Project();
    Project(const Project& other) = delete;
    Project(Project&& other) = default;

    virtual ~Project() override;

    Project& operator=(const Project& other) = delete;
    Project& operator=(Project&& other) = default;

public:
    /// \copydoc IProject::loadBinaryFile
    bool loadBinaryFile(const QString& filePath) override;

    /// \copydoc IProject::loadSaveFile
    bool loadSaveFile(const QString& filePath) override;

    /// \copydoc IProject::writeSavefile
    bool writeSaveFile(const QString& filePath) override;

    /// \copydoc IProject::isBinaryLoaded
    bool isBinaryLoaded() const override;

    /// \copydoc IProject::unload
    void unloadBinaryFile() override;

    /// \copydoc IProject::decodeBinaryFile
    bool decodeBinaryFile() override;

public:
    BinaryFile *getLoadedBinaryFile() override { return m_loadedBinary.get(); }
    const BinaryFile *getLoadedBinaryFile() const override { return m_loadedBinary.get(); }

    const Prog *getProg() const override { return m_prog.get(); }
    Prog *getProg() override { return m_prog.get(); }

    ITypeRecovery *getTypeRecoveryEngine() const override { return m_typeRecovery.get(); }

private:
    /// Load all plugins from the plugin directory.
    void loadPlugins();

    /// Get the best loader that is able to load the file at \p filePath
    IFileLoader *getBestLoader(const QString& filePath) const;

    /**
     * Create a Prog from a loaded binary file. Returns nullptr on failure.
     */
    Prog *createProg(BinaryFile *file, const QString& name = "");

    void loadSymbols();

    bool decodeAll();

private:
    std::unique_ptr<BinaryFile> m_loadedBinary;
    std::unique_ptr<Prog> m_prog;

    std::unique_ptr<IFrontEnd> m_fe;               ///< front end
    std::unique_ptr<ITypeRecovery> m_typeRecovery; ///< middle end

    // Plugins
    std::vector<std::shared_ptr<LoaderPlugin> > m_loaderPlugins;
};
