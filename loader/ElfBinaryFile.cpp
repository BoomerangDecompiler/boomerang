/*
 * Copyright (C) 1997-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*******************************************************************************
 * File: ElfBinaryFile.cc
 * Desc: This file contains the implementation of the class ElfBinaryFile.
 ******************************************************************************/

/*
 * $Revision$
 *
 * ELF binary file format.
 *  This file implements the class ElfBinaryFile, derived from class
 *  BinaryFile. See ElfBinaryFile.h and BinaryFile.h for details
 *  MVE 30/9/97
 * 20 Aug 01 - Icer: line 373, cast pName in elf_hash() to make
 * 		     the prototypes happy.
 * 10 Mar 02 - Mike: Mods for stand alone operation; constuct function
 * 21 May 02 - Mike: Slight mod for gcc 3.1
*/

/*==============================================================================
 * Dependencies.
 *============================================================================*/

//#include "global.h"
#include "ElfBinaryFile.h"
// #ifndef NODETAILS		CC: 5Apr01
#include "ElfDetails.h"     // dumpElf32() etc
// #endif
#include <values.h>
#include <sys/types.h>      // Next three for open()
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>         // Close()
typedef std::map<std::string, int, std::less<std::string> >     StrIntMap;

ElfBinaryFile::ElfBinaryFile(bool bArchive /* = false */)
    : BinaryFile(bArchive)      // Initialise base class
{
    m_fd = -1;
    m_pFileName = 0;
    m_elf = 0;
    Init();                 // Initialise all the common stuff
}

ElfBinaryFile::~ElfBinaryFile()
{
    if (m_pImportStubs)
        // Delete the array of import stubs
        delete [] m_pImportStubs;
}

// Reset internal state, except for those that keep track of which member
// we're up to
void ElfBinaryFile::Init()
{
    m_pShdrs = 0;           // No section headers
    m_pHeader = 0;
    m_pReloc = 0;
    m_pSym = 0;
    m_uPltMin = 0;          // No PLT limits
    m_uPltMax = 0;
    m_pLastName = 0;
    m_iLastSize = 0;
    m_pImportStubs = 0;
}

bool ElfBinaryFile::RealLoad(const char* sName)
{
    if (m_bArchive)
    {
        // This is a member of an archive. Should not be using this
        // function at all
        return false;
    }

    Elf_Cmd cmd;        /* Command to be executed */

    m_pFileName = sName;
    m_fd = open (sName, O_RDONLY);
    if (m_fd == -1) return 0;

    if (elf_version (EV_CURRENT) == EV_NONE) 
    {
        fprintf (stderr, "Library out of date\n");
        exit (-1);
    }

    /* Set up the file for reading */
    cmd = ELF_C_READ;
    m_arf = elf_begin (m_fd, cmd, (Elf *)0);
    if (m_arf == 0) return 0;

    /* Process only the first elf header that might be in the file */
    m_elf = elf_begin (m_fd, cmd, m_arf);
    if (m_elf == 0) return 0;
    // Beware! Multi-part archives can have parts that have no headers.
    // These must be ignored.
    while ((m_pHeader = elf32_getehdr(m_elf)) == 0)
    {
        cmd = elf_next(m_elf);
        elf_end(m_elf);
        m_elf = elf_begin (m_fd, cmd, m_arf);
        if (m_elf == 0) return 0;
    }
    return ProcessElfFile();
}

// Load the next member (if any) of a multipart archive
bool ElfBinaryFile::GetNextMember()
{
    Elf_Cmd cmd = elf_next(m_elf);      // Move to next part of archive
    elf_end(m_elf);             // Unload the old part

    /* Process the next elf header that might be in the file */
    m_elf = elf_begin (m_fd, cmd, m_arf);
    if (m_elf == 0) return 0;
    Init();                     // Reset internal state
    return ProcessElfFile();
}

// Clean up and unload the binary image
void ElfBinaryFile::UnLoad()
{
    if (m_pShdrs) delete [] m_pShdrs;
    if (m_elf) elf_end (m_elf);
    if (m_arf) elf_end (m_arf);
    Init();                     // Set all internal state to 0

    close (m_fd);
} 

int ElfBinaryFile::ProcessElfFile()
{
    Elf_Scn *scn;
    int i = 1;          // counter - # sects. Start @ 1, total m_iNumSections
    char* pName;        // Section's name
    char* pScnNames;    /* Pointer to the section header string table */
    Elf_Data* d_shstr;  /* Need this pointer to free the data */
    Elf_Data* d;        /* Pointer to data in sections of interest */

    m_pHeader = elf32_getehdr (m_elf);
    assert (m_pHeader);

    // Allocate room for all the Elf sections (including the silly first one)
    m_iNumSections = m_pHeader->e_shnum;    // Number of sections
    m_pSections = new SectionInfo[m_iNumSections];
    if (m_pSections == 0) return 0;         // Failed!
    // Initialise to zero; especially for flags and addresses
    memset(m_pSections, '\0', m_iNumSections*sizeof(SectionInfo));

    // shstrndx is the section index of the section header string table
    Elf32_Half shstrndx = m_pHeader->e_shstrndx;
    // Get the section descriptor for the section header string table
    scn = elf_getscn(m_elf, shstrndx);
    // Data of the section header string table
    d_shstr = elf_getdata(scn, NULL);
    pScnNames = (char*) d_shstr->d_buf;     // Actual string table

    int n = m_pHeader->e_shnum;             // Number of elf sections
    bool bGotCode = false;                  // True when have seen a code sect
    for (i=0; i < n; i++)
    {
        // Get section information.
        scn = elf_getscn(m_elf, i);

        /* Get section header information */
        Elf32_Shdr* pShdr = elf32_getshdr (scn);
        assert (pShdr);
 
        pName = elf_strptr (m_elf, shstrndx, (size_t)pShdr->sh_name);
        m_pSections[i].pSectionName = pName;
        d = elf_getdata(scn, NULL);
        if (d) m_pSections[i].uHostAddr = (ADDRESS)d->d_buf;
        else m_pSections[i].uHostAddr = 0;
        m_pSections[i].uNativeAddr = pShdr->sh_addr;
        m_pSections[i].uType = pShdr->sh_type;
        m_pSections[i].uSectionSize = pShdr->sh_size;
        m_pSections[i].uSectionEntrySize = pShdr->sh_entsize;
        if ((pShdr->sh_flags & SHF_WRITE) == 0)
            m_pSections[i].bReadOnly = true;
        // Can't use the SHF_ALLOC bit to determine bss section; the bss section
        // has SHF_ALLOC but also SHT_NOBITS. (But many other sections, such as
        // .comment, also have SHT_NOBITS). So for now, just use the name
//      if ((pShdr->sh_flags & SHF_ALLOC) == 0)
        if (strcmp(pName, ".bss") == 0)
            m_pSections[i].bBss = true;
        if (pShdr->sh_flags & SHF_EXECINSTR) {
            m_pSections[i].bCode = true;
            bGotCode = true;            // We've got to a code section
        }
        // Deciding what is data and what is not is actually quite tricky but
        // important. For example, it's crucial to flag the .exception_ranges
        // section as data, otherwise there is a "hole" in the allocation map,
        // that means that there is more than one "delta" from a read-only
        // section to a page, and in the end using -C results in a file that
        // looks OK but when run just says "Killed".
        // So we use the Elf designations; it seems that ALLOC.!EXEC -> data
        // But we don't want sections before the .text section, like .interp,
        // .hash, etc etc. Hence bGotCode.
        // NOTE: this ASSUMES that sections appear in a sensible order in
        // the input binary file: junk, code, rodata, data, bss
        if (bGotCode && ((pShdr->sh_flags & (SHF_EXECINSTR | SHF_ALLOC)) ==
          SHF_ALLOC) && (pShdr->sh_type != SHT_NOBITS))
            m_pSections[i].bData = true;
    }

    // Add symbol info. Note that some symbols will be in the main table only,
    // and others in the dynamic table only. So the best idea is to add symbols
    // for both sections (if any).
    AddSyms(".symtab", ".strtab");
    AddSyms(".dynsym", ".dynstr");

    // Save the relocation to symbol table info
    PSectionInfo pRel = GetSectionInfoByName(".rela.text"); 
    if (pRel)
    {
        m_bAddend = true;               // Remember its a relA table
        m_pReloc = pRel->uHostAddr;     // Save pointer to reloc table
        SetRelocInfo(pRel);
    }
    else
    {
        m_bAddend = false;
        pRel = GetSectionInfoByName(".rel.text");
        if (pRel)
        {
            SetRelocInfo(pRel);
            m_pReloc = pRel->uHostAddr;     // Save pointer to reloc table
        }
    }

    // Find the PLT limits. Required for IsDynamicLinkedProc(), e.g.
    PSectionInfo pPlt = GetSectionInfoByName(".plt");
    if (pPlt)
    {
        m_uPltMin = pPlt->uNativeAddr;
        m_uPltMax = pPlt->uNativeAddr + pPlt->uSectionSize;
    }

    // Find the space occupied by the loaded image
    ADDRESS base = MAXINT;
    ADDRESS top = 0;
    Elf32_Phdr *phdr = elf32_getphdr(m_elf);
    for (int i = 0; i < m_pHeader->e_phnum; i++) {
        if( phdr[i].p_type == PT_LOAD ) {
            if( phdr[i].p_vaddr < base )
                base = phdr[i].p_vaddr;
            if( phdr[i].p_vaddr + phdr[i].p_memsz > top )
                top = phdr[i].p_vaddr + phdr[i].p_memsz;
        }
    }

    m_uBaseAddr = base;
    m_uImageSize = top - base + 1;
    return 1;                           // Success
}

// Like a replacement for elf_strptr()
char* ElfBinaryFile::GetStrPtr(int idx, int offset)
{
    if (idx < 0)
    {
        // Most commonly, this will be an index of -1, because a call
        // to GetSectionIndexByName() failed
        fprintf(stderr, "Error! GetStrPtr passed index of %d\n", idx);
        return "Error!";
    }
    // Get a pointer to the start of the string table
    char* pSym = (char*)m_pSections[idx].uHostAddr;
    // Just add the offset
    return pSym + offset;
}

// Add appropriate symbols to the symbol table.
// sSymSect is the name of the symbol section (e.g. ".dynsym"), and sStrSect
// is the name of the associated string section (e.g. ".dynstr")
void ElfBinaryFile::AddSyms(const char* sSymSect, const char* sStrSect)
{
    PSectionInfo pSect = GetSectionInfoByName(sSymSect);
    if (pSect == 0) return;
    // Calc number of symbols
    int nSyms = pSect->uSectionSize / pSect->uSectionEntrySize;
    // Pointer to symbols
    m_pSym = (Elf32_Sym*) pSect->uHostAddr;
    // Get index to string table
    int idx = GetSectionIndexByName(sStrSect);

    // Index 0 is a dummy entry
    for (int i = 1; i < nSyms; i++)
    {
        if (ELF32_ST_TYPE(m_pSym[i].st_info) == STT_FUNC) {
            ADDRESS val = (ADDRESS)m_pSym[i].st_value;
            std::string str(GetStrPtr(idx, m_pSym[i].st_name));
            // Hack of the "@@GLIBC_2.0" of Linux, if present
            unsigned pos;
            if ((pos = str.find("@@")) != std::string::npos)
                str.erase(pos);
            std::map<ADDRESS, std::string>::iterator aa = m_SymA.find(val);
            // Ensure no overwriting
            if (aa == m_SymA.end())
                m_SymA[val] = str;
        }
    }
    ADDRESS uMain = GetMainEntryPoint();
    if (m_SymA.find(uMain) == m_SymA.end())
    {
        // Ugh - main mustn't have the STT_FUNC attribute. Add it
        std::string sMain("main");
        m_SymA[uMain] = sMain;
    }
    return;
}

// This function will be required only by very low level applocations,
// such as the elfSparc executable file dumper
Elf_Scn* ElfBinaryFile::GetElfScn(int idx)
{
    return elf_getscn(m_elf, idx);
}

// Note: this function overrides a simple "return 0" function in the
// base class (i.e. BinaryFile::SymbolByAddress())
char* ElfBinaryFile::SymbolByAddress(const ADDRESS dwAddr)
{
    std::map<ADDRESS, std::string>::iterator aa = m_SymA.find(dwAddr);
    if (aa == m_SymA.end())
        return 0;
    return (char*)aa->second.c_str();
}

bool ElfBinaryFile::ValueByName(const char* pName, SymValue* pVal,
    bool bNoTypeOK /* = false */)
{
    Elf32_Word  hash, numBucket, numChain, y;
    Elf32_Word  *pBuckets, *pChains;    // For symbol table work
    int         found;
    Elf32_Word* pHash;              // Pointer to hash table
    Elf32_Sym*  pSym;               // Pointer to the symbol table
    Elf32_Word  iStr;               // Section index of the string table
    PSectionInfo pSect;

    pSect = GetSectionInfoByName(".dynsym");
    if (pSect == 0)
    {
        // We have a file with no .dynsym section, and hence no .hash
        // section (from my understanding - MVE). It seems that the only
        // alternative is to linearly search the symbol tables.
        // This must be one of the big reasons that linking is so slow!
        // (at least, for statically linked files)
        // Note MVE: We can't use m_SymA because we may need the size
        return SearchValueByName(pName, pVal);
    }
    pSym = (Elf32_Sym*)pSect->uHostAddr;
    if (pSym == 0) return false;
    pSect = GetSectionInfoByName(".hash");
    if (pSect == 0) return false;
    pHash = (Elf32_Word*) pSect->uHostAddr;
    iStr = GetSectionIndexByName(".dynstr");
    
    // First organise the hash table
    numBucket = pHash[0];
    numChain = pHash[1];
    pBuckets = &pHash[2];
    pChains = &pBuckets[numBucket];

    // Hash the symbol
    // cast pName to const unsigned char * to make elf_hash happy
    //hash = elf_hash((const unsigned char *)pName) % numBucket;
    hash = elf_hash(pName) % numBucket;
    /* Now look it up in the bucket list */
    y = pBuckets[hash];
    // Beware of symbol tables with 0 in the buckets, e.g. libstdc++.
    // In that case, set found to false.
    found = (y != 0);
    if (y)
    {
        while (strcmp(pName, GetStrPtr(iStr, pSym[y].st_name)) != 0)
        {
            y = pChains[y];
            if (y == 0)
            {
                found = false;
                break;
            }
        }
    }
    // Beware of symbols with STT_NOTYPE, e.g. "open" in libstdc++ !
    // But sometimes "main" has the STT_NOTYPE attribute, so if bNoTypeOK
    // is passed as true, return true
    if (found && 
        (bNoTypeOK || (ELF32_ST_TYPE(pSym[y].st_info) != STT_NOTYPE)))
    {
        pVal->uSymAddr = pSym[y].st_value;
        pVal->iSymSize = pSym[y].st_size;
        return true;
    }
    else
    {
        // We may as well do a linear search of the main symbol table. Some
        // symbols (e.g. init_dummy) are in the main symbol table, but not
        // in the hash table
        return SearchValueByName(pName, pVal);
    }
}

// Lookup the symbol table using linear searching. See comments above
// about why this appears to be needed.
bool ElfBinaryFile::SearchValueByName(const char* pName, SymValue* pVal,
    const char* pSectName, const char* pStrName)
{
    // Note: this assumes .symtab. Many files don't have this section!!!
    PSectionInfo pSect, pStrSect;

    pSect = GetSectionInfoByName(pSectName);
    if (pSect == 0) return false;
    pStrSect = GetSectionInfoByName(pStrName);
    if (pStrSect == 0) return false;
    const char* pStr = (const char*) pStrSect->uHostAddr;
    // Find number of symbols
    int n = pSect->uSectionSize / pSect->uSectionEntrySize;
    Elf32_Sym* pSym = (Elf32_Sym*)pSect->uHostAddr;
    // Search all the symbols. It may be possible to start later than
    // index 0
    for (int i=0; i < n; i++)
    {
        int idx = pSym[i].st_name;
        if (strcmp(pName, pStr+idx) == 0)
        {
            // We have found the symbol
            pVal->uSymAddr = pSym[i].st_value;
            pVal->iSymSize = pSym[i].st_size;
            return true;
        }
    }
    return false;           // Not found (this table)
}

// Search for the given symbol. First search .symtab (if present); if not
// found or the table has been stripped, search .dynstr
bool ElfBinaryFile::SearchValueByName(const char* pName, SymValue* pVal)
{
    if (SearchValueByName(pName, pVal, ".symtab", ".strtab"))
        return true;
    return SearchValueByName(pName, pVal, ".dynsym", ".dynstr");
}


ADDRESS ElfBinaryFile::GetAddressByName(const char* pName,
    bool bNoTypeOK /* = false */)
{
    if (pName == m_pLastName)
        return m_uLastAddr;
    SymValue Val;
    bool bSuccess = ValueByName(pName, &Val, bNoTypeOK);
    if (bSuccess)
    {
        m_pLastName = pName;
        m_iLastSize = Val.iSymSize;
        m_uLastAddr = Val.uSymAddr;
        return Val.uSymAddr;
    }
    else return 0;
}

int ElfBinaryFile::GetSizeByName(const char* pName,
    bool bNoTypeOK /* = false */)
{
    if (pName == m_pLastName)
        return m_iLastSize;
    SymValue Val;
    bool bSuccess = ValueByName(pName, &Val, bNoTypeOK);
    if (bSuccess)
    {
        m_pLastName = pName;
        m_iLastSize = Val.iSymSize;
        m_uLastAddr = Val.uSymAddr;
        return Val.iSymSize;
    }
    else return 0;
}

// Guess the size of a function by finding the next symbol after it, and
// subtracting the distance. This function is NOT efficient; it has to
// compare the closeness of ALL symbols in the symbol table
int ElfBinaryFile::GetDistanceByName(const char* sName, const char* pSectName)
{
    int size = GetSizeByName(sName);
    if (size) return size;          // No need to guess!
    // No need to guess, but if there are fillers, then subtracting labels
    // will give a better answer for coverage purposes. For example, switch_cc
    // But some programs (e.g. switch_ps) have the switch tables between the
    // end of _start and main! So we are better off overall not trying to
    // guess the size of _start
    unsigned value = GetAddressByName(sName);
    if (value == 0) return 0;       // Symbol doesn't even exist!

    PSectionInfo pSect;
    pSect = GetSectionInfoByName(pSectName);
    if (pSect == 0) return 0;
    // Find number of symbols
    int n = pSect->uSectionSize / pSect->uSectionEntrySize;
    Elf32_Sym* pSym = (Elf32_Sym*)pSect->uHostAddr;
    // Search all the symbols. It may be possible to start later than
    // index 0
    unsigned closest = 0xFFFFFFFF;
    int idx = -1;
    for (int i=0; i < n; i++)
    {
        if ((pSym[i].st_value > value) && (pSym[i].st_value < closest))
        {
            idx = i;
            closest = pSym[i].st_value;
        }
    }
    if (idx == -1) return 0;
    // Do some checks on the symbol's value; it might be at the end of the
    // .text section
    pSect = GetSectionInfoByName(".text");
    ADDRESS low = pSect->uNativeAddr;
    ADDRESS hi = low + pSect->uSectionSize;
    if ((value >= low) && (value < hi))
    {
        // Our symbol is in the .text section. Put a ceiling of the end of
        // the section on closest.
        if (closest > hi) closest = hi;
    }
    return closest - value;
}

int ElfBinaryFile::GetDistanceByName(const char* sName)
{
    int val = GetDistanceByName(sName, ".symtab");
    if (val) return val;
    return GetDistanceByName(sName, ".dynsym");
}


ADDRESS ElfBinaryFile::GetFirstHeaderAddress()
{
    return (ADDRESS) elf32_getehdr (m_elf);
}

Elf32_Phdr* ElfBinaryFile::GetProgHeader(int idx)
{
    // Get the indicated prog hdr
    if (idx < 0) return 0;          // Index out of bounds
    if (m_pHeader == 0) return 0;   // No file loaded
    if (idx > m_pHeader->e_phnum) return 0;
    return elf32_getphdr(m_elf) + idx;
}

Elf32_Shdr* ElfBinaryFile::GetSectionHeader(int idx)
{
    // Get indicated section hdr
    if (idx < 0) return 0;          // Index out of bounds
    if (m_pHeader == 0) return 0;   // No file loaded
    if (idx > m_pHeader->e_shnum) return 0;
    return elf32_getshdr(elf_getscn(m_elf, idx));
}

void ElfBinaryFile::SetRelocInfo(PSectionInfo pSect)
{
    // Calc number of symbols
    int nRelocs = pSect->uSectionSize / pSect->uSectionEntrySize;
    // Pointer to symbols
    Elf32_Rel* pRel  = (Elf32_Rel*) pSect->uHostAddr;
    // Get info on symtab
    PSectionInfo pSymSect = GetSectionInfoByName(".symtab");
    if (pSymSect == 0) return;
    Elf32_Sym* pSym = (Elf32_Sym*) pSymSect->uHostAddr;
    // Get index to string table
    int idx = GetSectionIndexByName(".strtab");
 
    // Allocate the symbols
    int res = m_Reloc.Init(nRelocs);
    if (res == 0)
    {
        fprintf(stderr, "Could not allocate space for %d relocations\n",
            nRelocs);
        return;
    }
    for (int i = 0; i < nRelocs; i++)
    {
        // Get the symtab index
        int j = ELF32_R_SYM(pRel->r_info);
        // Note that some of these will be "no name" symbols, that are
        // used merely to indicate the section that the symbols are relative
        // to. But these must be included in the table, because the fact
        // that they are relocated at all indicates that they are pointers,
        // not constants, and this is valuable information.
        m_Reloc.Add((ADDRESS)pRel->r_offset, GetStrPtr(idx, pSym[j].st_name));
        // Move to the next entry. Note: variable size!
        pRel = (Elf32_Rel*) ((char*)pRel + pSect->uSectionEntrySize);
    }
    m_Reloc.Sort();           // Sort the symbols, so they can be searched
}

const char* ElfBinaryFile::GetRelocSym(ADDRESS uNative)
{
    const char* p = m_Reloc.Find(uNative);
    if (p == 0) return 0;
    // There are some "no name" symbols that are used only to indicate
    // what section the item is relative to. We treat these as not having
    // a symbol (i.e. return a null pointer, rather than a zero length
    // string).
    if (p[0] == '\0') return 0;
    return p;
}
    
bool ElfBinaryFile::IsAddressRelocatable(ADDRESS uNative)
{
    if (m_pReloc == 0) return false;
    int idx = m_Reloc.FindIndex(uNative);
    return (idx != -1);
}

bool ElfBinaryFile::IsDynamicLinkedProc(ADDRESS uNative)
{
    if (m_uPltMin == 0) return false;
#if 0           // What was I thinking? This seems to be a mixture of
                // code using m_Sym (SymTab), and m_pSym (Elf32_Sym*)!
    if (m_pSym == 0) return false;
    int idx = m_Sym.FindIndex(uNative);
    if (idx == -1) return false;
    return (m_pSym[idx].st_value >= m_uPltMin) &&
        (m_pSym[idx].st_value < m_uPltMax);
#endif
    return (uNative >= m_uPltMin) && (uNative < m_uPltMax);
}


//
// GetEntryPoints()
// Returns a list of pointers to SectionInfo structs representing entry
// points to the program
// Item 0 is the main() function; items 1 and 2 are .init and .fini
//
std::list<SectionInfo*>& ElfBinaryFile::GetEntryPoints(
    const char* pEntry /* = "main" */) {
    SectionInfo* pSect = GetSectionInfoByName(".text");
    ADDRESS uMain = GetAddressByName(pEntry, true);
    ADDRESS delta = uMain - pSect->uNativeAddr;
    pSect->uNativeAddr += delta;
    pSect->uHostAddr += delta;
    // Adjust uSectionSize so uNativeAddr + uSectionSize still is end of sect
    pSect->uSectionSize -= delta;       
    m_EntryPoint.push_back(pSect);
    // .init and .fini sections
    pSect = GetSectionInfoByName(".init");
    m_EntryPoint.push_back(pSect);
    pSect = GetSectionInfoByName(".fini");
    m_EntryPoint.push_back(pSect);
    return m_EntryPoint;
}


//
// GetMainEntryPoint()
// Returns the entry point to main (this should be a label in elf
// binaries generated by compilers).
//
ADDRESS ElfBinaryFile::GetMainEntryPoint()
{
    return GetAddressByName ("main", true);
}

ADDRESS ElfBinaryFile::GetEntryPoint()
{
    return m_pHeader->e_entry;
}

ADDRESS ElfBinaryFile::NativeToHostAddress(ADDRESS uNative)
{
    if (m_iNumSections == 0) return 0;
    return m_pSections[1].uHostAddr - m_pSections[1].uNativeAddr + uNative; 
}

// This is not complete. Need to decide what to do about things like G
// (for the Global Offset Table): do we need to allocate one? Initialise
// it?
#if 0
WORD ElfBinaryFile::ApplyRelocation(ADDRESS uNative, WORD wWord)
{
    if (m_pHeader == 0) return 0;       // No file loaded
    int idx = m_Reloc.FindIndex(uNative);
    if (idx == -1) return (ADDRESS)-1;
    ADDRESS wRes = 0;

    switch (m_pHeader->e_machine)
    {
        case EM_SPARC:
        {
            Elf32_Rela* pRel = (Elf32_Rela*)m_pReloc + idx;
            int iType = ELF32_R_TYPE(pRel->r_info);
            int idxSym = ELF32_R_SYM(pRel->r_info);
            ADDRESS SplusA = (ADDRESS)m_pSym[idxSym].st_value + pRel->r_addend;
            switch (iType)
            {
                case R_SPARC_NONE:
                case R_SPARC_COPY:
                    return 0;

                case R_SPARC_8:
                    wRes = wWord | (SplusA & 0xFF);
                    break;
                case R_SPARC_16:
                    wRes = wWord | (SplusA & 0xFFFF);
                    break;
                case R_SPARC_32:
                    wRes = SplusA;
                    break;
                case R_SPARC_DISP8:
                    wRes = wWord | ((SplusA - uNative) & 0xFF);
                    break;
                case R_SPARC_DISP16:
                    wRes = wWord | ((SplusA - uNative) & 0xFFFF);
                    break;
                case R_SPARC_DISP32:
                    wRes = SplusA - uNative;
                    break;
                case R_SPARC_WDISP30:
                    wRes = wWord | ((SplusA - uNative) >> 2);
                    break;
                case R_SPARC_WDISP22:
                    wRes = wWord | (((SplusA - uNative) >> 2) & 0x3FFFFF);
                    break;
                case R_SPARC_HI22:
                    wRes = wWord | ((SplusA >> 10) & 0x3FFFFF);
                    break;
                case R_SPARC_22:
                    wRes = wWord | (SplusA & 0x3FFFFF);
                    break;
                case R_SPARC_13:
                    wRes = wWord | (SplusA & 0x1FFF);
                    break;
                case R_SPARC_LO10:
                    wRes = wWord | (SplusA & 0x3FF);
                    break;
                case R_SPARC_GOT10:
                case R_SPARC_GOT13:
                case R_SPARC_GOT22:
                case R_SPARC_PC10:
                case R_SPARC_PC22:
                case R_SPARC_WPLT30:
                case R_SPARC_GLOB_DAT:
                case R_SPARC_JMP_SLOT:
                case R_SPARC_RELATIVE:
                case R_SPARC_UA32:
                case R_SPARC_PLT32:
                case R_SPARC_HIPLT22:
                case R_SPARC_LOPLT10:
                case R_SPARC_PCPLT32:
                case R_SPARC_PCPLT22:
                case R_SPARC_PCPLT10:
                case R_SPARC_10:
                case R_SPARC_11:
                case R_SPARC_WDISP16:
                case R_SPARC_WDISP19:
                case R_SPARC_7:
                case R_SPARC_5:
                case R_SPARC_6:
                default:
                    return 0;
            }
            break;
        }


        case EM_386:
        {
            Elf32_Rel* pRel = (Elf32_Rel*)m_pReloc + idx;
            int iType = ELF32_R_TYPE(pRel->r_info);
            switch (iType)
            {
            }
            break;
        }

        default:
            fprintf(stderr, "Machine type %d not implemented for relocation\n",
                m_pHeader->e_machine);
            return 0;
    }
    return wRes;
}
#endif
ADDRESS ElfBinaryFile::GetRelocatedAddress(ADDRESS uNative)
{
    // Not implemented yet. But we need the function to make it all link
    return 0;
}

bool ElfBinaryFile::PostLoad(void* handle)
{
    // This function is called after an archive member has been loaded
    // by ElfArchiveFile

    // Save the elf pointer
    m_elf = (Elf*) handle;

    return ProcessElfFile();
}

bool ElfBinaryFile::DisplayDetails(const char* fileName, FILE* f /* = stdout */)
{
// Early versions of Redhat Linux have a problem with the sys/link.h file.
// This may be fixed in more modern distributions
#ifndef LINUX
    Elf32_Ehdr* pHdr;
    pHdr = (Elf32_Ehdr*) GetFirstHeaderAddress();
    dumpElf32(pHdr, fileName, f);

    // Also dump the program headers
    for (int i = 0; i < pHdr->e_phnum; i++)
    {
        dumpPhdr (GetProgHeader(i), f);   
    }

    // Also the section headers
    int nsh = pHdr->e_shnum;
    for (int i = 0; i < nsh; i++)
    {
        dumpShdr(GetSectionHeader(i), i, f);    
    }
    return 1;
#else
    return 0;
#endif
}

// Open this binaryfile for reading AND writing
bool ElfBinaryFile::Open(const char* sName)
{
    if (m_bArchive)
    {
        // This is a member of an archive. Should not be using this
        // function at all
        return false;
    }

    Elf_Cmd cmd;        /* Command to be executed */

    m_pFileName = sName;
    m_fd = open (sName, O_RDWR);
    if (m_fd == -1) return 0;

    if (elf_version (EV_CURRENT) == EV_NONE) 
    {
        fprintf (stderr, "Library out of date\n");
        exit (-1);
    }

    /* Set up the file for reading and writing */
    cmd = ELF_C_RDWR;
    m_arf = elf_begin (m_fd, cmd, (Elf *)0);
    if (m_arf == 0) return 0;

    /* Process only the first elf header that might be in the file */
    m_elf = elf_begin (m_fd, cmd, m_arf);
    if (m_elf == 0) return 0;
    // Beware! Multi-part archives can have parts that have no headers.
    // These must be ignored.
    while ((m_pHeader = elf32_getehdr(m_elf)) == 0)
    {
        cmd = elf_next(m_elf);
        elf_end(m_elf);
        m_elf = elf_begin (m_fd, cmd, m_arf);
        if (m_elf == 0) return 0;
    }
    return ProcessElfFile();
}


// This is a special elf-only function, needed by copyrelbss
void ElfBinaryFile::SetLinkAndInfo(int idx, int link, int info)
{
    Elf_Scn *scn = elf_getscn(m_elf, idx);
    Elf32_Shdr* pShdr = elf32_getshdr(scn);
    assert (pShdr);

    pShdr->sh_link = link;
    pShdr->sh_info = info;
    elf_update(m_elf, ELF_C_NULL);
    elf_update(m_elf, ELF_C_WRITE);
}

void ElfBinaryFile::Close()
{
    elf_update(m_elf, ELF_C_NULL);
    elf_update(m_elf, ELF_C_WRITE);
    UnLoad();
}

LOAD_FMT ElfBinaryFile::GetFormat() const
{
    return LOADFMT_ELF;
}

MACHINE ElfBinaryFile::GetMachine() const
{
    if (m_pHeader->e_machine == EM_SPARC)
	return MACHINE_SPARC;
    else if (m_pHeader->e_machine == EM_386)
	return MACHINE_PENTIUM;
    // what sort of machine is this?
    assert(false);
    return (MACHINE)-1;
}

bool ElfBinaryFile::isLibrary() const
{
    return (m_pHeader->e_type == ET_DYN);
}

std::list<const char *> ElfBinaryFile::getDependencyList()
{
    std::list<const char *> result;
    ADDRESS stringtab = NO_ADDRESS;
    PSectionInfo dynsect = GetSectionInfoByName(".dynamic");
    if( dynsect == NULL )
        return result; /* no dynamic section = statically linked */

    for( Elf32_Dyn *dyn = (Elf32_Dyn *)dynsect->uHostAddr;
         dyn->d_tag != DT_NULL; dyn++ ) {
        if( dyn->d_tag == DT_STRTAB ) {
            stringtab = (ADDRESS)dyn->d_un.d_ptr;
            break;
        }
    }
    
    if( stringtab == NO_ADDRESS ) /* No string table = no names */
        return result;
    stringtab = NativeToHostAddress( stringtab );
    
    for( Elf32_Dyn *dyn = (Elf32_Dyn *)dynsect->uHostAddr;
         dyn->d_tag != DT_NULL; dyn++ ) {
        if( dyn->d_tag == DT_NEEDED ) {
            const char *need = (char *)stringtab + dyn->d_un.d_val;
            if( need != NULL )
                result.push_back( need );
        }
    }
    return result;
}

ADDRESS ElfBinaryFile::getImageBase()
{
    return m_uBaseAddr;
}

size_t ElfBinaryFile::getImageSize()
{
    return m_uImageSize;
}

/*==============================================================================
 * FUNCTION:      ElfBinaryFile::GetImportStubs
 * OVERVIEW:      Get an array of addresses of imported function stubs
 *                  This function relies on the fact that the symbols are
 *                  sorted by address, and that Elf PLT entries have successive
 *                  addresses beginning soon after m_PltMin
 * PARAMETERS:    numImports - reference to integer set to the number of these
 * RETURNS:       An array of native ADDRESSes
 *============================================================================*/
ADDRESS* ElfBinaryFile::GetImportStubs(int& numImports)
{
    ADDRESS a = m_uPltMin;
    int n = 0;
    std::map<ADDRESS, std::string>::iterator aa = m_SymA.find(a);
    std::map<ADDRESS, std::string>::iterator ff = aa;
    bool delDummy = false;
    if (aa == m_SymA.end()) {
        // Need to insert a dummy entry at m_uPltMin
        delDummy = true;
        m_SymA[a] = std::string();
        ff = m_SymA.find(a);
        aa = ff;
        aa++;
    }
    while ((aa != m_SymA.end()) && (a < m_uPltMax)) {
        n++;
        a = aa->first;
        aa++;
    }
    // Allocate an array of ADDRESSESes
    m_pImportStubs = new ADDRESS[n];
    aa = ff;                // Start at first
    a = aa->first;
    int i=0;
    while ((aa != m_SymA.end()) && (a < m_uPltMax)) {
        m_pImportStubs[i++] = a;
        a = aa->first;
        aa++;
    }
    if (delDummy)
        m_SymA.erase(ff);           // Delete dummy entry
    numImports = n;
    return m_pImportStubs;
}

// Create a new section
void new_section(Elf *elf, Elf_Scn **scn, Elf32_Shdr **hdr, Elf_Data **data)
{
    // Create a new section
    *scn = elf_newscn(elf);

    // Get header & initialize to zeroes
    *hdr = elf32_getshdr(*scn);
    (*hdr)->sh_name      = 0;           // No name
    (*hdr)->sh_type      = SHT_NULL;    // No type
    (*hdr)->sh_flags     = 0;
    (*hdr)->sh_addr      = 0;
    (*hdr)->sh_offset    = 0;
    (*hdr)->sh_size      = 0;
    (*hdr)->sh_link      = 0;
    (*hdr)->sh_info      = 0;
    // Char level alignment by default
    (*hdr)->sh_addralign = 1;
    (*hdr)->sh_entsize   = 0;

    // Get data & initialize to zeroes 
    *data = elf_newdata(*scn);
    (*data)->d_buf = NULL;
    (*data)->d_size = 0;
    (*data)->d_off = 0;
    // Char level alignment by default 
    (*data)->d_align = 1;
}
 
/*==============================================================================
 * FUNCTION:    ElfBinaryFile::writeObjectFile
 * OVERVIEW:    Writes an ELF object file.
 *              This function does not really reuse the code of the class :-(
 *              because it was design for reading purposes.
 * PARAMETERS:  - pname: path + file name (without ".o")
 *              - text:  binary code for text
 *              - txtsz: code sice (in bytes)
 *              - reloc: relocation table
 * RETURNS:     <nothing>
 *============================================================================*/
void ElfBinaryFile::writeObjectFile
    (std::string &path, const char* name, void *ptxt, int txtsz, RelocMap& reloc)
{
// Special section names
#define SHSTRTAB    ".shstrtab"
#define TEXT        ".text"
#define STRTAB      ".strtab"
#define SYMTAB      ".symtab"
#define RELA_TEXT   ".rela.text"

#define FIRST_GLB   2

// So far, only dealing with Sparc binaries -- FIXME!
#if TGT != SPARC
    error("Sorry, I don't know how to generate non-SPARC object files\n");
    exit(1);
#endif

    // Locals
    Elf_Scn    *scn;    // Section handler
    Elf32_Shdr *shdr;   // Section header handler
    Elf_Data   *sdata;  // Section data handler

    /*
     * Create the output (.o) ELF file
     */
    std::string Name = path + name + std::string(".o");
    int fout = open(Name.c_str(), O_RDWR|O_TRUNC|O_CREAT, 0666);
    elf_version(EV_CURRENT);
    Elf *elf = elf_begin(fout, ELF_C_WRITE, (Elf *)0);

    /*
     * General ELF header file
     */
    Elf32_Ehdr *ehdr = elf32_newehdr(elf);

    // Set header file
    ehdr->e_ident[EI_MAG0]    = 0x7f;
    ehdr->e_ident[EI_MAG1]    = 'E';
    ehdr->e_ident[EI_MAG2]    = 'L';
    ehdr->e_ident[EI_MAG3]    = 'F';
    ehdr->e_ident[EI_CLASS]   = ELFCLASS32;
    ehdr->e_ident[EI_DATA]    = ELFDATA2MSB;
    ehdr->e_ident[EI_VERSION] = EV_CURRENT;  

    // Frequently accessed section indices:
    int next_idx      = 0;              // Section [0] is always NULL!
    int shstrtab_idx  = ++next_idx;     // String table for section names 
    int text_idx      = ++next_idx;     // Text section
    int strtab_idx    = ++next_idx;     // String table
    int symtab_idx    = ++next_idx;     // Symbol table
//  int rela_text_idx = ++next_idx;     // Relocation table

    // General header information
    ehdr->e_type      = ET_REL;
    ehdr->e_machine   = EM_SPARC;       // Fixme!
    ehdr->e_version   = EV_CURRENT;
    ehdr->e_entry     = 0;
    ehdr->e_phoff     = 0;
    ehdr->e_shoff     = sizeof(Elf32_Ehdr);        
    ehdr->e_flags     = 0;
    ehdr->e_ehsize    = sizeof(Elf32_Ehdr);
    ehdr->e_phentsize = 0;
    ehdr->e_phnum     = 0;
    ehdr->e_shentsize = sizeof(Elf32_Shdr);
    ehdr->e_shstrndx  = shstrtab_idx;
    ehdr->e_shnum     = next_idx;
    // Not including .rela.text if ther is no relocation info
    if (reloc.size() == 0) ehdr->e_shnum--;

    /*
     * Section table for section names
     */
    new_section(elf, &scn, &shdr, &sdata);

    // Offset into string table for section names
    // Note: "+1" is for allocating the '\0' character
    int next_ofs      = 0;                  // Entry [0] is always NULL!
    int shstrtab_ofs  = next_ofs += 1;      // String table for section names
    int text_ofs      = next_ofs += strlen(SHSTRTAB) + 1;   // Text section
    int strtab_ofs    = next_ofs += strlen(TEXT)     + 1;   // String table
    int symtab_ofs    = next_ofs += strlen(STRTAB)   + 1;   // Symbol table
    int rela_text_ofs = next_ofs += strlen(SYMTAB)   + 1;   // Relocation table
    next_ofs += strlen(RELA_TEXT) + 1;      // Total string table size

    // Fill string pool for section names
    sdata->d_size = next_ofs;       // String table size
    sdata->d_buf = malloc(sdata->d_size);
    strcpy((char*)sdata->d_buf + 0,             "");
    strcpy((char*)sdata->d_buf + shstrtab_ofs,  SHSTRTAB);
    strcpy((char*)sdata->d_buf + text_ofs,      TEXT);
    strcpy((char*)sdata->d_buf + strtab_ofs,    STRTAB);
    strcpy((char*)sdata->d_buf + symtab_ofs,    SYMTAB);
    strcpy((char*)sdata->d_buf + rela_text_ofs, RELA_TEXT);

    // Fill header section
    shdr->sh_name = shstrtab_ofs;   // Offset to name
    shdr->sh_type = SHT_STRTAB;     // Table for string sections
    shdr->sh_size = sdata->d_size;  // Size of table

    /*
     * Text section
     */
    new_section(elf, &scn, &shdr, &sdata);

    // Set text buffer -- Data was already allocated!
    sdata->d_buf = ptxt;
    sdata->d_size = txtsz;
    sdata->d_align = SIZEOF_INT;   // Instr alignment (from config.h)
    
    // Fill header section
    shdr->sh_name = text_ofs;       // Offset to name
    shdr->sh_type = SHT_PROGBITS;   // Text section
    shdr->sh_flags = SHF_ALLOC | SHF_EXECINSTR;
    shdr->sh_size = sdata->d_size;  // Text size
    shdr->sh_addralign = SIZEOF_INT;  // Instr alignment

    /*
     * String table
     */
    new_section(elf, &scn, &shdr, &sdata);

    // Mapping between symbols and string table offsets
    StrIntMap str_ofs;
    str_ofs.clear();
    
    // Offset into string table from reloc info
    // Note: "+1" is for allocating the '\0' character
    next_ofs      = 0 + 1;              // Entry [0] is always NULL!
    str_ofs[name] = next_ofs;           // First entry is for this proc name
    next_ofs     += strlen(name) + 1;   // Next entry

    // Now the rest of symbols, from relocation info
    RelocMap::const_iterator rit;
    for (rit = reloc.begin(); rit != reloc.end(); rit++)
    {
        // Is symbol already in string table?
        if (str_ofs.find(rit->second) == str_ofs.end())
        {
            // Compute offset
            str_ofs[rit->second] = next_ofs;
            next_ofs += (rit->second).length() + 1; 
        }
    }
    // At this point, 'next_ofs' is the size of string table

    // Fill string pool for symbols
    sdata->d_size = next_ofs;       // String table size
    sdata->d_buf = malloc(sdata->d_size);
    strcpy((char*)sdata->d_buf + 0, "");
    StrIntMap::const_iterator sit;
    for (sit = str_ofs.begin(); sit != str_ofs.end(); sit++)
        strcpy((char*)sdata->d_buf + sit->second, (sit->first).c_str());
    
    // Fill header section
    shdr->sh_name  = strtab_ofs;    // Offset to name
    shdr->sh_type  = SHT_STRTAB;    // String table
    shdr->sh_flags = SHF_ALLOC;     // No special flags
    shdr->sh_size  = sdata->d_size; // Size of table

    /*
     * Symbol table
     * - Entry [0] is null
     * - Entry [1] is for .text section
     * - Entry [2] is the first global symbol
     * - Last entry is for current proc.
     */
    new_section(elf, &scn, &shdr, &sdata);
    
    // Mapping between symbols and symbol table entry
    StrIntMap sym_ent;
    sym_ent.clear();

    // Fill pool for symbol table
    int nitems     = str_ofs.size() + 2;  // [0,1 and n-1] always!
    sdata->d_size  = nitems * sizeof(Elf32_Sym);
    sdata->d_buf   = malloc(sdata->d_size);
    sdata->d_align = SIZEOF_INT;   // Instr alignment

    // First entry is null
    Elf32_Sym *sym  = &((Elf32_Sym*)(sdata->d_buf))[0];
    sym->st_info  = ELF32_ST_INFO(STB_LOCAL, STT_NOTYPE);
    sym->st_shndx = SHN_UNDEF;
    sym->st_value = sym->st_name = sym->st_size = sym->st_other = 0;

    // Second entry is .text section
    sym = &((Elf32_Sym*)(sdata->d_buf))[1];
    sym->st_info  = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
    sym->st_shndx = text_idx;
    sym->st_value = sym->st_name = sym->st_size = sym->st_other = 0;

    // The rest of global symbols...
    int i = FIRST_GLB;
    for (sit = str_ofs.begin(); sit != str_ofs.end(); sit++)
    {
        // Skip this proc symbol
        if (sit->first == name) continue;

        // Add symbol information
        sym = &((Elf32_Sym*)(sdata->d_buf))[i];
        sym->st_name  = sit->second;
        sym->st_info  = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE);
        sym->st_shndx = SHN_UNDEF;
        sym->st_value = sym->st_size = sym->st_other = 0;
        sym_ent[sit->first] = i++;
    }

    // Last entry is this procedure
    sym = &((Elf32_Sym*)(sdata->d_buf))[nitems - 1];
    sym->st_name  = str_ofs[name];
    sym->st_info  = ELF32_ST_INFO(STB_GLOBAL, STT_FUNC);
    sym->st_shndx = text_idx;       // Pointer to text section
    sym->st_size  = txtsz;          // Proc size
    sym->st_value = sym->st_other = 0;

    // Fill header section
    shdr->sh_name = symtab_ofs;     // Offset to name
    shdr->sh_type = SHT_SYMTAB;     // Symbol table
    shdr->sh_flags = SHF_ALLOC;
    shdr->sh_size = sdata->d_size;  // ST size
    shdr->sh_entsize = sizeof(Elf32_Sym);
    shdr->sh_addralign = SIZEOF_INT;  // Instr alignment
    shdr->sh_link = strtab_idx;     // Pointer to string table
    shdr->sh_info = FIRST_GLB;      // First global symbol

    /*
     * Relocation table
     * -- Do it only if there are relocatable addresses
     */
    if (reloc.size() != 0)
    {
        new_section(elf, &scn, &shdr, &sdata);

        // Fill pool for relocation table
        sdata->d_size  = reloc.size() * sizeof(Elf32_Rela);
        sdata->d_buf   = malloc(sdata->d_size);
        sdata->d_align = SIZEOF_INT;   // Instr alignment

        // Set relocation table from the relocation info
        for (i = 0, rit = reloc.begin(); rit != reloc.end(); i++, rit++)
        {
            // Set relocation info
            Elf32_Rela *rel = &((Elf32_Rela*)(sdata->d_buf))[i];
            rel->r_offset = rit->first;                      // Fixme!
#if TGT == SPARC
            rel->r_info   = ELF32_R_INFO(sym_ent[rit->second],R_SPARC_WDISP30);
#else
            rel->r_info   = ELF32_R_INFO(sym_ent[rit->second],/*Fixme!*/0);
#endif
            rel->r_addend = 0; 
        }

        // Fill header section
        shdr->sh_name = rela_text_ofs;  // Offset to name
        shdr->sh_type = SHT_RELA;       // Relocation table
        shdr->sh_flags = SHF_ALLOC;
        shdr->sh_size = sdata->d_size;  // Relocation table size
        shdr->sh_entsize = sizeof(Elf32_Rela);
        shdr->sh_addralign = SIZEOF_INT;  // Instr alignment
        shdr->sh_link = symtab_idx;     // Pointer to symbol table
        shdr->sh_info = text_idx;       // Pointer to text section
    }

    // Commiting ELF file
    elf_update(elf, ELF_C_WRITE);
    elf_end(elf);
    close(fout);
}

/*==============================================================================
 * FUNCTION:    ElfBinaryFile::GetDynamicGlobalMap
 * OVERVIEW:    Get a map from ADDRESS to const char*. This map contains the
 *                native addresses and symbolic names of global data items
 *                (if any) which are shared with dynamically linked libraries.
 *                Example: __iob (basis for stdout). The ADDRESS is the native
 *                address of a pointer to the real dynamic data object.
 * NOTE:        The caller should delete the returned map.
 * PARAMETERS:  None
 * RETURNS:     Pointer to a new map with the info, or 0 if none
 *============================================================================*/
std::map<ADDRESS, const char*>* ElfBinaryFile::GetDynamicGlobalMap()
{
    SectionInfo* pSect = GetSectionInfoByName(".rel.bss");
    if (pSect == 0)
        pSect = GetSectionInfoByName(".rela.bss");
    if (pSect == 0) {
        // This could easily mean that this file has no dynamic globals, and
        // that is fine.
        return 0;
    }
    int numEnt = pSect->uSectionSize / pSect->uSectionEntrySize;
    SectionInfo* sym = GetSectionInfoByName(".dynsym");
    if (sym == 0) {
        fprintf(stderr, "Could not find section .dynsym in source binary file");
        return 0;
    }
    Elf32_Sym* pSym = (Elf32_Sym*)sym->uHostAddr;
    int idxStr = GetSectionIndexByName(".dynstr");
    if (idxStr == -1) {
        fprintf(stderr, "Could not find section .dynstr in source binary file");
        return 0;
    }

    std::map<ADDRESS, const char*>* ret = new std::map<ADDRESS, const char*>;
    unsigned p = pSect->uHostAddr;
    for (int i=0; i < numEnt; i++) {
        // The ugly p[1] below is because it p might point to an
        // Elf32_Rela struct, or an Elf32_Rel struct
        int sym = ELF32_R_SYM(((int*)p)[1]);
        int name = pSym[sym].st_name;       // Index into string table
        const char* s = GetStrPtr(idxStr, name);
        ADDRESS val = ((int*)p)[0];
        // Add the (val, s) mapping to ret
        (*ret)[val] = s;
        p += pSect->uSectionEntrySize;
    }

    return ret;
}

// This function is called via dlopen/dlsym; it returns a new BinaryFile
// derived concrete object. After this object is returned, the virtual function
// call mechanism will call the rest of the code in this library
// It needs to be C linkage so that it its name is not mangled
extern "C" {
    BinaryFile* construct()
    {
        return new ElfBinaryFile;
    }    
}
