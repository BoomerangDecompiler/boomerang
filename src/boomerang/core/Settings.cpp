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


#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"

Settings::Settings()
{
    m_workingDirectory = QDir("./").absolutePath();

    setDataDirectory("../lib/boomerang");
    setOutputDirectory("./output");
}


void Settings::setWorkingDirectory(const QString& directoryPath)
{
    m_workingDirectory = QDir(directoryPath);
    LOG_VERBOSE("wd now '%1'", m_workingDirectory.absolutePath());
}


void Settings::setDataDirectory(const QString& directoryPath)
{
    m_dataDirectory = m_workingDirectory.absoluteFilePath(directoryPath);
    LOG_VERBOSE("dd now '%1'", m_dataDirectory.absolutePath());
}


void Settings::setOutputDirectory(const QString& directoryPath)
{
    m_outputDirectory = m_workingDirectory.absoluteFilePath(directoryPath);
    LOG_VERBOSE("od now '%1'", m_outputDirectory.absolutePath());
}
