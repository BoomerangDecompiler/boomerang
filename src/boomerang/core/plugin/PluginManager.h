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
    PluginManager();

public:
    bool loadPlugin(const QString &path);
    bool loadPluginsFromDir(const QString &dir);

    void unloadPlugins();

    const std::vector<Plugin *> &getPluginsByType(PluginType ptype);

    const Plugin *getPluginByName(const QString &name) const;

private:
    std::map<QString, std::unique_ptr<Plugin>> m_plugins;
    std::map<PluginType, std::vector<Plugin *>> m_pluginsByType;
};
