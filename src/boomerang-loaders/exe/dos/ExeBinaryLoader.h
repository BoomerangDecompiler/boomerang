#pragma once

/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file ExeBinaryLoader.h
 * \brief This file contains the definition of the class ExeBinaryLoader.
 * This file contains the definition of the ExeBinaryLoader class.
 *
 * At present, there is no support for a symbol table. Exe files do
 * not use dynamic linking, but it is possible that some files may
 * have debug symbols (in Microsoft Codeview or Borland formats),
 * and these may be implemented in the future. The debug info may
 * even be exposed as another pseudo section.
 */

#include "boomerang/loader/IFileLoader.h"

class QFile;

#pragma pack(push,1)
struct PSP               /*        PSP structure                 */
{
    SWord int20h;        /* interrupt 20h                        */
    SWord eof;           /* segment, end of allocation block     */
    Byte  res1;          /* reserved                             */
    Byte  dosDisp[5];    /* far call to DOS function dispatcher  */
    Byte  int22h[4];     /* vector for terminate routine         */
    Byte  int23h[4];     /* vector for ctrl+break routine        */
    Byte  int24h[4];     /* vector for error routine             */
    Byte  res2[22];      /* reserved                             */
    SWord segEnv;        /* segment address of environment block */
    Byte  res3[34];      /* reserved                             */
    Byte  int21h[6];     /* opcode for int21h and far return     */
    Byte  res4[6];       /* reserved                             */
    Byte  fcb1[16];      /* default file control block 1         */
    Byte  fcb2[16];      /* default file control block 2         */
    Byte  res5[4];       /* reserved                             */
    Byte  cmdTail[0x80]; /* command tail and disk transfer area  */
};

struct ExeHeader          /*      EXE file header          */
{
    Byte  sigLo;          /* .EXE signature: 0x4D 0x5A     */
    Byte  sigHi;
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
};

#pragma pack(pop)


class ExeBinaryLoader : public IFileLoader
{
public:
    ExeBinaryLoader();

    /// \copydoc IFileLoader::initialize
    void initialize(IBinaryImage *image, IBinarySymbolTable *symbols) override;

    /// \copydoc IFileLoader::canLoad
    int canLoad(QIODevice& fl) const override;

    /// \copydoc IFileLoader::loadFromMemory
    bool loadFromMemory(QByteArray& data) override;

    /// \copydoc IFileLoader::unload
    void unload() override;

    /// \copydoc IFileLoader::close
    void close() override;

    /// \copydoc IFileLoader::getFormat
    LoadFmt getFormat() const override;

    /// \copydoc IFileLoader::getMachine
    Machine getMachine() const override;

    /// \copydoc IFileLoader::getMainEntryPoint
    Address getMainEntryPoint() override;

    /// \copydoc IFileLoader::getEntryPoint
    Address getEntryPoint() override;


private:
    ExeHeader *m_header;  ///< Pointer to header
    Byte *m_loadedImage;  ///< Pointer to image buffer
    int m_imageSize;      ///< Size of image
    int m_numReloc;       ///< Number of relocation entries
    DWord *m_relocTable;  ///< The relocation table
    Address m_uInitPC;    ///< Initial program counter
    Address m_uInitSP;    ///< Initial stack pointer
    IBinaryImage *m_image;
    IBinarySymbolTable *m_symbols;
};
