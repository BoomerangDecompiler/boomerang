#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Plugin.h"


Plugin::Plugin(const QString &pluginPath)
    : m_pluginHandle(pluginPath)
    , m_ifc(nullptr)
{
    if (!init()) {
        throw "Plugin initialization function not found!";
    }
}


Plugin::~Plugin()
{
    deinit();
    // library is automatically unloaded
}


const PluginInfo *Plugin::getInfo() const
{
    PluginInfoFunction infoFunction = getFunction<PluginInfoFunction>("getInfo");
    if (!infoFunction) {
        return nullptr;
    }
    else {
        return infoFunction();
    }
}


bool Plugin::init()
{
    const PluginInfo *info = getInfo();
    if (!info) {
        return false;
    }

    assert(m_ifc == nullptr);
    PluginInitFunction initFunction = getFunction<PluginInitFunction>("initPlugin");
    if (!initFunction) {
        return false;
    }

    m_ifc = initFunction();
    return m_ifc != nullptr;
}


bool Plugin::deinit()
{
    assert(m_ifc != nullptr);
    PluginDeinitFunction deinitFunction = getFunction<PluginDeinitFunction>("deinitPlugin");
    if (!deinitFunction) {
        return false;
    }

    deinitFunction();
    m_ifc = nullptr;
    return true;
}
