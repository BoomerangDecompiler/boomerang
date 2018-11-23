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


#include "boomerang/core/plugin/Plugin.h"


class PluginManager
{
public:
    PluginManager(Project *project);

public:
    bool loadPlugin(const QString &path);

    /// \param depth
    ///     < 0: recurse infinitely
    ///       0: only this directory
    ///     > 0: only this number of subdirectory levels
    bool loadPluginsFromDir(const QString &dir, int depth = 0);

    void unloadPlugins();

    const std::vector<Plugin *> &getPluginsByType(PluginType ptype);

    Plugin *getPluginByName(const QString &name);
    const Plugin *getPluginByName(const QString &name) const;

private:
    Project *m_project;

    std::map<QString, std::unique_ptr<Plugin>> m_plugins;
    std::map<PluginType, std::vector<Plugin *>> m_pluginsByType;
};
