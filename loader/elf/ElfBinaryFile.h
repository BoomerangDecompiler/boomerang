/*
 * Copyright (C) 1998-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file ElfBinaryFile.h
 * \brief This file contains the definition of the class ElfBinaryFile.
*/

#ifndef __ELFBINARYFILE_H__
#define __ELFBINARYFILE_H__

/***************************************************************************/ /**
  * Dependencies.
  ******************************************************************************/

#include "BinaryFile.h"
struct Elf32_Phdr;
struct Elf32_Shdr;
struct Elf32_Rel;
struct Elf32_Sym;
struct Translated_ElfSym;
typedef std::map<ADDRESS, QString, std::less<ADDRESS>> RelocMap;

typedef struct {
    ADDRESS uSymAddr; // Symbol native address
    int iSymSize;     // Size associated with symbol
} SymValue;

class ElfBinaryFile : public QObject, public LoaderInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LoaderInterface_iid)
    Q_INTERFACES(LoaderInterface)
  public:
    ElfBinaryFile(); // Constructor
    void UnLoad() override;
    ~ElfBinaryFile() override;
    void initialize(IBoomerang *sys) override;
    void Close() override;
    LOAD_FMT GetFormat() const override;
    MACHINE getMachine() const override;
    bool isLibrary() const;
    QStringList getDependencyList();
    ADDRESS getImageBase() override;
    size_t getImageSize() override;

    // Relocation functions
    bool IsRelocationAt(ADDRESS uNative) override;

    // Write an ELF object file for a given procedure
    void writeObjectFile(QString &path, const char *name, void *ptxt, int txtsz, RelocMap &reloc);

    // Analysis functions
    virtual ADDRESS GetMainEntryPoint() override;
    virtual ADDRESS GetEntryPoint() override;

    ADDRESS NativeToHostAddress(ADDRESS uNative);

    bool LoadFromMemory(QByteArray &img);

  private:
    // Apply relocations; important when compiled without -fPIC
    void applyRelocations();
    // Not meant to be used externally, but sometimes you just have to have it.
    const char *GetStrPtr(int idx, int offset); // Calc string pointer
    void Init();          // Initialise most member variables
    int ProcessElfFile(); // Does most of the work
    void AddSyms(int secIndex);
    void AddRelocsAsSyms(uint32_t secIndex);
    void SetRelocInfo(SectionInfo * pSect);
    bool PostLoad(void *handle) override; // Called after archive member loaded
    // Search the .rel[a].plt section for an entry with symbol table index i.
    // If found, return the native address of the associated PLT entry.
    ADDRESS findRelPltOffset(int i);

    // Internal elf reading methods
    int elfRead2(const short *ps) const;    // Read a short with endianness care
    int elfRead4(const int *pi) const;      // Read an int with endianness care
    void elfWrite4(int *pi, int val); // Write an int with endianness care

    long m_lImageSize;                      // Size of image in bytes
    char *m_pImage;                         // Pointer to the loaded image
    Elf32_Phdr *m_pPhdrs;                   // Pointer to program headers
    Elf32_Shdr *m_pShdrs;                   // Array of section header structs
    char *m_pStrings;                       // Pointer to the string section
    char m_elfEndianness;                   // 1 = Big Endian
    // SymTab      m_Reloc;                 // Object to store the reloc syms
    const Elf32_Rel *m_pReloc;              // Pointer to the relocation section
    const Elf32_Sym *m_pSym;                // Pointer to loaded symbol section
    bool m_bAddend;                         // true if reloc table has addend
    ADDRESS m_uLastAddr;                    // Save last address looked up
    int m_iLastSize;                        // Size associated with that name
    ADDRESS m_uPltMin;                      // Min address of PLT table
    ADDRESS m_uPltMax;                      // Max address (1 past last) of PLT
    ADDRESS *m_pImportStubs;                // An array of import stubs
    ADDRESS m_uBaseAddr;                    // Base image virtual address
    size_t m_uImageSize;                    // total image size (bytes)
    ADDRESS first_extern;                   // where the first extern will be placed
    ADDRESS next_extern;                    // where the next extern will be placed
    int *m_sh_link;                         // pointer to array of sh_link values
    int *m_sh_info;                         // pointer to array of sh_info values

    std::vector<struct SectionParam> ElfSections;
    class IBinaryImage *Image;
    class IBinarySymbolTable *Symbols;
    void markImports();
    void processSymbol(Translated_ElfSym &sym, int e_type, int i);
};

#endif // #ifndef __ELFBINARYFILE_H__
