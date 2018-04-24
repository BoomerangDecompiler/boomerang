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


#include "boomerang/codegen/CCodeGenerator.h"
#include "boomerang/core/Boomerang.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/db/Prog.h"
#include "boomerang/type/dfa/DFATypeRecovery.h"
#include "boomerang/util/Log.h"


Project::Project()
    : m_typeRecovery(new DFATypeRecovery())
    , m_codeGenerator(new CCodeGenerator())
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
        LOG_WARN("Cannot load '%1': Unrecognized binary file format.", filePath);
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

    m_loadedBinary.reset(new BinaryFile(srcFile.readAll(), loader));

    if (loader->loadFromFile(m_loadedBinary.get()) == false) {
        LOG_WARN("Loading '%1 failed", filePath);
        return false;
    }

    m_loadedBinary->getImage()->updateTextLimits();

    return createProg(m_loadedBinary.get(), QFileInfo(filePath).baseName()) != nullptr;
}


bool Project::loadSaveFile(const QString& /*filePath*/)
{
    LOG_ERROR("Loading save files is not implemented.");
    return false;
}


bool Project::writeSaveFile(const QString& /*filePath*/)
{
    LOG_ERROR("Saving save files is not implemented.");
    return false;
}


bool Project::isBinaryLoaded() const
{
    return m_loadedBinary != nullptr;
}


void Project::unloadBinaryFile()
{
    m_prog.reset();
    m_loadedBinary.reset();
}


bool Project::decodeBinaryFile()
{
    if (!getProg()) {
        LOG_ERROR("Cannot decode binary file: No binary file is loaded.");
        return false;
    }
    else if (!m_fe) {
        LOG_ERROR("Cannot decode binary file: No suitable frontend found.");
        return false;
    }

    loadSymbols();

    if (!Boomerang::get()->getSettings()->m_entryPoints.empty()) { // decode only specified procs
        // decode entry points from -e (and -E) switch(es)
        for (auto& elem : Boomerang::get()->getSettings()->m_entryPoints) {
            LOG_MSG("Decoding specified entrypoint at address %1", elem);
            m_prog->decodeEntryPoint(elem);
        }
    }
    else if (!decodeAll()) { // decode everything
        return false;
    }

    LOG_MSG("Finishing decode...");
    m_prog->finishDecode();

    Boomerang::get()->alertEndDecode();

    LOG_MSG("Found %1 procs", m_prog->getNumFunctions());

    if (SETTING(generateSymbols)) {
        m_prog->printSymbolsToFile();
    }

    if (SETTING(generateCallGraph)) {
        m_prog->printCallGraph();
    }

    return true;
}


bool Project::decompileBinaryFile()
{
    if (!m_prog) {
        LOG_ERROR("Cannot decompile binary file: No binary file is loaded.");
        return false;
    }
    else if (!m_fe) {
        LOG_ERROR("Cannot decompile binary file: No suitable frontend found.");
        return false;
    }

    m_prog->decompile();
    return true;
}


bool Project::generateCode(Module *module)
{
    if (!m_prog) {
        LOG_ERROR("Cannot generate code: No binary file is loaded.");
        return false;
    }
    else if (!m_fe) {
        LOG_ERROR("Cannot generate code: No suitable frontend found.");
        return false;
    }

    LOG_MSG("Generating code...");
    m_codeGenerator->generateCode(getProg(), module);
    return true;
}


Prog *Project::createProg(BinaryFile *file, const QString& name)
{
    if (!file) {
        LOG_ERROR("Cannot create Prog without a binary file!");
        return nullptr;
    }

    // unload old Prog before creating a new one
    m_fe.reset();
    m_prog.reset();

    m_prog.reset(new Prog(name, this));
    m_fe.reset(IFrontEnd::instantiate(getLoadedBinaryFile(), getProg()));

    m_prog->setFrontEnd(m_fe.get());
    return m_prog.get();
}


void Project::loadSymbols()
{
    // Add symbols from -s switch(es)
    for (const std::pair<Address, QString>& elem : Boomerang::get()->getSettings()->m_symbolMap) {
        m_fe->addSymbol(elem.first, elem.second);
    }

    m_fe->readLibraryCatalog(); // Needed before readSymbolFile()

    for (auto& elem : Boomerang::get()->getSettings()->m_symbolFiles) {
        LOG_MSG("Reading symbol file '%1'", elem);
        m_prog->readSymbolFile(elem);
    }
}


bool Project::decodeAll()
{
    if (SETTING(decodeMain)) {
        LOG_MSG("Decoding entry point...");
    }

    if (!m_fe || !m_fe->decode(SETTING(decodeMain))) {
        LOG_ERROR("Aborting load due to decode failure");
        return false;
    }

    bool gotMain = false;
    Address mainAddr = m_fe->getMainEntryPoint(gotMain);
    if (gotMain) {
        m_prog->addEntryPoint(mainAddr);
    }

    if (SETTING(decodeChildren)) {
        // this causes any undecoded userprocs to be decoded
        LOG_MSG("Decoding anything undecoded...");
        if (!m_fe->decode(Address::INVALID)) {
            LOG_ERROR("Aborting load due to decode failure");
            return false;
        }
    }

    return true;
}


void Project::loadPlugins()
{
    LOG_MSG("Loading plugins...");

    QDir pluginsDir = Boomerang::get()->getSettings()->getPluginDirectory();
    if (!pluginsDir.exists() || !pluginsDir.cd("loader")) {
        LOG_ERROR("Cannot open loader plugin directory '%1'!", pluginsDir.absolutePath());
        return;
    }

    for (QString fileName : pluginsDir.entryList(QDir::Files)) {
        const QString sofilename = pluginsDir.absoluteFilePath(fileName);

#ifdef _WIN32
        if (!sofilename.endsWith(".dll")) {
            continue;
        }
#endif
        try {
            std::unique_ptr<LoaderPlugin> loaderPlugin(new LoaderPlugin(sofilename));
            m_loaderPlugins.push_back(std::move(loaderPlugin));
        }
        catch (const char *errmsg) {
            LOG_WARN("Unable to load plugin: %1", errmsg);
        }
    }

    if (m_loaderPlugins.empty()) {
        LOG_ERROR("No loader plugins found, unable to load any binaries.");
    }
    else {
        LOG_MSG("Loaded plugins:");
        for (const auto& plugin : m_loaderPlugins) {
            LOG_MSG("  %1 %2 (by '%3')",
                    plugin->getInfo()->name,
                    plugin->getInfo()->version,
                    plugin->getInfo()->author
                   );
        }
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
    for (const std::unique_ptr<LoaderPlugin>& p : m_loaderPlugins) {
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

