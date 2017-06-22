#pragma once

/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <string>
#include <memory>
#include <vector>

#include "boomerang/loader/IFileLoader.h"
#include "boomerang/include/types.h"
#include "boomerang/core/Plugin.h"

/// Given a pointer p, returns the 16 bits (halfword) in the two bytes
/// starting at p.
#define LH(p)    ((int)((Byte *)(p))[0] + ((int)((Byte *)(p))[1] << 8))


/**
 * This class deals with loading and determining the type of binary input files.
 * Input files can ve either executables or dynamic libraries.
 */
class BinaryFileFactory
{
public:

	BinaryFileFactory();

	/// @param pluginsPath Path of the directory where the loader plugins are located.
	static void setPluginsPath(const std::string& pluginsPath);
	
	/// Load the binary file located at @p filePath.
	/// Automatically returns the appropriate loader for the binary file.
	IFileLoader *loadFile(const std::string& filePath);
	
private:
	/**
	 * Test all plugins against the file, select the one with the best match, and then return an
	 * instance of the appropriate subclass.
	 * @param filePath - name of the file to load
	 * @return Instance of the plugin that can load the file with given @p filePath
	 */
	IFileLoader *getInstanceFor(const std::string& filePath);
	
	/// load all suitable plugins from the plugin directory.
	void populatePlugins();
	
private:
	std::vector<std::shared_ptr<LoaderPlugin> > m_loaderPlugins; /// all loaded loader plugins.
	static std::string m_pluginsPath; ///< Path to the direcory containing the loader plugins.
};
