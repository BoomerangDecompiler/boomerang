/* * Copyright (C) 2000-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: HpSomBinaryFile.h
 * Desc: This file contains the definition of the class HpSomBinaryFile.
*/

/* $Revision$
 *
 * 22 Jun 00 - Mike: Initial revision
 * 09 May 01 - Mike: Read the imports table so can identify library functions
 * 14 May 01 - Mike: Added GetAddressByName()
 * 01 Aug 01 - Mike: GetGlobalPointerInfo() returns unsigned ints now
 * 03 Aug 01 - Mike: Added a few useful structs
 * 10 Aug 01 - Mike: Added GetDynamicGlobalMap()
*/

#ifndef __HPSOMBINARYFILE_H__
#define __HPSOMBINARYFILE_H__

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "BinaryFile.h"
#include "SymTab.h"
#include <set>

struct import_entry {
    int         name;
    short       reserved2;
    Byte        type;
    Byte        reserved1;
};

struct export_entry {
    int         next;
    int         name;
    int         value;
    int         size;       // Also misc_info
    Byte        type;
    char        reserved1;
    short       module_index;
};

struct space_dictionary_record {
    unsigned    name;
    unsigned    flags;
    int         space_number;
    int         subspace_index;
    unsigned    subspace_quantity;
    int         loader_fix_index;
    unsigned    loader_fix_quantity;
    int         init_pointer_index;
    unsigned    init_pointer_quantity;
};

struct subspace_dictionary_record {
    int         space_index;
    unsigned    flags;
    int         file_loc_init_value;
    unsigned    initialization_length;
    unsigned    subspace_start;
    unsigned    subspace_length;
    unsigned    alignment;
    unsigned    name;
    int         fixup_request_index;
    int         fixup_request_quantity;
};

struct plt_record {
    ADDRESS     value;                      // Address in the library
    ADDRESS     r19value;                   // r19 value needed
};

struct symElem {
    const char* name;                       // Simple symbol table entry
    ADDRESS     value;
};

class HpSomBinaryFile : public BinaryFile
{
public:
                  HpSomBinaryFile();          // Constructor
    virtual       ~HpSomBinaryFile();
    virtual void  UnLoad();                   // Unload the image
    virtual bool  Open(const char* sName);    // Open the file for r/w; pv
    virtual void  Close();                    // Close file opened with Open()
    virtual bool  PostLoad(void* handle);     // For archive files only
    virtual LOAD_FMT GetFormat() const;       // Get format i.e. LOADFMT_PALM

    virtual bool isLibrary() const;
    virtual std::list<const char *> getDependencyList();
    virtual ADDRESS getImageBase();
    virtual size_t getImageSize();

    // Get a symbol given an address
    char*   SymbolByAddress(const ADDRESS dwAddr);
    // Lookup the name, return the address
    virtual ADDRESS GetAddressByName(char* pName, bool bNoTypeOK = false);
    // Return true if the address matches the convention for A-line system calls
    bool          IsDynamicLinkedProc(ADDRESS uNative);

    // Specific to BinaryFile objects that implement a "global pointer"
    // Gets a pair of unsigned integers representing the address of %agp (first)
    // and the value for GLOBALOFFSET (unused for pa-risc)
    virtual std::pair<unsigned,unsigned> GetGlobalPointerInfo();

    // Get a map from ADDRESS to const char*. This map contains the native
    // addresses and symbolic names of global data items (if any) which are
    // shared with dynamically linked libraries. Example: __iob (basis for
    // stdout).The ADDRESS is the native address of a pointer to the real
    // dynamic data object.
    // The caller should delete the returned map.
    virtual std::map<ADDRESS, const char*>* GetDynamicGlobalMap();

//
//  --  --  --  --  --  --  --  --  --  --  --
//
// Internal information
// Dump headers, etc
//virtual bool    DisplayDetails(const char* fileName, FILE* f = stdout);


    // Analysis functions
    virtual std::list<SectionInfo*>& GetEntryPoints(const char* pEntry = "main");
    virtual ADDRESS GetMainEntryPoint();
    virtual ADDRESS GetEntryPoint();

//    bool        IsDynamicLinkedProc(ADDRESS wNative);
//    ADDRESS     NativeToHostAddress(ADDRESS uNative);
  protected:
    virtual bool  RealLoad(const char* sName); // Load the file; pure virtual

    
private:
    // Private method to get the start and length of a given subspace
    std::pair<ADDRESS, int> getSubspaceInfo(const char* ssname);

    unsigned char*  m_pImage;                   // Points to loaded image
    SymTab          symbols;                    // Symbol table object
//  ADDRESS         mainExport;                 // Export entry for "main"
    std::set<ADDRESS>    imports;                    // Set of imported proc addr's
};

#endif      // #ifndef __HPSOMBINARYFILE_H__
