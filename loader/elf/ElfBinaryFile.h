/*
 * Copyright (C) 1998-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: ElfBinaryFile.h
 * Desc: This file contains the definition of the class ElfBinaryFile.
*/

/* $Revision$
 * 12 Sep 01 - Mike: Replaced SymTab object with map from ADDRESS to string
 * 09 Mar 02 - Mike: Changes for stand alone compilation
 * 01 Oct 02 - Mike: Removed elf library (and include file) dependencies
 * 02 Oct 02 - Mike: elfRead2 and elfRead4 are const now
*/

#ifndef __ELFBINARYFILE_H__
#define __ELFBINARYFILE_H__

/***************************************************************************//**
 * Dependencies.
 ******************************************************************************/

#include "BinaryFile.h"
//#include "../SymTab.h"                    // For SymTab (probably unused)
typedef std::map<ADDRESS,std::string,std::less<ADDRESS> >  RelocMap;

typedef struct
{
    ADDRESS        uSymAddr;            // Symbol native address
    int            iSymSize;            // Size associated with symbol
} SymValue;

// Internal elf info
typedef struct {
    char    e_ident[4];
    char    e_class;
    char    endianness;
    char    version;
    char    osAbi;
    char    pad[8];
    short   e_type;
    short   e_machine;
    int     e_version;
    int     e_entry;
    int     e_phoff;
    int     e_shoff;
    int     e_flags;
    short   e_ehsize;
    short   e_phentsize;
    short   e_phnum;
    short   e_shentsize;
    short   e_shnum;
    short   e_shstrndx;
} Elf32_Ehdr;

#define EM_SPARC        2            // Sun SPARC
#define EM_386            3            // Intel 80386 or higher
#define EM_68K            4            // Motorola 68000
#define EM_MIPS            8            // MIPS
#define EM_PA_RISC        15            // HP PA-RISC
#define EM_SPARC32PLUS        18            // Sun SPARC 32+
#define EM_PPC            20            // PowerPC
#define EM_X86_64        62
#define EM_ST20            0xa8            // ST20 (made up... there is no official value?)

#define ET_DYN    3        // Elf type (dynamic library)

#define R_386_32 1
#define R_386_PC32 2

// Program header
typedef struct {
    int     p_type;     /* entry type */
    int     p_offset;     /* file offset */
    int     p_vaddr;     /* virtual address */
    int     p_paddr;     /* physical address */
    int     p_filesz;     /* file size */
    int     p_memsz;     /* memory size */
    int     p_flags;     /* entry flags */
    int     p_align;     /* memory/file alignment */
} Elf32_Phdr;

// Section header
typedef struct {
    int    sh_name;
    int    sh_type;
    int    sh_flags;
    int    sh_addr;
    int    sh_offset;
    int    sh_size;
    int    sh_link;
    int    sh_info;
    int    sh_addralign;
    int    sh_entsize;
} Elf32_Shdr;

#define SHF_WRITE        1        // Writeable
#define SHF_ALLOC        2        // Consumes memory in exe
#define SHF_EXECINSTR    4        // Executable

#define SHT_NOBITS        8        // Bss
#define SHT_REL            9        // Relocation table (no addend)
#define SHT_RELA        4        // Relocation table (with addend, e.g. RISC)
#define SHT_SYMTAB        2        // Symbol table
#define SHT_DYNSYM        11        // Dynamic symbol table

typedef struct {
    int                st_name;
    unsigned        st_value;
    int                st_size;
    unsigned char    st_info;
    unsigned char    st_other;
    short            st_shndx;
} Elf32_Sym;

typedef struct {
    unsigned r_offset;
    int r_info;
} Elf32_Rel;

#define     ELF32_R_SYM(info)         ((info)>>8)
#define ELF32_ST_BIND(i)        ((i) >> 4)
#define ELF32_ST_TYPE(i)        ((i) & 0xf)
#define ELF32_ST_INFO(b, t)        (((b)<<4)+((t)&0xf))
#define STT_NOTYPE    0            // Symbol table type: none
#define STT_FUNC    2                // Symbol table type: function
#define STT_SECTION    3
#define STT_FILE 4
#define STB_GLOBAL    1
#define STB_WEAK    2

typedef struct {
    short d_tag;                /* how to interpret value */
    union {
        int     d_val;
        int     d_ptr;
        int     d_off;
    } d_un;
} Elf32_Dyn;

// Tag values
#define DT_NULL        0        // Last entry in list
#define DT_STRTAB    5        // String table
#define DT_NEEDED    1        // A needed link-type object

#define E_REL        1        // Relocatable file type

class ElfBinaryFile : public BinaryFile
{
public:
    ElfBinaryFile(bool bArchive = false);    // Constructor
    virtual void        UnLoad();                        // Unload the image
    virtual                ~ElfBinaryFile();                // Destructor
    bool        GetNextMember();                // Load next member of archive
    virtual bool        Open(const char* sName);        // Open the file for r/w; pv
    virtual void        Close();                        // Close file opened with Open()
    virtual LOAD_FMT    GetFormat() const;            // Get format (e.g. LOADFMT_ELF)
    virtual MACHINE        GetMachine() const;            // Get machine (e.g. MACHINE_SPARC)
    virtual const char*    getFilename() const { return m_pFileName; }
    virtual bool        isLibrary() const;
    virtual std::list<const char *> getDependencyList();
    virtual ADDRESS        getImageBase();
    virtual size_t        getImageSize();


    // Header functions
    //virtual ADDRESS    GetFirstHeaderAddress();        // Get ADDRESS of main header
    //        ADDRESS        GetNextHeaderAddress();            // Get any other headers

    char            readNative1(ADDRESS a);             // Read 1 bytes from native addr
    int             readNative2(ADDRESS a);             // Read 2 bytes from native addr
    int             readNative4(ADDRESS a);             // Read 4 bytes from native addr
    QWord        readNative8(ADDRESS a);                // Read 8 bytes from native addr
    float        readNativeFloat4(ADDRESS a);    // Read 4 bytes as float
    double        readNativeFloat8(ADDRESS a);    // Read 8 bytes as float

    void        writeNative4(ADDRESS nat, uint32_t n);

    // Symbol functions
    const char* SymbolByAddress(ADDRESS uAddr); // Get name of symbol
    // Get value of symbol, if any
    ADDRESS        GetAddressByName(const char* pName, bool bNoTypeOK = false);
    // Get the size associated with the symbol
    int            GetSizeByName(const char* pName, bool bNoTypeOK = false);
    // Get the size associated with the symbol; guess if necessary
    int            GetDistanceByName(const char* pName);
    int            GetDistanceByName(const char* pName, const char* pSectName);
    // Add an extra symbol
    void        AddSymbol(ADDRESS uNative, const char *pName);
    void        dumpSymbols();                // For debugging

    virtual ADDRESS*    GetImportStubs(int& numImports);
    virtual std::vector<ADDRESS> GetExportedAddresses(bool funcsOnly = true);


    // Relocation functions
    bool        IsAddressRelocatable(ADDRESS uNative);
    ADDRESS        GetRelocatedAddress(ADDRESS uNative);
    //ADDRESS        ApplyRelocation(ADDRESS uNative, ADDRESS uWord);
    // Get symbol associated with relocation at address, if any
    //const char* GetRelocSym(ADDRESS uNative, ADDRESS *a = nullptr, unsigned int *sz = nullptr);
    virtual bool IsRelocationAt(ADDRESS uNative);
    virtual    const char *getFilenameSymbolFor(const char *sym);

    // Write an ELF object file for a given procedure
    void        writeObjectFile(std::string &path, const char* name, void *ptxt, int txtsz, RelocMap& reloc);
    // Apply relocations; important when compiled without -fPIC
    void        applyRelocations();

    //
    //    --    --    --    --    --    --    --    --    --    --    --
    //
    // Internal information
    // Dump headers, etc
    //virtual bool    DisplayDetails(const char* fileName, FILE* f = stdout);


    // Analysis functions
    virtual std::list<SectionInfo*>& GetEntryPoints(const char* pEntry = "main");
    virtual ADDRESS        GetMainEntryPoint();
    virtual ADDRESS        GetEntryPoint();

    bool        IsDynamicLinkedProc(ADDRESS wNative);
    ADDRESS     NativeToHostAddress(ADDRESS uNative);
    // Get a map from ADDRESS to const char*. This map contains the native addresses and symbolic names of global
    // data items (if any) which are shared with dynamically linked libraries. Example: __iob (basis for stdout).
    // The ADDRESS is the native address of a pointer to the real dynamic data object.
    virtual std::map<ADDRESS, const char*>* GetDynamicGlobalMap();

    virtual std::map<ADDRESS, std::string> &getSymbols() { return m_SymTab; }

    // Not meant to be used externally, but sometimes you just have to have it.
    const char *    GetStrPtr(int idx, int offset); // Calc string pointer

    // Similarly here; sometimes you just need to change a section's link and info fields
    // idx is the section index; link and info are indices to other
    // sections that will be idx's sh_link and sh_info respectively
    void        SetLinkAndInfo(int idx, int link, int info);

    const char* m_pFileName;            // Pointer to input file name
protected:
    virtual bool        RealLoad(const char* sName); // Load the file; pure virtual

private:
    void        Init();                 // Initialise most member variables
    int         ProcessElfFile();       // Does most of the work
    void        AddSyms(int secIndex);
    void        AddRelocsAsSyms(int secIndex);
    void        SetRelocInfo(PSectionInfo pSect);
    bool        ValueByName(const char* pName, SymValue* pVal, bool bNoTypeOK = false);
    bool        SearchValueByName(const char* pName, SymValue* pVal);
    bool        SearchValueByName(const char* pName, SymValue* pVal, const char* pSectName, const char* pStrName);
    bool        PostLoad(void* handle); // Called after archive member loaded
    // Search the .rel[a].plt section for an entry with symbol table index i.
    // If found, return the native address of the associated PLT entry.
    ADDRESS        findRelPltOffset(int i, ADDRESS addrRelPlt, int sizeRelPlt, int numRelPlt, ADDRESS addrPlt);

    // Internal elf reading methods
    int         elfRead2(short* ps) const;        // Read a short with endianness care
    int         elfRead4(int*   pi) const;        // Read an int with endianness care
    void        elfWrite4(int*    pi, int val);    // Write an int with endianness care

    FILE*        m_fd;                            // File stream
    long        m_lImageSize;                    // Size of image in bytes
    char*       m_pImage;                        // Pointer to the loaded image
    Elf32_Phdr* m_pPhdrs;                        // Pointer to program headers
    Elf32_Shdr* m_pShdrs;                        // Array of section header structs
    char*       m_pStrings;                        // Pointer to the string section
    char        m_elfEndianness;                // 1 = Big Endian
    std::map<ADDRESS, std::string> m_SymTab;    // Map from address to symbol name; contains symbols from the
    // various elf symbol tables, and possibly some symbols with fake
    // addresses
    //SymTab      m_Reloc;                        // Object to store the reloc syms
    Elf32_Rel*  m_pReloc;                        // Pointer to the relocation section
    Elf32_Sym*  m_pSym;                            // Pointer to loaded symbol section
    bool        m_bAddend;                        // true if reloc table has addend
    ADDRESS     m_uLastAddr;                    // Save last address looked up
    int         m_iLastSize;                    // Size associated with that name
    ADDRESS     m_uPltMin;                        // Min address of PLT table
    ADDRESS     m_uPltMax;                        // Max address (1 past last) of PLT
    std::list<SectionInfo*>  m_EntryPoint;        // A list of one entry point
    ADDRESS*    m_pImportStubs;                    // An array of import stubs
    ADDRESS     m_uBaseAddr;                    // Base image virtual address
    size_t      m_uImageSize;                    // total image size (bytes)
    ADDRESS        first_extern;                    // where the first extern will be placed
    ADDRESS        next_extern;                    // where the next extern will be placed
    int*        m_sh_link;                        // pointer to array of sh_link values
    int*        m_sh_info;                        // pointer to array of sh_info values
};

#endif        // #ifndef __ELFBINARYFILE_H__
