/* File: BinaryFileFactory.cpp
 * Desc: This file contains the implementation of the factory function
 * BinaryFile::getInstanceFor(), and also BinaryFile::Load()
 *
 * This function determines the type of a binary and loads the appropriate
 * loader class dynamically.
 */

#include "BinaryFileFactory.h"

#include "boomerang/util/Log.h"
#include "boomerang/core/Boomerang.h"

#include "boomerang/db/Project.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySymbols.h"

#include "boomerang/loader/IFileLoader.h"

#include <cstdio>

#include <QDir>
#include <QFile>

#include <QPluginLoader>
#include <QCoreApplication>
#include <QString>
#include <QDebug>


#define LMMH(x)                                                                                                  \
    ((unsigned)((Byte *)(&x))[0] + ((unsigned)((Byte *)(&x))[1] << 8) + ((unsigned)((Byte *)(&x))[2] << 16) + \
     ((unsigned)((Byte *)(&x))[3] << 24))

BinaryFileFactory::BinaryFileFactory()
{
    populatePlugins();
}


IFileLoader *BinaryFileFactory::loadFile(const QString& filePath)
{
    LOG_MSG("Loading binary file '%1'", filePath);

    IBinaryImage       *image   = Boomerang::get()->getImage();
    IBinarySymbolTable *symbols = Boomerang::get()->getSymbols();

    image->reset();
    symbols->clear();

    // Find loader plugin to load file
    IFileLoader *loader = getInstanceFor(filePath);

    if (loader == nullptr) {
        LOG_WARN("Cannot load %1: Unrecognised binary file format.", filePath);
        return nullptr;
    }

    loader->initialize(image, symbols);

    QFile srcFile(filePath);

    if (false == srcFile.open(QFile::ReadOnly)) {
        LOG_WARN("Opening '%1' failed");
        return nullptr;
    }

    Boomerang::get()->getProject()->getFiledata().clear();
    Boomerang::get()->getProject()->getFiledata() = srcFile.readAll();

    if (loader->loadFromMemory(Boomerang::get()->getProject()->getFiledata()) == 0) {
        LOG_WARN("Loading '%1 failed", filePath);
        return nullptr;
    }

    image->calculateTextLimits();
    return loader;
}


IFileLoader *BinaryFileFactory::getInstanceFor(const QString& filePath)
{
    QFile f(filePath);

    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Unable to open binary file: " << filePath;
        return nullptr;
    }

    IFileLoader *bestLoader = nullptr;
    int         bestScore   = 0;

    // get the best plugin for loading this file
    for (std::shared_ptr<LoaderPlugin>& p : m_loaderPlugins) {
        f.seek(0); // reset the file offset for the next plugin
        IFileLoader *loader = p->get();

        int score = loader->canLoad(f);

        if (score > bestScore) {
            bestScore  = score;
            bestLoader = loader;
        }
    }

    return bestLoader;
}


void BinaryFileFactory::populatePlugins()
{
    QDir pluginsDir = Boomerang::get()->getDataDirectory();
    if (!pluginsDir.cd("plugins/loader/")) {
        LOG_ERROR("Cannot open loader plugin directory!");
    }

    for (QString fileName : pluginsDir.entryList(QDir::Files)) {
        const QString sofilename = pluginsDir.absoluteFilePath(fileName);

        try {
            std::shared_ptr<LoaderPlugin> loaderPlugin(new LoaderPlugin(sofilename));
            m_loaderPlugins.push_back(loaderPlugin);
        }
        catch (const char *errmsg) {
            LOG_WARN("Unable to load plugin: %1", errmsg);
        }
    }

    if (m_loaderPlugins.empty()) {
        LOG_ERROR("No loader plugins found, unable to load any binaries.");
    }
}
