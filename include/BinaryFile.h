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

/***************************************************************************/ /**
  * Dependencies.
  *============================================================================*/

#include "types.h"
//#include "SymTab.h"    // Was used for relocaton stuff
#include <list>
#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include <cstdio> // For FILE
#include <QtCore/QStringList>
// Note: #including windows.h causes problems later in the objective C code.

// Given a pointer p, returns the 16 bits (halfword) in the two bytes
// starting at p.
#define LH(p) ((int)((Byte *)(p))[0] + ((int)((Byte *)(p))[1] << 8))

// SectionInfo structure. GetSectionInfo returns a pointer to an array of
// these structs. All information about the sections is contained in these
// structures.

struct SectionInfo {
    SectionInfo();            // Constructor
    virtual ~SectionInfo() {} // Quell a warning in gcc

    // Windows's PE file sections can contain any combination of code, data and bss.
    // As such, it can't be correctly described by SectionInfo, why we need to override
    // the behaviour of (at least) the question "Is this address in BSS".
    virtual bool isAddressBss(ADDRESS /*a*/) const { return bBss != 0; }

    char *pSectionName;         // Name of section
    ADDRESS uNativeAddr;        // Logical or native load address
    ADDRESS uHostAddr;          // Host or actual address of data
    uint32_t uSectionSize;      // Size of section in bytes
    uint32_t uSectionEntrySize; // Size of one section entry (if applic)
    unsigned uType;             // Type of section (format dependent)
    unsigned bCode : 1;         // Set if section contains instructions
    unsigned bData : 1;         // Set if section contains data
    unsigned bBss : 1;          // Set if section is BSS (allocated only)
    unsigned bReadOnly : 1;     // Set if this is a read only section
};

typedef SectionInfo *PSectionInfo;

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
enum MACHINE {
    MACHINE_PENTIUM,
    MACHINE_SPARC,
    MACHINE_HPRISC,
    MACHINE_PALM,
    MACHINE_PPC,
    MACHINE_ST20,
    MACHINE_MIPS,
    MACHINE_68K
};

class BinaryFileFactory {
    void *dlHandle; // Needed for UnLoading the library
    QObject *getInstanceFor(const QString &sName);
    static QString m_base_path; //!< path from which the executable is being ran, used to find lib/ directory

  public:
    static void setBasePath(const QString &path) {
        m_base_path = path;
    } //!< sets the base directory for plugin search
    QObject *Load(const QString &sName);
    void UnLoad();
};

#define LoaderInterface_iid "org.boomerang.LoaderInterface"
#define SymTableInterface_iid "org.boomerang.LoaderInterface.SymTable"
#define ObjcInterface_iid "org.boomerang.LoaderInterface.ObjC"
#define BinaryInterface_iid "org.boomerang.LoaderInterface.Data"
#define SectionsInterface_iid "org.boomerang.LoaderInterface.Sections"
//TODO: create a default implmentation for this interface that will notify the user about the missing functionality ?
class BinaryData {
  public:
    virtual char readNative1(ADDRESS a) = 0;
    // Read 2 bytes from given native address a; considers endianness
    virtual int readNative2(ADDRESS a) = 0; // {return 0;}
    // Read 4 bytes from given native address a; considers endianness
    virtual int readNative4(ADDRESS a) = 0; // {return 0;}
    // Read 8 bytes from given native address a; considers endianness
    virtual QWord readNative8(ADDRESS a) = 0; // {return 0;}
    // Read 4 bytes as a float; consider endianness
    virtual float readNativeFloat4(ADDRESS a) = 0; // {return 0.;}
    // Read 8 bytes as a float; consider endianness
    virtual double readNativeFloat8(ADDRESS a) = 0; // {return 0.;}
};
Q_DECLARE_INTERFACE(BinaryData, BinaryInterface_iid)

class SymbolTableInterface {
  public:
    //! Lookup the address, return the name, or 0 if not found
    virtual const char *SymbolByAddress(ADDRESS uNative) = 0;
    //! Lookup the name, return the address. If not found, return NO_ADDRESS
    virtual ADDRESS GetAddressByName(const char *pName, bool bNoTypeOK = false) = 0;
    virtual void AddSymbol(ADDRESS /*uNative*/, const char * /*pName*/) = 0;
    //! Lookup the name, return the size
    virtual int GetSizeByName(const char *pName, bool bTypeOK = false) = 0;
    /***************************************************************************/ /**
      *
      * \brief Get an array of addresses of imported function stubs
      * Set number of these to numImports
      * \param numExports size of returned array
      * \returns  array of stubs
      ******************************************************************************/
    virtual ADDRESS *GetImportStubs(int &numImports) = 0;
    //! \return a filename for given symbol
    virtual const char *getFilenameSymbolFor(const char * /*sym*/) = 0;
};
Q_DECLARE_INTERFACE(SymbolTableInterface, SymTableInterface_iid)

class ObjcAccessInterface {
  public:
    virtual std::map<std::string, ObjcModule> &getObjcModules() = 0;
};
Q_DECLARE_INTERFACE(ObjcAccessInterface, ObjcInterface_iid)

class LoaderInterface {
  public:
    typedef std::map<ADDRESS, std::string> tMapAddrToString;

  public:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // General loader functions
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void UnLoad() = 0;                //!< Unload the file. Pure virtual
    virtual bool Open(const char *sName) = 0; //!< Open the file for r/w; pure virt
    virtual void Close() = 0;                 //!< Close file opened with Open()
    virtual LOAD_FMT GetFormat() const = 0;   //!< Get the format (e.g. LOADFMT_ELF)
    virtual MACHINE GetMachine() const = 0;   //!< Get the expected machine (e.g. MACHINE_PENTIUM)
    virtual QString getFilename() const = 0;
    virtual bool RealLoad(const QString &sName) = 0;

    //! Return whether or not the object is a library file.
    virtual bool isLibrary() const = 0;
    /// Return whether the object can be relocated if necessary
    /// (ie if it is not tied to a particular base address). If not, the object
    /// must be loaded at the address given by getImageBase()
    virtual bool isRelocatable() const { return isLibrary(); }
    //! Return a list of library names which the binary file depends on
    virtual QStringList getDependencyList() = 0;
    /// Return the virtual address at which the binary expects to be loaded.
    /// For position independent / relocatable code this should be NO_ADDDRESS
    virtual ADDRESS getImageBase() = 0;
    virtual size_t getImageSize() = 0; //!< Return the total size of the loaded image
    virtual std::vector<ADDRESS> GetExportedAddresses(bool /*funcsOnly*/ = true) { return std::vector<ADDRESS>(); }

    virtual bool IsRelocationAt(ADDRESS /*uNative*/) { return false; }

    virtual bool IsDynamicLinkedProc(ADDRESS /*uNative*/) { return false; }
    virtual bool IsStaticLinkedLibProc(ADDRESS /*uNative*/) { return false; }
    virtual bool IsDynamicLinkedProcPointer(ADDRESS /*uNative*/) { return false; }
    virtual ADDRESS IsJumpToAnotherAddr(ADDRESS /*uNative*/) { return NO_ADDRESS; }
    virtual tMapAddrToString &getSymbols() = 0;
    virtual bool hasDebugInfo() { return false; }
    virtual const char *GetDynamicProcName(ADDRESS /*uNative*/) {
        return "dynamic";
    }

    virtual std::list<SectionInfo *> &GetEntryPoints(const char *pEntry = "main") = 0;
    virtual ADDRESS GetMainEntryPoint() = 0;
    virtual ADDRESS GetEntryPoint() = 0; //!< Return the "real" entry point, ie where execution of the program begins
    ///////////////////////////////////////////////////////////////////////////////
    // Internal information
    // Dump headers, etc
    virtual bool DisplayDetails(const char* /*fileName*/, FILE* /*f*/ /* = stdout */) {
        return false;            // Should always be overridden
        // Should display file header, program
        // headers and section headers, as well
        // as contents of each of the sections.
    }
    /***************************************************************************/ /**
      *
      * Specific to BinaryFile objects that implement a "global pointer"
      * Gets a pair of unsigned integers representing the address of the
      * abstract global pointer (%agp) (in first) and a constant that will
      * be available in the csrparser as GLOBALOFFSET (second). At present,
      * the latter is only used by the Palm machine, to represent the space
      * allocated below the %a5 register (i.e. the difference between %a5 and
      * %agp). This value could possibly be used for other purposes.
      *
      ******************************************************************************/
    virtual std::pair<ADDRESS, unsigned> GetGlobalPointerInfo() {return {NO_ADDRESS,0};}

    /***************************************************************************/ /**
      *
      * \brief Get a map from native addresses to symbolic names of global data items
      * (if any).
      *
      * Those are shared with dynamically linked libraries.
      * Example: __iob (basis for stdout).
      * The ADDRESS is the native address of a pointer to the real dynamic data object.
      * If the derived class doesn't implement this function, return an empty map
      *
      * \note Caller should delete the returned map
      * \returns  map of globals
      ******************************************************************************/
    virtual std::map<ADDRESS, const char *> *GetDynamicGlobalMap() { return nullptr; }


    // Special load function for archive members
  protected:
    virtual bool PostLoad(void *handle) = 0; //!< Called after loading archive member
};
Q_DECLARE_INTERFACE(LoaderInterface, LoaderInterface_iid)
class SectionInterface {
public:
    virtual int GetNumSections() const =0;                 // Return number of sections
    virtual SectionInfo *GetSectionInfo(int idx) const=0; // Return section struct
    virtual SectionInfo *GetSectionInfoByName(const char *sName)=0;
    virtual SectionInfo *GetSectionInfoByAddr(ADDRESS uEntry) const=0;
    virtual bool isReadOnly(ADDRESS uEntry)=0; //!< returns true if the given address is in a read only section
    virtual int GetSectionIndexByName(const char *sName)=0;
    virtual bool isStringConstant(ADDRESS /*uEntry*/) { return false; }
    //! returns true if the given address is in a "strings" section
    virtual bool isCFStringConstant(ADDRESS /*uEntry*/) { return false; }
    virtual ADDRESS getLimitTextLow() = 0;
    virtual ADDRESS getLimitTextHigh() = 0;
    virtual ptrdiff_t getTextDelta() = 0;
    virtual void getTextLimits() = 0;

};
Q_DECLARE_INTERFACE(SectionInterface, SectionsInterface_iid)
struct LoaderPluginWrapper {
    QObject *plugin;
    template <class T> T *iface() { return plugin ? qobject_cast<T *>(plugin) : nullptr; }
};
#if 0
class BinaryFile : public QObject, public LoaderInterface {

    Q_OBJECT
    friend class ArchiveFile;       // So can use the protected Load()
    friend class BinaryFileFactory; // So can use getTextLimits
  public:
    BinaryFile(bool bArchive = false);
    virtual ~BinaryFile() {}

    virtual bool isStringConstant(ADDRESS /*uEntry*/) {
        return false;
    } //! returns true if the given address is in a "strings" section
    virtual bool isCFStringConstant(ADDRESS /*uEntry*/) { return false; }

    // Symbol table functions
    // virtual const char*     SymbolByAddress(ADDRESS uNative);

    virtual void AddSymbol(ADDRESS /*uNative*/, const char * /*pName*/) {}

    virtual ADDRESS *GetImportStubs(int &numImports);
    virtual const char *getFilenameSymbolFor(const char * /*sym*/) { return nullptr; }

    // Relocation table functions
    // virtual bool    IsAddressRelocatable(ADDRESS uNative);
    // virtual ADDRESS GetRelocatedAddress(ADDRESS uNative);
    // virtual    ADDRESS    ApplyRelocation(ADDRESS uNative, ADDRESS uWord);
    // Get symbol associated with relocation at address, if any
    // virtual const char* GetRelocSym(ADDRESS uNative, ADDRESS *a = nullptr, unsigned int *sz = nullptr) { return
    // nullptr; }

    virtual std::pair<ADDRESS, unsigned> GetGlobalPointerInfo();

    virtual std::map<ADDRESS, const char *> *GetDynamicGlobalMap();

    // Analysis functions
    int GetSectionIndexByName(const char *sName);

    virtual tMapAddrToString &getFuncSymbols() {
        static tMapAddrToString def;
        return def;
    }
    virtual tMapAddrToString &getSymbols() override {
        static tMapAddrToString def;
        return def;
    }

    virtual bool hasDebugInfo() override { return false; }

    ///////////////////////////////////////////////////////////////////////////////
};
#endif
class LoaderCommon : public SectionInterface {
public:
    LoaderCommon(bool bArch = false) {
        m_bArchive      = bArch;        // Remember whether an archive member
        m_iNumSections  = 0;            // No sections yet
        m_pSections     = nullptr;            // No section data yet
    }
    int GetNumSections() const override {return m_iNumSections;} // Return number of sections
    SectionInfo *GetSectionInfo(int idx) const override {
        return m_pSections + idx;
    }
    //! Find section index given name, or -1 if not found
    int GetSectionIndexByName(const char* sName) override {
        for (int i=0; i < m_iNumSections; i++) {
            if (strcmp(m_pSections[i].pSectionName, sName) == 0) {
                return i;
            }
        }
        return -1;
    }
    //! Find the end of a section, given an address in the section
    SectionInfo *GetSectionInfoByAddr(ADDRESS uEntry) const override {
        PSectionInfo pSect;
        for (int i=0; i < m_iNumSections; i++) {
            pSect = &m_pSections[i];
            if ((uEntry >= pSect->uNativeAddr) && (uEntry < pSect->uNativeAddr + pSect->uSectionSize)) {
                // We have the right section
                return pSect;
            }
        }
        // Failed to find the address
        return nullptr;
    }

    bool isReadOnly(ADDRESS uEntry) override {
        PSectionInfo p = GetSectionInfoByAddr(uEntry);
        return p && p->bReadOnly;
    }
    //! Find section info given name, or 0 if not found
    SectionInfo * GetSectionInfoByName(const char* sName) override {
        int i = GetSectionIndexByName(sName);
        if (i == -1)
            return nullptr;
        return &m_pSections[i];
    }
    ADDRESS getLimitTextLow() override { return limitTextLow; }
    ADDRESS getLimitTextHigh() override { return limitTextHigh; }
    ptrdiff_t getTextDelta() override { return textDelta; }
    void getTextLimits() override;
protected:
  // Data
  bool m_bArchive;          //!< True if archive member
  int m_iNumSections;       //!< Number of sections
  SectionInfo *m_pSections; //!< The section info
  ADDRESS m_uInitPC;        //!< Initial program counter
  ADDRESS m_uInitSP;        //!< Initial stack pointer
  ADDRESS limitTextLow;     //!< Public addresses being the lowest used native address (inclusive)
  ADDRESS limitTextHigh;    //!< the highest used address (not inclusive) in the text segment
  // Also the difference between the host and native addresses (host - native)
  // At this stage, we are assuming that the difference is the same for all
  // text sections of the BinaryFile image
  ptrdiff_t textDelta;
};
#endif // #ifndef __BINARYFILE_H__
