/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: Win32BinaryFile.h
 * Desc: This file contains the definition of the class Win32BinaryFile.
*/

#ifndef __WIN32BINARYFILE_H__
#define __WIN32BINARYFILE_H_

#include "BinaryFile.h"

/* $Revision$
 * This file contains the definition of the Win32BinaryFile class, and some
 * other definitions specific to the exe version of the BinaryFile object
*/
/* At present, there is no support for a symbol table. Win32 files do
        not use dynamic linking, but it is possible that some files may
        have debug symbols (in Microsoft Codeview or Borland formats),
        and these may be implemented in the future. The debug info may
        even be exposed as another pseudo section
 * 02 Jun 00 - Mike: Added LMMH for 32 bit endianness conversions
 * 16 Apr 01 - Brian: Removed redefinition of the LH macro. LH is now
 *             defined in BinaryFile.h.
 */

#define LMMH(p) ((int)((Byte *)(&p))[0] + ((int)((Byte *)(&p))[1] << 8) + \
    ((int)((Byte *)(&p))[2] << 16) + ((int)((Byte *)(&p))[3] << 24))

typedef struct {                /* exe file header, just the signature really */
         Byte   sigLo;          /* .EXE signature: 0x4D 0x5A     */
         Byte   sigHi;
} Header;

//#ifdef WIN32
#pragma pack(1)
//#endif

typedef struct {
  Byte sigLo;
  Byte sigHi;
  SWord sigver;
  SWord cputype;
  SWord numObjects;
  DWord TimeDate;
  DWord Reserved1;
  DWord Reserved2;
  SWord NtHdrSize;
  SWord Flags;
  SWord Reserved3;
  Byte LMajor;
  Byte LMinor;
  DWord Reserved4;
  DWord Reserved5;
  DWord Reserved6;
  DWord EntrypointRVA;
  DWord Reserved7;
  DWord Reserved8;
  DWord Imagebase;
  DWord ObjectAlign;
  DWord FileAlign;
  SWord OSMajor;
  SWord OSMinor;
  SWord UserMajor;
  SWord UserMinor;
  SWord SubsysMajor;
  SWord SubsysMinor;
  DWord Reserved9;
  DWord ImageSize;
  DWord HeaderSize;
  DWord FileChecksum;
  SWord Subsystem;
  SWord DLLFlags;
  DWord StackReserveSize;
  DWord StackCommitSize;
  DWord HeapReserveSize;
  DWord HeapCommitSize;
  DWord Reserved10;
  DWord nInterestingRVASizes;
  DWord ExportTableRVA;
  DWord TotalExportDataSize;
  DWord ImportTableRVA;
  DWord TotalImportDataSize;
  DWord ResourceTableRVA;
  DWord TotalResourceDataSize;
  DWord ExceptionTableRVA;
  DWord TotalExceptionDataSize;
  DWord SecurityTableRVA;
  DWord TotalSecurityDataSize;
  DWord FixupTableRVA;
  DWord TotalFixupDataSize;
  DWord DebugTableRVA;
  DWord TotalDebugDirectories;
  DWord ImageDescriptionRVA;
  DWord TotalDescriptionSize;
  DWord MachineSpecificRVA;
  DWord MachineSpecificSize;
  DWord ThreadLocalStorageRVA;
  DWord TotalTLSSize;
} PEHeader;

typedef struct {
  char ObjectName[8];
  DWord VirtualSize;
  DWord RVA;
  DWord PhysicalSize;
  DWord PhysicalOffset;
  DWord Reserved1;
  DWord Reserved2;
  DWord Reserved3;
  DWord Flags;
} PEObject;

//#ifdef WIN32
#pragma pack(4)
//#endif

class Win32BinaryFile : public BinaryFile
{
public:
                                Win32BinaryFile();                      
// Default constructor
  virtual bool  Open(const char* sName);        // Open the file for r/w; ???
  virtual void  Close();                        // Close file opened with Open()
  virtual void  UnLoad();                       // Unload the image
  virtual char* SymbolByAddr(ADDRESS a);
  virtual LOAD_FMT GetFormat() const;           // Get format (i.e.
                                                // LOADFMT_Win32)
  virtual bool isLibrary() const;
  virtual std::list<const char *> getDependencyList();
  virtual ADDRESS getImageBase();
  virtual size_t getImageSize();

  virtual std::list<SectionInfo*>& GetEntryPoints(const char* pEntry = "main");
  virtual ADDRESS GetMainEntryPoint();
  virtual ADDRESS GetEntryPoint();
  DWord getDelta();
        char*   SymbolByAddress(const ADDRESS dwAddr);  // Get sym from addr

//
//      --      --      --      --      --      --      --      --      --
//
        // Internal information
        // Dump headers, etc
virtual bool    DisplayDetails(const char* fileName, FILE* f = stdout);

  protected:
    virtual bool  RealLoad(const char* sName); // Load the file; pure virtual

  private:

        bool    PostLoad(void* handle); // Called after archive member loaded

        Header* m_pHeader;              // Pointer to header
        PEHeader* m_pPEHeader;          // Pointer to pe header
        Byte*   m_pImage;               // Pointer to image
        int     m_cbImage;              // Size of image
        int     m_cReloc;               // Number of relocation entries
        DWord*  m_pRelocTable;          // The relocation table
        char *  base;

};

#endif          // ifndef __WIN32BINARYFILE_H__
