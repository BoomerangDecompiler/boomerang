#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Settings.h"

#include "boomerang/util/log/Log.h"


Settings::Settings()
{
    m_workingDirectory = QDir("./").absolutePath();

    setDataDirectory("../share/boomerang");
    setPluginDirectory("../lib/boomerang/plugins");
    setOutputDirectory("./output");
}


void Settings::setWorkingDirectory(const QString &directoryPath)
{
    m_workingDirectory = QDir(directoryPath);
    LOG_VERBOSE("wd now '%1'", m_workingDirectory.absolutePath());
}


void Settings::setDataDirectory(const QString &directoryPath)
{
    m_dataDirectory = m_workingDirectory.absoluteFilePath(directoryPath);
    LOG_VERBOSE("dd now '%1'", m_dataDirectory.absolutePath());
}


void Settings::setPluginDirectory(const QString &directoryPath)
{
    m_pluginDirectory = m_workingDirectory.absoluteFilePath(directoryPath);
    LOG_VERBOSE("pd now '%1'", m_pluginDirectory.absolutePath());
}


void Settings::setOutputDirectory(const QString &directoryPath)
{
    m_outputDirectory = m_workingDirectory.absoluteFilePath(directoryPath);
    LOG_VERBOSE("od now '%1'", m_outputDirectory.absolutePath());
}
