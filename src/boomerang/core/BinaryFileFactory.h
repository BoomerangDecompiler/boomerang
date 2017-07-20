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
#include "boomerang/util/types.h"
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

    /// Load the binary file located at \p filePath.
    /// Automatically returns the most appropriate loader for the binary file.
    IFileLoader *loadFile(const QString& filePath);

private:
    /**
     * Test all plugins against the file, select the one with the best match, and then return an
     * instance of the appropriate subclass.
     * \param filePath - name of the file to load
     * \returns Instance of the plugin that can load the file with given \p filePath
     */
    IFileLoader *getInstanceFor(const QString& filePath);

    /// load all suitable plugins from the plugin directory.
    void populatePlugins();

private:
    std::vector<std::shared_ptr<LoaderPlugin> > m_loaderPlugins; /// all loaded loader plugins.
    static QString m_pluginsPath;                                ///< Path to the direcory containing the loader plugins.
};
