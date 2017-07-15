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

#include "boomerang/db/project.h"
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

std::string BinaryFileFactory::m_pluginsPath = "";


BinaryFileFactory::BinaryFileFactory()
{
    populatePlugins();
}


void BinaryFileFactory::setPluginsPath(const std::string& pluginsPath)
{
    BinaryFileFactory::m_pluginsPath = pluginsPath;
}


IFileLoader *BinaryFileFactory::loadFile(const std::string& filePath)
{
    IBinaryImage       *image   = Boomerang::get()->getImage();
    IBinarySymbolTable *symbols = Boomerang::get()->getSymbols();

    image->reset();
    symbols->clear();

    // Find loader plugin to load file
    IFileLoader *loader = getInstanceFor(filePath);

    if (loader == nullptr) {
        qWarning() << "Unrecognised binary file format.";
        return nullptr;
    }

    loader->initialize(image, symbols);

    QFile srcFile(QString(filePath.c_str()));

    if (false == srcFile.open(QFile::ReadOnly)) {
        qWarning() << "Opening '" << filePath.c_str() << "' failed";
        return nullptr;
    }

    Boomerang::get()->getProject()->getFiledata().clear();
    Boomerang::get()->getProject()->getFiledata() = srcFile.readAll();

    if (loader->loadFromMemory(Boomerang::get()->getProject()->getFiledata()) == 0) {
        qWarning() << "Loading '" << filePath.c_str() << "' failed";
        return nullptr;
    }

    image->calculateTextLimits();
    return loader;
}


IFileLoader *BinaryFileFactory::getInstanceFor(const std::string& filePath)
{
    QFile f(filePath.c_str());

    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Unable to open binary file: " << filePath.c_str();
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
    QDir pluginsDir(qApp->applicationDirPath());

    pluginsDir.cd("../lib");

    if (!qApp->libraryPaths().contains(pluginsDir.absolutePath())) {
        qApp->addLibraryPath(pluginsDir.absolutePath());
    }

    for (QString fileName : pluginsDir.entryList(QDir::Files)) {
        QString sofilename = pluginsDir.absoluteFilePath(fileName);

        try {
            std::shared_ptr<LoaderPlugin> loaderPlugin(new LoaderPlugin(sofilename));
            m_loaderPlugins.push_back(loaderPlugin);
        }
        catch (const char *errmsg) {
            qCritical() << "Unable to load plugin: " << errmsg;
        }
    }

    if (m_loaderPlugins.empty()) {
        qCritical() << "No loader plugins found!";
    }
}
