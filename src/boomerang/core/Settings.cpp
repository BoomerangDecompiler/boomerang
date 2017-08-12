#include "Settings.h"

#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"

Settings::Settings()
    : m_workingDirectory("./")
    , m_outputDirectory("./output/")
{}



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


bool Settings::setOutputDirectory(const QString& path)
{
    m_outputDirectory = QDir(path);

    // Create the output directory, if needed
    if (!createDirectory(path)) {
        LOG_ERROR("Could not create output directory %1", m_outputDirectory.path());
        return false;
    }

    return true;
}

QString Settings::getFilename() const
{
    return "";
}
