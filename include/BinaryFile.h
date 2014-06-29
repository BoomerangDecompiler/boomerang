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
  ******************************************************************************/

#include "types.h"
#include "db/SectionInfo.h"

//#include "SymTab.h"    // Was used for relocaton stuff
#include <QStringList>
#include <QDebug>
#include <QString>
#include <cassert>
#include <list>
#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include <cstdio> // For FILE
// Given a pointer p, returns the 16 bits (halfword) in the two bytes
// starting at p.
#define LH(p) ((int)((Byte *)(p))[0] + ((int)((Byte *)(p))[1] << 8))


// Objective-C stuff
class ObjcIvar {
public:
    QString name, type;
    unsigned offset;
};

class ObjcMethod {
public:
    QString name, types;
    ADDRESS addr;
};

class ObjcClass {
public:
    QString name;
    std::map<QString, ObjcIvar> ivars;
    std::map<QString, ObjcMethod> methods;
};

class ObjcModule {
public:
    QString name;
    std::map<QString, ObjcClass> classes;
};

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
    MACHINE_UNKNOWN=0,
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
    // void *dlHandle; // TODO: consider replacing this with QPluginLoader instances to allow unloading ?
    QObject *getInstanceFor(const QString &sName);
    static QString m_base_path; //!< path from which the executable is being ran, used to find lib/ directory

public:
    static void setBasePath(const QString &path) { m_base_path = path; } //!< sets the base directory for plugin search
    QObject *Load(const QString &sName);
    void UnLoad();
};

#define LoaderInterface_iid "org.boomerang.LoaderInterface"
#define SymTableInterface_iid "org.boomerang.LoaderInterface.SymTable"
#define ObjcInterface_iid "org.boomerang.LoaderInterface.ObjC"
#define SectionsInterface_iid "org.boomerang.LoaderInterface.Sections"
// TODO: create a default implmentation for this interface that will notify the user about the missing functionality ?

class SymbolTableInterface {
public:
    virtual ~SymbolTableInterface() {}

    //! Lookup the address, return the name, or 0 if not found
    virtual QString symbolByAddress(ADDRESS uNative) = 0;
    //! Lookup the name, return the address. If not found, return NO_ADDRESS
    virtual ADDRESS GetAddressByName(const QString &pName, bool bNoTypeOK = false) = 0;
    virtual void AddSymbol(ADDRESS /*uNative*/, const QString & /*pName*/) = 0;
    //! Lookup the name, return the size
    virtual int GetSizeByName(const QString &pName, bool bTypeOK = false) = 0;
    /***************************************************************************/ /**
      *
      * \brief Get an array of addresses of imported function stubs
      * Set number of these to numImports
      * \param numImports size of returned array
      * \returns  array of stubs
      ******************************************************************************/
    virtual ADDRESS *GetImportStubs(int &numImports) = 0;
    //! \return a filename for given symbol
    virtual QString getFilenameSymbolFor(const char * /*sym*/) = 0;
};
Q_DECLARE_INTERFACE(SymbolTableInterface, SymTableInterface_iid)

class ObjcAccessInterface {
public:
    virtual ~ObjcAccessInterface() {}

    virtual std::map<QString, ObjcModule> &getObjcModules() = 0;
};
Q_DECLARE_INTERFACE(ObjcAccessInterface, ObjcInterface_iid)

class LoaderInterface {
public:
    typedef std::map<ADDRESS, QString> tMapAddrToString;

public:
    virtual ~LoaderInterface() {}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // General loader functions
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void UnLoad() = 0;                //!< Unload the file. Pure virtual
    virtual bool Open(const char *sName) = 0; //!< Open the file for r/w; pure virt
    virtual void Close() = 0;                 //!< Close file opened with Open()
    virtual LOAD_FMT GetFormat() const = 0;   //!< Get the format (e.g. LOADFMT_ELF)
    virtual MACHINE getMachine() const = 0;   //!< Get the expected machine (e.g. MACHINE_PENTIUM)
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
    virtual const QString &GetDynamicProcName(ADDRESS /*uNative*/) {
        static QString default_val("dynamic");
        return default_val;
    }

    virtual std::list<SectionInfo *> &GetEntryPoints(const char *pEntry = "main") = 0;
    virtual ADDRESS GetMainEntryPoint() = 0;
    virtual ADDRESS GetEntryPoint() = 0; //!< Return the "real" entry point, ie where execution of the program begins
    ///////////////////////////////////////////////////////////////////////////////
    // Internal information
    // Dump headers, etc
    virtual bool DisplayDetails(const char * /*fileName*/, FILE * /*f*/ /* = stdout */) {
        return false; // Should always be overridden
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
    virtual std::pair<ADDRESS, unsigned> GetGlobalPointerInfo() { return {NO_ADDRESS, 0}; }

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
struct LoaderPluginWrapper {
    QObject *plugin;
    template <class T> T *iface() { return plugin ? qobject_cast<T *>(plugin) : nullptr; }
};

#endif // #ifndef __BINARYFILE_H__
