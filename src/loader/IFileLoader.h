#pragma once

#include <string>
#include <dlfcn.h>

class IBinaryFile;

#include "loader/IBinaryFile.h"
#include "boom_base/Plugin.h"

class LoaderInterface
{
public:
	virtual ~LoaderInterface() {}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// General loader functions
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void initialize(IBoomerang *sys) = 0;

	/// Unload the file.
	/// Cleans up and unloads the binary image.
	virtual void unload() = 0;
	virtual void close()  = 0;               ///< Close file opened with Open()
	virtual LOAD_FMT getFormat() const = 0;  ///< Get the format (e.g. LOADFMT_ELF)
	virtual MACHINE getMachine() const = 0;  ///< Get the expected machine (e.g. MACHINE_PENTIUM)

	/// Checks if the file can be loaded by this loader.
	virtual bool canLoad(QIODevice& data) const = 0;

	/// @returns true for a good load
	virtual bool loadFromMemory(QByteArray& data) = 0;

	/// @returns the address of main()/WinMain() etc.
	virtual ADDRESS getMainEntryPoint() = 0;

	/// @returns the "real" entry point, ie where execution of the program begins
	virtual ADDRESS getEntryPoint() = 0;

	/// Return the virtual address at which the binary expects to be loaded.
	/// For position independent / relocatable code this should be NO_ADDDRESS
	virtual ADDRESS getImageBase() = 0;
	virtual size_t getImageSize()  = 0; ///< Return the total size of the loaded image

public:
	/// Relocation functions
	virtual bool isRelocationAt(ADDRESS /*uNative*/) { return false; }

	virtual ADDRESS isJumpToAnotherAddr(ADDRESS /*uNative*/) { return NO_ADDRESS; }
	virtual bool hasDebugInfo() { return false; }

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


/// Class for loading a binary file or an assembly listing from a file.
class IFileLoader : public LoaderInterface
{
public:
//	IFileLoader();
//	virtual ~IFileLoader() = default;

public:
	/// Test if this file can be loaded by this loader.
	/// This method does not necessarily need to read the whole file,
	/// only the part needed to understand the file format
//	virtual bool canLoadFile(const std::string& path) const = 0;

	/// Load the file with path @p path into memory.
//	virtual IBinaryFile* loadFile(const std::string& path) = 0;
};

typedef Plugin<IFileLoader> LoaderPlugin;
