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
 * This object provides 3 pseudo-sections:
 *  $HEADER is the main (elf) header of the file (Elf32_Ehdr)
 *  $PHEADER is the program header of the file (array of Elf32_Phdr structs)
 *  This is as per the file's program header, so the first enrty here
 *  corresponds to the all zeroes first section. The second entry here
 *  corresponds to the first real section of the file, which is the
 *  fourth section presented in the SECTION_INFO array.
 *  $SHEADER is an array of Elf32_Shdr structs; the first element
 *  corresponds to the first real section of the elf file (which
 *  is the fourth section presented in the SECTION_INFO array).
 *
 * 3 Mar 98 - Cristina
 *  changed ADDR for ADDRESS for consistency with other tools.
 * 11 Mar 98 - Cristina  
 *  replaced BOOL for bool type (C++'s), same for TRUE and FALSE.
 * 27 Mar 98 - Cristina
 *  added GetMainEntryPoint().
 * 27 May 98 - Mike
 *  Mods for BinaryFile
 * 6th Aug 98 - Mike
 *  GetEntryPoints returns list of SectionInfo* now
 * 21 Aug 98 - Mike
 *  Added bNoTypeOK to ValueByName(), GetAddressByName(), GetSizeByName
 * 14th Jan 99 - Mike
 *  Added functions like dumpVerneed() to dump these section types
 * 20 Apr 99 - Mike: Added GetDistanceByName()
 * 26 Sep 99 - Mike: Made GetStrPtr() public
 * 10 Aug 01 - Mike: Added GetDynamicGlobalMap(); define ELF32_R_SYM if nec
 * 12 Sep 01 - Mike: Replaced SymTab object with map from ADDRESS to string
 * 09 Mar 02 - Mike: Changes for stand alone compilation
*/

#ifndef __ELFBINARYFILE_H__
#define __ELFBINARYFILE_H__

// In case the below is not found in sys/elf.h
#ifndef  ELF32_R_SYM
#define  ELF32_R_SYM(info)       ((info)>>8)
#endif

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "syself.h"
#include "BinaryFile.h"
#include "SymTab.h"                 // For SymTab (probably unused)
typedef std::map<ADDRESS,std::string,std::less<ADDRESS> >  RelocMap;

typedef struct
{
    ADDRESS     uSymAddr;           // Symbol native address
    int         iSymSize;           // Size associated with symbol
} SymValue;

class ElfBinaryFile : public BinaryFile
{
public:
                ElfBinaryFile(bool bArchive = false);   // Constructor
  virtual       ~ElfBinaryFile();
  virtual void  UnLoad();                       // Unload the image
    bool        GetNextMember();                // Load next member of archive
  virtual bool  Open(const char* sName);        // Open the file for r/w; pv
  virtual void  Close();                        // Close file opened with Open()
  virtual LOAD_FMT GetFormat() const;           // Get format (e.g. LOADFMT_ELF)
  virtual MACHINE GetMachine() const;           // Get machine (e.g. MACHINE_SPARC)
  virtual bool isLibrary() const;
  virtual std::list<const char *> getDependencyList();
  virtual ADDRESS getImageBase();
  virtual size_t getImageSize();

                // Elf specific functions
    Elf32_Phdr* GetProgHeader(int idx);         // Get the indicated prog hdr
    Elf32_Shdr* GetSectionHeader(int idx);      // Get indicated section hdr
// Mike: deprecated! Remove when possible!
//    Elf_Scn*    GetElfScn(int idx);             // Do a elf_getscn()

                // Header functions
virtual ADDRESS GetFirstHeaderAddress();        // Get ADDRESS of main header
    ADDRESS     GetNextHeaderAddress();         // Get any other headers

                // Symbol functions
    char*       SymbolByAddress(ADDRESS uAddr); // Get name of symbol
                // Get value of symbol, if any
    ADDRESS     GetAddressByName(const char* pName, bool bNoTypeOK = false);
                // Get the size associated with the symbol
    int         GetSizeByName(const char* pName, bool bNoTypeOK = false);
                // Get the size associated with the symbol; guess if necessary
    int         GetDistanceByName(const char* pName);
    int         GetDistanceByName(const char* pName, const char* pSectName);
virtual ADDRESS* GetImportStubs(int& numImports);

                // Relocation functions
    bool        IsAddressRelocatable(ADDRESS uNative);
    ADDRESS     GetRelocatedAddress(ADDRESS uNative);
//  WORD        ApplyRelocation(ADDRESS uNative, WORD wWord);
                // Get symbol associated with relocation at address, if any
    const char* GetRelocSym(ADDRESS uNative);
                // Write an ELF object file for a given procedure
    void        writeObjectFile(std::string &path, const char* name,
                    void *ptxt, int txtsz, RelocMap& reloc);

//
//  --  --  --  --  --  --  --  --  --  --  --
//
                // Internal information
    // Dump headers, etc
virtual bool    DisplayDetails(const char* fileName, FILE* f = stdout);


                // Analysis functions
    virtual std::list<SectionInfo*>& GetEntryPoints(const char* pEntry = "main");
    virtual ADDRESS GetMainEntryPoint();
    virtual ADDRESS GetEntryPoint();

    bool        IsDynamicLinkedProc(ADDRESS wNative);
    ADDRESS     NativeToHostAddress(ADDRESS uNative);
    // Get a map from ADDRESS to const char*. This map contains the native
    // addresses and symbolic names of global data items (if any) which are
    // shared with dynamically linked libraries. Example: __iob (basis for
    // stdout).The ADDRESS is the native address of a pointer to the real
    // dynamic data object.
    // The caller should delete the returned map.
    virtual std::map<ADDRESS, const char*>* GetDynamicGlobalMap();

                // Not meant to be used externally, but sometimes you just
                // have to have it.
    char*       GetStrPtr(int idx, int offset); // Calc string pointer

                // Similarly here; sometimes you just need to change a section's
                // link and info fields
                // idx is the section index; link and info are indices to other
                // sections that will be idx's sh_link and sh_info respectively
    void        SetLinkAndInfo(int idx, int link, int info);

    const char* m_pFileName;            // Pointer to input file name
  protected:
    virtual bool  RealLoad(const char* sName); // Load the file; pure virtual

  private:
    void        Init();                 // Initialise most member variables
    int         ProcessElfFile();       // Does most of the work
    void        AddSyms(const char* pSymScn, const char* sStrScn);  
    void        SetRelocInfo(PSectionInfo pSect);
    bool        ValueByName(const char* pName, SymValue* pVal,
                    bool bNoTypeOK = false);
    bool        SearchValueByName(const char* pName, SymValue* pVal);
    bool        SearchValueByName(const char* pName, SymValue* pVal,
                    const char* pSectName, const char* pStrName);
    bool        PostLoad(void* handle); // Called after archive member loaded

    // For DisplayDetail()
    void        dumpShdr(Elf32_Shdr *pShdr, int idxElf, FILE* f);
    void        dumpSymtab(char* sSymName, char* sStrName,
                    Elf32_Shdr* pShdr, FILE* f);
//    void        dumpDynTab(Elf_Scn *scn, const char* name, FILE* f);
//    void        dumpPLT(Elf_Scn *scn, Elf32_Shdr *shdr, Elf32_Addr sh_addr,
//                    FILE* f);
//    void        dumpVerdef(Elf_Scn *scn, const char* scn_name,
//                    Elf32_Shdr* pShdr, FILE* f);
//    void        dumpVerneed(Elf_Scn *scn, const char* scn_name,
//                    Elf32_Shdr* pShdr, FILE* f);
//    void        dumpVersym(Elf_Scn *scn, const char* scn_name,
//                    Elf32_Shdr* pShdr, FILE* f);

    int         m_fd;                   // File descriptor
//    Elf*        m_arf;                  // Archive pointer
//    Elf*        m_elf;                  // Pointer to main elf info
    Elf32_Shdr* m_pShdrs;               // Array of section header structs
    std::map<ADDRESS, std::string> m_SymA;        // Map from address to symbol name
    SymTab      m_Reloc;                // Object to store the reloc syms
    Elf32_Ehdr* m_pHeader;              // Pointer to header
    ADDRESS     m_pReloc;               // Pointer to the relocation section
    Elf32_Sym*  m_pSym;                 // Pointer to loaded symbol section
    bool        m_bAddend;              // true if reloc table has addend
    const char* m_pLastName;            // Save pointer to last name looked up
    ADDRESS     m_uLastAddr;            // Save last address looked up
    int         m_iLastSize;            // Size associated with that name
    ADDRESS     m_uPltMin;              // Min address of PLT table
    ADDRESS     m_uPltMax;              // Max address (1 past last) of PLT
    std::list<SectionInfo*>  m_EntryPoint;   // A list of one entry point
    ADDRESS*    m_pImportStubs;         // An array of import stubs
    ADDRESS     m_uBaseAddr;            // Base image virtual address
    size_t      m_uImageSize;           // total image size (bytes)
};

#endif      // #ifndef __ELFBINARYFILE_H__
