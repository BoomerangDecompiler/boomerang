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
#define __MACHOBINARYFILE_H__

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
#define _BMMH(x)                                                                                                       \
    ((unsigned)((Byte *)(&x))[3] + ((unsigned)((Byte *)(&x))[2] << 8) + ((unsigned)((Byte *)(&x))[1] << 16) +          \
     ((unsigned)((Byte *)(&x))[0] << 24))
// With this one, x IS a pounsigneder
#define _BMMH2(x)                                                                                                      \
    ((unsigned)((Byte *)(x))[3] + ((unsigned)((Byte *)(x))[2] << 8) + ((unsigned)((Byte *)(x))[1] << 16) +             \
     ((unsigned)((Byte *)(x))[0] << 24))

#define _BMMHW(x) (((unsigned)((Byte *)(&x))[1]) + ((unsigned)((Byte *)(&x))[0] << 8))

struct mach_header;

class MachOBinaryFile : public QObject,
                        public LoaderInterface,
                        public SymbolTableInterface,
                        public ObjcAccessInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LoaderInterface_iid)
    Q_INTERFACES(LoaderInterface)
    Q_INTERFACES(ObjcAccessInterface)
    Q_INTERFACES(SymbolTableInterface)

  public:
    MachOBinaryFile();                    // Default constructor
    virtual ~MachOBinaryFile();           // Destructor
    virtual bool Open(const char *sName); // Open the file for r/w; ???
    virtual void Close();                 // Close file opened with Open()
    virtual void UnLoad();                // Unload the image
    virtual LOAD_FMT GetFormat() const;   // Get format (i.e. LOADFMT_MACHO)
    virtual MACHINE getMachine() const;   // Get machine (i.e. MACHINE_PPC)
    QString getFilename() const override { return m_pFileName; }
    virtual bool isLibrary() const;
    virtual QStringList getDependencyList();
    virtual ADDRESS getImageBase();
    virtual size_t getImageSize();

    virtual std::list<SectionInfo *> &GetEntryPoints(const char *pEntry = "main");
    virtual ADDRESS GetMainEntryPoint();
    virtual ADDRESS GetEntryPoint();
    DWord getDelta();
    QString symbolByAddress(ADDRESS dwAddr) override;                        // Get sym from addr
    ADDRESS GetAddressByName(const QString &name, bool bNoTypeOK = false) override; // Find addr given name
    void AddSymbol(ADDRESS uNative, const QString &pName) override;
    //! Lookup the name, return the size
    int GetSizeByName(const QString &pName, bool bTypeOK = false) override;
    ADDRESS *GetImportStubs(int &numImports) override;
    QString getFilenameSymbolFor(const char *) override;

    //
    //        --        --        --        --        --        --        --        --        --
    //
    // Internal information
    // Dump headers, etc
    bool DisplayDetails(const char *fileName, FILE *f = stdout) override ;

  protected:
    int machORead2(short *ps) const; // Read 2 bytes from native addr
    int machORead4(int *pi) const;   // Read 4 bytes from native addr

    // void *            BMMH(void *x);
    //    char *              BMMH(char *x);
    //    const char *        BMMH(const char *x);
    // unsigned int        BMMH(long int & x);
    int32_t BMMH(int32_t x);
    uint32_t BMMH(uint32_t x);
    unsigned short BMMHW(unsigned short x);

  public:
    bool IsDynamicLinkedProc(ADDRESS uNative) { return dlprocs.find(uNative) != dlprocs.end(); }
    const QString &GetDynamicProcName(ADDRESS uNative) override;

    tMapAddrToString &getSymbols() override { return m_SymA; }
    std::map<QString, ObjcModule> &getObjcModules() override  { return modules; }

  protected:
    bool RealLoad(const QString &sName) override; // Load the file; pure virtual

  private:
    bool PostLoad(void *handle);  // Called after archive member loaded
    void findJumps(ADDRESS curr); // Find names for jumps to IATs

    struct mach_header *header; // The Mach-O header
    char *base;                 // Beginning of the loaded image
    QString m_pFileName;
    ADDRESS entrypoint, loaded_addr;
    unsigned loaded_size;
    MACHINE machine;
    bool swap_bytes;
    tMapAddrToString m_SymA, dlprocs;
    std::map<QString, ObjcModule> modules;
    std::vector<struct section> sections;
    class IBinaryImage *Image;
};
#endif // ifndef __MACHOBINARYFILE_H__
