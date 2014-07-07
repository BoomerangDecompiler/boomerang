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

class PalmBinaryFile : public QObject, public LoaderInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LoaderInterface_iid)
    Q_INTERFACES(LoaderInterface)

  public:
    PalmBinaryFile(); // Constructor
    virtual ~PalmBinaryFile();
    void initialize(IBoomerang *sys) override;
    void UnLoad() override;                // Unload the image
    void Close() override;                 // Close file opened with Open()
    bool PostLoad(void *handle) override;  // For archive files only
    LOAD_FMT GetFormat() const override;   // Get format i.e. LOADFMT_PALM
    MACHINE getMachine() const override;   // Get machine i.e. MACHINE_PALM
    QString getFilename() const override { return m_pFileName; }

    bool isLibrary() const;
    ADDRESS getImageBase() override;
    size_t getImageSize() override;

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
    virtual ADDRESS GetMainEntryPoint() override;
    virtual ADDRESS GetEntryPoint() override;

    //    bool        IsDynamicLinkedProc(ADDRESS wNative);
    //    ADDRESS     NativeToHostAddress(ADDRESS uNative);

  protected:
    bool RealLoad(const QString &sName) override; // Load the file; pure virtual

private:
    void addTrapSymbols();
    unsigned char *m_pImage; //!< Points to loaded image
    unsigned char *m_pData;  //!< Points to data
    // Offset from start of data to where register a5 should be initialised to
    unsigned int m_SizeBelowA5;
    QString m_pFileName;
    class IBinaryImage *Image;
    class IBinarySymbolTable *Symbols;

};

#endif // #ifndef __PALMBINARYFILE_H__
