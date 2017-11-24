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

#include <QByteArray>
#include <memory>
#include <vector>


class IBinaryImage;

class Project : public IProject
{
public:
    Project();
    Project(Project& other) = delete;
    Project(Project&& other) = default;

    virtual ~Project() override;

    Project& operator=(Project& other) = delete;
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

    /// \copydoc IProject::getImage
    IBinaryImage *getImage() override { return m_image.get(); }

    /// \copydoc IProject::getImage
    const IBinaryImage *getImage() const override { return m_image.get(); }

    ITypeRecovery *getTypeRecoveryEngine() const override { return m_typeRecovery.get(); }

    /// Get the best loader that is able to load the file at \p filePath
    IFileLoader *getBestLoader(const QString& filePath) const override;

private:
    /// Load all plugins from the plugin directory.
    void loadPlugins();

private:
    QByteArray m_fileBytes;
    std::unique_ptr<IBinaryImage> m_image; ///< raw memory interface
    std::unique_ptr<ITypeRecovery> m_typeRecovery; ///< type recovery engine

    // Plugins
    std::vector<std::shared_ptr<LoaderPlugin> > m_loaderPlugins;
};
