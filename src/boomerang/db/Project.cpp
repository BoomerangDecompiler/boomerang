#include "Project.h"

#include "boomerang/db/BinaryImage.h"
#include "boomerang/util/Log.h"
#include "boomerang/core/BinaryFileFactory.h"

Project::Project()
{
    m_image.reset(new BinaryImage);
}


Project::~Project()
{
}


bool Project::loadBinaryFile(const QString& filePath)
{
    BinaryFileFactory bff;
    IFileLoader* loader = bff.loadFile(filePath);

    return loader != nullptr;
}


bool Project::loadSaveFile(const QString& /*filePath*/)
{
    LOG_FATAL("Loading save files is not implemented.");
    return false;
}


bool Project::writeSaveFile(const QString& /*filePath*/)
{
    LOG_FATAL("Saving save files is not implemented.");
    return false;
}


bool Project::isBinaryLoaded() const
{
    return false; // stub
}


void Project::unload()
{
}
