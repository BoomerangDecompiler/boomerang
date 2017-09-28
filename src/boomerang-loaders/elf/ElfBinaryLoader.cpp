#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ElfBinaryLoader.h"


/***************************************************************************/ /**
 * \file ElfBinaryLoader.cpp
 * Desc: This file contains the implementation of the class ElfBinaryLoader.
 ******************************************************************************/

#include "ElfTypes.h"

#include "boomerang/core/IBoomerang.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySymbols.h"
#include "boomerang/db/IBinarySection.h"
#include "boomerang/util/Log.h"

#include <sys/types.h> // Next three for open()
#include <sys/stat.h>
#include <fcntl.h>
#include <cstddef>
#include <cassert>
#include <cstring>
#include <inttypes.h>
#include <QBuffer>
#include <QFile>


struct SectionParam
{
    QString  Name;
    Address  SourceAddr;
    size_t   Size;
    size_t   entry_size;
    bool     ReadOnly;
    bool     Bss;
    bool     Code;
    bool     Data;
    HostAddress  imagePtr;
    unsigned sectionType;  ///< Type of section (format dependent)
};

// not part of anonymous namespace, since it would create an ambiguity
// anonymous_namespace::Translated_ElfSym vs. ElfTypes.h/Translated_ElfSym declarations
struct Translated_ElfSym
{
    QString          Name;
    ElfSymType       Type;
    ElfSymBinding    Binding;
    ElfSymVisibility Visibility;
    uint32_t         SymbolSize;
    uint16_t         SectionIdx;
    Address          Value;
};

typedef std::map<QString, int, std::less<QString> > StrIntMap;


ElfBinaryLoader::ElfBinaryLoader()
    : m_nextExtern(Address::ZERO)
{
    init(); // Initialise all the common stuff
}


ElfBinaryLoader::~ElfBinaryLoader()
{
    // Delete the array of import stubs
    delete  []m_importStubs;
    delete  []m_shLink;
    delete  []m_shInfo;
}


void ElfBinaryLoader::initialize(IBinaryImage *image, IBinarySymbolTable *symbols)
{
    m_binaryImage = image;
    m_symbols     = symbols;
}


void ElfBinaryLoader::init()
{
    m_loadedImage   = nullptr;
    m_elfHeader     = nullptr;  // No ELF header
    m_programHdrs   = nullptr;  // No program headers
    m_sectionhdrs   = nullptr;  // No section headers
    m_strings       = nullptr;  // No strings
    m_relocSection  = nullptr;
    m_symbolSection = nullptr;
    m_pltMin        = Address::ZERO;  // No PLT limits
    m_pltMax        = Address::ZERO;
    m_lastSize      = 0;
    m_importStubs   = nullptr;
    m_elfSections.clear();
}


// Hand decompiled from sparc library function
extern "C" { // So we can call this with dlopen() in test function
Q_DECL_EXPORT
unsigned elf_hash(const char *o0)
{
    int        o3  = *o0;
    const char *g1 = o0;
    unsigned   o4  = 0;

    while (o3 != 0) {
        o4 <<= 4;
        o3  += o4;
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


bool ElfBinaryLoader::loadFromMemory(QByteArray& img)
{
    m_loadedImageSize = img.size();

    // Allocate memory to hold the file
    m_loadedImage = (Byte *)img.data();

    m_elfHeader = (Elf32_Ehdr *)img.data(); // Save a lot of casts

    // Basic checks
    if ((m_elfHeader->e_ident[0] != 0x7F) ||
        (m_elfHeader->e_ident[1] != 'E') ||
        (m_elfHeader->e_ident[2] != 'L') ||
        (m_elfHeader->e_ident[3] != 'F')) {
        LOG_ERROR("Cannot load ELF file: Bad magic %1 %2 %3 %4",
                  m_elfHeader->e_ident[0],
                  m_elfHeader->e_ident[1],
                  m_elfHeader->e_ident[2],
                  m_elfHeader->e_ident[3]);
        return false;
    }

    switch (m_elfHeader->endianness)
    {
    case 1: // little endian
        m_bigEndian = false;
        break;

    case 2: // big endian
        m_bigEndian = true;
        break;

    default:
        LOG_WARN("Unknown ELF Endianness %1, file may be corrupted.", m_elfHeader->endianness);
        return false;
    }

    // Set up program header pointer (in case needed)
    DWord phOffset = elfRead4(&m_elfHeader->e_phoff);

    if (phOffset > 0) {
        m_programHdrs = (Elf32_Phdr *)(m_loadedImage + phOffset);
    }

    // Set up section header pointer
    DWord shOffset = elfRead4(&m_elfHeader->e_shoff);

    if (shOffset > 0) {
        m_sectionhdrs = (Elf32_Shdr *)(m_loadedImage + shOffset);
    }

    // Set up section header string table pointer
    // NOTE: it does not appear that endianness affects shorts.. they are always in little endian format
    // Gerard: I disagree. I need the elfRead on linux/i386
    const DWord stringSectionIndex = elfRead2(&m_elfHeader->e_shstrndx);

    if (stringSectionIndex > 0) {
        m_strings = (const char *)(m_loadedImage + elfRead4(&m_sectionhdrs[stringSectionIndex].sh_offset));
    }

    // Number of sections
    const SWord numSections = elfRead2(&m_elfHeader->e_shnum);
    // Set up the m_sh_link and m_sh_info arrays
    m_shLink = new int[numSections];
    m_shInfo = new int[numSections];

    // Number of elf sections
    bool    seenCode         = false; // True when have seen a code sect
    Address arbitaryLoadAddr = Address(0x08000000);

    for (SWord i = 0; i < numSections; i++) {
        // Get section information.
        const Elf32_Shdr *sectionHeader = m_sectionhdrs + i;

        if ((Byte *)sectionHeader > m_loadedImage + m_loadedImageSize) {
            LOG_ERROR("Section %1 header is outside the image size", i);
            return false;
        }

        const char *sectionName = m_strings + elfRead4(&sectionHeader->sh_name);

        if ((Byte *)sectionName > m_loadedImage + m_loadedImageSize) {
            LOG_ERROR("Name for section %1 is outside the image size", i);
            return false;
        }

        SectionParam sect;
        sect.Name = sectionName;
        // Can't use the SHF_ALLOC bit to determine bss section; the bss section has SHF_ALLOC but also SHT_NOBITS.
        // (But many other sections, such as .comment, also have SHT_NOBITS). So for now, just use the name
        //      if ((elfRead4(&pShdr->sh_flags) & SHF_ALLOC) == 0)
        sect.Bss      = (strcmp(sectionName, ".bss") == 0);
        sect.Code     = false;
        sect.Data     = false;
        sect.ReadOnly = false;
        int _off = elfRead4(&sectionHeader->sh_offset);

        if (_off) {
            sect.imagePtr = HostAddress(m_loadedImage) + _off;
        }

        sect.SourceAddr = Address(elfRead4(&sectionHeader->sh_addr));
        sect.Size       = elfRead4(&sectionHeader->sh_size);

        if (sect.SourceAddr.isZero() && strncmp(sectionName, ".rel", 4)) {
            const DWord align = elfRead4(&sectionHeader->sh_addralign);

            if (align > 1) {
                if (arbitaryLoadAddr.value() % align != 0) {
                    arbitaryLoadAddr += align - (arbitaryLoadAddr.value() % align);
                }
            }

            sect.SourceAddr   = arbitaryLoadAddr;
            arbitaryLoadAddr += sect.Size ? sect.Size : 1;
        }

        sect.sectionType      = elfRead4(&sectionHeader->sh_type);
        m_shLink[i]     = elfRead4(&sectionHeader->sh_link);
        m_shInfo[i]     = elfRead4(&sectionHeader->sh_info);
        sect.entry_size = elfRead4(&sectionHeader->sh_entsize);

        if (sect.SourceAddr + sect.Size > m_nextExtern) {
            m_firstExtern = m_nextExtern = sect.SourceAddr + sect.Size;
        }

        if ((elfRead4(&sectionHeader->sh_flags) & SHF_WRITE) == 0) {
            sect.ReadOnly = true;
        }

        if (elfRead4(&sectionHeader->sh_flags) & SHF_EXECINSTR) {
            sect.Code = true;
            seenCode  = true; // We've got to a code section
        }

        // Deciding what is data and what is not is actually quite tricky but important.
        // For example, it's crucial to flag the .exception_ranges section as data,
        // otherwise there is a "hole" in the allocation map, that means
        // that there is more than one "delta" from a read-only section to a page,
        // and in the end using -C results in a file that looks OK
        // but when run just says "Killed". So we use the Elf designations;
        // it seems that ALLOC.!EXEC -> data
        // But we don't want sections before the .text section, like .interp, .hash, etc etc.
        // Hence seenCode.
        //
        // NOTE: this ASSUMES that sections appear in a sensible order in the input binary file:
        // junk, code, rodata, data, bss
        if (seenCode &&
            ((elfRead4(&sectionHeader->sh_flags) & (SHF_EXECINSTR | SHF_ALLOC)) == SHF_ALLOC) &&
            (elfRead4(&sectionHeader->sh_type) != SHT_NOBITS)) {
            sect.Data = true;
        }

        m_elfSections.push_back(sect);
    } // for each section

    // assign arbitary addresses to .rel.* sections too
    for (SectionParam& sect : m_elfSections) {
        if (sect.SourceAddr.isZero() && sect.Name.startsWith(".rel")) {
            sect.SourceAddr   = arbitaryLoadAddr;
            arbitaryLoadAddr += sect.Size ? sect.Size : 1;
        }
    }

    // Inform Boomerang about new sections
    for (SectionParam par : m_elfSections) {
        if (par.Size == 0) {
            // this is most probably the NULL section; if it is not, warn the user
            if (par.Name != "") {
                LOG_WARN("Not adding 0 sized section %1", par.Name);
            }
            continue;
        }

        IBinarySection *sect = m_binaryImage->createSection(par.Name, par.SourceAddr, par.SourceAddr + par.Size);
        assert(sect);

        if (sect) {
            sect->setBss(par.Bss)
               .setCode(par.Code)
               .setData(par.Data)
               .setEndian(m_bigEndian)
               .setHostAddr(par.imagePtr)
               .setEntrySize(par.entry_size);

            if (!(par.Bss || par.SourceAddr.isZero())) {
                sect->addDefinedArea(par.SourceAddr, par.SourceAddr + par.Size);
            }
        }
    }

    // Add symbol info. Note that some symbols will be in the main table only, and others in the dynamic table only.
    // So the best idea is to add symbols for all sections of the appropriate type
    for (unsigned i = 1; i < m_elfSections.size(); ++i) {
        const unsigned int sectionType = m_elfSections[i].sectionType;

        if ((sectionType == SHT_SYMTAB) || (sectionType == SHT_DYNSYM)) {
            addSymbolsForSection(i);
        }
    }

    // Save the relocation to symbol table info
    IBinarySection *section = m_binaryImage->getSectionByName(".rela.text");

    if (section) {
        m_relocHasAddend = true;                                     // Remember its a relA table
        m_relocSection   = (Elf32_Rel *)section->getHostAddr().value(); // Save pointer to reloc table
    }
    else {
        m_relocHasAddend = false;
        section             = m_binaryImage->getSectionByName(".rel.text");

        if (section) {
            m_relocSection = (Elf32_Rel *)section->getHostAddr().value();          // Save pointer to reloc table
        }
    }

    // Find the PLT limits. Required for IsDynamicLinkedProc(), e.g.
    IBinarySection *pltSection = m_binaryImage->getSectionByName(".plt");

    if (pltSection) {
        m_pltMin = pltSection->getSourceAddr();
        m_pltMax = pltSection->getSourceAddr() + pltSection->getSize();
    }

    // Apply relocations; important when the input program is not compiled with -fPIC
    applyRelocations();
    markImports();
    return true; // Success
}


void ElfBinaryLoader::unload()
{
    init(); // Set all internal state to 0
}


const char *ElfBinaryLoader::getStrPtr(int sectionIdx, int offset)
{
    if (sectionIdx < 0) {
        // Most commonly, this will be an index of -1, because a call to GetSectionIndexByName() failed
        LOG_ERROR("Invalid index %1", sectionIdx);
        return nullptr;
    }

    // Get a pointer to the start of the string table
    const char *stringSym = (const char *)m_elfSections[sectionIdx].imagePtr.value();
    // Just add the offset
    return stringSym + offset;
}


Address ElfBinaryLoader::findRelPltOffset(int i)
{
    const IBinarySection *siPlt    = m_binaryImage->getSectionByName(".plt");
    Address              addrPlt   = siPlt ? siPlt->getSourceAddr() : Address::ZERO;
    const IBinarySection *siRelPlt = m_binaryImage->getSectionByName(".rel.plt");
    int sizeRelPlt = 8; // Size of each entry in the .rel.plt table

    if (siRelPlt == nullptr) {
        siRelPlt   = m_binaryImage->getSectionByName(".rela.plt");
        sizeRelPlt = 12; // Size of each entry in the .rela.plt table is 12 bytes
    }

    HostAddress addrRelPlt = HostAddress::ZERO;
    int     numRelPlt  = 0;

    if (siRelPlt) {
        addrRelPlt = siRelPlt->getHostAddr();
        numRelPlt  = sizeRelPlt ? siRelPlt->getSize() / sizeRelPlt : 0;
    }
    else {
        return Address::INVALID; // neither .rel.plt nor .rela.plt are available
    }

    int first = i;

    if (first >= numRelPlt) {
        first = numRelPlt - 1;
    }

    int curr         = first;
    int pltEntrySize = siPlt->getEntrySize();

    if (pltEntrySize == 0) {
        return Address::INVALID;
    }

    do {
        // Each entry is sizeRelPlt bytes, and will contain the offset, then the info (addend optionally follows)
        DWord *pltEntry = (DWord *)(addrRelPlt + (curr * sizeRelPlt)).value();
        const int   entry     = elfRead4(pltEntry + 1);
        const int   sym       = entry >> 8;         // The symbol index is in the top 24 bits (Elf32 only)
        const int   entryType = entry & 0xFF;

        if (sym == i) {
            const IBinarySection *targetSect = m_binaryImage->getSectionByAddr(Address(elfRead4(pltEntry)));

            if (targetSect->getName().contains("got")) {
                int c           = elfRead4(pltEntry) - targetSect->getSourceAddr().value();
                int plt_offset2 = elfRead4((DWord *)(targetSect->getHostAddr() + c).value());
                int plt_idx     = (plt_offset2 % pltEntrySize);

                if (entryType == R_386_JUMP_SLOT) {
                    return Address(plt_offset2 - 6);
                }

                return addrPlt + plt_idx * pltEntrySize;
            }

            const int plt_offset = elfRead4(pltEntry) - siPlt->getSourceAddr().value();
            // Found! Now we want the native address of the associated PLT entry.
            // For now, assume a size of 0x10 for each PLT entry, and assume that each entry in the .rel.plt section
            // corresponds exactly to an entry in the .plt (except there is one dummy .plt entry)
            return addrPlt + plt_offset;
        }

        if (--curr < 0) {
            curr = numRelPlt - 1;
        }
    } while (curr != first); // Will eventually wrap around to first if not present

    return Address::ZERO;   // Exit if this happens
}


void ElfBinaryLoader::processSymbol(Translated_ElfSym& sym, int e_type, int i)
{
    static QString       currentFile;
    bool                 imported = sym.SectionIdx == SHT_NULL;
    bool                 local    = sym.Binding == STB_LOCAL || sym.Binding == STB_WEAK;
    const IBinarySection *siPlt   = m_binaryImage->getSectionByName(".plt");

    if (sym.Value.isZero() && siPlt) { // && i < max_i_for_hack) {
        // Special hack for gcc circa 3.3.3: (e.g. test/pentium/settest).  The value in the dynamic symbol table
        // is zero!  I was assuming that index i in the dynamic symbol table would always correspond to index i
        // in the .plt section, but for fedora2_true, this doesn't work. So we have to look in the .rel[a].plt
        // section. Thanks, gcc!  Note that this hack can cause strange symbol names to appear
        sym.Value = findRelPltOffset(i);
    }
    else if (e_type == E_REL) {
        if (sym.SectionIdx < m_elfSections.size()) {
            sym.Value += m_elfSections[sym.SectionIdx].SourceAddr;
        }
    }

    // try to find given symbol, if it has Value of 0, try to use the name.
    const IBinarySymbol *symbol = sym.Value.isZero() ? m_symbols->find(sym.Name) : m_symbols->find(sym.Value);

    // Ensure no overwriting (except functions)
    if (symbol != nullptr) { // TODO: if symbol already exists
        return;
    }

    if ((sym.Binding == STB_WEAK) && (sym.Type == STT_NOTYPE)) {
        return;
    }

    if (sym.Type == STT_FILE) {
        currentFile = sym.Name;
        return;
    }

    if ((sym.Binding != STB_LOCAL) && !currentFile.isEmpty()) {
        // first non-local symbol, clear the current_file
        currentFile.clear();
    }

    if (sym.Name.isEmpty()) {
        return;
    }

    if (sym.Value.isZero()) {
        LOG_WARN("Skipping symbol %1 with unknown location", sym.Name);
        return;
    }

    // TODO: add more symbol information here (function/export etc. ) ?
    IBinarySymbol& new_symbol(m_symbols->create(sym.Value, sym.Name, local));
    new_symbol.setSize(elfRead4(&m_symbolSection[i].st_size));

    if (imported) {
        new_symbol.setAttr("Imported", true);
    }

    if (sym.Type == STT_FUNC) {
        new_symbol.setAttr("Function", true);
    }

    if (!currentFile.isEmpty()) {
        new_symbol.setAttr("SourceFile", currentFile);
    }
}


void ElfBinaryLoader::addSymbolsForSection(int secIndex)
{
    const SWord symbolType       = elfRead2(&m_elfHeader->e_type);
    const SectionParam& section  = m_elfSections[secIndex];
    const int strSectionIdx      = m_shLink[secIndex]; // sh_link points to the string table
    const int numSymbols         = section.Size / section.entry_size;

    m_symbolSection = (const Elf32_Sym *)section.imagePtr.value(); // Pointer to symbols

    // Index 0 is a dummy entry
    for (int i = 1; i < numSymbols; i++) {
        Translated_ElfSym translatedSym;
        Address val = Address(elfRead4(&m_symbolSection[i].st_value));
        int nameIdx = elfRead4(&m_symbolSection[i].st_name);

        if (nameIdx == 0) { /* Silly symbols with no names */
            continue;
        }

        const QString symbolName = getStrPtr(strSectionIdx, nameIdx);
        // Hack off the "@@GLIBC_2.0" of Linux, if present
        translatedSym.Name       = symbolName.left(symbolName.indexOf("@@"));
        translatedSym.Type       = ELF32_ST_TYPE(m_symbolSection[i].st_info);
        translatedSym.Binding    = ELF32_ST_BIND(m_symbolSection[i].st_info);
        translatedSym.Visibility = ELF32_ST_VISIBILITY(m_symbolSection[i].st_other);
        translatedSym.SymbolSize = ELF32_ST_VISIBILITY(m_symbolSection[i].st_size);
        translatedSym.SectionIdx = elfRead2(&m_symbolSection[i].st_shndx);
        translatedSym.Value      = val;
        processSymbol(translatedSym, symbolType, i);
    }

    const Address addressOfMain = getMainEntryPoint();

    if ((addressOfMain != Address::INVALID) && (nullptr == m_symbols->find(addressOfMain))) {
        // Ugh - main mustn't have the STT_FUNC attribute. Add it
        m_symbols->create(addressOfMain, "main");
    }
}


void ElfBinaryLoader::addRelocsAsSyms(uint32_t relSecIdx)
{
    if (relSecIdx >= m_elfSections.size()) {
        // invalid section index
        return;
    }

    const SectionParam& section = m_elfSections[relSecIdx];

    m_relocSection = (const Elf32_Rel *)section.imagePtr.value();    // Pointer to symbols
    const int symSecIdx = m_shLink[relSecIdx];
    const int strSecIdx = m_shLink[symSecIdx];
    const int numRelocs = section.Size / section.entry_size;

    // Index 0 is a dummy entry
    for (int i = 1; i < numRelocs; i++) {
        const Address val      = Address(elfRead4(&m_relocSection[i].r_offset));
        const int     symIndex = elfRead4(&m_relocSection[i].r_info) >> 8;
        const int     flags    = elfRead4(&m_relocSection[i].r_info) & 0xFF;

        if (flags == R_386_32) {
            // Lookup the value of the symbol table entry
            Address a = Address(elfRead4(&m_symbolSection[symIndex].st_value));

            if (m_symbolSection[symIndex].st_info & STT_SECTION) {
                a = m_elfSections[elfRead2(&m_symbolSection[symIndex].st_shndx)].SourceAddr;
            }

            // Overwrite the relocation value... ?
            m_binaryImage->writeNative4(val, a.value());
            continue;
        }

        if ((flags & R_386_PC32) == 0) {
            continue;
        }

        if (symIndex == 0) { /* Silly symbols with no names */
            continue;
        }

        QString symbolName = getStrPtr(strSecIdx, elfRead4(&m_symbolSection[symIndex].st_name));

        symbolName = symbolName.left(symbolName.indexOf("@@")); // Hack off the "@@GLIBC_2.0" of Linux, if present

        std::map<Address, QString>::iterator it;
        auto symbol = m_symbols->find(symbolName);
        // Add new extern
              Address location = symbol ? symbol->getLocation() : m_nextExtern;

        if (nullptr == symbol) {
            m_symbols->create(m_nextExtern, symbolName);
            m_nextExtern += 4;
        }

        m_binaryImage->writeNative4(val, (location - val - 4).value());
    }
}


Address ElfBinaryLoader::getMainEntryPoint()
{
    auto sym = m_symbols->find("main");

    if (sym) {
        return sym->getLocation();
    }

    return Address::INVALID;
}


Address ElfBinaryLoader::getEntryPoint()
{
    return Address(elfRead4(&m_elfHeader->e_entry));
}


HostAddress ElfBinaryLoader::nativeToHostAddress(Address addr)
{
    if (m_binaryImage->getNumSections() == 0) {
        return HostAddress::ZERO;
    }

    return m_binaryImage->getSection(1)->getHostAddr() - m_binaryImage->getSection(1)->getSourceAddr() + addr;
}


void ElfBinaryLoader::close()
{
    unload();
}


LoadFmt ElfBinaryLoader::getFormat() const
{
    return LoadFmt::ELF;
}


Machine ElfBinaryLoader::getMachine() const
{
    const SWord elfMachine = elfRead2(&m_elfHeader->e_machine);

    if ((elfMachine == EM_SPARC) || (elfMachine == EM_SPARC32PLUS)) {
        return Machine::SPARC;
    }
    else if (elfMachine == EM_386) {
        return Machine::PENTIUM;
    }
    else if (elfMachine == EM_PA_RISC) {
        return Machine::HPRISC;
    }
    else if (elfMachine == EM_68K) {
        return Machine::PALM; // Unlikely
    }
    else if (elfMachine == EM_PPC) {
        return Machine::PPC;
    }
    else if (elfMachine == EM_ST20) {
        return Machine::ST20;
    }
    else if (elfMachine == EM_MIPS) {
        return Machine::MIPS;
    }
    else if (elfMachine == EM_X86_64) {
        LOG_ERROR("The AMD x86-64 architecture is not supported yet.");
        return (Machine) - 1;
    }

    // An unknown machine type
    LOG_ERROR("Unsupported machine type %1", elfMachine);
    return (Machine) - 1;
}


bool ElfBinaryLoader::isLibrary() const
{
    const SWord type = elfRead2(&((Elf32_Ehdr *)m_loadedImage)->e_type);

    return (type == ET_DYN);
}


QStringList ElfBinaryLoader::getDependencyList()
{
    QStringList    result;
    Address        stringtab = Address::INVALID;
    IBinarySection *dynsect  = m_binaryImage->getSectionByName(".dynamic");

    if (dynsect == nullptr) {
        return result; /* no dynamic section = statically linked */
    }

    for (Elf32_Dyn* dyn = (Elf32_Dyn *)dynsect->getHostAddr().value(); dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_STRTAB) {
            stringtab = Address(dyn->d_un.d_ptr);
            break;
        }
    }

    if (stringtab == Address::INVALID) { /* No string table = no names */
        return result;
    }

    HostAddress strTab = nativeToHostAddress(stringtab);

    for (Elf32_Dyn* dyn = (Elf32_Dyn *)dynsect->getHostAddr().value(); dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_NEEDED) {
            const char *need = (char *)(strTab + dyn->d_un.d_val).value();

            if (need != nullptr) {
                result << need;
            }
        }
    }

    return result;
}


void ElfBinaryLoader::markImports()
{
    IBinarySymbolTable::const_iterator first = m_symbols->begin();
    IBinarySymbolTable::const_iterator last  = m_symbols->begin();
    IBinarySymbolTable::const_iterator end   = m_symbols->end();

    for ( ; first != end; ++first) {
        if ((*first)->getLocation() >= m_pltMin) {
            break;
        }
    }

    for (last = first; last != end; ++last) {
        if ((*last)->getLocation() >= m_pltMax) {
            break;
        }

        const IBinarySymbol *sym = *last;
        sym->setAttr("Imported", true);
    }
}


SWord ElfBinaryLoader::elfRead2(const SWord *ps) const
{
    assert(ps);
    return Util::readWord(ps, m_bigEndian);
}


DWord ElfBinaryLoader::elfRead4(const DWord *pi) const
{
    assert(pi);
    return Util::readDWord(pi, m_bigEndian);
}


void ElfBinaryLoader::elfWrite4(DWord *pi, DWord val)
{
    assert(pi);
    Util::writeDWord(pi, val, m_bigEndian);
}


void ElfBinaryLoader::applyRelocations()
{
    int nextFakeLibAddr = -2; // See R_386_PC32 below; -1 sometimes used for main

    if (m_loadedImage == nullptr) {
        return; // No file loaded
    }

    const SWord machine = elfRead2(&m_elfHeader->e_machine);
    const SWord e_type  = elfRead2(&m_elfHeader->e_type);

    switch (machine)
    {
    case EM_SPARC:
        for (size_t i = 1; i < m_elfSections.size(); ++i) {
            const SectionParam& ps(m_elfSections[i]);

            if ((ps.sectionType != SHT_REL) && (ps.sectionType == SHT_RELA)) {
                DWord *relocEntry = (DWord *)ps.imagePtr.value();
                const DWord sizeInBytes    = ps.Size;

                // NOTE: the r_offset is different for .o files (E_REL in the e_type header field) than for exe's
                // and shared objects!
                // ADDRESS destNatOrigin = Address::ZERO, destHostOrigin = Address::ZERO;
                for (unsigned u = 0; u < sizeInBytes; u += sizeof(Elf32_Rela)) {
                    Elf32_Rela r;
                    r.r_offset = elfRead4(relocEntry++);
                    r.r_info   = elfRead4(relocEntry++);
                    r.r_addend = elfRead4(relocEntry++);

                    unsigned char relType = r.r_info & 0xFF;
                    // DWord symTabIndex     = r.r_info >> 8;

                    switch (relType)
                    {
                    case R_SPARC_NONE: // just ignore (common)
                        break;

                        // TODO These relocation types need to be implemented.
                    case R_SPARC_HI22:
                    case R_SPARC_LO10:
                    case R_SPARC_COPY:
                    case R_SPARC_GLOB_DAT:
                    case R_SPARC_JMP_SLOT:
                    default:
                        LOG_WARN("Unhandled SPARC relocation type %1", relType);
                        break;
                    }
                }
            }
        }

        LOG_WARN("Unhandled relocation!");
        break; // Not implemented yet

    case EM_386:

        for (size_t i = 1; i < m_elfSections.size(); ++i) {
            const SectionParam& ps(m_elfSections[i]);

            if (ps.sectionType == SHT_REL) {
                // A section such as .rel.dyn or .rel.plt (without an addend field).
                // Each entry has 2 words: r_offet and r_info. The r_offset is just the offset from the beginning
                // of the section (section given by the section header's sh_info) to the word to be modified.
                // r_info has the type in the bottom byte, and a symbol table index in the top 3 bytes.
                // A symbol table offset of 0 (STN_UNDEF) means use value 0. The symbol table involved comes from
                // the section header's sh_link field.
                DWord *relocEntry = (DWord *)ps.imagePtr.value();
                const DWord sizeInBytes = ps.Size;

                // NOTE: the r_offset is different for .o files (E_REL in the e_type header field) than for exe's
                // and shared objects!
                Address destNatOrigin = Address::ZERO;
                HostAddress destHostOrigin = HostAddress::ZERO;

                if (e_type == E_REL) {
                    int destSection = m_shInfo[i];
                    destNatOrigin  = m_elfSections[destSection].SourceAddr;
                    destHostOrigin = m_elfSections[destSection].imagePtr;
                }

                const int        symSection    = m_shLink[i];          // Section index for the associated symbol table
                const int        strSectionIdx = m_shLink[symSection]; // Section index for the string section assoc with this
                const char      *strSection    = (const char *)m_elfSections[strSectionIdx].imagePtr.value();
                const Elf32_Sym *symOrigin     = (const Elf32_Sym *)m_elfSections[symSection].imagePtr.value();

                for (unsigned u = 0; u < sizeInBytes; u += 2 * sizeof(unsigned)) {
                    const DWord r_offset    = elfRead4(relocEntry++);
                    const DWord info        = elfRead4(relocEntry++);
                    const Byte  relType     = (Byte)(info & 0xFF);
                    const DWord symTabIndex = info >> 8;
                    DWord *relocDestination; // Pointer to the word to be relocated

                    if (e_type == E_REL) {
                        relocDestination = ((DWord *)(destHostOrigin + r_offset).value());
                    }
                    else {
                        const IBinarySection *destSec = m_binaryImage->getSectionByAddr(Address(r_offset));
                        relocDestination  = (DWord *)(destSec->getHostAddr() - destSec->getSourceAddr() + r_offset).value();
                        destNatOrigin     = Address::ZERO;
                    }

                    Address  A, S = Address::ZERO, P;

                    switch (relType)
                    {
                    case R_386_NONE:
                        break;

                    case R_386_32: // S + A
                        S = Address(elfRead4(&symOrigin[symTabIndex].st_value));

                        if (e_type == E_REL) {
                            const SWord sectionIdx = elfRead2(&symOrigin[symTabIndex].st_shndx);

                            if (sectionIdx < m_elfSections.size()) {
                                S += m_elfSections[sectionIdx].SourceAddr;
                            }
                        }

                        A = Address(elfRead4(relocDestination));
                        elfWrite4(relocDestination, (S + A).value());
                        break;

                    case R_386_PC32: // S + A - P

                        if (ELF32_ST_TYPE(symOrigin[symTabIndex].st_info) == STT_SECTION) {
                            const SWord sectionIdx = elfRead2(&symOrigin[symTabIndex].st_shndx);

                            if (sectionIdx < m_elfSections.size()) {
                                S += m_elfSections[sectionIdx].SourceAddr;
                            }
                        }
                        else {
                            S = Address(elfRead4(&symOrigin[symTabIndex].st_value));

                            if (S.isZero()) {
                                // This means that the symbol doesn't exist in this module, and is not accessed
                                // through the PLT, i.e. it will be statically linked, e.g. strcmp. We have the
                                // name of the symbol right here in the symbol table entry, but the only way
                                // to communicate with the loader is through the target address of the call.
                                // So we use some very improbable addresses (e.g. -1, -2, etc) and give them entries
                                // in the symbol table
                                DWord nameOffset  = elfRead4((DWord *)&symOrigin[symTabIndex].st_name);
                                const char *symbolName = strSection + nameOffset;

                                // this is too slow, I'm just going to assume it is 0
                                // S = GetAddressByName(pName);
                                // if (S == (e_type == E_REL ? 0x8000000 : 0)) {
                                S = Address(((int)nextFakeLibAddr--) & Address::getSourceMask()); // Allocate a new fake address
                                IBinarySymbol& newFunction = m_symbols->create(S, symbolName);
                                newFunction.setAttr("Function", true);
                                newFunction.setAttr("Imported", true);
                                // }
                            }
                            else if (e_type == E_REL) {
                                const SWord sectionIdx = elfRead2(&symOrigin[symTabIndex].st_shndx);

                                if (sectionIdx < m_elfSections.size()) {
                                    S += m_elfSections[sectionIdx].SourceAddr;
                                }
                            }
                        }

                        A = Address(elfRead4(relocDestination));
                        P = destNatOrigin + r_offset;
                        elfWrite4(relocDestination, (S + A - P).value());
                        break;

                    case R_386_GLOB_DAT:
                    case R_386_JMP_SLOT:
                    case R_386_RELATIVE:
                        // No need to do anything with these, if a shared object
                        break;

                    default:
                        LOG_WARN("Unknown x86 relocation type %1", (int)relType);
                    }
                }
            }
        }
        break;

    default:
        break; // Not implemented
    }
}


bool ElfBinaryLoader::isRelocationAt(Address addr)
{
    if (m_loadedImage == nullptr) {
        return false; // No file loaded
    }

    const SWord machine = elfRead2(&m_elfHeader->e_machine);
    const SWord e_type  = elfRead2(&m_elfHeader->e_type);

    switch (machine)
    {
    case EM_386:

        for (size_t i = 1; i < m_elfSections.size(); ++i) {
            const SectionParam& ps(m_elfSections[i]);

            if (ps.sectionType == SHT_REL) {
                // A section such as .rel.dyn or .rel.plt (without an addend field).
                // Each entry has 2 words: r_offet and r_info. The r_offset is just the offset from the beginning
                // of the section (section given by the section header's sh_info) to the word to be modified.
                // r_info has the type in the bottom byte, and a symbol table index in the top 3 bytes.
                // A symbol table offset of 0 (STN_UNDEF) means use value 0. The symbol table involved comes from
                // the section header's sh_link field.
                DWord *relocEntry = (DWord *)ps.imagePtr.value();
                DWord size    = ps.Size;

                // NOTE: the r_offset is different for .o files (E_REL in the e_type header field) than for exe's
                // and shared objects!
                Address destNatOrigin = Address::ZERO;
                HostAddress destHostOrigin = HostAddress::ZERO;

                if (e_type == E_REL) {
                    int destSection = m_shInfo[i];
                    destNatOrigin  = m_elfSections[destSection].SourceAddr;
                    destHostOrigin = m_elfSections[destSection].imagePtr;
                }

                for (DWord u = 0; u < size; u += 2 * sizeof(DWord)) {
                    DWord r_offset = elfRead4(relocEntry++);
                    DWord info     = elfRead4(relocEntry++);
                    Q_UNUSED(info);

                    Address relocDestination; // Pointer to the word to be relocated
                    if (e_type == E_REL) {
                        relocDestination = destNatOrigin + r_offset;
                    }
                    else {
                        const IBinarySection *destSec = m_binaryImage->getSectionByAddr(Address(r_offset));
                        relocDestination      = destSec->getSourceAddr() + r_offset;
                        destNatOrigin = Address::ZERO;
                    }

                    if (addr == relocDestination) {
                        return true;
                    }
                }
            }
        }

        break;

    case EM_SPARC:
    default:
        LOG_WARN("Unhandled relocation!");
        break; // Not implemented yet
    }

    return false;
}


#define TESTMAGIC4(buf, off, a, b, c, d)    (buf[off] == a && buf[off + 1] == b && buf[off + 2] == c && buf[off + 3] == d)

int ElfBinaryLoader::canLoad(QIODevice& fl) const
{
    const QByteArray contents = fl.read(4);

    return TESTMAGIC4(contents.data(), 0, '\177', 'E', 'L', 'F') ? 4 : 0;
}


BOOMERANG_LOADER_PLUGIN(ElfBinaryLoader,
    "ELF32 loader plugin", BOOMERANG_VERSION, "Boomerang developers")
