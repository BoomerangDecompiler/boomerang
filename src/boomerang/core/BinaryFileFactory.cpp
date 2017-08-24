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


#define LMMH(x)                                                                                                  \
    ((unsigned)((Byte *)(&x))[0] + ((unsigned)((Byte *)(&x))[1] << 8) + ((unsigned)((Byte *)(&x))[2] << 16) + \
     ((unsigned)((Byte *)(&x))[3] << 24))

BinaryFileFactory::BinaryFileFactory()
{
    populatePlugins();
}


IFileLoader *BinaryFileFactory::loadFile(const QString& filePath)
{
    // Find loader plugin to load file
    IFileLoader *loader = getInstanceFor(filePath);

    bool ok = Boomerang::get()->getOrCreateProject()->loadBinaryFile(filePath);

    return ok ? loader : nullptr;
}


IFileLoader *BinaryFileFactory::getInstanceFor(const QString& filePath)
{
    QFile f(filePath);

    if (!f.open(QFile::ReadOnly)) {
        LOG_ERROR("Unable to open binary file: %1", filePath);
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
    QDir pluginsDir = Boomerang::get()->getSettings()->getDataDirectory();
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
