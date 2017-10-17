#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExeBinaryLoader.h"


/**
 * \file ExeBinaryLoader.cpp
 * Desc: This file contains the implementation of the class ExeBinaryLoader.
 *       See ExeBinaryLoader.h and BinaryLoader.h for details
 *       MVE 08/10/97
 * 21 May 02 - Mike: Slight mod for gcc 3.1
 */


#include "boomerang/core/IBoomerang.h"
#include "boomerang/loader/IFileLoader.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySection.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"

#include <QBuffer>
#include <QFile>
#include <cassert>

ExeBinaryLoader::ExeBinaryLoader()
{
}


void ExeBinaryLoader::initialize(IBinaryImage *image, IBinarySymbolTable *symbols)
{
    m_image   = image;
    m_symbols = symbols;
}


bool ExeBinaryLoader::loadFromMemory(QByteArray& data)
{
    //
    QBuffer fp(&data);
    int     i, cb;
    Byte    buf[4];
    int     fCOM;


    // Always just 3 sections
    m_header = new ExeHeader;

    fp.open(QBuffer::ReadOnly);

    /* Read in first 2 bytes to check EXE signature */
    if (fp.read((char *)m_header, 2) != 2) {
        LOG_ERROR("Cannot read exe file");
        return false;
    }

    // Check for the "MZ" exe header
    if (!(fCOM = ((m_header->sigLo != 0x4D) || (m_header->sigHi != 0x5A)))) {
        /* Read rest of m_pHeader */
        fp.seek(0);

        if (fp.read((char *)m_header, sizeof(ExeHeader)) != sizeof(ExeHeader)) {
            LOG_ERROR("Cannot read Exe file");
            return false;
        }

        /* This is a typical DOS kludge! */
        if (LH(&m_header->relocTabOffset) == 0x40) {
            LOG_ERROR("File is NE format executable");
            return false;
        }

        /* Calculate the load module size.
         * This is the number of pages in the file
         * less the length of the m_pHeader and reloc table
         * less the number of bytes unused on last page
         */
        cb = (DWord)LH(&m_header->numPages) * 512 - (DWord)LH(&m_header->numParaHeader) * 16;

        if (m_header->lastPageSize) {
            cb -= 512 - LH(&m_header->lastPageSize);
        }

        /* We quietly ignore minAlloc and maxAlloc since for our
         * purposes it doesn't really matter where in real memory
         * the m_am would end up.  EXE m_ams can't really rely on
         * their load location so setting the PSP segment to 0 is fine.
         * Certainly m_ams that prod around in DOS or BIOS are going
         * to have to load DS from a constant so it'll be pretty
         * obvious.
         */
        m_numReloc = (SWord)LH(&m_header->numReloc);

        /* Allocate the relocation table */
        if (m_numReloc) {
            m_relocTable = new DWord[m_numReloc];
            fp.seek(LH(&m_header->relocTabOffset));

            /* Read in seg:offset pairs and convert to Image ptrs */
            for (i = 0; i < m_numReloc; i++) {
                fp.read((char *)buf, 4);
                m_relocTable[i] = LH(buf) + (((int)LH(buf + 2)) << 4);
            }
        }

        /* Seek to start of image */
        fp.seek((int)LH(&m_header->numParaHeader) * 16);

        // Initial PC and SP. Note that we fake the seg:offset by putting
        // the segment in the top half, and offset int he bottom
        m_uInitPC = Address((LH(&m_header->initCS)) << 16) + Address(LH(&m_header->initIP));
        m_uInitSP = Address((LH(&m_header->initSS)) << 16) + Address(LH(&m_header->initSP));
    }
    else {
        /* COM file
         * In this case the load module size is just the file length
         */
        cb = fp.size();

        /* COM programs start off with an ORG 100H (to leave room for a PSP)
         * This is also the implied start address so if we load the image
         * at offset 100H addresses should all line up properly again.
         */
        m_uInitPC  = Address(0x100);
        m_uInitSP  = Address(0xFFFE);
        m_numReloc = 0;

        fp.seek(0);
    }

    /* Allocate a block of memory for the image. */
    m_imageSize   = cb;
    m_loadedImage = new uint8_t[m_imageSize];

    if (cb != fp.read((char *)m_loadedImage, (size_t)cb)) {
        LOG_ERROR("Cannot read exe file!");
        return false;
    }

    /* Relocate segment constants */
    if (m_numReloc) {
        for (i = 0; i < m_numReloc; i++) {
            Byte  *p = &m_loadedImage[m_relocTable[i]];
            SWord w  = (SWord)LH(p);
            *p++ = (Byte)(w & 0x00FF);
            *p   = (Byte)((w & 0xFF00) >> 8);
        }
    }

    fp.close();

    // TODO: prevent overlapping of those 3 sections
    IBinarySection *header = m_image->createSection("$HEADER", Address(0x4000), Address(0x4000) + sizeof(ExeHeader));
    header->setHostAddr(HostAddress(m_header))
       .setEntrySize(1);
    // The text and data section
    IBinarySection *text = m_image->createSection(".text", Address(0x10000), Address(0x10000) + sizeof(m_imageSize));
    text->setCode(true)
       .setData(true)
       .setHostAddr(HostAddress(m_loadedImage))
       .setEntrySize(1);
    IBinarySection *reloc = m_image->createSection("$RELOC", Address(0x4000) + sizeof(ExeHeader), Address(0x4000) + sizeof(ExeHeader) + sizeof(DWord) * m_numReloc);
    reloc->setHostAddr(HostAddress(m_relocTable))
       .setEntrySize(sizeof(DWord));
    return true;
}


// Clean up and unload the binary image
void ExeBinaryLoader::unload()
{
    delete m_header;
    delete[] m_loadedImage;
    delete[] m_relocTable;
}


LoadFmt ExeBinaryLoader::getFormat() const
{
    return LoadFmt::EXE;
}


Machine ExeBinaryLoader::getMachine() const
{
    return Machine::PENTIUM;
}


void ExeBinaryLoader::close()
{
    // Not implemented yet
}


// Should be doing a search for this
Address ExeBinaryLoader::getMainEntryPoint()
{
    return Address::INVALID;
}


Address ExeBinaryLoader::getEntryPoint()
{
    // Check this...
    return Address((LH(&m_header->initCS) << 4) + LH(&m_header->initIP));
}


#define TESTMAGIC2(buf, off, a, b)    (buf[off] == a && buf[off + 1] == b)

int ExeBinaryLoader::canLoad(QIODevice& fl) const
{
    unsigned char buf[4];

    fl.read((char *)buf, sizeof(buf));

    if (TESTMAGIC2(buf, 0, 'M', 'Z')) { /* DOS-based file */
        return 2;
    }

    return 0;
}


BOOMERANG_LOADER_PLUGIN(ExeBinaryLoader,
                        "DOS Exe loader plugin", BOOMERANG_VERSION, "Boomerang developers")
