#pragma once

#include <string>
#include <dlfcn.h>

#include "boomerang/loader/IBinaryFile.h"
#include "boomerang/core/Plugin.h"

class IBinaryImage;
class IBinarySymbolTable;


/**
 * Abstract class for loading binary and text files for analysis and decompilation.
 * The derived classes define the actual functionality of loading files and are
 * implemented as plugins.
 */
class IFileLoader
{
public:
	IFileLoader()          = default;
	virtual ~IFileLoader() = default;

	/**
	 * Initialize this loader.
	 * @param image   Binary image to load this file into.
	 * @param symbols Symbol table to fill
	 */
	virtual void initialize(IBinaryImage *image, IBinarySymbolTable *symbols) = 0;

	/// Test if this file can be loaded by this loader.
	/// This method does not necessarily need to read the whole file,
	/// only the part needed to understand the file format
//	virtual bool canLoadFile(const std::string& path) const = 0;

	/// Checks if the file can be loaded by this loader.
	/// If the file can be loaded, the function returns a score ( > 0)
	/// corresponding to the number of bytes checked.
	/// If the file cannot be loaded, this function returns 0.
	virtual int canLoad(QIODevice& data) const = 0;

	/// Load the file with path @p path into memory.
//	virtual IBinaryFile* loadFromFile(const std::string& path) = 0;

	/// Load the file from an already existing buffer.
	/// @note @p data cannot be const
	/// @returns true for a good load
	virtual bool loadFromMemory(QByteArray& data) = 0;

	/// Unload the file.
	/// Cleans up and unloads the binary image.
	virtual void unload() = 0;

	/// Close file opened with Open()
	virtual void close() = 0;

	/// Get the format (e.g. LOADFMT_ELF)
	virtual LoadFmt getFormat() const = 0;

	/// Get the expected machine (e.g. MACHINE_PENTIUM)
	virtual Machine getMachine() const = 0;

	/// @returns the address of main()/WinMain() etc.
	virtual Address getMainEntryPoint() = 0;

	/// @returns the "real" entry point, ie where execution of the program begins
	virtual Address getEntryPoint() = 0;

	/// @returns the virtual address at which the binary expects to be loaded.
	/// For position independent / relocatable code this should be NO_ADDDRESS
	virtual Address getImageBase() = 0;

	/// @returns the total size of the loaded image
	virtual size_t getImageSize() = 0;

public:
	/// Relocation functions
	virtual bool isRelocationAt(Address /*uNative*/) { return false; }

	virtual Address isJumpToAnotherAddr(Address /*uNative*/) { return NO_ADDRESS; }
	virtual bool hasDebugInfo() { return false; }

	/// Internal information
	/// Dump headers, etc
	/// Should always be overridden
	/// Should display file header, program
	/// headers and section headers, as well
	/// as contents of each of the sections.
	virtual bool displayDetails(const char * /*fileName*/, FILE * /*f*/ /* = stdout */)
	{
		return false;
	}

protected:
	/// Special load function for archive members
	/// Called after loading archive member
	virtual bool postLoad(void *handle) = 0;
};

typedef Plugin<IFileLoader> LoaderPlugin;
