#pragma once

#define IPROJECT

class IBinaryImage;
class QByteArray;
struct ITypeRecovery;

/**
 * @brief The Project interface class
 */
class IProject
{
public:
	virtual ~IProject() {}

	virtual QByteArray& getFiledata()             = 0;
	virtual const QByteArray& getFiledata() const = 0;
	virtual IBinaryImage *getOrCreateImage()      = 0;

	virtual const ITypeRecovery *getTypeEngine() const = 0;
};
