/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file DOS4GWBinaryLoader.h
 * \brief This file contains the definition of the class DOS4GWBinaryLoader.
 */

#pragma once

#include "boomerang/loader/IFileLoader.h"

#include <string>

class QFile;

/**
 * This file contains the definition of the DOS4GWBinaryLoader class.
 * At present, this loader supports the OS2 file format (also known as
 * the Linear eXecutable format) as much as I've found necessary to
 * inspect old DOS4GW apps.  This loader could also be used for decompiling
 * Win9x VxD files or, of course, OS2 binaries, but you're probably better off
 * making a specific loader for each of these.
 */

// Given a little endian value x, load its value assuming little endian order
// Note: must be able to take address of x
// Note: Unlike the LH macro in BinaryFile.h, the paraeter is not a pointer
#define LMMH(x)                                                                                                  \
    ((unsigned)((Byte *)(&x))[0] + ((unsigned)((Byte *)(&x))[1] << 8) + ((unsigned)((Byte *)(&x))[2] << 16) + \
     ((unsigned)((Byte *)(&x))[3] << 24))
// With this one, x IS a pounsigneder
#define LMMH2(x)                                                                                           \
    ((unsigned)((Byte *)(x))[0] + ((unsigned)((Byte *)(x))[1] << 8) + ((unsigned)((Byte *)(x))[2] << 16) + \
     ((unsigned)((Byte *)(x))[3] << 24))
#define LMMHw(x)    ((unsigned)((Byte *)(&x))[0] + ((unsigned)((Byte *)(&x))[1] << 8))


typedef struct  /* exe file header, just the signature really */
{
    Byte sigLo; /* .EXE signature: 0x4D 0x5A     */
    Byte sigHi;
} Header;

// The following structures should have their members byte aligned
#pragma pack(push,1)
typedef struct
{
    Byte  sigLo;
    Byte  sigHi;
    Byte  byteord;
    Byte  wordord;
    DWord formatlvl;
    SWord cputype;
    SWord ostype;
    DWord modulever;
    DWord moduleflags;
    DWord modulenumpages;
    DWord eipobjectnum;
    DWord eip;
    DWord espobjectnum;
    DWord esp;
    DWord pagesize;
    DWord pageoffsetshift;
    DWord fixupsectionsize;
    DWord fixupsectionchksum;
    DWord loadersectionsize;
    DWord loadersectionchksum;
    DWord objtbloffset;
    DWord numobjsinmodule;
    DWord objpagetbloffset;
    DWord objiterpagesoffset;
    DWord resourcetbloffset;
    DWord numresourcetblentries;
    DWord residentnametbloffset;
    DWord entrytbloffset;
    DWord moduledirectivesoffset;
    DWord nummoduledirectives;
    DWord fixuppagetbloffset;
    DWord fixuprecordtbloffset;
    DWord importtbloffset;
    DWord numimportmoduleentries;
    DWord importproctbloffset;
    DWord perpagechksumoffset;
    DWord datapagesoffset;
    DWord numpreloadpages;
    DWord nonresnametbloffset;
    DWord nonresnametbllen;
    DWord nonresnametblchksum;
    DWord autodsobjectnum;
    DWord debuginfooffset;
    DWord debuginfolen;
    DWord numinstancepreload;
    DWord numinstancedemand;
    DWord heapsize;
} LXHeader;

typedef struct
{
    DWord VirtualSize;
    DWord RelocBaseAddr;
    DWord ObjectFlags;
    DWord PageTblIdx;
    DWord NumPageTblEntries;
    DWord Reserved1;
} LXObject;

typedef struct
{
    DWord pagedataoffset;
    SWord datasize;
    SWord flags;
} LXPage;

// this is correct for internal fixups only
typedef struct
{
    unsigned char src;
    unsigned char flags;
    short         srcoff;
    //    unsigned char object;         // these are now variable length
    //    unsigned short trgoff;
} LXFixup;

#pragma pack(pop)


class DOS4GWBinaryLoader : public IFileLoader
{
public:
    DOS4GWBinaryLoader();
    virtual ~DOS4GWBinaryLoader();

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

    DWord getDelta();

protected:
    SWord dos4gwRead2(const void* src) const; // Read 2 bytes from native addr
    DWord dos4gwRead4(const void* src) const;   // Read 4 bytes from native addr

private:
    LXHeader *m_pLXHeader  = nullptr;     ///< Pointer to lx header
    LXObject *m_pLXObjects = nullptr;     ///< Pointer to lx objects
    int m_cbImage;                        ///< Size of image
    char *base;                           ///< Beginning of the loaded image

    /// Map from address of dynamic pointers to library procedure names:
    IBinarySymbolTable *m_symbols;
    IBinaryImage *m_image;
};
