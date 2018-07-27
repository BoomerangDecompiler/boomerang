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
#include "boomerang/core/Watcher.h"
#include "boomerang/db/Prog.h"
#include "boomerang/decomp/ProgDecompiler.h"
#include "boomerang/type/dfa/DFATypeRecovery.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/ProgSymbolWriter.h"
#include "boomerang/util/CallGraphDotWriter.h"


Project::Project()
    : m_settings(new Settings())
    , m_typeRecovery(new DFATypeRecovery())
    , m_codeGenerator(new CCodeGenerator())
{
}


Project::~Project()
{
}


const char *Project::getVersionStr() const
{
    return BOOMERANG_VERSION;
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

    if (!getSettings()->m_entryPoints.empty()) { // decode only specified procs
        // decode entry points from -e (and -E) switch(es)
        for (auto& elem : getSettings()->m_entryPoints) {
            LOG_MSG("Decoding specified entrypoint at address %1", elem);
            m_prog->decodeEntryPoint(elem);
        }
    }
    else if (!decodeAll()) { // decode everything
        return false;
    }

    this->alertEndDecode();

    LOG_MSG("Found %1 procs", m_prog->getNumFunctions());

    if (getSettings()->generateSymbols) {
        ProgSymbolWriter().writeSymbolsToFile(getProg(), "symbols.h");
    }

    if (getSettings()->generateCallGraph) {
        CallGraphDotWriter().writeCallGraph(getProg(), "callgraph.dot");
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

    ProgDecompiler dcomp(m_prog.get());
    dcomp.decompile();

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
    for (const std::pair<Address, QString>& elem : getSettings()->m_symbolMap) {
        m_loadedBinary->getSymbols()->createSymbol(elem.first, elem.second);
    }

    m_fe->readLibraryCatalog(); // Needed before readSymbolFile()

    for (auto& sf : getSettings()->m_symbolFiles) {
        LOG_MSG("Reading symbol file '%1'", sf);
        m_fe->addSymbolsFromSymbolFile(sf);
    }
}


bool Project::decodeAll()
{
    if (getSettings()->decodeMain) {
        LOG_MSG("Decoding entry point...");
    }

    if (!m_fe || !m_fe->decodeEntryPointsRecursive(getSettings()->decodeMain)) {
        LOG_ERROR("Aborting load due to decode failure");
        return false;
    }

    bool gotMain = false;
    Address mainAddr = m_fe->getMainEntryPoint(gotMain);
    if (gotMain) {
        m_prog->addEntryPoint(mainAddr);
    }

    if (getSettings()->decodeChildren) {
        // this causes any undecoded userprocs to be decoded
        LOG_MSG("Decoding anything undecoded...");
        if (!m_fe->decodeUndecoded()) {
            LOG_ERROR("Aborting load due to decode failure");
            return false;
        }
    }

    return true;
}


void Project::loadPlugins()
{
    LOG_MSG("Loading plugins...");

    QDir pluginsDir = getSettings()->getPluginDirectory();
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


void Project::addWatcher(IWatcher *watcher)
{
    m_watchers.insert(watcher);
}


void Project::alertDecompileDebugPoint(UserProc *p, const char *description)
{
    for (IWatcher *elem : m_watchers) {
        elem->onDecompileDebugPoint(p, description);
    }
}


void Project::alertFunctionCreated(Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->onFunctionCreated(function);
    }
}


void Project::alertFunctionRemoved(Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->onFunctionRemoved(function);
    }
}


void Project::alertSignatureUpdated(Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->onSignatureUpdated(function);
    }
}


void Project::alertInstructionDecoded(Address pc, int numBytes)
{
    for (IWatcher *it : m_watchers) {
        it->onInstructionDecoded(pc, numBytes);
    }
}


void Project::alertBadDecode(Address pc)
{
    for (IWatcher *it : m_watchers) {
        it->onBadDecode(pc);
    }
}


void Project::alertFunctionDecoded(Function *p, Address pc, Address last, int numBytes)
{
    for (IWatcher *it : m_watchers) {
        it->onFunctionDecoded(p, pc, last, numBytes);
    }
}


void Project::alertStartDecode(Address start, int numBytes)
{
    for (IWatcher *it : m_watchers) {
        it->onStartDecode(start, numBytes);
    }
}


void Project::alertEndDecode()
{
    for (IWatcher *it : m_watchers) {
        it->onEndDecode();
    }
}


void Project::alertStartDecompile(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->onStartDecompile(proc);
    }
}


void Project::alertProcStatusChanged(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->onProcStatusChange(proc);
    }
}


void Project::alertEndDecompile(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->onEndDecompile(proc);
    }
}


void Project::alertDiscovered(Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->onFunctionDiscovered(function);
    }
}


void Project::alertDecompiling(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->onDecompileInProgress(proc);
    }
}


void Project::alertDecompilationEnd()
{
    for (IWatcher *w : m_watchers) {
        w->onDecompilationEnd();
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

