#pragma once


#include "boomerang/core/BinaryFileFactory.h"
#include "boomerang/util/Address.h"
#include "boomerang/util/types.h"

#include <QByteArray>

typedef QByteArray IFileData;

class IBoomerang;
class QIODevice;

/// This enum allows a sort of run time type identification, without using
/// compiler specific features
enum class LoadFmt : uint8_t
{
	ELF,
	PE,
	PALM,
	PAR,
	EXE,
	MACHO,
	LX,
	COFF
};

/// determines which instruction set to use
enum class Machine : uint8_t
{
	UNKNOWN = 0,
	PENTIUM,
	SPARC,
	HPRISC,
	PALM,
	PPC,
	ST20,
	MIPS,
	M68K
};


/// This class represents the structured data in a binary file.
class IBinaryFile
{
public:
	/// @param data Raw file data
	IBinaryFile(IFileData *data);
	virtual ~IBinaryFile() = default;

public:
	/// Get the format (e.g. LOADFMT_ELF)
	virtual LoadFmt getFormat() const = 0;

	/// Get the target instruction set identifier
	virtual Machine getMachine() const = 0;

	/// Return the virtual address at which the binary expects to be loaded.
	/// For position independent / relocatable code this should be Address::INVALID
	virtual Address getImageBase() const = 0;

	/// Return the total size of the loaded image
	virtual size_t getImageSize() const = 0;

	/// Get the address of the "real" entry point, e.g. _start()
	virtual Address getEntryPoint() const = 0;

	/// Get the address of main()/WinMain
	virtual Address getMainEntryPoint() const = 0;

	/// Check if this file has debug info available
	virtual bool hasDebugInfo() const { return false; }

	virtual bool isLibraryFile() const = 0;
	virtual bool isBigEndian() const   = 0;
};
