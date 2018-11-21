#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PluginManager.h"

#include "boomerang/util/log/Log.h"

#include <QDir>


PluginManager::PluginManager()
{
}


bool PluginManager::loadPlugin(const QString &path)
{
    try {
        std::unique_ptr<Plugin> newPlugin = std::make_unique<Plugin>(path);

        const QString pluginName = newPlugin->getInfo()->name;
        if (m_plugins.find(pluginName) != m_plugins.end()) {
            throw "A plugin with the same name already exists";
        }

        Plugin *loadedPlugin  = newPlugin.get();
        m_plugins[pluginName] = std::move(newPlugin);
        m_pluginsByType[loadedPlugin->getInfo()->type].push_back(loadedPlugin);
        return true;
    }
    catch (const char *errmsg) {
        LOG_WARN("Unable to load plugin: %1", errmsg);
        return false;
    }
}


bool PluginManager::loadPluginsFromDir(const QString &dir)
{
    QDir pluginsDir(dir);
    if (!pluginsDir.exists()) {
        return false;
    }

    for (QString fileName : pluginsDir.entryList(QDir::Files)) {
        const QString sofilename = pluginsDir.absoluteFilePath(fileName);

#ifdef _WIN32
        if (!sofilename.endsWith(".dll")) {
            continue;
        }
#endif
        loadPlugin(sofilename);
    }

    if (getPluginsByType(PluginType::FileLoader).empty()) {
        LOG_ERROR("No loader plugins found, unable to load any binaries.");
    }
    else {
        LOG_MSG("Loaded plugins:");
        for (const Plugin *plugin : getPluginsByType(PluginType::FileLoader)) {
            LOG_MSG("  %1 %2 (by '%3')", plugin->getInfo()->name, plugin->getInfo()->version,
                    plugin->getInfo()->author);
        }
    }

    return true;
}


void PluginManager::unloadPlugins()
{
    m_pluginsByType.clear();
}


const std::vector<Plugin *> &PluginManager::getPluginsByType(PluginType ptype)
{
    return m_pluginsByType[ptype];
}


const Plugin *PluginManager::getPluginByName(const QString &name) const
{
    auto it = m_plugins.find(name);
    return (it != m_plugins.end()) ? it->second.get() : nullptr;
}
