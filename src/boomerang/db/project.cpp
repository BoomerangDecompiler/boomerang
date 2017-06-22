
#include "project.h"

#include "boomerang/db/BinaryImage.h"

#include <cassert>


Project::~Project()
{
	delete m_image;
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


IBinaryImage *Project::getOrCreateImage()
{
	if (!m_image) {
		m_image = new BinaryImage;
	}

	return m_image;
}
