/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: Win32BinaryFile.cc
 * Desc: This file contains the implementation of the class Win32BinaryFile.
 */

/* Win32 binary file format.
 *  This file implements the class Win32BinaryFile, derived from class
 *  BinaryFile. See Win32BinaryFile.h and BinaryFile.h for details.
 * 02/06/2000 - Trent: Created
 * 02 Jun 00 - Mike: endianness conversions
 * 05 Jun 00 - Mike: find main using a very simple pattern
 * 15 Feb 01 - Nathan: move main finder out to prog.cc
 * 16 Apr 01 - Brian: Use the LH macro now defined in BinaryFile.h.
 */

#include "BinaryFile.h"
#include "Win32BinaryFile.h"

Win32BinaryFile::Win32BinaryFile()
{
}

bool Win32BinaryFile::Open(const char* sName) {
    return Load(sName);
}

void Win32BinaryFile::Close() {
    UnLoad();
}

std::list<SectionInfo*>& Win32BinaryFile::GetEntryPoints(
    const char* pEntry)
{
    fprintf(stderr,"really don't know how to implement GetEntryPoints\n");
    exit(0);
    static std::list<SectionInfo*> l;
    return l;
}

ADDRESS Win32BinaryFile::GetEntryPoint()
{
    return (ADDRESS)(LMMH(m_pPEHeader->EntrypointRVA) +
                     LMMH(m_pPEHeader->Imagebase));
}

ADDRESS Win32BinaryFile::GetMainEntryPoint()
{
    /* Should we try checking the symbol table for WinMain, LibMain, etc? */
    return NO_ADDRESS;
}

bool Win32BinaryFile::RealLoad(const char* sName)
{

    FILE *fp = fopen(sName,"rb");

    DWord peoffLE, peoff;
    fseek(fp, 0x3c, SEEK_SET);
    fread(&peoffLE, 4, 1, fp);      // Note: peoffLE will be in Little Endian
    peoff = LMMH(peoffLE);

    PEHeader tmphdr;

    fseek(fp, peoff, SEEK_SET);
    fread(&tmphdr, sizeof(tmphdr), 1, fp);
    // Note: all tmphdr fields will be little endian

    base = (char *)malloc(LMMH(tmphdr.ImageSize));

    if (!base) {
      fprintf(stderr,"Cannot allocate memory for copy of image\n");
      return false;
    }

    fseek(fp, 0, SEEK_SET);

    fread(base, LMMH(tmphdr.HeaderSize), 1, fp);

    m_pHeader = (Header *)base;
    if (m_pHeader->sigLo!='M' || m_pHeader->sigHi!='Z') {
        fprintf(stderr,"error loading file %s, bad magic\n", sName);
        return false;
    }

    m_pPEHeader = (PEHeader *)(base+peoff);
    if (m_pPEHeader->sigLo!='P' || m_pPEHeader->sigHi!='E') {
        fprintf(stderr,"error loading file %s, bad PE magic\n", sName);
        return false;
    }

printf("Image Base %08X, real base %p\n", LMMH(m_pPEHeader->Imagebase), base);

    PEObject *o = (PEObject *)(
        ((char *)m_pPEHeader) + LH(&m_pPEHeader->NtHdrSize) + 24);
    m_iNumSections = LH(&m_pPEHeader->numObjects);
    m_pSections = new SectionInfo[m_iNumSections];
    for (int i=0; i<m_iNumSections; i++, o++) {
      printf("%.8s RVA=%08X Offset=%08X size=%08X\n",
        (char*)o->ObjectName, LMMH(o->RVA), LMMH(o->PhysicalOffset),
        LMMH(o->VirtualSize));
      m_pSections[i].pSectionName = new char[9];
      strncpy(m_pSections[i].pSectionName, o->ObjectName, 8);
      m_pSections[i].uNativeAddr=(ADDRESS)(LMMH(o->RVA) +
        LMMH(m_pPEHeader->Imagebase));
      m_pSections[i].uHostAddr=(ADDRESS)(LMMH(o->RVA) + base);
      m_pSections[i].uSectionSize=LMMH(o->VirtualSize);
      DWord Flags = LMMH(o->Flags);
      m_pSections[i].bBss       = Flags&0x80?1:0;
      m_pSections[i].bCode      = Flags&0x20?1:0;
      m_pSections[i].bData      = Flags&0x40?1:0;
      m_pSections[i].bReadOnly  = Flags&0x80000000?0:1;
      fseek(fp, LMMH(o->PhysicalOffset), SEEK_SET);
      memset(base + LMMH(o->RVA), 0, LMMH(o->VirtualSize));
      fread(base + LMMH(o->RVA), LMMH(o->PhysicalSize), 1, fp);
    }

    fclose(fp);
    return true;
}

// Clean up and unload the binary image
void Win32BinaryFile::UnLoad()
{
} 

bool Win32BinaryFile::PostLoad(void* handle)
{
    return false;
}

char* Win32BinaryFile::SymbolByAddr(ADDRESS dwAddr)
{
    // No symbol table handled at present
    return 0;
}

bool Win32BinaryFile::DisplayDetails(const char* fileName, FILE* f
     /* = stdout */)
{
    return false;
}

LOAD_FMT Win32BinaryFile::GetFormat() const
{
    return LOADFMT_PE;
}

bool Win32BinaryFile::isLibrary() const
{
    return ( m_pPEHeader->Flags & 0x2000 != 0 );
}

ADDRESS Win32BinaryFile::getImageBase()
{
    return m_pPEHeader->Imagebase;
}

size_t Win32BinaryFile::getImageSize()
{
    return m_pPEHeader->ImageSize;
}

std::list<const char *> Win32BinaryFile::getDependencyList()
{
    return std::list<const char *>(); /* FIXME */
}

// We at least need to be able to name the main function and system calls
char* Win32BinaryFile::SymbolByAddress(const ADDRESS dwAddr)
{
    if (dwAddr == GetMainEntryPoint())
        return "main";
    else return 0;
}

DWord Win32BinaryFile::getDelta()
{
    return (DWord)base - LMMH(m_pPEHeader->Imagebase); 
}

// This function is called via dlopen/dlsym; it returns a new BinaryFile
// derived concrete object. After this object is returned, the virtual function
// call mechanism will call the rest of the code in this library
// It needs to be C linkage so that it its name is not mangled
extern "C" {
    BinaryFile* construct()
    {
        return new Win32BinaryFile;
    }    
}
