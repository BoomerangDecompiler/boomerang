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

/// This enum allows a sort of run time type identification, without using
/// compiler specific features
enum LOAD_FMT
{
	LOADFMT_ELF,
	LOADFMT_PE,
	LOADFMT_PALM,
	LOADFMT_PAR,
	LOADFMT_EXE,
	LOADFMT_MACHO,
	LOADFMT_LX,
	LOADFMT_COFF
};

enum MACHINE
{
	MACHINE_UNKNOWN = 0,
	MACHINE_PENTIUM,
	MACHINE_SPARC,
	MACHINE_HPRISC,
	MACHINE_PALM,
	MACHINE_PPC,
	MACHINE_ST20,
	MACHINE_MIPS,
	MACHINE_68K
};


class BinaryFileFactory
{
private:
	static QString m_basePath; ///< path from which the executable is being ran, used to find lib/ directory
	std::vector<QObject *> m_loaderPlugins;

	/**
	 * Test all plugins against the file, select the one with the best match, and then return an
	 * instance of the appropriate subclass.
	 * @param sName - name of the file to load
	 * @return Instance of the plugin that can load the file with given @p sName
	 */
	QObject *getInstanceFor(const QString& sName);
	void populatePlugins();

public:
	BinaryFileFactory();

	static void setBasePath(const QString& path) { m_basePath = path; } ///< sets the base directory for plugin search
	QObject *load(const QString& sName);
	void unload();
};


#define LoaderInterface_iid    "org.boomerang.LoaderInterface"
#define ObjcInterface_iid      "org.boomerang.LoaderInterface.ObjC"
// TODO: create a default implmentation for this interface that will notify the user about the missing functionality ?


class ObjcAccessInterface
{
public:
	virtual ~ObjcAccessInterface() {}

	virtual std::map<QString, ObjcModule>& getObjcModules() = 0;
};

Q_DECLARE_INTERFACE(ObjcAccessInterface, ObjcInterface_iid)


class LoaderInterface
{
public:
	virtual ~LoaderInterface() {}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// General loader functions
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void initialize(IBoomerang *sys) = 0;
	virtual void unload() = 0;                         ///< Unload the file. Pure virtual
	virtual void close()  = 0;                         ///< Close file opened with Open()
	virtual LOAD_FMT getFormat() const            = 0; ///< Get the format (e.g. LOADFMT_ELF)
	virtual MACHINE getMachine() const            = 0; ///< Get the expected machine (e.g. MACHINE_PENTIUM)
	virtual bool loadFromMemory(QByteArray& data) = 0;
	virtual int canLoad(QIODevice& data) const    = 0;

	/// Return the virtual address at which the binary expects to be loaded.
	/// For position independent / relocatable code this should be NO_ADDDRESS
	virtual ADDRESS getImageBase() = 0;
	virtual size_t getImageSize()  = 0; ///< Return the total size of the loaded image

	virtual bool isRelocationAt(ADDRESS /*uNative*/) { return false; }

	virtual ADDRESS isJumpToAnotherAddr(ADDRESS /*uNative*/) { return NO_ADDRESS; }
	virtual bool hasDebugInfo() { return false; }

	/// @returns the address of main()/WinMain() etc.
	virtual ADDRESS getMainEntryPoint() = 0;

	/// @returns the "real" entry point, ie where execution of the program begins
	virtual ADDRESS getEntryPoint() = 0;

	///////////////////////////////////////////////////////////////////////////////
	// Internal information
	// Dump headers, etc
	virtual bool displayDetails(const char * /*fileName*/, FILE * /*f*/ /* = stdout */)
	{
		return false; // Should always be overridden
		// Should display file header, program
		// headers and section headers, as well
		// as contents of each of the sections.
	}

	// Special load function for archive members

protected:
	virtual bool postLoad(void *handle) = 0; ///< Called after loading archive member
};

Q_DECLARE_INTERFACE(LoaderInterface, LoaderInterface_iid)


struct LoaderPluginWrapper
{
	QObject *m_plugin;
	template<class T>
	T *iface() { return m_plugin ? qobject_cast<T *>(m_plugin) : nullptr; }
};
