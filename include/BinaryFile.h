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
class IBoomerang;

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
#define ObjcInterface_iid "org.boomerang.LoaderInterface.ObjC"
// TODO: create a default implmentation for this interface that will notify the user about the missing functionality ?


class ObjcAccessInterface {
public:
    virtual ~ObjcAccessInterface() {}

    virtual std::map<QString, ObjcModule> &getObjcModules() = 0;
};
Q_DECLARE_INTERFACE(ObjcAccessInterface, ObjcInterface_iid)

class LoaderInterface {
public:
    virtual ~LoaderInterface() {}

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // General loader functions
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void initialize(IBoomerang *sys) = 0;
    virtual void UnLoad() = 0;                //!< Unload the file. Pure virtual
    virtual void Close() = 0;                 //!< Close file opened with Open()
    virtual LOAD_FMT GetFormat() const = 0;   //!< Get the format (e.g. LOADFMT_ELF)
    virtual MACHINE getMachine() const = 0;   //!< Get the expected machine (e.g. MACHINE_PENTIUM)
    virtual QString getFilename() const = 0;
    virtual bool RealLoad(const QString &sName) = 0;

    /// Return the virtual address at which the binary expects to be loaded.
    /// For position independent / relocatable code this should be NO_ADDDRESS
    virtual ADDRESS getImageBase() = 0;
    virtual size_t getImageSize() = 0; //!< Return the total size of the loaded image

    virtual bool IsRelocationAt(ADDRESS /*uNative*/) { return false; }

    virtual ADDRESS IsJumpToAnotherAddr(ADDRESS /*uNative*/) { return NO_ADDRESS; }
    virtual bool hasDebugInfo() { return false; }

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
