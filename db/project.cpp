#include "project.h"
#include "BinaryImage.h"

Project::~Project()
{
    delete Image;
}

bool Project::serializeTo(QIODevice &dev)
{
    return false;
}

bool Project::serializeFrom(QIODevice &dev)
{
    return false;
}

IBinaryImage *Project::image()
{
    if(!Image)
        Image = new BinaryImage;
    return Image;
}
