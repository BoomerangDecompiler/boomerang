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

#include "ElfTypes.h"
#include "config.h"
#include "util.h"
#include "IBoomerang.h"
#include "IBinaryImage.h"
#include "IBinarySymbols.h"

#include <QtCore/QDebug>
#include <sys/types.h> // Next three for open()
#include <sys/stat.h>
#include <fcntl.h>
#include <cstddef>
#include <cassert>
#include <cstring>
#include <inttypes.h>

struct SectionParam {
    QString Name;
    ADDRESS SourceAddr;
    size_t Size;
    size_t entry_size;
    bool ReadOnly;
    bool Bss;
    bool Code;
    bool Data;
    ADDRESS image_ptr;
    unsigned uType;             // Type of section (format dependent)
};
// not part of anonymous namespace, since it would create an ambiguity
// anonymous_namespace::Translated_ElfSym vs. ElfTypes.h/Translated_ElfSym declarations
struct Translated_ElfSym{
    QString Name;
    ElfSymType Type;
    ElfSymBinding Binding;
    ElfSymVisibility Visibility;
    uint32_t SymbolSize;
    uint16_t SectionIdx;
    ADDRESS Value;
};

typedef std::map<QString, int, std::less<QString>> StrIntMap;

ElfBinaryFile::ElfBinaryFile() : next_extern(ADDRESS::g(0L)) {
    m_fd = nullptr;
    m_pFileName = nullptr;
    Init(); // Initialise all the common stuff
}

ElfBinaryFile::~ElfBinaryFile() {
    if (m_pImportStubs)
        // Delete the array of import stubs
        delete []m_pImportStubs;
    delete  []m_pImage;
    delete  []m_sh_link;
    delete  []m_sh_info;

}

void ElfBinaryFile::initialize(IBoomerang *sys)
{
    Image = sys->getImage();
    Symbols = sys->getSymbols();
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
    ElfSections.clear();
}

// Hand decompiled from sparc library function
extern "C" { // So we can call this with dlopen()
Q_DECL_EXPORT
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
    //    if (m_bArchive) {
    //        // This is a member of an archive. Should not be using this function at all
    //        return false;
    //    }

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

    i = 1;       // counter - # sects. Start @ 1, total Image->GetNumSections()
    char *pName; // Section's name

    // Number of sections
    uint32_t numSections = elfRead2(&pHeader->e_shnum);
    // Set up the m_sh_link and m_sh_info arrays
    m_sh_link = new int[numSections];
    m_sh_info = new int[numSections];
    // Number of elf sections
    bool bGotCode = false; // True when have seen a code sect
    ADDRESS arbitaryLoadAddr = ADDRESS::g(0x08000000);

    for (unsigned i = 0; i < numSections; i++) {
        // Get section information.
        Elf32_Shdr *pShdr = m_pShdrs + i;
        if ((char *)pShdr > m_pImage + m_lImageSize) {
            fprintf(stderr,"section %d header is outside the image size\n",i);
            return false;
        }
        pName = m_pStrings + elfRead4(&pShdr->sh_name);
        if (pName > m_pImage + m_lImageSize) {
            fprintf(stderr,"name for section %d is outside the image size\n",i);
            return false;
        }

        SectionParam sect;
        sect.Name = pName;
        // Can't use the SHF_ALLOC bit to determine bss section; the bss section has SHF_ALLOC but also SHT_NOBITS.
        // (But many other sections, such as .comment, also have SHT_NOBITS). So for now, just use the name
        //      if ((elfRead4(&pShdr->sh_flags) & SHF_ALLOC) == 0)
        sect.Bss = (strcmp(pName, ".bss") == 0);
        sect.Code = false;
        sect.Data = false;
        sect.ReadOnly = false;
        int off = elfRead4(&pShdr->sh_offset);
        if (off)
            sect.image_ptr = ADDRESS::host_ptr(m_pImage + off);
        sect.SourceAddr = elfRead4(&pShdr->sh_addr);
        sect.Size = elfRead4(&pShdr->sh_size);
        if (sect.SourceAddr.isZero() && strncmp(pName, ".rel", 4)) {
            int align = elfRead4(&pShdr->sh_addralign);
            if (align > 1) {
                if (arbitaryLoadAddr.m_value % align)
                    arbitaryLoadAddr += align - (arbitaryLoadAddr.m_value % align);
            }
            sect.SourceAddr = arbitaryLoadAddr;
            arbitaryLoadAddr += sect.Size ? sect.Size : 1;
        }
        sect.uType = elfRead4(&pShdr->sh_type);
        m_sh_link[i] = elfRead4(&pShdr->sh_link);
        m_sh_info[i] = elfRead4(&pShdr->sh_info);
        sect.entry_size = elfRead4(&pShdr->sh_entsize);
        if (sect.SourceAddr + sect.Size > next_extern)
            first_extern = next_extern = sect.SourceAddr + sect.Size;
        if ((elfRead4(&pShdr->sh_flags) & SHF_WRITE) == 0)
            sect.ReadOnly = true;
        if (elfRead4(&pShdr->sh_flags) & SHF_EXECINSTR) {
            sect.Code = true;
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
            sect.Data = true;
        ElfSections.push_back(sect);
    } // for each section

    // assign arbitary addresses to .rel.* sections too
    for (SectionParam &sect : ElfSections) {
        if (sect.SourceAddr.isZero() && sect.Name.startsWith(".rel")) {
            sect.SourceAddr = arbitaryLoadAddr;
            arbitaryLoadAddr += sect.Size ? sect.Size : 1;
        }
    }
    // Inform Boomerang about new sections
    for(SectionParam par : ElfSections) {
        if(par.Size==0) {
            qDebug() << "Not adding 0 sized section " << par.Name;
            continue;
        }
        IBinarySection *sect  = Image->createSection(par.Name,par.SourceAddr,par.SourceAddr+par.Size);
        assert(sect);
        if(sect) {
            sect->setBss(par.Bss)
                    .setCode(par.Code)
                    .setData(par.Data)
                    .setEndian(m_elfEndianness)
                    .setHostAddr(par.image_ptr)
                    .setEntrySize(par.entry_size);
            if( !(par.Bss || par.SourceAddr.isZero()) ) {
                sect->addDefinedArea(par.SourceAddr,par.SourceAddr+par.Size);
            }
        }

    }
    // Add symbol info. Note that some symbols will be in the main table only, and others in the dynamic table only.
    // So the best idea is to add symbols for all sections of the appropriate type
    for (unsigned i = 1; i < ElfSections.size(); ++i) {
        unsigned uType = ElfSections[i].uType;
        if (uType == SHT_SYMTAB || uType == SHT_DYNSYM) {
            AddSyms(i);
        }
    }

    // Save the relocation to symbol table info
    IBinarySection * pRel = Image->GetSectionInfoByName(".rela.text");
    if (pRel) {
        m_bAddend = true;                                // Remember its a relA table
        m_pReloc = (Elf32_Rel *)pRel->hostAddr().m_value; // Save pointer to reloc table
        // SetRelocInfo(pRel);
    } else {
        m_bAddend = false;
        pRel = Image->GetSectionInfoByName(".rel.text");
        if (pRel) {
            // SetRelocInfo(pRel);
            m_pReloc = (Elf32_Rel *)pRel->hostAddr().m_value; // Save pointer to reloc table
        }
    }

    // Find the PLT limits. Required for IsDynamicLinkedProc(), e.g.
    IBinarySection * pPlt = Image->GetSectionInfoByName(".plt");
    if (pPlt) {
        m_uPltMin = pPlt->sourceAddr();
        m_uPltMax = pPlt->sourceAddr() + pPlt->size();
    }

    // Apply relocations; important when the input program is not compiled with -fPIC
    applyRelocations();
    markImports();
    return true; // Success
}

// Clean up and unload the binary image
void ElfBinaryFile::UnLoad() {
    if (m_pImage)
        delete[] m_pImage;
    if(m_fd) {
        fclose(m_fd);
        m_fd = nullptr;
    }
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
    char *pSym = (char *)ElfSections[idx].image_ptr.m_value;
    // Just add the offset
    return pSym + offset;
}

// Search the .rel[a].plt section for an entry with symbol table index i.
// If found, return the native address of the associated PLT entry.
// A linear search will be needed. However, starting at offset i and searching backwards with wraparound should
// typically minimise the number of entries to search
ADDRESS ElfBinaryFile::findRelPltOffset(int i) {
    const IBinarySection * siPlt = Image->GetSectionInfoByName(".plt");
    ADDRESS addrPlt = siPlt ? siPlt->sourceAddr() : ADDRESS::g(0L);
    const IBinarySection * siRelPlt = Image->GetSectionInfoByName(".rel.plt");
    int sizeRelPlt = 8; // Size of each entry in the .rel.plt table
    if (siRelPlt == nullptr) {
        siRelPlt = Image->GetSectionInfoByName(".rela.plt");
        sizeRelPlt = 12; // Size of each entry in the .rela.plt table is 12 bytes
    }
    ADDRESS addrRelPlt = ADDRESS::g(0L);
    int numRelPlt = 0;
    if (siRelPlt) {
        addrRelPlt = siRelPlt->hostAddr();
        numRelPlt = sizeRelPlt ? siRelPlt->size() / sizeRelPlt : 0;
    }

    int first = i;
    if (first >= numRelPlt)
        first = numRelPlt - 1;
    int curr = first;
    int pltEntrySize = siPlt->getEntrySize();
    do {
        // Each entry is sizeRelPlt bytes, and will contain the offset, then the info (addend optionally follows)
        int *pEntry = (int *)(addrRelPlt + (curr * sizeRelPlt)).m_value;
        int entry = elfRead4(pEntry + 1); // Read pEntry[1]
        int sym = entry >> 8;             // The symbol index is in the top 24 bits (Elf32 only)
        int entry_type = entry&0xFF;
        if (sym == i) {
            const IBinarySection * targetSect =Image->getSectionInfoByAddr(ADDRESS::n(elfRead4(pEntry)));
            if(targetSect->getName().contains("got")) {
                int c = elfRead4(pEntry) - targetSect->sourceAddr().m_value;
                int plt_offset2 = elfRead4((int *)(targetSect->hostAddr() + c).m_value);
                int plt_idx = (plt_offset2 % pltEntrySize);
                if(entry_type == R_386_JUMP_SLOT) {
                    return ADDRESS::n(plt_offset2-6);
                }
                return addrPlt + plt_idx*pltEntrySize;
                qDebug() << "x";
            }
            int plt_offset = elfRead4(pEntry) - siPlt->sourceAddr().m_value;
            // Found! Now we want the native address of the associated PLT entry.
            // For now, assume a size of 0x10 for each PLT entry, and assume that each entry in the .rel.plt section
            // corresponds exactly to an entry in the .plt (except there is one dummy .plt entry)
            return addrPlt + plt_offset;
            return addrPlt + pltEntrySize * (curr + 1);
            //return ADDRESS::n(elfRead4(pEntry));
            //return addrPlt + 0xC * (curr + 1);
        }
        if (--curr < 0)
            curr = numRelPlt - 1;
    } while (curr != first); // Will eventually wrap around to first if not present
    return ADDRESS::g(0L);   // Exit if this happens
}

void ElfBinaryFile::processSymbol(Translated_ElfSym &sym,int e_type, int i)
{
    static QString current_file;
    bool imported = sym.SectionIdx == SHT_NULL;
    bool local = sym.Binding==STB_LOCAL||sym.Binding==STB_WEAK;
    const IBinarySection * siPlt = Image->GetSectionInfoByName(".plt");
    if (sym.Value.isZero() && siPlt) { //&& i < max_i_for_hack) {
        // Special hack for gcc circa 3.3.3: (e.g. test/pentium/settest).  The value in the dynamic symbol table
        // is zero!  I was assuming that index i in the dynamic symbol table would always correspond to index i
        // in the .plt section, but for fedora2_true, this doesn't work. So we have to look in the .rel[a].plt
        // section. Thanks, gcc!  Note that this hack can cause strange symbol names to appear
        sym.Value = findRelPltOffset(i);
    } else if (e_type == E_REL) {
        if (sym.SectionIdx < ElfSections.size())
            sym.Value += ElfSections[sym.SectionIdx].SourceAddr;
    }
    // try to find given symbol, if it has Value of 0, try to use the name.
    const IBinarySymbol *symbol = sym.Value.isZero() ? Symbols->find(sym.Name) : Symbols->find(sym.Value);
    // Ensure no overwriting (except functions)
    if(symbol!=nullptr) //TODO: if symbol already exists
        return;
    if(sym.Binding==STB_WEAK &&  sym.Type==STT_NOTYPE) {
        return;
    }
    if(sym.Type==STT_FILE) {
        current_file = sym.Name;
        return;
    }
    if(sym.Binding!=STB_LOCAL && !current_file.isEmpty()) {
        // first non-local symbol, clear the current_file
        current_file.clear();
    }
    if(sym.Name.isEmpty()) {
        return;
    }
    if(sym.Value.isZero()) {
        qDebug() << "Skipping symbol "<<sym.Name<<"with unknown location!";
        return;
    }
    // TODO: add more symbol information here (function/export etc. ) ?
    IBinarySymbol &new_symbol(Symbols->create(sym.Value,sym.Name,local));
    new_symbol.setSize(elfRead4(&m_pSym[i].st_size));
    if(imported)
        new_symbol.setAttr("Imported",true);
    if(sym.Type==STT_FUNC)
        new_symbol.setAttr("Function",true);
    if(!current_file.isEmpty())
        new_symbol.setAttr("SourceFile",current_file);
}

// Add appropriate symbols to the symbol table.  secIndex is the section index of the symbol table.
void ElfBinaryFile::AddSyms(int secIndex) {
    int e_type = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_type);
    const SectionParam &pSect(ElfSections[secIndex]);
    // Calc number of symbols
    int nSyms = pSect.Size / pSect.entry_size;
    m_pSym = (const Elf32_Sym *)pSect.image_ptr.m_value; // Pointer to symbols
    int strIdx = m_sh_link[secIndex];               // sh_link points to the string table

    // Index 0 is a dummy entry
    for (int i = 1; i < nSyms; i++) {
        Translated_ElfSym trans;
        ADDRESS val = ADDRESS::g(elfRead4((int *)&m_pSym[i].st_value));
        int name = elfRead4(&m_pSym[i].st_name);
        if (name == 0) /* Silly symbols with no names */
            continue;
        QString str(GetStrPtr(strIdx, name));
        // Hack off the "@@GLIBC_2.0" of Linux, if present
        trans.Name = str.left(str.indexOf("@@"));
        trans.Type = ELF32_ST_TYPE(m_pSym[i].st_info);
        trans.Binding = ELF32_ST_BIND(m_pSym[i].st_info);
        trans.Visibility = ELF32_ST_VISIBILITY(m_pSym[i].st_other);
        trans.SymbolSize = ELF32_ST_VISIBILITY(m_pSym[i].st_size);
        trans.SectionIdx = elfRead2(&m_pSym[i].st_shndx);
        trans.Value = val;
        processSymbol(trans,e_type, i);
    }
    ADDRESS uMain = GetMainEntryPoint();
    if (uMain != NO_ADDRESS && nullptr == Symbols->find(uMain) ) {
        // Ugh - main mustn't have the STT_FUNC attribute. Add it
        Symbols->create(uMain,"main");
    }
}

// FIXME: this function is way off the rails. It seems to always overwrite the relocation entry with the 32 bit value
// from the symbol table. Totally invalid for SPARC, and most X86 relocations!
// So currently not called
void ElfBinaryFile::AddRelocsAsSyms(uint32_t relSecIdx) {
    if(relSecIdx >= ElfSections.size() )
        return;
    const SectionParam &pSect(ElfSections[relSecIdx]);
    // Calc number of relocations
    int nRelocs = pSect.Size / pSect.entry_size;
    m_pReloc = (const Elf32_Rel *)pSect.image_ptr.m_value; // Pointer to symbols
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
                a = ElfSections[elfRead2(&m_pSym[symIndex].st_shndx)].SourceAddr;
            // Overwrite the relocation value... ?
            Image->writeNative4(val, a.m_value);
            continue;
        }
        if ((flags & R_386_PC32) == 0)
            continue;
        if (symIndex == 0) /* Silly symbols with no names */
            continue;
        QString str(GetStrPtr(strSecIdx, elfRead4(&m_pSym[symIndex].st_name)));

        str = str.left(str.indexOf("@@")); // Hack off the "@@GLIBC_2.0" of Linux, if present

        std::map<ADDRESS, QString>::iterator it;
        auto symbol = Symbols->find(str);
        // Add new extern
        ADDRESS location = symbol ? symbol->getLocation() : next_extern;
        if (nullptr == symbol) {
            Symbols->create(next_extern,str);
            next_extern += 4;
        }
        Image->writeNative4(val, (location - val - 4).m_value);
    }
    return;
}

//
// GetMainEntryPoint()
// Returns the entry point to main (this should be a label in elf binaries generated by compilers).
//
ADDRESS ElfBinaryFile::GetMainEntryPoint() {
    auto sym = Symbols->find("main");
    if(sym)
        return sym->getLocation();
    return NO_ADDRESS;
}

ADDRESS ElfBinaryFile::GetEntryPoint() { return ADDRESS::g(elfRead4(&((Elf32_Ehdr *)m_pImage)->e_entry)); }

// FIXME: the below assumes a fixed delta
ADDRESS ElfBinaryFile::NativeToHostAddress(ADDRESS uNative) {
    if (Image->GetNumSections() == 0)
        return ADDRESS::g(0L);
    return Image->GetSectionInfo(1)->hostAddr() - Image->GetSectionInfo(1)->sourceAddr() + uNative;
}

bool ElfBinaryFile::PostLoad(void *handle) {
    Q_UNUSED(handle);
    // This function is called after an archive member has been loaded by ElfArchiveFile

    // Save the elf pointer
    // m_elf = (Elf*) handle;

    // return ProcessElfFile();
    return false;
}

void ElfBinaryFile::Close() {
    UnLoad();
}

LOAD_FMT ElfBinaryFile::GetFormat() const { return LOADFMT_ELF; }

MACHINE ElfBinaryFile::getMachine() const {
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
        fprintf(stderr,"Error: ElfBinaryFile::GetMachine: The AMD x86-64 architecture is not supported yet\n");
        return (MACHINE)-1;
    }
    // An unknown machine type
    fprintf(stderr,"Error: ElfBinaryFile::GetMachine: Unsupported machine type: %d (0x%x)\n",machine,machine);
    fprintf(stderr,"(Please add a description for this type, thanks!)\n");
    return (MACHINE)-1;
}

bool ElfBinaryFile::isLibrary() const {
    int type = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_type);
    return (type == ET_DYN);
}
//! Return a list of library names which the binary file depends on
QStringList ElfBinaryFile::getDependencyList() {
    QStringList result;
    ADDRESS stringtab = NO_ADDRESS;
    IBinarySection * dynsect = Image->GetSectionInfoByName(".dynamic");
    if (dynsect == nullptr)
        return result; /* no dynamic section = statically linked */

    Elf32_Dyn *dyn;
    for (dyn = (Elf32_Dyn *)dynsect->hostAddr().m_value; dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_STRTAB) {
            stringtab = ADDRESS::g(dyn->d_un.d_ptr);
            break;
        }
    }

    if (stringtab == NO_ADDRESS) /* No string table = no names */
        return result;
    stringtab = NativeToHostAddress(stringtab);

    for (dyn = (Elf32_Dyn *)dynsect->hostAddr().m_value; dyn->d_tag != DT_NULL; dyn++) {
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
  * \brief      Mark all imported symbols as such.
  * This function relies on the fact that the symbols are sorted by address, and that Elf PLT
  * entries have successive addresses beginning soon after m_PltMin
  ******************************************************************************/
void ElfBinaryFile::markImports() {
    IBinarySymbolTable::const_iterator first=Symbols->begin();
    IBinarySymbolTable::const_iterator last=Symbols->begin();
    IBinarySymbolTable::const_iterator end=Symbols->end();
    for( ; first!=end; ++first ) {
        if((*first)->getLocation()>=m_uPltMin) {
            break;
        }
    }
    for( last=first; last!=end; ++last ) {
        if((*last)->getLocation()>=m_uPltMax) {
            break;
        }
        const IBinarySymbol *sym = *last;
        sym->setAttr("Imported",true);
    }
}

/***************************************************************************/ /**
  *
  * \brief    Read a 2 or 4 byte quantity from host address (C pointer) p
  * \note        Takes care of reading the correct endianness, set early on into m_elfEndianness
  * \param    ps or pi: host pointer to the data
  * \returns        An integer representing the data
  ******************************************************************************/
int ElfBinaryFile::elfRead2(const short *ps) const {
    const unsigned char *p = (const unsigned char *)ps;
    if (m_elfEndianness) {
        // Big endian
        return (int)((p[0] << 8) + p[1]);
    } else {
        // Little endian
        return (int)(p[0] + (p[1] << 8));
    }
}
int ElfBinaryFile::elfRead4(const int *pi) const {
    const short *p = (const short *)pi;
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
void ElfBinaryFile::applyRelocations() {
    int nextFakeLibAddr = -2; // See R_386_PC32 below; -1 sometimes used for main
    if (m_pImage == nullptr)
        return; // No file loaded
    int machine = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_machine);
    int e_type = elfRead2(&((Elf32_Ehdr *)m_pImage)->e_type);
    switch (machine) {
    case EM_SPARC: {
        for (unsigned i = 1; i < ElfSections.size(); ++i) {
            const SectionParam &ps(ElfSections[i]);
            if (ps.uType == SHT_REL) {
            }
            else if (ps.uType == SHT_RELA) {
                int *pReloc = (int *)ps.image_ptr.m_value;
                unsigned size = ps.Size;
                // NOTE: the r_offset is different for .o files (E_REL in the e_type header field) than for exe's
                // and shared objects!
                //ADDRESS destNatOrigin = ADDRESS::g(0L), destHostOrigin = ADDRESS::g(0L);
                for (unsigned u = 0; u < size; u += sizeof(Elf32_Rela)) {
                    Elf32_Rela r;
                    r.r_offset = elfRead4(pReloc++);
                    r.r_info = elfRead4(pReloc++);
                    r.r_addend = elfRead4(pReloc++);
                    unsigned char relType = (unsigned char)r.r_info;
                    //unsigned symTabIndex = r.r_info >> 8;
                    switch (relType) {
                    case 0: // R_386_NONE: just ignore (common)
                        break;
                    case R_SPARC_GLOB_DAT:
                        break;
                    }
                }

            }
        }
        qDebug() << "Unhandled relocation !";
        break; // Not implemented yet
    }
    case EM_386: {
        for (unsigned i = 1; i < ElfSections.size(); ++i) {
            const SectionParam &ps(ElfSections[i]);

            if (ps.uType == SHT_REL) {
                // A section such as .rel.dyn or .rel.plt (without an addend field).
                // Each entry has 2 words: r_offet and r_info. The r_offset is just the offset from the beginning
                // of the section (section given by the section header's sh_info) to the word to be modified.
                // r_info has the type in the bottom byte, and a symbol table index in the top 3 bytes.
                // A symbol table offset of 0 (STN_UNDEF) means use value 0. The symbol table involved comes from
                // the section header's sh_link field.
                int *pReloc = (int *)ps.image_ptr.m_value;
                unsigned size = ps.Size;
                // NOTE: the r_offset is different for .o files (E_REL in the e_type header field) than for exe's
                // and shared objects!
                ADDRESS destNatOrigin = ADDRESS::g(0L), destHostOrigin = ADDRESS::g(0L);
                if (e_type == E_REL) {
                    int destSection = m_sh_info[i];
                    destNatOrigin = ElfSections[destSection].SourceAddr;
                    destHostOrigin = ElfSections[destSection].image_ptr;
                }
                int symSection = m_sh_link[i];          // Section index for the associated symbol table
                int strSection = m_sh_link[symSection]; // Section index for the string section assoc with this
                char *pStrSection = (char *)ElfSections[strSection].image_ptr.m_value;
                const Elf32_Sym *symOrigin = (const Elf32_Sym *)ElfSections[symSection].image_ptr.m_value;
                for (unsigned u = 0; u < size; u += 2 * sizeof(unsigned)) {
                    unsigned r_offset = elfRead4(pReloc++);
                    unsigned info = elfRead4(pReloc++);
                    unsigned char relType = (unsigned char)info;
                    unsigned symTabIndex = info >> 8;
                    int *pRelWord; // Pointer to the word to be relocated
                    if (e_type == E_REL)
                        pRelWord = ((int *)(destHostOrigin + r_offset).m_value);
                    else {
                        const IBinarySection *destSec =Image->getSectionInfoByAddr(ADDRESS::n(r_offset));
                        pRelWord = (int *)(destSec->hostAddr() - destSec->sourceAddr() + r_offset).m_value;
                        destNatOrigin = ADDRESS::g(0L);
                    }
                    ADDRESS A, S = ADDRESS::g(0L), P;
                    unsigned nsec;
                    switch (relType) {
                    case 0: // R_386_NONE: just ignore (common)
                        break;
                    case 1: // R_386_32: S + A
                        S = elfRead4((int *)&symOrigin[symTabIndex].st_value);
                        if (e_type == E_REL) {
                            nsec = elfRead2(&symOrigin[symTabIndex].st_shndx);
                            if (nsec < ElfSections.size() )
                                S += ElfSections[nsec].SourceAddr;
                        }
                        A = elfRead4(pRelWord);
                        elfWrite4(pRelWord, (S + A).m_value);
                        break;
                    case 2: // R_386_PC32: S + A - P
                        if (ELF32_ST_TYPE(symOrigin[symTabIndex].st_info) == STT_SECTION) {
                            nsec = elfRead2(&symOrigin[symTabIndex].st_shndx);
                            if (nsec < ElfSections.size() )
                                S += ElfSections[nsec].SourceAddr;
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
                                Symbols->create(S,pName);
                                //}
                            } else if (e_type == E_REL) {
                                nsec = elfRead2(&symOrigin[symTabIndex].st_shndx);
                                if (nsec < ElfSections.size() )
                                    S += ElfSections[nsec].SourceAddr;
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
        qDebug() << "Unhandled relocation !";
        break; // Not implemented yet
    case EM_386: {
        for (unsigned i = 1; i < ElfSections.size(); ++i) {
            const SectionParam &ps(ElfSections[i]);
            if (ps.uType == SHT_REL) {
                // A section such as .rel.dyn or .rel.plt (without an addend field).
                // Each entry has 2 words: r_offet and r_info. The r_offset is just the offset from the beginning
                // of the section (section given by the section header's sh_info) to the word to be modified.
                // r_info has the type in the bottom byte, and a symbol table index in the top 3 bytes.
                // A symbol table offset of 0 (STN_UNDEF) means use value 0. The symbol table involved comes from
                // the section header's sh_link field.
                int *pReloc = (int *)ps.image_ptr.m_value;
                unsigned size = ps.Size;
                // NOTE: the r_offset is different for .o files (E_REL in the e_type header field) than for exe's
                // and shared objects!
                ADDRESS destNatOrigin = ADDRESS::g(0L), destHostOrigin;
                if (e_type == E_REL) {
                    int destSection = m_sh_info[i];
                    destNatOrigin = ElfSections[destSection].SourceAddr;
                    destHostOrigin = ElfSections[destSection].image_ptr;
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
                        const IBinarySection *destSec = Image->getSectionInfoByAddr(ADDRESS::g(r_offset));
                        pRelWord = destSec->sourceAddr() + r_offset;
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
