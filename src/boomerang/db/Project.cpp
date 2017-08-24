#include "Project.h"

#include "boomerang/core/BinaryFileFactory.h"
#include "boomerang/core/Boomerang.h"
#include "boomerang/db/BinaryImage.h"
#include "boomerang/db/IBinarySymbols.h"

#include "boomerang/util/Log.h"


Project::Project()
{
    m_image.reset(new BinaryImage);
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
        unloadBinaryFile();
    }

    IBinaryImage       *image   = Boomerang::get()->getImage();
    IBinarySymbolTable *symbols = Boomerang::get()->getSymbols();

    loader->initialize(image, symbols);

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

    image->calculateTextLimits();

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
    return false; // stub
}


void Project::unloadBinaryFile()
{
    IBinaryImage       *image   = Boomerang::get()->getImage();
    IBinarySymbolTable *symbols = Boomerang::get()->getSymbols();

    image->reset();
    symbols->clear();

    m_fileBytes.clear();
}


void Project::loadPlugins()
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


IFileLoader* Project::getBestLoader(const QString& filePath) const
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
