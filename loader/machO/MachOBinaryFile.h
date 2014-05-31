/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file MachOBinaryFile.h
 * \brief This file contains the definition of the class MachOBinaryFile.
*/

#ifndef __MACHOBINARYFILE_H__
#define __MACHOBINARYFILE_H_

#include "BinaryFile.h"
#include <string>
#include <vector>

/**
 * This file contains the definition of the MachOBinaryFile class, and some
 * other definitions specific to the Mac OS-X version of the BinaryFile object
 * This is my bare bones implementation of a Mac OS-X binary loader.
 */

// Given a little endian value x, load its value assuming big endian order
// Note: must be able to take address of x
// Note: Unlike the LH macro in BinaryFile.h, the parameter is not a pointer
#define _BMMH(x) ((unsigned)((Byte *)(&x))[3] + ((unsigned)((Byte *)(&x))[2] << 8) + \
    ((unsigned)((Byte *)(&x))[1] << 16) + ((unsigned)((Byte *)(&x))[0] << 24))
// With this one, x IS a pounsigneder
#define _BMMH2(x) ((unsigned)((Byte *)(x))[3] + ((unsigned)((Byte *)(x))[2] << 8) + \
    ((unsigned)((Byte *)(x))[1] << 16) + ((unsigned)((Byte *)(x))[0] << 24))

#define _BMMHW(x) (((unsigned)((Byte *)(&x))[1]) + ((unsigned)((Byte *)(&x))[0] << 8))

#ifndef _MACH_MACHINE_H_                // On OS X, this is already defined
typedef uint32_t cpu_type_t;        // I guessed
typedef uint32_t cpu_subtype_t;    // I guessed
typedef uint32_t vm_prot_t;        // I guessed
#endif

struct mach_header;

class MachOBinaryFile : public QObject,
        public BinaryData,
        public LoaderInterface,
        public SymbolTableInterface,
        public ObjcAccessInterface,
        public LoaderCommon
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LoaderInterface_iid)
    Q_INTERFACES(LoaderInterface)
    Q_INTERFACES(BinaryData)
    Q_INTERFACES(SectionInterface)
    Q_INTERFACES(SymbolTableInterface)

public:
    MachOBinaryFile();                // Default constructor
    virtual             ~MachOBinaryFile();                // Destructor
    virtual bool        Open(const char* sName);        // Open the file for r/w; ???
    virtual void        Close();                        // Close file opened with Open()
    virtual void        UnLoad();                        // Unload the image
    virtual LOAD_FMT    GetFormat() const;            // Get format (i.e. LOADFMT_MACHO)
    virtual MACHINE     GetMachine() const;            // Get machine (i.e. MACHINE_PPC)
    QString getFilename() const override { return m_pFileName; }
    virtual bool        isLibrary() const;
    virtual QStringList getDependencyList();
    virtual ADDRESS        getImageBase();
    virtual size_t        getImageSize();

    virtual std::list<SectionInfo*>& GetEntryPoints(const char* pEntry = "main");
    virtual ADDRESS        GetMainEntryPoint();
    virtual ADDRESS        GetEntryPoint();
    DWord        getDelta();
    const char*    SymbolByAddress(ADDRESS dwAddr) override; // Get sym from addr
    ADDRESS        GetAddressByName(const char* name, bool bNoTypeOK = false) override; // Find addr given name
    void        AddSymbol(ADDRESS uNative, const char *pName) override;
    //! Lookup the name, return the size
    int GetSizeByName(const char *pName, bool bTypeOK = false) override;
    ADDRESS *GetImportStubs(int &numImports) override;
    const char *getFilenameSymbolFor(const char *) override;


    //
    //        --        --        --        --        --        --        --        --        --
    //
    // Internal information
    // Dump headers, etc
    virtual bool    DisplayDetails(const char* fileName, FILE* f = stdout);

protected:

    int machORead2(short *ps) const; // Read 2 bytes from native addr
    int machORead4(int *pi) const;     // Read 4 bytes from native addr

    //void *            BMMH(void *x);
//    char *              BMMH(char *x);
//    const char *        BMMH(const char *x);
    //unsigned int        BMMH(long int & x);
    int32_t BMMH(int32_t x);
    uint32_t            BMMH(uint32_t x);
    unsigned short      BMMHW(unsigned short x);

public:
    virtual bool        isReadOnly(ADDRESS uEntry);
    virtual bool        isStringConstant(ADDRESS uEntry);
    virtual bool        isCFStringConstant(ADDRESS uEntry);
    virtual char        readNative1(ADDRESS a);         // Read 1 bytes from native addr
    virtual int         readNative2(ADDRESS a);            // Read 2 bytes from native addr
    virtual int         readNative4(ADDRESS a);            // Read 4 bytes from native addr
    virtual QWord       readNative8(ADDRESS a);    // Read 8 bytes from native addr
    virtual float       readNativeFloat4(ADDRESS a);    // Read 4 bytes as float
    virtual double      readNativeFloat8(ADDRESS a); // Read 8 bytes as float

    virtual bool        IsDynamicLinkedProc(ADDRESS uNative) { return dlprocs.find(uNative) != dlprocs.end(); }
    virtual const char* GetDynamicProcName(ADDRESS uNative);

    virtual std::map<ADDRESS, std::string> &getSymbols() { return m_SymA; }
    virtual std::map<std::string, ObjcModule> &getObjcModules() { return modules; }

protected:
    bool        RealLoad(const QString &sName) override; // Load the file; pure virtual

private:

    bool        PostLoad(void* handle); // Called after archive member loaded
    void        findJumps(ADDRESS curr);// Find names for jumps to IATs

    struct mach_header *header;      // The Mach-O header
    char *        base;                    // Beginning of the loaded image
    QString m_pFileName;
    ADDRESS        entrypoint, loaded_addr;
    unsigned    loaded_size;
    MACHINE         machine;
    bool            swap_bytes;
    std::map<ADDRESS, std::string> m_SymA, dlprocs;
    std::map<std::string, ObjcModule> modules;
    std::vector<struct section> sections;
};
#endif            // ifndef __WIN32BINARYFILE_H__
