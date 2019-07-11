#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/ifc/IFileLoader.h"

#include <string>

class QFile;


typedef struct /* exe file header, just the signature really */
{
    Byte sigLo; /* .EXE signature: 0x4D 0x5A     */
    Byte sigHi;
} Header;

// The following structures should have their members byte aligned
#pragma pack(push, 1)
typedef struct
{
    Byte sigLo;
    Byte sigHi;
    Byte byteord;
    Byte wordord;
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
    short srcoff;
    //    unsigned char object;         // these are now variable length
    //    unsigned short trgoff;
} LXFixup;

#pragma pack(pop)


/**
 * Loader for DOS/4GW executable files.
 * At present, this loader supports the OS2 file format (also known as
 * the Linear eXecutable format) as much as I've found necessary to
 * inspect old DOS4GW apps.  This loader could also be used for decompiling
 * Win9x VxD files or, of course, OS2 binaries, but you're probably better off
 * making a specific loader for each of these.
 */
class BOOMERANG_PLUGIN_API DOS4GWBinaryLoader : public IFileLoader
{
public:
    DOS4GWBinaryLoader(Project *project);
    virtual ~DOS4GWBinaryLoader();

public:
    /// \copydoc IFileLoader::initialize
    void initialize(BinaryFile *file, BinarySymbolTable *symbols) override;

    /// \copydoc IFileLoader::canLoad
    int canLoad(QIODevice &fl) const override;

    /// \copydoc IFileLoader::loadFromMemory
    bool loadFromMemory(QByteArray &data) override;

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
    SWord dos4gwRead2(const void *src) const; ///< Read 2 bytes from native addr
    DWord dos4gwRead4(const void *src) const; ///< Read 4 bytes from native addr

private:
    LXHeader m_LXHeader = {};          ///< LX header
    std::vector<LXObject> m_LXObjects; ///< LX objects
    std::vector<char> m_imageBase;     ///< LoadedImage

    /// Map from address of dynamic pointers to library procedure names:
    BinarySymbolTable *m_symbols = nullptr;
    BinaryImage *m_image         = nullptr;
};
