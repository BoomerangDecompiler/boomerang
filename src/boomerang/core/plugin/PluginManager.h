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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/core/plugin/Plugin.h"


/// Loads, provides access to and unloads plugins.
/// Plugins are implemented as shared libraries.
class BOOMERANG_API PluginManager final
{
public:
    PluginManager(Project *project);

    PluginManager(const PluginManager &) = delete;
    PluginManager(PluginManager &&)      = default;

    ~PluginManager();

    PluginManager &operator=(const PluginManager &) = delete;
    PluginManager &operator=(PluginManager &&) = default;

public:
    /// Loads a single plugin from \p path.
    /// \returns true on success, or false if an error occurred (e.g. if the plugin already exists).
    bool loadPlugin(const QString &path);

    /// \param dir Base directory to search for plugins.
    /// \param depth
    ///     < 0: recurse infinitely
    ///       0: only this directory
    ///     > 0: only this number of subdirectory levels
    /// \returns true on success, false on failure.
    bool loadPluginsFromDir(const QString &dir, int depth = 0);

    /// Unloads all plugins.
    void unloadPlugins();

    /// Returns all plugins of a given type. Returns an empty vector if no plugins
    /// of the given type are loaded.
    const std::vector<Plugin *> &getPluginsByType(PluginType ptype);

    Plugin *getPluginByName(const QString &name);
    const Plugin *getPluginByName(const QString &name) const;

private:
    Project *m_project;

    std::map<QString, std::unique_ptr<Plugin>> m_plugins;        ///< All plugins sorted by name.
    std::map<PluginType, std::vector<Plugin *>> m_pluginsByType; ///< All plugins grouped by type.
};
