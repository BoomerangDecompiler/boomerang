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


class QString;


/**
 * Wrapper class for platform specific handles to dynamic libraries.
 */
class PluginHandle
{
public:
    typedef void *Symbol;

public:
    PluginHandle(const QString &filePath);
    PluginHandle(const PluginHandle &other) = delete;
    PluginHandle(PluginHandle &&other)      = default;

    ~PluginHandle();

    PluginHandle &operator=(const PluginHandle &other) = delete;
    PluginHandle &operator=(PluginHandle &&other) = default;

public:
    Symbol getSymbol(const char *name) const;

private:
    void *m_handle;
};
