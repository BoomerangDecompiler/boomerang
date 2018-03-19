#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Project.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/type/dfa/DFATypeRecovery.h"
#include "boomerang/util/Log.h"


Project::Project()
    : m_typeRecovery(new DFATypeRecovery())
{
    loadPlugins();
}


Project::~Project()
{
}


bool Project::loadBinaryFile(const QString& filePath)
{
    LOG_MSG("Loading binary file '%1'", filePath);

    // Find loader plugin to load file
    IFileLoader *loader = getBestLoader(filePath);

    if (loader == nullptr) {
        LOG_WARN("Cannot load %1: Unrecognized binary file format.", filePath);
        return false;
    }

    if (isBinaryLoaded()) {
        loader->unload();
        unloadBinaryFile();
    }

    QFile srcFile(filePath);
    if (false == srcFile.open(QFile::ReadOnly)) {
        LOG_WARN("Opening '%1' failed");
        return false;
    }

    m_loadedBinary.reset(new BinaryFile(srcFile.readAll()));

    if (loader->loadFromFile(m_loadedBinary.get()) == false) {
        LOG_WARN("Loading '%1 failed", filePath);
        return false;
    }

    m_loadedBinary->getImage()->updateTextLimits();

    return true;
}


bool Project::loadSaveFile(const QString& /*filePath*/)
{
    LOG_FATAL("Loading save files is not implemented.");
    return false;
}


bool Project::writeSaveFile(const QString& /*filePath*/)
{
    LOG_FATAL("Saving save files is not implemented.");
    return false;
}


bool Project::isBinaryLoaded() const
{
    return m_loadedBinary != nullptr;
}


void Project::unloadBinaryFile()
{
    m_loadedBinary.reset();
}


void Project::loadPlugins()
{
    QDir pluginsDir = Boomerang::get()->getSettings()->getPluginDirectory();
    if (!pluginsDir.exists() || !pluginsDir.cd("loader")) {
        LOG_ERROR("Cannot open loader plugin directory '%1'!", pluginsDir.absolutePath());
    }

    for (QString fileName : pluginsDir.entryList(QDir::Files)) {
        const QString sofilename = pluginsDir.absoluteFilePath(fileName);

#ifdef _WIN32
        if (!sofilename.endsWith(".dll")) {
            continue;
        }
#endif
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


IFileLoader *Project::getBestLoader(const QString& filePath) const
{
    QFile inputBinary(filePath);

    if (!inputBinary.open(QFile::ReadOnly)) {
        LOG_ERROR("Unable to open binary file: %1", filePath);
        return nullptr;
    }

    IFileLoader *bestLoader = nullptr;
    int         bestScore   = 0;

    // get the best plugin for loading this file
    for (const std::shared_ptr<LoaderPlugin>& p : m_loaderPlugins) {
        inputBinary.seek(0); // reset the file offset for the next plugin
        IFileLoader *loader = p->get();

        int score = loader->canLoad(inputBinary);

        if (score > bestScore) {
            bestScore  = score;
            bestLoader = loader;
        }
    }

    return bestLoader;
}
