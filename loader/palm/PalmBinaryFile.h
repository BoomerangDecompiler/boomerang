/*
 * Copyright (C) 2000-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file PalmBinaryFile.h
 * \brief This file contains the definition of the class PalmBinaryFile.
*/
#ifndef __PALMBINARYFILE_H__
#define __PALMBINARYFILE_H__

/***************************************************************************/ /**
  * Dependencies.
  ******************************************************************************/

#include "BinaryFile.h"
#include <QtCore/QObject>

class PalmBinaryFile : public QObject,
                       public LoaderInterface,
                       public SymbolTableInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LoaderInterface_iid)
    Q_INTERFACES(LoaderInterface)
    Q_INTERFACES(SymbolTableInterface)

  public:
    PalmBinaryFile(); // Constructor
    virtual ~PalmBinaryFile();
    void UnLoad() override;                // Unload the image
    bool Open(const char *sName) override; // Open the file for r/w; pv
    void Close() override;                 // Close file opened with Open()
    bool PostLoad(void *handle) override;  // For archive files only
    LOAD_FMT GetFormat() const override;   // Get format i.e. LOADFMT_PALM
    MACHINE getMachine() const override;   // Get machine i.e. MACHINE_PALM
    QString getFilename() const override { return m_pFileName; }

    bool isLibrary() const override;
    QStringList getDependencyList() override;
    ADDRESS getImageBase() override;
    size_t getImageSize() override;

    // Get a symbol given an address
    QString symbolByAddress(ADDRESS dwAddr) override;
    ADDRESS GetAddressByName(const QString &pName, bool bNoTypeOK = false) override;
    void AddSymbol(ADDRESS /*uNative*/, const QString & /*pName*/) override;
    int GetSizeByName(const QString &pName, bool bTypeOK = false) override;
    ADDRESS *GetImportStubs(int &numImports) override;
    QString getFilenameSymbolFor(const char * /*sym*/) override;
    tMapAddrToString &getSymbols() override;

    // Return true if the address matches the convention for A-line system calls
    bool IsDynamicLinkedProc(ADDRESS uNative) override;

    // Specific to BinaryFile objects that implement a "global pointer"
    // Gets a pair of unsigned integers representing the address of %agp (first) and the value for GLOBALOFFSET (second)
    Q_INVOKABLE std::pair<ADDRESS, unsigned> GetGlobalPointerInfo();

    // Palm specific calls

    // Get the ID number for this application. It's possible that the app uses
    // this number internally, so this needs to be used in the final make
    int GetAppID() const;

    // Generate binary files for non code and data sections
    void GenerateBinFiles(const QString &path) const;

    //
    //  --  --  --  --  --  --  --  --  --  --  --
    //
    // Internal information
    // Dump headers, etc
    // virtual bool    DisplayDetails(const char* fileName, FILE* f = stdout);

    // Analysis functions
    virtual std::list<SectionInfo *> &GetEntryPoints(const char *pEntry = "main") override;
    virtual ADDRESS GetMainEntryPoint() override;
    virtual ADDRESS GetEntryPoint() override;

    //    bool        IsDynamicLinkedProc(ADDRESS wNative);
    //    ADDRESS     NativeToHostAddress(ADDRESS uNative);

  protected:
    bool RealLoad(const QString &sName) override; // Load the file; pure virtual

  private:
    tMapAddrToString m_symTable;
    unsigned char *m_pImage; //!< Points to loaded image
    unsigned char *m_pData;  //!< Points to data
    // Offset from start of data to where register a5 should be initialised to
    unsigned int m_SizeBelowA5;
    QString m_pFileName;
    class IBinaryImage *Image;

};

#endif // #ifndef __PALMBINARYFILE_H__
