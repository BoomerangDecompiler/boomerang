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


#include "boomerang/core/plugin/PluginHandle.h"
#include "boomerang/core/plugin/PluginType.h"

#include <QString>

#include <cassert>


struct PluginInfo
{
    PluginType type;     ///< type of plugin (loader, etc)
    const char *name;    ///< Name of this plugin
    const char *version; ///< Plugin version
    const char *author;  ///< Plugin creator (copyright information)
};


/**
 * Class for managing an interface plugin.
 *
 * - The plugin must define the following functions (with extern "C" linkage):
 *   - IFC* initPlugin(): to initialize the plugin class and allocate resources etc.
 *     You must ensure the returned pointer is valid until deinitPlugin() is called.
 *   - void deinitPlugin(): to deinitialize the plugin and free resources.
 *   - const PluginInfo* getInfo(): To get information about the plugin.
 *     May be called before initPlugin().
 */
class Plugin
{
    using PluginInitFunction   = void *(*)(Project *);
    using PluginDeinitFunction = void (*)();
    using PluginInfoFunction   = const PluginInfo *(*)();

public:
    /// Create a plugin from a dynamic library file.
    /// \param pluginPath path to the library file.
    explicit Plugin(Project *project, const QString &pluginPath);

    Plugin(const Plugin &other) = delete;
    Plugin(Plugin &&other)      = default;

    ~Plugin();

    Plugin &operator=(const Plugin &other) = delete;
    Plugin &operator=(Plugin &&other) = default;

public:
    /// Get information about the plugin.
    const PluginInfo *getInfo() const;

    template<typename IFC>
    IFC *getIfc()
    {
        return static_cast<IFC *>(m_ifc);
    }

    template<typename IFC>
    const IFC *getIfc() const
    {
        return static_cast<const IFC *>(m_ifc);
    }

private:
    /// Initialize the plugin.
    bool init(Project *project);

    /// De-initialize the plugin.
    bool deinit();

    /// Given a non-mangled function name (e.g. initPlugin),
    /// get the function pointer for the function exported by this plugin.
    template<class FuncPtr>
    FuncPtr getFunction(const char *name) const
    {
        PluginHandle::Symbol symbol = m_pluginHandle.getSymbol(name);

        if (!symbol) {
            return nullptr;
        }
        else {
            return *reinterpret_cast<FuncPtr *>(&symbol);
        }
    }

private:
    PluginHandle m_pluginHandle; ///< handle to the dynamic library
    void *m_ifc;                 ///< Interface pointer (e.g. IDecoder * for decoder plugins)
};


/**
 * Usage:
 *   BOOMERANG_DEFINE_PLUGIN(PluginType::FileLoader, TestLoader,
 *                           "TestLoader Plugin", "3.1.4", "test");
 */
#define BOOMERANG_DEFINE_PLUGIN(Type, Classname, PName, PVersion, PAuthor)                         \
    static Classname *g_pluginInstance = nullptr;                                                  \
    extern "C"                                                                                     \
    {                                                                                              \
        typedef PluginIfcTraits<Type>::IFC Interface;                                              \
        static_assert(std::is_base_of<Interface, Classname>::value,                                \
                      #Classname " must be derived from the correct plugin interface!");           \
                                                                                                   \
        Q_DECL_EXPORT Interface *initPlugin(Project *project)                                      \
        {                                                                                          \
            if (!g_pluginInstance) {                                                               \
                g_pluginInstance = new Classname(project);                                         \
            }                                                                                      \
            return g_pluginInstance;                                                               \
        }                                                                                          \
                                                                                                   \
        Q_DECL_EXPORT void deinitPlugin()                                                          \
        {                                                                                          \
            delete g_pluginInstance;                                                               \
            g_pluginInstance = nullptr;                                                            \
        }                                                                                          \
                                                                                                   \
        Q_DECL_EXPORT const PluginInfo *getInfo()                                                  \
        {                                                                                          \
            static PluginInfo info;                                                                \
            info.name    = PName;                                                                  \
            info.version = PVersion;                                                               \
            info.author  = PAuthor;                                                                \
            info.type    = Type;                                                                   \
            return &info;                                                                          \
        }                                                                                          \
    }
