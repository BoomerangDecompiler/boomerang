/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
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
 * 25 Jun 02 - Mike: Added code to find WinMain by finding a call within 5
 *              instructions of a call to GetModuleHandleA
 * 07 Jul 02 - Mike: Added a LMMH() so code works on big-endian host
 * 08 Jul 02 - Mike: Changed algorithm to find main; now looks for ordinary
 *               call up to 10 instructions before an indirect call to exit
 */

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "BinaryFile.h"
#include "Win32BinaryFile.h"
#include "config.h"
#include <iostream>
#include <sstream>

extern "C" {
    int microX86Dis(void* p);           // From microX86dis.c
}

Win32BinaryFile::Win32BinaryFile() : m_pFileName(0)
{ }

Win32BinaryFile::~Win32BinaryFile()
{
	for (int i=0; i < m_iNumSections; i++) {
		if (m_pSections[i].pSectionName)
			delete [] m_pSections[i].pSectionName;
	}
	if (m_pSections) delete [] m_pSections;
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

// This is a bit of a hack, but no more than the rest of Windows :-O
// The pattern is to look for an indirect call (FF 15 opcode) to
// exit; within 10 instructions before that should be the call
// to WinMain (with no other calls inbetween)
// This pattern should work for "old style" and "new style" PE executables,
// as well as console mode PE files
ADDRESS Win32BinaryFile::GetMainEntryPoint() {
    // Start at program entry point
    unsigned p = LMMH(m_pPEHeader->EntrypointRVA);
    unsigned lim = p + 0x200;
    unsigned char op1, op2;
    unsigned addr, lastOrdCall;
    int gap;            // Number of instructions from the last ordinary call

    gap = 0xF0000000;   // Large positive number (in case no ordinary calls)
    while (p < lim) {
        op1 = *(unsigned char*)(p + base);
        op2 = *(unsigned char*)(p + base + 1);
        if (op1 == 0xE8) {
            // An ordinary call; this could be to winmain/main
            lastOrdCall = p;
            gap = 0;
        }
        else if (op1 == 0xFF && op2 == 0x15) { // Opcode FF 15 is indirect call
            // Get the 4 byte address from the instruction
            addr = LMMH(*(p + base + 2));
//          const char *c = dlprocptrs[addr].c_str();
//printf("Checking %x finding %s\n", addr, c);
            if (dlprocptrs[addr] == "exit") {
                if (gap <= 10) {
                    // This is it. The instruction at lastOrdCall is (win)main
                    addr = LMMH(*(lastOrdCall + base + 1));
                    addr += lastOrdCall + 5;    // Addr is dest of call
//printf("*** MAIN AT 0x%x ***\n", addr);
                    return addr + LMMH(m_pPEHeader->Imagebase);
                }
            }
        }
        int size = microX86Dis(p + base);
        if (size == 0x40) {
            fprintf(stderr, "Warning! Microdisassembler out of step at "
              "offset 0x%x\n", p);
            size = 1;
        }
        p += size;
        gap++;
    }
    return NO_ADDRESS;
}

bool Win32BinaryFile::RealLoad(const char* sName)
{
    m_pFileName = sName;
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

//printf("Image Base %08X, real base %p\n", LMMH(m_pPEHeader->Imagebase), base);

    PEObject *o = (PEObject *)(
        ((char *)m_pPEHeader) + LH(&m_pPEHeader->NtHdrSize) + 24);
    m_iNumSections = LH(&m_pPEHeader->numObjects);
    m_pSections = new SectionInfo[m_iNumSections];
    SectionInfo *reloc = NULL;
    for (int i=0; i<m_iNumSections; i++, o++) {
//printf("%.8s RVA=%08X Offset=%08X size=%08X\n",
//  (char*)o->ObjectName, LMMH(o->RVA), LMMH(o->PhysicalOffset),
//  LMMH(o->VirtualSize));
      m_pSections[i].pSectionName = new char[9];
      strncpy(m_pSections[i].pSectionName, o->ObjectName, 8);
      if (!strcmp(m_pSections[i].pSectionName, ".reloc"))
        reloc = &m_pSections[i];
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

    // Add the Import Address Table entries to the symbol table
    PEImportDtor* id = (PEImportDtor*)
      (LMMH(m_pPEHeader->ImportTableRVA) + base);
    while (id->originalFirstThunk != 0) {
        char* dllName = LMMH(id->name) + base;
        unsigned* iat = (unsigned*)(LMMH(id->originalFirstThunk) + base);
        unsigned iatEntry = LMMH(*iat);
        ADDRESS paddr = LMMH(id->firstThunk) + LMMH(m_pPEHeader->Imagebase);
        while (iatEntry) {
            if (iatEntry >> 31) {
                // This is an ordinal number (stupid idea)
                std::ostringstream ost;
                std::string nodots(dllName);
                int len = nodots.size();
                for (int j=0; j < len; j++)
                    if (nodots[j] == '.')
                        nodots[j] = '_';    // Dots can't be in identifiers
                ost << nodots << "_" << (iatEntry & 0x7FFFFFFF);                
                dlprocptrs[paddr] = ost.str();
                //printf("Added symbol %s value %x\n", ost.str().c_str(),paddr);
            } else {
                // Normal case (IMAGE_IMPORT_BY_NAME)                
                // Skip the useless hint (2 bytes)
                std::string name((const char*)(iatEntry+2+base));
                dlprocptrs[paddr] = name;
                dlprocptrs[(int)iat - (int)base + LMMH(m_pPEHeader->Imagebase)] = std::string("old_") + name; // add both possibilities
                //printf("Added symbol %s value %x\n", name.c_str(), paddr);
                //printf("Also added old_%s value %x\n", name.c_str(), (int)iat - (int)base + LMMH(m_pPEHeader->Imagebase));
            }
            iat++;
            iatEntry = LMMH(*iat);
            paddr+=4;
        }
        id++;
    }

    // Give the entry point a symbol
    ADDRESS entry = GetMainEntryPoint();
    if (entry != NO_ADDRESS) {
        std::map<ADDRESS, std::string>::iterator it = dlprocptrs.find(entry);
        if (it == dlprocptrs.end())
            dlprocptrs[entry] = "_init";
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

#ifndef WIN32
char* Win32BinaryFile::SymbolByAddress(ADDRESS dwAddr)
{
    std::map<ADDRESS, std::string>::iterator it = dlprocptrs.find(dwAddr);
    if (it == dlprocptrs.end())
        return 0;
    return (char*) it->second.c_str();
}

ADDRESS Win32BinaryFile::GetAddressByName(const char* pName,
    bool bNoTypeOK /* = false */) {
    // This is "looking up the wrong way" and hopefully is uncommon
    // Use linear search
    std::map<ADDRESS, std::string>::iterator it = dlprocptrs.begin();
    while (it != dlprocptrs.end()) {
        if (strcmp(it->second.c_str(), pName) == 0)
            return it->first;
        it++;
    }
    return 0;
}

void Win32BinaryFile::AddSymbol(ADDRESS uNative, const char *pName)
{
    dlprocptrs[uNative] = pName;
}

#endif

bool Win32BinaryFile::DisplayDetails(const char* fileName, FILE* f
     /* = stdout */)
{
    return false;
}

int Win32BinaryFile::win32Read2(short* ps) const {
    unsigned char* p = (unsigned char*)ps;
    // Little endian
    int n = (int)(p[0] + (p[1] << 8));
    return n;
}

int Win32BinaryFile::win32Read4(int* pi) const{
    short* p = (short*)pi;
    int n1 = win32Read2(p);
    int n2 = win32Read2(p+1);
    int n = (int) (n1 | (n2 << 16));
    return n;
}

// Read 2 bytes from given native address
int Win32BinaryFile::readNative2(ADDRESS nat) {
    PSectionInfo si = GetSectionInfoByAddr(nat);
    if (si == 0) return 0;
    ADDRESS host = si->uHostAddr - si->uNativeAddr + nat;
    int n = win32Read2((short*)host);
    return n;
}

// Read 4 bytes from given native address
int Win32BinaryFile::readNative4(ADDRESS nat) {
    PSectionInfo si = GetSectionInfoByAddr(nat);
    if (si == 0) return 0;
    ADDRESS host = si->uHostAddr - si->uNativeAddr + nat;
    int n = win32Read4((int*)host);
    return n;
}

// Read 8 bytes from given native address
long long Win32BinaryFile::readNative8(ADDRESS nat) {
    int raw[2];
#ifdef WORDS_BIGENDIAN      // This tests the  host  machine
    // Source and host are different endianness
    raw[1] = readNative4(nat);
    raw[0] = readNative4(nat+4);
#else
    // Source and host are same endianness
    raw[0] = readNative4(nat);
    raw[1] = readNative4(nat+4);
#endif
    return *(long long*)raw;
}

// Read 4 bytes as a float
float Win32BinaryFile::readNativeFloat4(ADDRESS nat) {
    int raw = readNative4(nat);
    // Ugh! gcc says that reinterpreting from int to float is invalid!!
    //return reinterpret_cast<float>(raw);    // Note: cast, not convert!!
    return *(float*)&raw;           // Note: cast, not convert
}

// Read 8 bytes as a float
double Win32BinaryFile::readNativeFloat8(ADDRESS nat) {
    int raw[2];
#ifdef WORDS_BIGENDIAN      // This tests the  host  machine
    // Source and host are different endianness
    raw[1] = readNative4(nat);
    raw[0] = readNative4(nat+4);
#else
    // Source and host are same endianness
    raw[0] = readNative4(nat);
    raw[1] = readNative4(nat+4);
#endif
    //return reinterpret_cast<double>(*raw);    // Note: cast, not convert!!
    return *(double*)raw;
}

bool Win32BinaryFile::IsDynamicLinkedProcPointer(ADDRESS uNative)
{
	if (dlprocptrs.find(uNative) != dlprocptrs.end())
		return true;
	return false;
}

const char *Win32BinaryFile::GetDynamicProcName(ADDRESS uNative)
{
	return dlprocptrs[uNative].c_str();
}

LOAD_FMT Win32BinaryFile::GetFormat() const
{
    return LOADFMT_PE;
}

MACHINE Win32BinaryFile::GetMachine() const
{
    return MACHINE_PENTIUM;
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

DWord Win32BinaryFile::getDelta()
{
    return (DWord)base - LMMH(m_pPEHeader->Imagebase); 
}

#ifndef WIN32
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
#endif
