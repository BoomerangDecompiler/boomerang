/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/**
 * \file: BinaryFile.h
 * @brief: This file contains the definition of the abstract class BinaryFile
*/

#ifndef __BINARYFILE_H__
#define __BINARYFILE_H__

/***************************************************************************//**
 * Dependencies.
 *============================================================================*/

#include "types.h"
//#include "SymTab.h"    // Was used for relocaton stuff
#include <list>
#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include <cstdio>       // For FILE
#include <QtCore/QStringList>
// Note: #including windows.h causes problems later in the objective C code.

// Given a pointer p, returns the 16 bits (halfword) in the two bytes
// starting at p.
#define LH(p)  ((int)((Byte *)(p))[0] + ((int)((Byte *)(p))[1] << 8))

// SectionInfo structure. GetSectionInfo returns a pointer to an array of
// these structs. All information about the sections is contained in these
// structures.

struct SectionInfo
{
                    SectionInfo();          // Constructor
virtual             ~SectionInfo() {}       // Quell a warning in gcc

                    // Windows's PE file sections can contain any combination of code, data and bss.
                    // As such, it can't be correctly described by SectionInfo, why we need to override
                    // the behaviour of (at least) the question "Is this address in BSS".
virtual bool        isAddressBss(ADDRESS /*a*/) const {
                        return bBss != 0;
                    }

        char *      pSectionName;           // Name of section
        ADDRESS     uNativeAddr;            // Logical or native load address
        ADDRESS     uHostAddr;              // Host or actual address of data
        uint32_t    uSectionSize;           // Size of section in bytes
        uint32_t    uSectionEntrySize;      // Size of one section entry (if applic)
        unsigned    uType;                  // Type of section (format dependent)
        unsigned    bCode:1;                // Set if section contains instructions
        unsigned    bData:1;                // Set if section contains data
        unsigned    bBss:1;                 // Set if section is BSS (allocated only)
        unsigned    bReadOnly:1;            // Set if this is a read only section
};

typedef SectionInfo* PSectionInfo;

// Objective-C stuff
class ObjcIvar {
public:
    std::string name, type;
    unsigned offset;
};

class ObjcMethod {
public:
    std::string name, types;
    ADDRESS addr;
};

class ObjcClass {
public:
    std::string name;
    std::map<std::string, ObjcIvar> ivars;
    std::map<std::string, ObjcMethod> methods;
};

class ObjcModule {
public:
    std::string name;
    std::map<std::string, ObjcClass> classes;
};

/*
 * callback function, which when given the name of a library, should return
 * a pointer to an opened BinaryFile, or nullptr if the name cannot be resolved.
 */
class BinaryFile;
typedef BinaryFile *(*get_library_callback_t)(char *name);

// This enum allows a sort of run time type identification, without using
// compiler specific features
enum LOAD_FMT {
    LOADFMT_ELF,
    LOADFMT_PE,
    LOADFMT_PALM,
    LOADFMT_PAR,
    LOADFMT_EXE,
    LOADFMT_MACHO,
    LOADFMT_LX,
    LOADFMT_COFF
};
enum MACHINE {MACHINE_PENTIUM, MACHINE_SPARC, MACHINE_HPRISC, MACHINE_PALM, MACHINE_PPC, MACHINE_ST20, MACHINE_MIPS, MACHINE_68K};

class BinaryFileFactory {
        void *          dlHandle;        // Needed for UnLoading the library
        BinaryFile *    getInstanceFor(const char *sName);
static  std::string     m_base_path; //!< path from which the executable is being ran, used to find lib/ directory

public:

static  void            setBasePath(const std::string &path) {m_base_path=path;} //!< sets the base directory for plugin search
        BinaryFile *    Load(const std::string &sName );
        void            UnLoad();
};
class BinaryData {
public:
    virtual char            readNative1(ADDRESS a)=0;
                            // Read 2 bytes from given native address a; considers endianness
    virtual int             readNative2(ADDRESS a)=0;// {return 0;}
                            // Read 4 bytes from given native address a; considers endianness
    virtual int             readNative4(ADDRESS a)=0;// {return 0;}
                            // Read 8 bytes from given native address a; considers endianness
    virtual QWord           readNative8(ADDRESS a)=0;// {return 0;}
                            // Read 4 bytes as a float; consider endianness
    virtual float           readNativeFloat4(ADDRESS a)=0;// {return 0.;}
                            // Read 8 bytes as a float; consider endianness
    virtual double          readNativeFloat8(ADDRESS a)=0;// {return 0.;}

};

class BinaryFile : public BinaryData {

  friend class ArchiveFile;            // So can use the protected Load()
  friend class BinaryFileFactory;    // So can use getTextLimits

public:
        typedef         std::map<ADDRESS, std::string> tMapAddrToString;
virtual                 ~BinaryFile() {}
                        BinaryFile(bool bArchive = false);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General loader functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

virtual void            UnLoad() = 0;           //!< Unload the file. Pure virtual
virtual bool            Open(const char* sName) = 0; //!< Open the file for r/w; pure virt
virtual void            Close() = 0;            //!< Close file opened with Open()
virtual LOAD_FMT        GetFormat() const = 0;  //!< Get the format (e.g. LOADFMT_ELF)
virtual MACHINE         GetMachine() const = 0; //!< Get the expected machine (e.g. MACHINE_PENTIUM)
virtual const char *    getFilename() const = 0;

                        //! Return whether or not the object is a library file.
virtual bool            isLibrary() const = 0;
                        /// Return whether the object can be relocated if necessary
                        /// (ie if it is not tied to a particular base address). If not, the object
                        /// must be loaded at the address given by getImageBase()
virtual bool            isRelocatable() const { return isLibrary(); }
                        //! Return a list of library names which the binary file depends on
virtual QStringList     getDependencyList() = 0;
                        /// Return the virtual address at which the binary expects to be loaded.
                        /// For position independent / relocatable code this should be NO_ADDDRESS
virtual ADDRESS         getImageBase() = 0;
virtual size_t          getImageSize() = 0; //!< Return the total size of the loaded image

// Section functions
        int             GetNumSections() const;        // Return number of sections
        SectionInfo *   GetSectionInfo(int idx) const; // Return section struct
        // Find section info given name, or 0 if not found
        SectionInfo *   GetSectionInfoByName(const char* sName);
        // Find the end of a section, given an address in the section
        SectionInfo *   GetSectionInfoByAddr(ADDRESS uEntry) const;

                        // returns true if the given address is in a read only section
        bool            isReadOnly(ADDRESS uEntry) {
                            PSectionInfo p = GetSectionInfoByAddr(uEntry);
                            return p && p->bReadOnly;
                        }
  // returns true if the given address is in a "strings" section
virtual bool            isStringConstant(ADDRESS /*uEntry*/) { return false; }
virtual bool            isCFStringConstant(ADDRESS /*uEntry*/) { return false; }
// Symbol table functions
virtual const char*     SymbolByAddress(ADDRESS uNative);
virtual ADDRESS         GetAddressByName(const char* pName, bool bNoTypeOK = false);
virtual void            AddSymbol(ADDRESS /*uNative*/, const char */*pName*/) { }

virtual int             GetSizeByName(const char* pName, bool bTypeOK = false);
virtual ADDRESS *       GetImportStubs(int& numImports);
virtual const char *    getFilenameSymbolFor(const char */*sym*/) { return nullptr; }
virtual std::vector<ADDRESS> GetExportedAddresses(bool /*funcsOnly*/ = true) { return std::vector<ADDRESS>(); }

// Relocation table functions
//virtual bool    IsAddressRelocatable(ADDRESS uNative);
//virtual ADDRESS GetRelocatedAddress(ADDRESS uNative);
//virtual    ADDRESS    ApplyRelocation(ADDRESS uNative, ADDRESS uWord);
        // Get symbol associated with relocation at address, if any
//virtual const char* GetRelocSym(ADDRESS uNative, ADDRESS *a = nullptr, unsigned int *sz = nullptr) { return nullptr; }
virtual bool            IsRelocationAt(ADDRESS /*uNative*/) { return false; }

virtual std::pair<ADDRESS,unsigned> GetGlobalPointerInfo();

virtual std::map<ADDRESS, const char*>* GetDynamicGlobalMap();

///////////////////////////////////////////////////////////////////////////////
// Internal information
                        // Dump headers, etc
virtual bool            DisplayDetails(const char* fileName, FILE* f = stdout);

        // Analysis functions
virtual bool            IsDynamicLinkedProc(ADDRESS uNative);
virtual bool            IsStaticLinkedLibProc(ADDRESS uNative);
virtual bool            IsDynamicLinkedProcPointer(ADDRESS uNative);
virtual ADDRESS         IsJumpToAnotherAddr(ADDRESS uNative);
virtual const char *    GetDynamicProcName(ADDRESS uNative);
virtual std::list<SectionInfo*>& GetEntryPoints(const char* pEntry = "main") = 0;
virtual ADDRESS         GetMainEntryPoint() = 0;
virtual ADDRESS         GetEntryPoint() = 0;  //!< Return the "real" entry point, ie where execution of the program begins
        int             GetSectionIndexByName(const char* sName);
virtual bool            RealLoad(const char* sName) = 0;

virtual tMapAddrToString &getFuncSymbols() { return *new std::map<ADDRESS, std::string>(); }
virtual tMapAddrToString &getSymbols() { return *new std::map<ADDRESS, std::string>(); }

virtual std::map<std::string, ObjcModule> &getObjcModules() { return *new std::map<std::string, ObjcModule>(); }

        ADDRESS         getLimitTextLow() { return limitTextLow; }
        ADDRESS         getLimitTextHigh() { return limitTextHigh; }
        ptrdiff_t       getTextDelta() { return textDelta; }

virtual bool            hasDebugInfo() { return false; }

///////////////////////////////////////////////////////////////////////////////
// Special load function for archive members
protected:
virtual bool            PostLoad(void* handle) = 0; //!< Called after loading archive member
public:
        void            getTextLimits();
protected:
        // Data
        bool            m_bArchive;                 //!< True if archive member
        int             m_iNumSections;             //!< Number of sections
        SectionInfo *   m_pSections;                //!< The section info
        ADDRESS         m_uInitPC;                  //!< Initial program counter
        ADDRESS         m_uInitSP;                  //!< Initial stack pointer
        ADDRESS         limitTextLow;               //!< Public addresses being the lowest used native address (inclusive)
        ADDRESS         limitTextHigh;              //!< the highest used address (not inclusive) in the text segment
        // Also the difference between the host and native addresses (host - native)
        // At this stage, we are assuming that the difference is the same for all
        // text sections of the BinaryFile image
        ptrdiff_t       textDelta;
};

#endif        // #ifndef __BINARYFILE_H__
