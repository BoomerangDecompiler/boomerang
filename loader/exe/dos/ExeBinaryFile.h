/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file ExeBinaryFile.h
 * \brief This file contains the definition of the class ExeBinaryFile.
 * This file contains the definition of the ExeBinaryFile class, and some other
 * definitions specific to the exe version of the BinaryFile object/
 *   At present, there is no support for a symbol table. Exe files do
 * not use dynamic linking, but it is possible that some files may
 * have debug symbols (in Microsoft Codeview or Borland formats),
 * and these may be implemented in the future. The debug info may
 * even be exposed as another pseudo section
 */

#ifndef __EXEBINARYFILE_H__
#define __EXEBINARYFILE_H__

#include "BinaryFile.h"

typedef struct {        /*        PSP structure                 */
    SWord int20h;       /* interrupt 20h                        */
    SWord eof;          /* segment, end of allocation block     */
    Byte res1;          /* reserved                             */
    Byte dosDisp[5];    /* far call to DOS function dispatcher  */
    Byte int22h[4];     /* vector for terminate routine         */
    Byte int23h[4];     /* vector for ctrl+break routine        */
    Byte int24h[4];     /* vector for error routine             */
    Byte res2[22];      /* reserved                             */
    SWord segEnv;       /* segment address of environment block */
    Byte res3[34];      /* reserved                             */
    Byte int21h[6];     /* opcode for int21h and far return     */
    Byte res4[6];       /* reserved                             */
    Byte fcb1[16];      /* default file control block 1         */
    Byte fcb2[16];      /* default file control block 2         */
    Byte res5[4];       /* reserved                             */
    Byte cmdTail[0x80]; /* command tail and disk transfer area  */
} PSP;

typedef struct {/*      EXE file header          */
    Byte sigLo; /* .EXE signature: 0x4D 0x5A     */
    Byte sigHi;
    SWord lastPageSize;   /* Size of the last page         */
    SWord numPages;       /* Number of pages in the file   */
    SWord numReloc;       /* Number of relocation items    */
    SWord numParaHeader;  /* # of paragraphs in the header */
    SWord minAlloc;       /* Minimum number of paragraphs  */
    SWord maxAlloc;       /* Maximum number of paragraphs  */
    SWord initSS;         /* Segment displacement of stack */
    SWord initSP;         /* Contents of SP at entry       */
    SWord checkSum;       /* Complemented checksum         */
    SWord initIP;         /* Contents of IP at entry       */
    SWord initCS;         /* Segment displacement of code  */
    SWord relocTabOffset; /* Relocation table offset       */
    SWord overlayNum;     /* Overlay number                */
} exeHeader;

class ExeBinaryFile : public QObject, public LoaderInterface, public LoaderCommon {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LoaderInterface_iid)
    Q_INTERFACES(LoaderInterface)
    Q_INTERFACES(SectionInterface)
  public:
    ExeBinaryFile();                       // Default constructor
    void UnLoad() override;                // Unload the image
    bool Open(const char *sName) override; // Open the file for r/w; pv
    void Close() override;                 // Close file opened with Open()
    bool PostLoad(void *handle) override;  // For archive files only
    LOAD_FMT GetFormat() const override;   // Get format (i.e. LOADFMT_EXE)
    MACHINE getMachine() const override;   // Get machine (i.e. MACHINE_PENTIUM)
    QString getFilename() const override { return m_pFileName; }

    bool isLibrary() const override;
    QStringList getDependencyList() override;
    ADDRESS getImageBase() override;
    size_t getImageSize() override;

    // Analysis functions
    std::list<SectionInfo *> &GetEntryPoints(const char *pEntry = "main") override;
    ADDRESS GetMainEntryPoint() override;
    ADDRESS GetEntryPoint() override;

    tMapAddrToString &getSymbols() override;

    //
    //  --  --  --  --  --  --  --  --  --  --  --
    //
    // Internal information
    // Dump headers, etc
    bool DisplayDetails(const char *fileName, FILE *f = stdout) override;

  protected:
    bool RealLoad(const QString &sName) override; // Load the file; pure virtual
  private:
    exeHeader *m_pHeader; // Pointer to header
    Byte *m_pImage;       // Pointer to image
    int m_cbImage;        // Size of image
    int m_cReloc;         // Number of relocation entries
    DWord *m_pRelocTable; // The relocation table
    QString m_pFileName;
    class IBinaryImage *Image;
};

#endif // ifndef __EXEBINARYFILE_H__
