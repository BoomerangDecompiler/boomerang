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
    : m_settings("test", "Boomerang")
{
    m_workingDirectory = QDir("./").absolutePath();

    m_settings.setValue("DataDirectory", m_settings.value("DataDirectory", m_workingDirectory.absoluteFilePath("data/")));
    m_settings.setValue("OutputDirectory", m_settings.value("OutputDirectory", m_workingDirectory.absoluteFilePath("output/")));
    m_settings.sync();
}


/**
 * Creates a directory and tests it.
 *
 * \param dir The name of the directory.
 *
 * \retval true The directory is valid.
 * \retval false The directory is invalid.
 */
bool createDirectory(const QString& dir)
{
    return QDir::root().mkpath(QFileInfo(dir).absolutePath());
}


void Settings::setOutputDirectory(const QString& path)
{
    m_settings.setValue("OutputDirectory", path);
}
