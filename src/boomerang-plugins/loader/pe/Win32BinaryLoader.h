#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/ifc/IFileLoader.h"

#include <string>

/**
 * This file contains the definition of the Win32BinaryLoader class.
 *
 * At present, there is no support for a symbol table. Win32 files do
 * not use dynamic linking, but it is possible that some files may
 * have debug symbols (in Microsoft Codeview or Borland formats),
 * and these may be implemented in the future. The debug info may
 * even be exposed as another pseudo section
 */


typedef struct /* exe file header, just the signature really */
{
    Byte sigLo; /* .EXE signature: 0x4D 0x5A     */
    Byte sigHi;
} Header;

#pragma pack(push, 1)

/// IMAGE_OPTIONAL_HEADER on Windows
typedef struct
{
    Byte sigLo;
    Byte sigHi;
    SWord sigver;
    SWord cputype;       ///< Machine
    SWord numObjects;    ///< number of sections
    DWord TimeDate;      ///< DateTime stamp
    DWord Reserved1;     ///< Pointer to symbol table (deprecated)
    DWord Reserved2;     ///< number of entries in symbol table (deprecated)
    SWord NtHdrSize;     ///< Size of the optional header
    SWord Flags;         ///< Characteristics
    SWord Reserved3;     ///< Magic number
    Byte LMajor;         ///< Linker version major
    Byte LMinor;         ///< Linker version minor
    DWord Reserved4;     ///< SizeOfCode (sum of all sections)
    DWord Reserved5;     ///< Size of initialized data
    DWord Reserved6;     ///< Size of uninitialized data
    DWord EntrypointRVA; ///< Address of entry point (RVA)
    DWord Reserved7;     ///< Base of code (RVA)
    DWord Reserved8;     ///< Base of data (RVA)

    // Windows specific
    DWord Imagebase;            ///< image base
    DWord ObjectAlign;          ///< section alignment
    DWord FileAlign;            ///< File alignment
    SWord OSMajor;              ///< OS version major
    SWord OSMinor;              ///< OS version minor
    SWord UserMajor;            ///< Image version major
    SWord UserMinor;            ///< Image version minor
    SWord SubsysMajor;          ///< Subsystem version major
    SWord SubsysMinor;          ///< Subsystem version minor
    DWord Reserved9;            ///< Win32 version value (zero-filled)
    DWord ImageSize;            ///< Size of image
    DWord HeaderSize;           ///< Size of headers
    DWord FileChecksum;         ///< File checksum (usually 0)
    SWord Subsystem;            ///< Subsystem
    SWord DLLFlags;             ///< DLL characteristics
    DWord StackReserveSize;     ///< Size of stack reserve
    DWord StackCommitSize;      ///< Size of stack commit
    DWord HeapReserveSize;      ///< Size of heap reserve
    DWord HeapCommitSize;       ///< Size of heap commit
    DWord Reserved10;           ///< Loader flags (zero-filled)
    DWord nInterestingRVASizes; ///< Number of RVA and sizes (see below)

    // data directories
    DWord ExportTableRVA;         ///< Export table RVA
    DWord TotalExportDataSize;    ///< Export table size
    DWord ImportTableRVA;         ///< Import table RVA
    DWord TotalImportDataSize;    ///< Import table size
    DWord ResourceTableRVA;       ///< Resource table RVA
    DWord TotalResourceDataSize;  ///< Resource table size
    DWord ExceptionTableRVA;      ///< Exception table RVA
    DWord TotalExceptionDataSize; ///< Exception table size
    DWord SecurityTableRVA;       ///< Certificate table RVA
    DWord TotalSecurityDataSize;  ///< Certificate table size
    DWord FixupTableRVA;          ///< Base relocation table RVA
    DWord TotalFixupDataSize;     ///< Base relocation table size
    DWord DebugTableRVA;          ///< Debug table RVA
    DWord TotalDebugDirectories;  ///< Debug table size
    DWord ImageDescriptionRVA;    ///< Architecture data table RVA
    DWord TotalDescriptionSize;   ///< Architecture data table size
    DWord MachineSpecificRVA;     ///< Global pointer RVA
    DWord MachineSpecificSize;    ///< Must be 0
    DWord ThreadLocalStorageRVA;  ///< TLS table RVA
    DWord TotalTLSSize;           ///< TLS table size
} PEHeader;


/// The real Win32 name of this struct is IMAGE_SECTION_HEADER
typedef struct
{
    char ObjectName[8]; // Name
    DWord VirtualSize;
    DWord RVA;            // VirtualAddress
    DWord PhysicalSize;   // SizeOfRawData
    DWord PhysicalOffset; // PointerToRawData
    DWord Reserved1;      // PointerToRelocations
    DWord Reserved2;      // PointerToLinenumbers
    DWord Reserved3;      // WORD NumberOfRelocations; WORD NumberOfLinenumbers;
    DWord Flags;          // Characteristics
} PEObject;

typedef struct
{
    DWord originalFirstThunk; // 0 for end of array; also ptr to hintNameArray
    DWord preSnapDate;        // Time and date the import data was pre-snapped
    // or zero if not pre-snapped
    SWord verMajor;   // Major version number of dll being ref'd
    SWord verMinor;   // Minor "         "
    DWord name;       // RVA of dll name (ascii)
    DWord firstThunk; // RVA of start of import address table (IAT)
} PEImportDtor;

typedef struct
{
    DWord flags;         // Reserved; 0
    DWord stamp;         // Time/date stamp export data was created
    SWord verMajor;      // Version number can be ...
    SWord verMinor;      //     ... set by user
    DWord name;          // RVA of the ascii string containing the name of the DLL
    DWord base;          // Starting ordinal number for exports in this image. Usually set to 1.
    DWord numEatEntries; // Number of entries in EAT (Export ADdress Table)
    DWord numNptEntries; // Number of entries in NPT (Name Pointer Table) (also #entries in the
                         // Ordinal Table)
    DWord eatRVA;        // RVA of the EAT
    DWord nptRVA;        // RVA of the NPT
    DWord otRVA;         // RVA of the OT
} PEExportDtor;

#pragma pack(pop)


/// Class for loading Win32 PE ".exe" files.
class BOOMERANG_PLUGIN_API Win32BinaryLoader : public IFileLoader
{
public:
    Win32BinaryLoader(Project *project);
    virtual ~Win32BinaryLoader();

public:
    /// \copydoc IFileLoader::initialize
    void initialize(BinaryFile *file, BinarySymbolTable *symbols) override;

    /// \copydoc IFileLoader::canLoad
    int canLoad(QIODevice &fl) const override;

    /// \copydoc IFileLoader::loadFromMemory
    bool loadFromMemory(QByteArray &arr) override;

    /// \copydoc IFileLoader::unload
    void unload() override;

    /// \copydoc IFileLoader::close
    void close() override;

    /// \copydoc IFileLoader::getFormat
    LoadFmt getFormat() const override;

    /// \copydoc IFileLoader::getMachine
    Machine getMachine() const override;

    /// \copydoc IFileLoader::getMainEntryPoint
    Address getMainEntryPoint() override;

    /// \copydoc IFileLoader::getEntryPoint
    Address getEntryPoint() override;

public:
    /// \copydoc IFileLoader::getJumpTarget
    Address getJumpTarget(Address addr) const override;

    /// \copydoc IFileLoader::hasDebugInfo
    bool hasDebugInfo() const override { return m_hasDebugInfo; }

    bool isLibrary() const;

protected:
    SWord win32Read2(const void *src) const; ///< Read 2 bytes from native addr
    DWord win32Read4(const void *src) const; ///< Read 4 bytes from native addr

public:
    bool isStaticLinkedLibProc(Address addr) const;
    bool isMinGWsAllocStack(Address addr) const;
    bool isMinGWsFrameInit(Address addr) const;
    bool isMinGWsFrameEnd(Address addr) const;
    bool isMinGWsCleanupSetup(Address addr) const;
    bool isMinGWsMalloc(Address addr) const;

protected:
    void processIAT();
    void readDebugData(QString exename);

private:
    /// Find names for jumps to IATs
    void findJumps(Address curr);

private:
    char *m_image;     ///< Beginning of the loaded image
    DWord m_imageSize; ///< Size of image, in bytes

    Header *m_header;     ///< Pointer to header
    PEHeader *m_peHeader; ///< Pointer to pe header
    int m_numRelocs;      ///< Number of relocation entries
    bool m_hasDebugInfo;
    bool m_mingwMain;

    BinaryImage *m_binaryImage;
    BinarySymbolTable *m_symbols;
};
