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

#include <QCoreApplication>


Settings::Settings()
{
    m_workingDirectory.setPath(QDir("./").absolutePath());

    const QString appDirPath = QCoreApplication::applicationDirPath();
    setDataDirectory(appDirPath + "/../share/boomerang");
    setPluginDirectory(appDirPath + "/../lib/boomerang/plugins");
    setOutputDirectory("./output");
}


void Settings::setWorkingDirectory(const QString &directoryPath)
{
    m_workingDirectory.setPath(directoryPath);
    LOG_VERBOSE("wd now '%1'", m_workingDirectory.absolutePath());
}


void Settings::setDataDirectory(const QString &directoryPath)
{
    m_dataDirectory.setPath(m_workingDirectory.absoluteFilePath(directoryPath));
    LOG_VERBOSE("dd now '%1'", m_dataDirectory.absolutePath());
}


void Settings::setPluginDirectory(const QString &directoryPath)
{
    m_pluginDirectory.setPath(m_workingDirectory.absoluteFilePath(directoryPath));
    LOG_VERBOSE("pd now '%1'", m_pluginDirectory.absolutePath());
}


void Settings::setOutputDirectory(const QString &directoryPath)
{
    m_outputDirectory.setPath(m_workingDirectory.absoluteFilePath(directoryPath));
    LOG_VERBOSE("od now '%1'", m_outputDirectory.absolutePath());
}
