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
#include "boomerang/db/BinaryImage.h"
#include "boomerang/db/IBinarySymbols.h"
#include "boomerang/type/dfa/DFATypeRecovery.h"
#include "boomerang/util/Log.h"


Project::Project()
    : m_image(new BinaryImage)
    , m_typeRecovery(new DFATypeRecovery())
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

    IBinarySymbolTable *symbols = Boomerang::get()->getSymbols();

    loader->initialize(getImage(), symbols);

    QFile srcFile(filePath);

    if (false == srcFile.open(QFile::ReadOnly)) {
        LOG_WARN("Opening '%1' failed");
        return false;
    }

    m_fileBytes = srcFile.readAll();

    if (loader->loadFromMemory(m_fileBytes) == false) {
        LOG_WARN("Loading '%1 failed", filePath);
        return false;
    }

    m_image->updateTextLimits();

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
    return !m_image->empty();
}


void Project::unloadBinaryFile()
{
    Boomerang::get()->getSymbols()->clear();

    m_fileBytes.clear();
    m_image->reset();
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
    QFile f(filePath);

    if (!f.open(QFile::ReadOnly)) {
        LOG_ERROR("Unable to open binary file: %1", filePath);
        return nullptr;
    }

    IFileLoader *bestLoader = nullptr;
    int         bestScore   = 0;

    // get the best plugin for loading this file
    for (const std::shared_ptr<LoaderPlugin>& p : m_loaderPlugins) {
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
