#pragma once

/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/**
 * @file: BinaryFile.h
 * @brief: This file contains the definition of the abstract class BinaryFile
 */


/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/

#include "include/types.h"
#include "db/SectionInfo.h"

#include "loader/IFileLoader.h"

#include <QStringList>
#include <QDebug>
#include <QString>
#include <cassert>
#include <list>
#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include <cstdio> // For FILE

/// Given a pointer p, returns the 16 bits (halfword) in the two bytes
/// starting at p.
#define LH(p)    ((int)((Byte *)(p))[0] + ((int)((Byte *)(p))[1] << 8))

class IBoomerang;

// Objective-C stuff
class ObjcIvar
{
public:
	QString name, type;
	unsigned offset;
};

class ObjcMethod
{
public:
	QString name, types;
	ADDRESS addr;
};

class ObjcClass
{
public:
	QString name;
	std::map<QString, ObjcIvar> ivars;
	std::map<QString, ObjcMethod> methods;
};

class ObjcModule
{
public:
	QString name;
	std::map<QString, ObjcClass> classes;
};


class BinaryFileFactory
{
private:
	static QString m_basePath; ///< path from which the executable is being ran, used to find lib/ directory
	std::vector<std::shared_ptr<LoaderPlugin> > m_loaderPlugins;

	/**
	 * Test all plugins against the file, select the one with the best match, and then return an
	 * instance of the appropriate subclass.
	 * @param sName - name of the file to load
	 * @return Instance of the plugin that can load the file with given @p sName
	 */
	IFileLoader *getInstanceFor(const QString& sName);
	void populatePlugins();

public:
	BinaryFileFactory();

	static void setBasePath(const QString& path) { m_basePath = path; } ///< sets the base directory for plugin search
	IFileLoader *load(const QString& sName);
	void unload();
};


class ObjcAccessInterface
{
public:
	virtual ~ObjcAccessInterface() {}

	virtual std::map<QString, ObjcModule>& getObjcModules() = 0;
};
