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


class QFile;


#pragma pack(push, 1)
struct PSP
{
    SWord int20h;       ///< interrupt 20h
    SWord eof;          ///< segment, end of allocation block
    Byte res1;          ///< reserved
    Byte dosDisp[5];    ///< far call to DOS function dispatcher
    Byte int22h[4];     ///< vector for terminate routine
    Byte int23h[4];     ///< vector for ctrl+break routine
    Byte int24h[4];     ///< vector for error routine
    Byte res2[22];      ///< reserved
    SWord segEnv;       ///< segment address of environment block
    Byte res3[34];      ///< reserved
    Byte int21h[6];     ///< opcode for int21h and far return
    Byte res4[6];       ///< reserved
    Byte fcb1[16];      ///< default file control block 1
    Byte fcb2[16];      ///< default file control block 2
    Byte res5[4];       ///< reserved
    Byte cmdTail[0x80]; ///< command tail and disk transfer area
};

/// EXE file header
struct ExeHeader
{
    Byte sigLo;           ///< 0x00     .EXE signature: 0x4D 0x5A
    Byte sigHi;           ///< 0x01
    SWord lastPageSize;   ///< 0x02     Size of the last page (mod 512 bytes)
    SWord numPages;       ///< 0x04     Number of (512-byte) pages in the file
    SWord numReloc;       ///< 0x06     Number of relocation items
    SWord numParaHeader;  ///< 0x08     # of paragraphs in the header
    SWord minAlloc;       ///< 0x0A     Minimum number of paragraphs
    SWord maxAlloc;       ///< 0x0C     Maximum number of paragraphs (usually 0xFFFF)
    SWord initSS;         ///< 0x0E     Segment displacement of stack
    SWord initSP;         ///< 0x10     Contents of SP at entry
    SWord checkSum;       ///< 0x12     Complemented checksum
    SWord initIP;         ///< 0x14     Contents of IP at entry
    SWord initCS;         ///< 0x16     Segment displacement of code
    SWord relocTabOffset; ///< 0x18     Relocation table offset
    SWord overlayNum;     ///< 0x1A     Overlay number
};
#pragma pack(pop)


/**
 * A loader for DOS executable files.
 *
 * At present, there is no support for a symbol table. Exe files do
 * not use dynamic linking, but it is possible that some files may
 * have debug symbols (in Microsoft Codeview or Borland formats),
 * and these may be implemented in the future. The debug info may
 * even be exposed as another pseudo section.
 */
class BOOMERANG_PLUGIN_API ExeBinaryLoader : public IFileLoader
{
public:
    ExeBinaryLoader(Project *project);

public:
    /// \copydoc IFileLoader::initialize
    void initialize(BinaryImage *image, BinarySymbolTable *symbols) override;

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

private:
    ExeHeader *m_header = nullptr;        ///< Pointer to header
    Byte *m_loadedImage = nullptr;        ///< Pointer to image buffer
    int m_imageSize     = 0;              ///< Size of image
    int m_numReloc      = 0;              ///< Number of relocation entries
    std::vector<DWord> m_relocTable;      ///< The relocation table
    Address m_uInitPC = Address::INVALID; ///< Initial program counter
    Address m_uInitSP = Address::INVALID; ///< Initial stack pointer

    BinaryImage *m_image         = nullptr;
    BinarySymbolTable *m_symbols = nullptr;
};
