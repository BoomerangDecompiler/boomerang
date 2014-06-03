/*
 * Copyright (C) 1997-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file ElfBinaryFile.cpp
  * Desc: This file contains the implementation of the class ElfBinaryFile.
  ******************************************************************************/

/***************************************************************************/ /**
  * Dependencies.
  ******************************************************************************/

#include "ElfBinaryFile.h"

#include "config.h"
#include "util.h"

#include <QtCore/QDebug>
#include <sys/types.h> // Next three for open()
#include <sys/stat.h>
#include <fcntl.h>
#include <cstddef>
#include <iostream>
#include <cassert>
#include <cstring>
#include <inttypes.h>

typedef std::map<std::string, int, std::less<std::string>> StrIntMap;

ElfBinaryFile::ElfBinaryFile(bool bArchive /* = false */)
    : LoaderCommon(bArchive), // Initialise base class
      next_extern(ADDRESS::g(0L)) {
    m_fd = nullptr;
    m_pFileName = nullptr;
    Init(); // Initialise all the common stuff
}

ElfBinaryFile::~ElfBinaryFile() {
    if (m_pImportStubs)
        // Delete the array of import stubs
        delete[] m_pImportStubs;
}

// Reset internal state, except for those that keep track of which member
// we're up to
void ElfBinaryFile::Init() {
    m_pImage = nullptr;
    m_pPhdrs = nullptr;   // No program headers
    m_pShdrs = nullptr;   // No section headers
    m_pStrings = nullptr; // No strings
    m_pReloc = nullptr;
    m_pSym = nullptr;
    m_uPltMin = 0; // No PLT limits
    m_uPltMax = 0;
    m_iLastSize = 0;
    m_pImportStubs = nullptr;
}

// Hand decompiled from sparc library function
extern "C" { // So we can call this with dlopen()
unsigned elf_hash(const char *o0) {
    int o3 = *o0;
    const char *g1 = o0;
    unsigned o4 = 0;
    while (o3 != 0) {
        o4 <<= 4;
        o3 += o4;
        g1++;
        o4 = o3 & 0xf0000000;
        if (o4 != 0) {
            int o2 = (int)((unsigned)o4 >> 24);
            o3 = o3 ^ o2;
        }
        o4 = o3 & ~o4;
        o3 = *g1;
    }
    return o4;
}
} // extern "C"

// Return true for a good load
bool ElfBinaryFile::RealLoad(const QString &sName) {
    int i;

    if (m_bArchive) {
        // This is a member of an archive. Should not be using this function at all
        return false;
    }

    m_pFileName = sName;
    m_fd = fopen(qPrintable(sName), "rb");
    if (m_fd == nullptr)
        return 0;

    // Determine file size
    if (fseek(m_fd, 0, SEEK_END)) {
        fprintf(stderr, "Error seeking to end of binary file\n");
        return false;
    }
    m_lImageSize = ftell(m_fd);

    // Allocate memory to hold the file
    m_pImage = new char[m_lImageSize];
    if (m_pImage == nullptr) {
        fprintf(stderr, "Could not allocate %ld bytes for program image\n", m_lImageSize);
        return false;
    }
    Elf32_Ehdr *pHeader = (Elf32_Ehdr *)m_pImage; // Save a lot of casts

    // Read the whole file in
    fseek(m_fd, 0, SEEK_SET);
    size_t size = fread(m_pImage, 1, m_lImageSize, m_fd);
    if (size != (size_t)m_lImageSize)
        fprintf(stderr, "WARNING! Only read %zd of %ld bytes of binary file!\n", size, m_lImageSize);

    // Basic checks
    if (strncmp(m_pImage, "\x7F"
                          "ELF",
                4) != 0) {
        fprintf(stderr, "Incorrect header: %02X %02X %02X %02X\n", pHeader->e_ident[0], pHeader->e_ident[1],
                pHeader->e_ident[2], pHeader->e_ident[3]);
        return 0;
    }
    if ((pHeader->endianness != 1) && (pHeader->endianness != 2)) {
        fprintf(stderr, "Unknown endianness %02X\n", pHeader->endianness);
        return 0;
    }
    // Needed for elfRead4 to work:
    m_elfEndianness = pHeader->endianness - 1;

    // Set up program header pointer (in case needed)
    i = elfRead4(&pHeader->e_phoff);
    if (i)
        m_pPhdrs = (Elf32_Phdr *)(m_pImage + i);

    // Set up section header pointer
    i = elfRead4(&pHeader->e_shoff);
    if (i)
        m_pShdrs = (Elf32_Shdr *)(m_pImage + i);

    // Set up section header string table pointer
    // NOTE: it does not appear that endianness affects shorts.. they are always in little endian format
    // Gerard: I disagree. I need the elfRead on linux/i386
    i = elfRead2(&pHeader->e_shstrndx); // pHeader->e_shstrndx;
    if (i)
        m_pStrings = m_pImage + elfRead4(&m_pShdrs[i].sh_offset);

    i = 1;       // counter - # sects. Start @ 1, total m_iNumSections
    char *pName; // Section's name

    // Number of sections
    m_iNumSections = elfRead2(&pHeader->e_shnum);

    // Allocate room for all the Elf sections (including the silly first one)
    m_pSections = new SectionInfo[m_iNumSections];
    if (m_pSections == nullptr)
        return false; // Failed!

    // Set up the m_sh_link and m_sh_info arrays
    m_sh_link = new int[m_iNumSections];
    m_sh_info = new int[m_iNumSections];
    // Number of elf sections
    bool bGotCode = false; // True when have seen a code sect
    ADDRESS arbitaryLoadAddr = ADDRESS::g(0x08000000);
    for (i = 0; i < m_iNumSections; i++) {
        // Get section information.
        Elf32_Shdr *pShdr = m_pShdrs + i;
        if ((char *)pShdr > m_pImage + m_lImageSize) {
            std::cerr << "section " << i << " header is outside the image size\n";
            return false;
        }
        pName = m_pStrings + elfRead4(&pShdr->sh_name);
        if (pName > m_pImage + m_lImageSize) {
            std::cerr << "name for section " << i << " is outside the image size\n";
            return false;
        }
        m_pSections[i].pSectionName = pName;
        int off = elfRead4(&pShdr->sh_offset);
        if (off)
            m_pSections[i].uHostAddr = ADDRESS::host_ptr(m_pImage + off);
        m_pSections[i].uNativeAddr = elfRead4(&pShdr->sh_addr);
        m_pSections[i].uSectionSize = elfRead4(&pShdr->sh_size);
        if (m_pSections[i].uNativeAddr.isZero() && strncmp(pName, ".rel", 4)) {
            int align = elfRead4(&pShdr->sh_addralign);
            if (align > 1) {
                if (arbitaryLoadAddr.m_value % align)
                    arbitaryLoadAddr += align - (arbitaryLoadAddr.m_value % align);
            }
            m_pSections[i].uNativeAddr = arbitaryLoadAddr;
            arbitaryLoadAddr += m_pSections[i].uSectionSize;
        }
        m_pSections[i].uType = elfRead4(&pShdr->sh_type);
        m_sh_link[i] = elfRead4(&pShdr->sh_link);
        m_sh_info[i] = elfRead4(&pShdr->sh_info);
        m_pSections[i].uSectionEntrySize = elfRead4(&pShdr->sh_entsize);
        if (m_pSections[i].uNativeAddr + m_pSections[i].uSectionSize > next_extern)
            first_extern = next_extern = m_pSections[i].uNativeAddr + m_pSections[i].uSectionSize;
        if ((elfRead4(&pShdr->sh_flags) & SHF_WRITE) == 0)
            m_pSections[i].bReadOnly = true;
        // Can't use the SHF_ALLOC bit to determine bss section; the bss section has SHF_ALLOC but also SHT_NOBITS.
        // (But many other sections, such as .comment, also have SHT_NOBITS). So for now, just use the name
        //      if ((elfRead4(&pShdr->sh_flags) & SHF_ALLOC) == 0)
        if (strcmp(pName, ".bss") == 0)
            m_pSections[i].bBss = true;
        if (elfRead4(&pShdr->sh_flags) & SHF_EXECINSTR) {
            m_pSections[i].bCode = true;
            bGotCode = true; // We've got to a code section
        }
        // Deciding what is data and what is not is actually quite tricky but important.
        // For example, it's crucial to flag the .exception_ranges section as data, otherwise there is a "hole" in the
        // allocation map, that means that there is more than one "delta" from a read-only section to a page, and in the
        // end using -C results in a file that looks OK but when run just says "Killed".
        // So we use the Elf designations; it seems that ALLOC.!EXEC -> data
        // But we don't want sections before the .text section, like .interp, .hash, etc etc. Hence bGotCode.
        // NOTE: this ASSUMES that sections appear in a sensible order in the input binary file:
        // junk, code, rodata, data, bss
        if (bGotCode && ((elfRead4(&pShdr->sh_flags) & (SHF_EXECINSTR | SHF_ALLOC)) == SHF_ALLOC) &&
            (elfRead4(&pShdr->sh_type) != SHT_NOBITS))
            m_pSections[i].bData = true;
    } // for each section

    // assign arbitary addresses to .rel.* sections too
    for (i = 0; i < m_iNumSections; i++)
        if (m_pSections[i].uNativeAddr.isZero() && !strncmp(m_pSections[i].pSectionName, ".rel", 4)) {
            m_pSections[i].uNativeAddr = arbitaryLoadAddr;
            arbitaryLoadAddr += m_pSections[i].uSectionSize;
        }

    // Add symbol info. Note that some symbols will be in the main table only, and others in the dynamic table only.
    // So the best idea is to add symbols for all sections of the appropriate type
    for (i = 1; i < m_iNumSections; ++i) {
        unsigned uType = m_pSections[i].uType;
        if (uType == SHT_SYMTAB || uType == SHT_DYNSYM)
            AddSyms(i);
#if 0 // Ick; bad logic. Done with fake library function pointers now (-2 .. -1024)
        if (uType == SHT_REL || uType == SHT_RELA)
            AddRelocsAsSyms(i);
#endif
    }

    // Save the relocation to symbol table info
    PSectionInfo pRel = GetSectionInfoByName(".rela.text");
    if (pRel) {
        m_bAddend = true;                                // Remember its a relA table
        m_pReloc = (Elf32_Rel *)pRel->uHostAddr.m_value; // Save pointer to reloc table
        // SetRelocInfo(pRel);
    } else {
        m_bAddend = false;
        pRel = GetSectionInfoByName(".rel.text");
        if (pRel) {
            // SetRelocInfo(pRel);
            m_pReloc = (Elf32_Rel *)pRel->uHostAddr.m_value; // Save pointer to reloc table
        }
    }

    // Find the PLT limits. Required for IsDynamicLinkedProc(), e.g.
    PSectionInfo pPlt = GetSectionInfoByName(".plt");
    if (pPlt) {
        m_uPltMin = pPlt->uNativeAddr;
        m_uPltMax = pPlt->uNativeAddr + pPlt->uSectionSize;
    }

    // Apply relocations; important when the input program is not compiled with -fPIC
    applyRelocations();

    return true; // Success
}

// Clean up and unload the binary image
void ElfBinaryFile::UnLoad() {
    if (m_pImage)
        delete[] m_pImage;
    fclose(m_fd);
    Init(); // Set all internal state to 0
}

// Like a replacement for elf_strptr()
const char *ElfBinaryFile::GetStrPtr(int idx, int offset) {
    if (idx < 0) {
        // Most commonly, this will be an index of -1, because a call to GetSectionIndexByName() failed
        fprintf(stderr, "Error! GetStrPtr passed index of %d\n", idx);
        return (char *)"Error!";
    }
    // Get a pointer to the start of the string table
    char *pSym = (char *)m_pSections[idx].uHostAddr.m_value;
    // Just add the offset
    return pSym + offset;
}

// Search the .rel[a].plt section for an entry with symbol table index i.
// If found, return the native address of the associated PLT entry.
// A linear search will be needed. However, starting at offset i and searching backwards with wraparound should
// typically minimise the number of entries to search
ADDRESS ElfBinaryFile::findRelPltOffset(int i, ADDRESS addrRelPlt, int sizeRelPlt, int numRelPlt, ADDRESS addrPlt) {
    int first = i;
    if (first >= numRelPlt)
        first = numRelPlt - 1;
    int curr = first;
    do {
        // Each entry is sizeRelPlt bytes, and will contain the offset, then the info (addend optionally follows)
        int *pEntry = (int *)(addrRelPlt + (curr * sizeRelPlt)).m_value;
        int entry = elfRead4(pEntry + 1); // Read pEntry[1]
        int sym = entry >> 8;             // The symbol index is in the top 24 bits (Elf32 only)
        if (sym == i) {
            // Found! Now we want the native address of the associated PLT entry.
            // For now, assume a size of 0x10 for each PLT entry, and assume that each entry in the .rel.plt section
            // corresponds exactly to an entry in the .plt (except there is one dummy .plt entry)
            return addrPlt + 0x10 * (curr + 1);
        }
        if (--curr < 0)
            curr = numRelPlt - 1;
    } while (curr != first); // Will eventually wrap around to first if not present
    return ADDRESS::g(0L);   // Exit if this happens
}

// Add appropriate symbols to the symbol table.  secIndex is the section index of the symbol table.
void ElfBinaryFile::AddSyms(int secIndex) {
    int e_type = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_type);
    PSectionInfo pSect = &m_pSections[secIndex];
    // Calc number of symbols
    int nSyms = pSect->uSectionSize / pSect->uSectionEntrySize;
    m_pSym = (Elf32_Sym *)pSect->uHostAddr.m_value; // Pointer to symbols
    int strIdx = m_sh_link[secIndex];               // sh_link points to the string table

    PSectionInfo siPlt = GetSectionInfoByName(".plt");
    ADDRESS addrPlt = siPlt ? siPlt->uNativeAddr : ADDRESS::g(0L);
    PSectionInfo siRelPlt = GetSectionInfoByName(".rel.plt");
    int sizeRelPlt = 8; // Size of each entry in the .rel.plt table
    if (siRelPlt == nullptr) {
        siRelPlt = GetSectionInfoByName(".rela.plt");
        sizeRelPlt = 12; // Size of each entry in the .rela.plt table is 12 bytes
    }
    ADDRESS addrRelPlt = ADDRESS::g(0L);
    int numRelPlt = 0;
    if (siRelPlt) {
        addrRelPlt = siRelPlt->uHostAddr;
        numRelPlt = sizeRelPlt ? siRelPlt->uSectionSize / sizeRelPlt : 0;
    }
    // Number of entries in the PLT:
    // int max_i_for_hack = siPlt ? (int)siPlt->uSectionSize / 0x10 : 0;
    // Index 0 is a dummy entry
    for (int i = 1; i < nSyms; i++) {
        ADDRESS val = ADDRESS::g(elfRead4((int *)&m_pSym[i].st_value));
        int name = elfRead4(&m_pSym[i].st_name);
        if (name == 0) /* Silly symbols with no names */
            continue;
        std::string str(GetStrPtr(strIdx, name));
        // Hack off the "@@GLIBC_2.0" of Linux, if present
        std::string::size_type pos;
        if ((pos = str.find("@@")) != std::string::npos)
            str.erase(pos);
        std::map<ADDRESS, std::string>::iterator aa = m_SymTab.find(val);
        // Ensure no overwriting (except functions)
        if (aa == m_SymTab.end() || ELF32_ST_TYPE(m_pSym[i].st_info) == STT_FUNC) {
            if (val.isZero() && siPlt) { //&& i < max_i_for_hack) {
                // Special hack for gcc circa 3.3.3: (e.g. test/pentium/settest).  The value in the dynamic symbol table
                // is zero!  I was assuming that index i in the dynamic symbol table would always correspond to index i
                // in the .plt section, but for fedora2_true, this doesn't work. So we have to look in the .rel[a].plt
                // section. Thanks, gcc!  Note that this hack can cause strange symbol names to appear
                val = findRelPltOffset(i, addrRelPlt, sizeRelPlt, numRelPlt, addrPlt);
            } else if (e_type == E_REL) {
                int nsec = elfRead2(&m_pSym[i].st_shndx);
                if (nsec >= 0 && nsec < m_iNumSections)
                    val += GetSectionInfo(nsec)->uNativeAddr;
            }

#define ECHO_SYMS 0
#if ECHO_SYMS
            std::cerr << "Elf AddSym: about to add " << str << " to address " << std::hex << val << std::dec << "\n";
#endif
            m_SymTab[val] = str;
        }
    }
    ADDRESS uMain = GetMainEntryPoint();
    if (uMain != NO_ADDRESS && m_SymTab.find(uMain) == m_SymTab.end()) {
        // Ugh - main mustn't have the STT_FUNC attribute. Add it
        std::string sMain("main");
        m_SymTab[uMain] = sMain;
    }
    return;
}

std::vector<ADDRESS> ElfBinaryFile::GetExportedAddresses(bool funcsOnly) {
    std::vector<ADDRESS> exported;

    int i;
    int secIndex = 0;
    for (i = 1; i < m_iNumSections; ++i) {
        unsigned uType = m_pSections[i].uType;
        if (uType == SHT_SYMTAB) {
            secIndex = i;
            break;
        }
    }
    if (secIndex == 0)
        return exported;

    int e_type = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_type);
    PSectionInfo pSect = &m_pSections[secIndex];
    // Calc number of symbols
    int nSyms = pSect->uSectionSize / pSect->uSectionEntrySize;
    m_pSym = (Elf32_Sym *)pSect->uHostAddr.m_value; // Pointer to symbols
    int strIdx = m_sh_link[secIndex];               // sh_link points to the string table

    // Index 0 is a dummy entry
    for (int i = 1; i < nSyms; i++) {
        ADDRESS val = ADDRESS::g(elfRead4((int *)&m_pSym[i].st_value));
        int name = elfRead4(&m_pSym[i].st_name);
        if (name == 0) /* Silly symbols with no names */
            continue;
        std::string str(GetStrPtr(strIdx, name));
        // Hack off the "@@GLIBC_2.0" of Linux, if present
        std::string::size_type pos;
        if ((pos = str.find("@@")) != std::string::npos)
            str.erase(pos);
        if (ELF32_ST_BIND(m_pSym[i].st_info) == STB_GLOBAL || ELF32_ST_BIND(m_pSym[i].st_info) == STB_WEAK) {
            if (funcsOnly == false || ELF32_ST_TYPE(m_pSym[i].st_info) == STT_FUNC) {
                if (e_type == E_REL) {
                    int nsec = elfRead2(&m_pSym[i].st_shndx);
                    if (nsec >= 0 && nsec < m_iNumSections)
                        val += GetSectionInfo(nsec)->uNativeAddr;
                }
                exported.push_back(val);
            }
        }
    }
    return exported;
}

// FIXME: this function is way off the rails. It seems to always overwrite the relocation entry with the 32 bit value
// from the symbol table. Totally invalid for SPARC, and most X86 relocations!
// So currently not called
void ElfBinaryFile::AddRelocsAsSyms(int relSecIdx) {
    PSectionInfo pSect = &m_pSections[relSecIdx];
    if (pSect == nullptr)
        return;
    // Calc number of relocations
    int nRelocs = pSect->uSectionSize / pSect->uSectionEntrySize;
    m_pReloc = (Elf32_Rel *)pSect->uHostAddr.m_value; // Pointer to symbols
    int symSecIdx = m_sh_link[relSecIdx];
    int strSecIdx = m_sh_link[symSecIdx];

    // Index 0 is a dummy entry
    for (int i = 1; i < nRelocs; i++) {
        ADDRESS val = ADDRESS::g(elfRead4((int *)&m_pReloc[i].r_offset));
        int symIndex = elfRead4(&m_pReloc[i].r_info) >> 8;
        int flags = elfRead4(&m_pReloc[i].r_info);
        if ((flags & 0xFF) == R_386_32) {
            // Lookup the value of the symbol table entry
            ADDRESS a = ADDRESS::g(elfRead4((int *)&m_pSym[symIndex].st_value));
            if (m_pSym[symIndex].st_info & STT_SECTION)
                a = GetSectionInfo(elfRead2(&m_pSym[symIndex].st_shndx))->uNativeAddr;
            // Overwrite the relocation value... ?
            writeNative4(val, a.m_value);
            continue;
        }
        if ((flags & R_386_PC32) == 0)
            continue;
        if (symIndex == 0) /* Silly symbols with no names */
            continue;
        std::string str(GetStrPtr(strSecIdx, elfRead4(&m_pSym[symIndex].st_name)));
        // Hack off the "@@GLIBC_2.0" of Linux, if present
        std::string::size_type pos;
        if ((pos = str.find("@@")) != std::string::npos)
            str.erase(pos);
        std::map<ADDRESS, std::string>::iterator it;
        // Linear search!
        for (it = m_SymTab.begin(); it != m_SymTab.end(); it++)
            if ((*it).second == str)
                break;
        // Add new extern
        if (it == m_SymTab.end()) {
            m_SymTab[next_extern] = str;
            it = m_SymTab.find(next_extern);
            next_extern += 4;
        }
        writeNative4(val, ((*it).first - val - 4).m_value);
    }
    return;
}

// Note: this function overrides a simple "return 0" function in the base class (i.e. BinaryFile::SymbolByAddress())
const char *ElfBinaryFile::SymbolByAddress(const ADDRESS dwAddr) {
    std::map<ADDRESS, std::string>::iterator aa = m_SymTab.find(dwAddr);
    if (aa == m_SymTab.end())
        return nullptr;
    return (char *)aa->second.c_str();
}

bool ElfBinaryFile::ValueByName(const char *pName, SymValue *pVal, bool bNoTypeOK /* = false */) {
    int hash, numBucket, y; // numChain,
    int *pBuckets, *pChains; // For symbol table work
    int found;
    int *pHash;      // Pointer to hash table
    Elf32_Sym *pSym; // Pointer to the symbol table
    int iStr;        // Section index of the string table
    PSectionInfo pSect;

    pSect = GetSectionInfoByName(".dynsym");
    if (pSect == nullptr) {
        // We have a file with no .dynsym section, and hence no .hash section (from my understanding - MVE).
        // It seems that the only alternative is to linearly search the symbol tables.
        // This must be one of the big reasons that linking is so slow! (at least, for statically linked files)
        // Note MVE: We can't use m_SymTab because we may need the size
        return SearchValueByName(pName, pVal);
    }
    pSym = (Elf32_Sym *)pSect->uHostAddr.m_value;
    if (pSym == nullptr)
        return false;
    pSect = GetSectionInfoByName(".hash");
    if (pSect == nullptr)
        return false;
    pHash = (int *)pSect->uHostAddr.m_value;
    iStr = GetSectionIndexByName(".dynstr");

    // First organise the hash table
    numBucket = elfRead4(&pHash[0]);
    /*numChain  = */ elfRead4(&pHash[1]); // NOTE: use numChain to guard y iterations ?
    pBuckets = &pHash[2];
    pChains = &pBuckets[numBucket];

    // Hash the symbol
    hash = elf_hash(pName) % numBucket;
    y = elfRead4(&pBuckets[hash]); // Look it up in the bucket list
    // Beware of symbol tables with 0 in the buckets, e.g. libstdc++.
    // In that case, set found to false.
    found = (y != 0);
    if (y) {
        while (strcmp(pName, GetStrPtr(iStr, elfRead4(&pSym[y].st_name))) != 0) {
            y = elfRead4(&pChains[y]);
            if (y == 0) {
                found = false;
                break;
            }
        }
    }
    // Beware of symbols with STT_NOTYPE, e.g. "open" in libstdc++ !
    // But sometimes "main" has the STT_NOTYPE attribute, so if bNoTypeOK is passed as true, return true
    if (found && (bNoTypeOK || (ELF32_ST_TYPE(pSym[y].st_info) != STT_NOTYPE))) {
        pVal->uSymAddr = elfRead4((int *)&pSym[y].st_value);
        int e_type = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_type);
        if (e_type == E_REL) {
            int nsec = elfRead2(&pSym[y].st_shndx);
            if (nsec >= 0 && nsec < m_iNumSections)
                pVal->uSymAddr += GetSectionInfo(nsec)->uNativeAddr;
        }
        pVal->iSymSize = elfRead4(&pSym[y].st_size);
        return true;
    } else {
        // We may as well do a linear search of the main symbol table. Some symbols (e.g. init_dummy) are
        // in the main symbol table, but not in the hash table
        return SearchValueByName(pName, pVal);
    }
}

// Lookup the symbol table using linear searching. See comments above for why this appears to be needed.
bool ElfBinaryFile::SearchValueByName(const char *pName, SymValue *pVal, const char *pSectName, const char *pStrName) {
    // Note: this assumes .symtab. Many files don't have this section!!!
    PSectionInfo pSect, pStrSect;

    pSect = GetSectionInfoByName(pSectName);
    if (pSect == nullptr)
        return false;
    pStrSect = GetSectionInfoByName(pStrName);
    if (pStrSect == nullptr)
        return false;
    const char *pStr = (const char *)pStrSect->uHostAddr.m_value;
    // Find number of symbols
    int n = pSect->uSectionSize / pSect->uSectionEntrySize;
    Elf32_Sym *pSym = (Elf32_Sym *)pSect->uHostAddr.m_value;
    // Search all the symbols. It may be possible to start later than index 0
    for (int i = 0; i < n; i++) {
        int idx = elfRead4(&pSym[i].st_name);
        if (strcmp(pName, pStr + idx) == 0) {
            // We have found the symbol
            pVal->uSymAddr = elfRead4((int *)&pSym[i].st_value);
            int e_type = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_type);
            if (e_type == E_REL) {
                int nsec = elfRead2(&pSym[i].st_shndx);
                if (nsec >= 0 && nsec < m_iNumSections)
                    pVal->uSymAddr += GetSectionInfo(nsec)->uNativeAddr;
            }
            pVal->iSymSize = elfRead4(&pSym[i].st_size);
            return true;
        }
    }
    return false; // Not found (this table)
}

// Search for the given symbol. First search .symtab (if present); if not found or the table has been stripped,
// search .dynstr
bool ElfBinaryFile::SearchValueByName(const char *pName, SymValue *pVal) {
    if (SearchValueByName(pName, pVal, ".symtab", ".strtab"))
        return true;
    return SearchValueByName(pName, pVal, ".dynsym", ".dynstr");
}

ADDRESS ElfBinaryFile::GetAddressByName(const char *pName, bool bNoTypeOK /* = false */) {
    SymValue Val;
    bool bSuccess = ValueByName(pName, &Val, bNoTypeOK);
    if (bSuccess) {
        m_iLastSize = Val.iSymSize;
        m_uLastAddr = Val.uSymAddr;
        return Val.uSymAddr;
    } else
        return NO_ADDRESS;
}

int ElfBinaryFile::GetSizeByName(const char *pName, bool bNoTypeOK /* = false */) {
    SymValue Val;
    bool bSuccess = ValueByName(pName, &Val, bNoTypeOK);
    if (bSuccess) {
        m_iLastSize = Val.iSymSize;
        m_uLastAddr = Val.uSymAddr;
        return Val.iSymSize;
    } else
        return 0;
}

// Guess the size of a function by finding the next symbol after it, and subtracting the distance.
// This function is NOT efficient; it has to compare the closeness of ALL symbols in the symbol table
int ElfBinaryFile::GetDistanceByName(const char *sName, const char *pSectName) {
    int size = GetSizeByName(sName);
    if (size)
        return size; // No need to guess!
    // No need to guess, but if there are fillers, then subtracting labels will give a better answer for coverage
    // purposes. For example, switch_cc. But some programs (e.g. switch_ps) have the switch tables between the
    // end of _start and main! So we are better off overall not trying to guess the size of _start
    ADDRESS value = GetAddressByName(sName);
    if (value.isZero())
        return 0; // Symbol doesn't even exist!

    PSectionInfo pSect;
    pSect = GetSectionInfoByName(pSectName);
    if (pSect == nullptr)
        return 0;
    // Find number of symbols
    int n = pSect->uSectionSize / pSect->uSectionEntrySize;
    Elf32_Sym *pSym = (Elf32_Sym *)pSect->uHostAddr.m_value;
    // Search all the symbols. It may be possible to start later than index 0
    ADDRESS closest = ADDRESS::g(0xFFFFFFFF); // TODO: should use something like ADDRESS::max ??
    int idx = -1;
    for (int i = 0; i < n; i++) {
        if ((pSym[i].st_value > value.m_value) && (pSym[i].st_value < closest.m_value)) {
            idx = i;
            closest = pSym[i].st_value;
        }
    }
    if (idx == -1)
        return 0;
    // Do some checks on the symbol's value; it might be at the end of the .text section
    pSect = GetSectionInfoByName(".text");
    ADDRESS low = pSect->uNativeAddr;
    ADDRESS hi = low + pSect->uSectionSize;
    if ((value >= low) && (value < hi)) {
        // Our symbol is in the .text section. Put a ceiling of the end of the section on closest.
        if (closest > hi)
            closest = hi;
    }
    return (closest - value).m_value;
}

int ElfBinaryFile::GetDistanceByName(const char *sName) {
    int val = GetDistanceByName(sName, ".symtab");
    if (val)
        return val;
    return GetDistanceByName(sName, ".dynsym");
}

bool ElfBinaryFile::IsDynamicLinkedProc(ADDRESS uNative) {
    if (uNative.m_value > (unsigned)-1024 && uNative != NO_ADDRESS)
        return true; // Say yes for fake library functions
    if (uNative >= first_extern && uNative < next_extern)
        return true; // Yes for externs (not currently used)
    if (m_uPltMin.isZero())
        return false;
    return (uNative >= m_uPltMin) && (uNative < m_uPltMax); // Yes if a call to the PLT (false otherwise)
}

//
// GetEntryPoints()
// Returns a list of pointers to SectionInfo structs representing entry points to the program
// Item 0 is the main() function; items 1 and 2 are .init and .fini
//
std::list<SectionInfo *> &ElfBinaryFile::GetEntryPoints(const char *pEntry /* = "main" */) {
    SectionInfo *pSect = GetSectionInfoByName(".text");
    ADDRESS uMain = GetAddressByName(pEntry, true);
    ptrdiff_t delta = (uMain - pSect->uNativeAddr).m_value;
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
// Returns the entry point to main (this should be a label in elf binaries generated by compilers).
//
ADDRESS ElfBinaryFile::GetMainEntryPoint() { return GetAddressByName("main", true); }

ADDRESS ElfBinaryFile::GetEntryPoint() { return ADDRESS::g(elfRead4(&((Elf32_Ehdr *)m_pImage)->e_entry)); }

// FIXME: the below assumes a fixed delta
ADDRESS ElfBinaryFile::NativeToHostAddress(ADDRESS uNative) {
    if (m_iNumSections == 0)
        return ADDRESS::g(0L);
    return m_pSections[1].uHostAddr - m_pSections[1].uNativeAddr + uNative;
}

ADDRESS ElfBinaryFile::GetRelocatedAddress(ADDRESS uNative) {
    // Not implemented yet. But we need the function to make it all link
    return ADDRESS::g(0L);
}

bool ElfBinaryFile::PostLoad(void *handle) {
    // This function is called after an archive member has been loaded by ElfArchiveFile

    // Save the elf pointer
    // m_elf = (Elf*) handle;

    // return ProcessElfFile();
    return false;
}

// Open this binaryfile for reading AND writing
bool ElfBinaryFile::Open(const char *sName) { return false; }

void ElfBinaryFile::Close() { UnLoad(); }

LOAD_FMT ElfBinaryFile::GetFormat() const { return LOADFMT_ELF; }

MACHINE ElfBinaryFile::GetMachine() const {
    int machine = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_machine);
    if ((machine == EM_SPARC) || (machine == EM_SPARC32PLUS))
        return MACHINE_SPARC;
    else if (machine == EM_386)
        return MACHINE_PENTIUM;
    else if (machine == EM_PA_RISC)
        return MACHINE_HPRISC;
    else if (machine == EM_68K)
        return MACHINE_PALM; // Unlikely
    else if (machine == EM_PPC)
        return MACHINE_PPC;
    else if (machine == EM_ST20)
        return MACHINE_ST20;
    else if (machine == EM_MIPS)
        return MACHINE_MIPS;
    else if (machine == EM_X86_64) {
        std::cerr << "Error: ElfBinaryFile::GetMachine: The AMD x86-64 architecture is not supported yet\n";
        return (MACHINE)-1;
    }
    // An unknown machine type
    std::cerr << "Error: ElfBinaryFile::GetMachine: Unsupported machine type: " << machine << " (0x" << std::hex
              << machine << ")\n";
    std::cerr << "(Please add a description for this type, thanks!)\n";
    return (MACHINE)-1;
}

bool ElfBinaryFile::isLibrary() const {
    int type = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_type);
    return (type == ET_DYN);
}

QStringList ElfBinaryFile::getDependencyList() {
    QStringList result;
    ADDRESS stringtab = NO_ADDRESS;
    PSectionInfo dynsect = GetSectionInfoByName(".dynamic");
    if (dynsect == nullptr)
        return result; /* no dynamic section = statically linked */

    Elf32_Dyn *dyn;
    for (dyn = (Elf32_Dyn *)dynsect->uHostAddr.m_value; dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_STRTAB) {
            stringtab = ADDRESS::g(dyn->d_un.d_ptr);
            break;
        }
    }

    if (stringtab == NO_ADDRESS) /* No string table = no names */
        return result;
    stringtab = NativeToHostAddress(stringtab);

    for (dyn = (Elf32_Dyn *)dynsect->uHostAddr.m_value; dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_NEEDED) {
            const char *need = (char *)(stringtab + dyn->d_un.d_val).m_value;
            if (need != nullptr)
                result << need;
        }
    }
    return result;
}

ADDRESS ElfBinaryFile::getImageBase() { return m_uBaseAddr; }

size_t ElfBinaryFile::getImageSize() { return m_uImageSize; }

/***************************************************************************/ /**
  *
  * \brief      Get an array of addresses of imported function stubs
  *                    This function relies on the fact that the symbols are sorted by address, and that Elf PLT
  *                    entries have successive addresses beginning soon after m_PltMin
  * \param      numImports - reference to integer set to the number of these
  * \returns          An array of native ADDRESSes
  ******************************************************************************/
ADDRESS *ElfBinaryFile::GetImportStubs(int &numImports) {
    ADDRESS a = m_uPltMin;
    int n = 0;
    std::map<ADDRESS, std::string>::iterator aa = m_SymTab.find(a);
    std::map<ADDRESS, std::string>::iterator ff = aa;
    bool delDummy = false;
    if (aa == m_SymTab.end()) {
        // Need to insert a dummy entry at m_uPltMin
        delDummy = true;
        m_SymTab[a] = std::string();
        ff = m_SymTab.find(a);
        aa = ff;
        aa++;
    }
    while ((aa != m_SymTab.end()) && (a < m_uPltMax)) {
        n++;
        a = aa->first;
        aa++;
    }
    // Allocate an array of ADDRESSESes
    m_pImportStubs = new ADDRESS[n];
    aa = ff; // Start at first
    a = aa->first;
    int i = 0;
    while ((aa != m_SymTab.end()) && (a < m_uPltMax)) {
        m_pImportStubs[i++] = a;
        a = aa->first;
        aa++;
    }
    if (delDummy)
        m_SymTab.erase(ff); // Delete dummy entry
    numImports = n;
    return m_pImportStubs;
}

/***************************************************************************/ /**
  *
  * \brief    Get a map from ADDRESS to const char*. This map contains the native addresses
  *                    and symbolic names of global data items (if any) which are shared with dynamically
  *                    linked libraries.
  *                    Example: __iob (basis for stdout). The ADDRESS is the native address of a pointer
  *                    to the real dynamic data object.
  * NOTE:        The caller should delete the returned map.
  * \param    None
  * \returns        Pointer to a new map with the info, or 0 if none
  ******************************************************************************/
std::map<ADDRESS, const char *> *ElfBinaryFile::GetDynamicGlobalMap() {
    std::map<ADDRESS, const char *> *ret = new std::map<ADDRESS, const char *>;
    SectionInfo *pSect = GetSectionInfoByName(".rel.bss");
    if (pSect == nullptr)
        pSect = GetSectionInfoByName(".rela.bss");
    if (pSect == nullptr) {
        // This could easily mean that this file has no dynamic globals, and
        // that is fine.
        return ret;
    }
    int numEnt = pSect->uSectionSize / pSect->uSectionEntrySize;
    SectionInfo *sym = GetSectionInfoByName(".dynsym");
    if (sym == nullptr) {
        fprintf(stderr, "Could not find section .dynsym in source binary file");
        return ret;
    }
    Elf32_Sym *pSym = (Elf32_Sym *)sym->uHostAddr.m_value;
    int idxStr = GetSectionIndexByName(".dynstr");
    if (idxStr == -1) {
        fprintf(stderr, "Could not find section .dynstr in source binary file");
        return ret;
    }

    ADDRESS p = pSect->uHostAddr;
    for (int i = 0; i < numEnt; i++) {
        // The ugly p[1] below is because it p might point to an Elf32_Rela struct, or an Elf32_Rel struct
        int sym = ELF32_R_SYM(((int *)p.m_value)[1]);
        int name = pSym[sym].st_name; // Index into string table
        const char *s = GetStrPtr(idxStr, name);
        ADDRESS val = ADDRESS::g(((int *)p.m_value)[0]);
        (*ret)[val] = s; // Add the (val, s) mapping to ret
        p += pSect->uSectionEntrySize;
    }

    return ret;
}

/***************************************************************************/ /**
  *
  * \brief    Read a 2 or 4 byte quantity from host address (C pointer) p
  * NOTE:        Takes care of reading the correct endianness, set early on into m_elfEndianness
  * \param    ps or pi: host pointer to the data
  * \returns        An integer representing the data
  ******************************************************************************/
int ElfBinaryFile::elfRead2(short *ps) const {
    unsigned char *p = (unsigned char *)ps;
    if (m_elfEndianness) {
        // Big endian
        return (int)((p[0] << 8) + p[1]);
    } else {
        // Little endian
        return (int)(p[0] + (p[1] << 8));
    }
}
int ElfBinaryFile::elfRead4(int *pi) const {
    short *p = (short *)pi;
    if (m_elfEndianness) {
        return (int)((elfRead2(p) << 16) + elfRead2(p + 1));
    } else
        return (int)(elfRead2(p) + (elfRead2(p + 1) << 16));
}

void ElfBinaryFile::elfWrite4(int *pi, int val) {
    char *p = (char *)pi;
    if (m_elfEndianness) {
        // Big endian
        *p++ = (char)(val >> 24);
        *p++ = (char)(val >> 16);
        *p++ = (char)(val >> 8);
        *p = (char)val;
    } else {
        *p++ = (char)val;
        *p++ = (char)(val >> 8);
        *p++ = (char)(val >> 16);
        *p = (char)(val >> 24);
    }
}

char ElfBinaryFile::readNative1(ADDRESS nat) {
    PSectionInfo si = GetSectionInfoByAddr(nat);
    if (si == nullptr) {
        qDebug() << "Target Memory access in unmapped Section " << nat.m_value;
        return -1;
        si = GetSectionInfo(0);
    }
    ADDRESS host = si->uHostAddr - si->uNativeAddr + nat;
    return *(char *)host.m_value;
}

// Read 2 bytes from given native address
int ElfBinaryFile::readNative2(ADDRESS nat) {
    PSectionInfo si = GetSectionInfoByAddr(nat);
    if (si == nullptr)
        return 0;
    ADDRESS host = si->uHostAddr - si->uNativeAddr + nat;
    return elfRead2((short *)host.m_value);
}

// Read 4 bytes from given native address
int ElfBinaryFile::readNative4(ADDRESS nat) {
    PSectionInfo si = GetSectionInfoByAddr(nat);
    if (si == nullptr)
        return 0;
    ADDRESS host = si->uHostAddr - si->uNativeAddr + nat;
    return elfRead4((int *)host.m_value);
}

void ElfBinaryFile::writeNative4(ADDRESS nat, uint32_t n) {
    PSectionInfo si = GetSectionInfoByAddr(nat);
    if (si == nullptr)
        return;
    ADDRESS host = si->uHostAddr - si->uNativeAddr + nat;
    uint8_t *host_ptr = (unsigned char *)host.m_value;
    if (m_elfEndianness) {
        host_ptr[0] = (n >> 24) & 0xff;
        host_ptr[1] = (n >> 16) & 0xff;
        host_ptr[2] = (n >> 8) & 0xff;
        host_ptr[3] = n & 0xff;
    } else {
        host_ptr[3] = (n >> 24) & 0xff;
        host_ptr[2] = (n >> 16) & 0xff;
        host_ptr[1] = (n >> 8) & 0xff;
        host_ptr[0] = n & 0xff;
    }
}

// Read 8 bytes from given native address
QWord ElfBinaryFile::readNative8(ADDRESS nat) {
    int raw[2];
#ifdef WORDS_BIGENDIAN     // This tests the  host     machine
    if (m_elfEndianness) { // This tests the source machine
#else
    if (!m_elfEndianness) {
#endif // Balance }
        // Source and host are same endianness
        raw[0] = readNative4(nat);
        raw[1] = readNative4(nat + 4);
    } else {
        // Source and host are different endianness
        raw[1] = readNative4(nat);
        raw[0] = readNative4(nat + 4);
    }
    // return reinterpret_cast<long long>(*raw);       // Note: cast, not convert!!
    return *(QWord *)raw;
}

// Read 4 bytes as a float
float ElfBinaryFile::readNativeFloat4(ADDRESS nat) {
    int raw = readNative4(nat);
    // Ugh! gcc says that reinterpreting from int to float is invalid!!
    // return reinterpret_cast<float>(raw);      // Note: cast, not convert!!
    return *(float *)&raw; // Note: cast, not convert
}

// Read 8 bytes as a float
double ElfBinaryFile::readNativeFloat8(ADDRESS nat) {
    int raw[2];
#ifdef WORDS_BIGENDIAN     // This tests the  host     machine
    if (m_elfEndianness) { // This tests the source machine
#else
    if (!m_elfEndianness) {
#endif // Balance }
        // Source and host are same endianness
        raw[0] = readNative4(nat);
        raw[1] = readNative4(nat + 4);
    } else {
        // Source and host are different endianness
        raw[1] = readNative4(nat);
        raw[0] = readNative4(nat + 4);
    }
    // return reinterpret_cast<double>(*raw);    // Note: cast, not convert!!
    return *(double *)raw;
}

// This function is called via dlopen/dlsym; it returns a Binary::getFile derived concrete object.
// After this object is returned, the virtual function call mechanism will call the rest of the code
// in this library. It needs to be C linkage so that it its name is not mangled
extern "C" {
#ifdef _WIN32
__declspec(dllexport)
#endif
    QObject *construct() {
    ElfBinaryFile *res = new ElfBinaryFile;
    assert(qobject_cast<BinaryData *>(res) != nullptr);
    return res;
}
}

void ElfBinaryFile::applyRelocations() {
    int nextFakeLibAddr = -2; // See R_386_PC32 below; -1 sometimes used for main
    if (m_pImage == nullptr)
        return; // No file loaded
    int machine = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_machine);
    int e_type = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_type);
    switch (machine) {
    case EM_SPARC:
        break; // Not implemented yet
    case EM_386: {
        for (int i = 1; i < m_iNumSections; ++i) {
            SectionInfo *ps = &m_pSections[i];
            if (ps->uType == SHT_REL) {
                // A section such as .rel.dyn or .rel.plt (without an addend field).
                // Each entry has 2 words: r_offet and r_info. The r_offset is just the offset from the beginning
                // of the section (section given by the section header's sh_info) to the word to be modified.
                // r_info has the type in the bottom byte, and a symbol table index in the top 3 bytes.
                // A symbol table offset of 0 (STN_UNDEF) means use value 0. The symbol table involved comes from
                // the section header's sh_link field.
                int *pReloc = (int *)ps->uHostAddr.m_value;
                unsigned size = ps->uSectionSize;
                // NOTE: the r_offset is different for .o files (E_REL in the e_type header field) than for exe's
                // and shared objects!
                ADDRESS destNatOrigin = ADDRESS::g(0L), destHostOrigin = ADDRESS::g(0L);
                if (e_type == E_REL) {
                    int destSection = m_sh_info[i];
                    destNatOrigin = m_pSections[destSection].uNativeAddr;
                    destHostOrigin = m_pSections[destSection].uHostAddr;
                }
                int symSection = m_sh_link[i];          // Section index for the associated symbol table
                int strSection = m_sh_link[symSection]; // Section index for the string section assoc with this
                char *pStrSection = (char *)m_pSections[strSection].uHostAddr.m_value;
                Elf32_Sym *symOrigin = (Elf32_Sym *)m_pSections[symSection].uHostAddr.m_value;
                for (unsigned u = 0; u < size; u += 2 * sizeof(unsigned)) {
                    unsigned r_offset = elfRead4(pReloc++);
                    unsigned info = elfRead4(pReloc++);
                    unsigned char relType = (unsigned char)info;
                    unsigned symTabIndex = info >> 8;
                    int *pRelWord; // Pointer to the word to be relocated
                    if (e_type == E_REL)
                        pRelWord = ((int *)(destHostOrigin + r_offset).m_value);
                    else {
                        SectionInfo *destSec = GetSectionInfoByAddr(ADDRESS::g(r_offset));
                        pRelWord = (int *)(destSec->uHostAddr - destSec->uNativeAddr + r_offset).m_value;
                        destNatOrigin = ADDRESS::g(0L);
                    }
                    ADDRESS A, S = ADDRESS::g(0L), P;
                    int nsec;
                    switch (relType) {
                    case 0: // R_386_NONE: just ignore (common)
                        break;
                    case 1: // R_386_32: S + A
                        S = elfRead4((int *)&symOrigin[symTabIndex].st_value);
                        if (e_type == E_REL) {
                            nsec = elfRead2(&symOrigin[symTabIndex].st_shndx);
                            if (nsec >= 0 && nsec < m_iNumSections)
                                S += GetSectionInfo(nsec)->uNativeAddr;
                        }
                        A = elfRead4(pRelWord);
                        elfWrite4(pRelWord, (S + A).m_value);
                        break;
                    case 2: // R_386_PC32: S + A - P
                        if (ELF32_ST_TYPE(symOrigin[symTabIndex].st_info) == STT_SECTION) {
                            nsec = elfRead2(&symOrigin[symTabIndex].st_shndx);
                            if (nsec >= 0 && nsec < m_iNumSections)
                                S = GetSectionInfo(nsec)->uNativeAddr;
                        } else {
                            S = elfRead4((int *)&symOrigin[symTabIndex].st_value);
                            if (S.isZero()) {
                                // This means that the symbol doesn't exist in this module, and is not accessed
                                // through the PLT, i.e. it will be statically linked, e.g. strcmp. We have the
                                // name of the symbol right here in the symbol table entry, but the only way
                                // to communicate with the loader is through the target address of the call.
                                // So we use some very improbable addresses (e.g. -1, -2, etc) and give them entries
                                // in the symbol table
                                int nameOffset = elfRead4((int *)&symOrigin[symTabIndex].st_name);
                                char *pName = pStrSection + nameOffset;
                                // this is too slow, I'm just going to assume it is 0
                                // S = GetAddressByName(pName);
                                // if (S == (e_type == E_REL ? 0x8000000 : 0)) {
                                S = nextFakeLibAddr--; // Allocate a new fake address
                                AddSymbol(S, pName);
                                //}
                            } else if (e_type == E_REL) {
                                nsec = elfRead2(&symOrigin[symTabIndex].st_shndx);
                                if (nsec >= 0 && nsec < m_iNumSections)
                                    S += GetSectionInfo(nsec)->uNativeAddr;
                            }
                        }
                        A = elfRead4(pRelWord);
                        P = destNatOrigin + r_offset;
                        elfWrite4(pRelWord, (S + A - P).m_value);
                        break;
                    case 7:
                    case 8:    // R_386_RELATIVE
                        break; // No need to do anything with these, if a shared object
                    default:
                        // std::cout << "Relocation type " << (int)relType << " not handled yet\n";
                        ;
                    }
                }
            }
        }
    }
    default:
        break; // Not implemented
    }
}

bool ElfBinaryFile::IsRelocationAt(ADDRESS uNative) {
    // int nextFakeLibAddr = -2;            // See R_386_PC32 below; -1 sometimes used for main
    if (m_pImage == nullptr)
        return false; // No file loaded
    int machine = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_machine);
    int e_type = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_type);
    switch (machine) {
    case EM_SPARC:
        break; // Not implemented yet
    case EM_386: {
        for (int i = 1; i < m_iNumSections; ++i) {
            SectionInfo *ps = &m_pSections[i];
            if (ps->uType == SHT_REL) {
                // A section such as .rel.dyn or .rel.plt (without an addend field).
                // Each entry has 2 words: r_offet and r_info. The r_offset is just the offset from the beginning
                // of the section (section given by the section header's sh_info) to the word to be modified.
                // r_info has the type in the bottom byte, and a symbol table index in the top 3 bytes.
                // A symbol table offset of 0 (STN_UNDEF) means use value 0. The symbol table involved comes from
                // the section header's sh_link field.
                int *pReloc = (int *)ps->uHostAddr.m_value;
                unsigned size = ps->uSectionSize;
                // NOTE: the r_offset is different for .o files (E_REL in the e_type header field) than for exe's
                // and shared objects!
                ADDRESS destNatOrigin = ADDRESS::g(0L), destHostOrigin;
                if (e_type == E_REL) {
                    int destSection = m_sh_info[i];
                    destNatOrigin = m_pSections[destSection].uNativeAddr;
                    destHostOrigin = m_pSections[destSection].uHostAddr;
                }
                // int symSection = m_sh_link[i];            // Section index for the associated symbol table
                // int strSection = m_sh_link[symSection];    // Section index for the string section assoc with this
                // char* pStrSection = (char*)m_pSections[strSection].uHostAddr;
                // Elf32_Sym* symOrigin = (Elf32_Sym*) m_pSections[symSection].uHostAddr;
                for (unsigned u = 0; u < size; u += 2 * sizeof(unsigned)) {
                    unsigned r_offset = elfRead4(pReloc++);
                    // unsigned info    = elfRead4(pReloc);
                    pReloc++;
                    // unsigned char relType = (unsigned char) info;
                    // unsigned symTabIndex = info >> 8;
                    ADDRESS pRelWord; // Pointer to the word to be relocated
                    if (e_type == E_REL)
                        pRelWord = destNatOrigin + r_offset;
                    else {
                        SectionInfo *destSec = GetSectionInfoByAddr(ADDRESS::g(r_offset));
                        pRelWord = destSec->uNativeAddr + r_offset;
                        destNatOrigin = 0;
                    }
                    if (uNative == pRelWord)
                        return true;
                }
            }
        }
    }
    default:
        break; // Not implemented
    }
    return false;
}

const char *ElfBinaryFile::getFilenameSymbolFor(const char *sym) {
    int i;
    int secIndex = 0;
    for (i = 1; i < m_iNumSections; ++i) {
        unsigned uType = m_pSections[i].uType;
        if (uType == SHT_SYMTAB) {
            secIndex = i;
            break;
        }
    }
    if (secIndex == 0)
        return nullptr;

    // int e_type = elfRead2(&((Elf32_Ehdr*)m_pImage)->e_type);
    PSectionInfo pSect = &m_pSections[secIndex];
    // Calc number of symbols
    int nSyms = pSect->uSectionSize / pSect->uSectionEntrySize;
    m_pSym = (Elf32_Sym *)pSect->uHostAddr.m_value; // Pointer to symbols
    int strIdx = m_sh_link[secIndex];               // sh_link points to the string table

    std::string filename;

    // Index 0 is a dummy entry
    for (int i = 1; i < nSyms; i++) {
        // ADDRESS val = (ADDRESS) elfRead4((int*)&m_pSym[i].st_value);
        int name = elfRead4(&m_pSym[i].st_name);
        if (name == 0) /* Silly symbols with no names */
            continue;
        std::string str(GetStrPtr(strIdx, name));
        // Hack off the "@@GLIBC_2.0" of Linux, if present
        std::string::size_type pos;
        if ((pos = str.find("@@")) != std::string::npos)
            str.erase(pos);
        if (ELF32_ST_TYPE(m_pSym[i].st_info) == STT_FILE) {
            filename = str;
            continue;
        }
        if (str == sym) {
            if (filename.length())
                return strdup(filename.c_str());
            return nullptr;
        }
    }
    return nullptr;
}

// A map for extra symbols, those not in the usual Elf symbol tables
void ElfBinaryFile::AddSymbol(ADDRESS uNative, const char *pName) { m_SymTab[uNative] = pName; }

void ElfBinaryFile::dumpSymbols() {
    std::map<ADDRESS, std::string>::iterator it;
    std::cerr << std::hex;
    for (it = m_SymTab.begin(); it != m_SymTab.end(); ++it)
        std::cerr << "0x" << it->first << " " << it->second << "        ";
    std::cerr << std::dec << "\n";
}
