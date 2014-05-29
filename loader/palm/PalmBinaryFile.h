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

/***************************************************************************//**
 * Dependencies.
 ******************************************************************************/

#include "BinaryFile.h"
#include <QtCore/QObject>

class PalmBinaryFile : public QObject,
        public LoaderInterface,
        public BinaryData,
        public LoaderCommon,
        public SymbolTableInterface
{
    Q_OBJECT
    Q_INTERFACES(LoaderInterface)
    Q_INTERFACES(SectionInterface)
    Q_INTERFACES(SymbolTableInterface)

public:
    PalmBinaryFile();               // Constructor
    virtual       ~PalmBinaryFile();
    void  UnLoad() override;                       // Unload the image
    bool  Open(const char* sName) override;        // Open the file for r/w; pv
    void  Close() override;                        // Close file opened with Open()
    bool  PostLoad(void* handle) override;         // For archive files only
    LOAD_FMT GetFormat() const override;           // Get format i.e. LOADFMT_PALM
    MACHINE GetMachine() const override;           // Get machine i.e. MACHINE_PALM
    const char *getFilename() const  override { return m_pFileName; }

    bool isLibrary() const  override;
    QStringList getDependencyList() override;
    ADDRESS getImageBase() override;
    size_t getImageSize() override;

    // Get a symbol given an address
    const char*    SymbolByAddress(ADDRESS dwAddr) override;
    ADDRESS GetAddressByName(const char *pName, bool bNoTypeOK = false) override;
    void AddSymbol(ADDRESS /*uNative*/, const char * /*pName*/) override;
    int GetSizeByName(const char *pName, bool bTypeOK = false) override;
    ADDRESS *GetImportStubs(int &numImports) override;
    const char *getFilenameSymbolFor(const char * /*sym*/) override;
    tMapAddrToString &getSymbols() override;


    // Return true if the address matches the convention for A-line system calls
    bool        IsDynamicLinkedProc(ADDRESS uNative) override;

    // Specific to BinaryFile objects that implement a "global pointer"
    // Gets a pair of unsigned integers representing the address of %agp (first) and the value for GLOBALOFFSET (second)
    Q_INVOKABLE std::pair<ADDRESS,unsigned> GetGlobalPointerInfo();

    // Palm specific calls

    // Get the ID number for this application. It's possible that the app uses
    // this number internally, so this needs to be used in the final make
    int           GetAppID() const;

    // Generate binary files for non code and data sections
    void          GenerateBinFiles(const std::string& path) const;

    //
    //  --  --  --  --  --  --  --  --  --  --  --
    //
    // Internal information
    // Dump headers, etc
    //virtual bool    DisplayDetails(const char* fileName, FILE* f = stdout);


    // Analysis functions
    virtual std::list<SectionInfo*>& GetEntryPoints(const char* pEntry = "main") override;
    virtual ADDRESS GetMainEntryPoint() override;
    virtual ADDRESS GetEntryPoint() override;

    //    bool        IsDynamicLinkedProc(ADDRESS wNative);
    //    ADDRESS     NativeToHostAddress(ADDRESS uNative);

    char    readNative1(ADDRESS nat) override;
    int     readNative2(ADDRESS nat) override;
    int     readNative4(ADDRESS nat) override;
    QWord   readNative8(ADDRESS nat) override;
    float   readNativeFloat4(ADDRESS nat) override;
    double  readNativeFloat8(ADDRESS nat) override;
protected:
    virtual bool  RealLoad(const char* sName) override; // Load the file; pure virtual

private:
    std::map<ADDRESS, std::string> m_symTable;
    unsigned char* m_pImage;                       //!< Points to loaded image
    unsigned char* m_pData;                        //!< Points to data
    // Offset from start of data to where register a5 should be initialised to
    unsigned int   m_SizeBelowA5;
    const char *   m_pFileName;
};

#endif      // #ifndef __PALMBINARYFILE_H__
