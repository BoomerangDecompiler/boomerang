#include "include/project.h"
#include "BinaryImage.h"
#include <cassert>
Project::~Project()
{
	delete Image;
}


bool Project::serializeTo(QIODevice& /*dev*/)
{
	assert(false);
	return false;
}


bool Project::serializeFrom(QIODevice& /*dev*/)
{
	return false;
}


IBinaryImage *Project::image()
{
	if (!Image) {
		Image = new BinaryImage;
	}

	return Image;
}
