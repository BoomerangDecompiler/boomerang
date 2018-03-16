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


#include "boomerang/core/IBoomerang.h"
#include "boomerang/loader/IFileLoader.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"

#include <QBuffer>
#include <QFile>

#include <cassert>


ExeBinaryLoader::ExeBinaryLoader()
{
}


void ExeBinaryLoader::initialize(BinaryImage *image, IBinarySymbolTable *symbols)
{
    m_image   = image;
    m_symbols = symbols;
}


bool ExeBinaryLoader::loadFromMemory(QByteArray& data)
{
    if (m_header) {
        delete m_header;
    }

    // Always just 3 sections
    m_header = new ExeHeader;

    QBuffer fp(&data);
    fp.open(QBuffer::ReadOnly);

    /* Read in first 2 bytes to check EXE signature */
    if (fp.read(reinterpret_cast<char *>(m_header), 2) != 2) {
        LOG_ERROR("Cannot read exe file");
        return false;
    }

    int     fCOM;
    int     cb;
    // Check for the "MZ" exe header
    if (!(fCOM = ((m_header->sigLo != 0x4D) || (m_header->sigHi != 0x5A)))) {
        /* Read rest of m_pHeader */
        fp.seek(0);

        if (fp.read(reinterpret_cast<char *>(m_header), sizeof(ExeHeader)) != sizeof(ExeHeader)) {
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
        cb = LH(&m_header->numPages) * 512U - LH(&m_header->numParaHeader) * 16U;

        if (m_header->lastPageSize > 0) {
            cb -= 512U - LH(&m_header->lastPageSize);
        }

        /* We quietly ignore minAlloc and maxAlloc since for our
         * purposes it doesn't really matter where in real memory
         * the m_am would end up.  EXE m_ams can't really rely on
         * their load location so setting the PSP segment to 0 is fine.
         * Certainly m_ams that prod around in DOS or BIOS are going
         * to have to load DS from a constant so it'll be pretty
         * obvious.
         */
        m_numReloc = LH(&m_header->numReloc);

        /* Allocate the relocation table */
        if (m_numReloc) {
            m_relocTable = new DWord[m_numReloc];
            fp.seek(LH(&m_header->relocTabOffset));

            /* Read in seg:offset pairs and convert to Image ptrs */
            Byte    buf[4];
            for (int i = 0; i < m_numReloc; i++) {
                fp.read(reinterpret_cast<char *>(buf), 4);
                m_relocTable[i] = Util::readDWord(buf, false);
            }
        }

        /* Seek to start of image */
        fp.seek(LH(&m_header->numParaHeader) * 16U);

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

    if (cb != fp.read(reinterpret_cast<char *>(m_loadedImage), cb)) {
        LOG_ERROR("Cannot read exe file!");
        return false;
    }

    /* Relocate segment constants */
    if (m_numReloc) {
        for (int i = 0; i < m_numReloc; i++) {
            Byte  *p = &m_loadedImage[m_relocTable[i]];
            Util::writeWord(p, LH(p), false);
        }
    }

    fp.close();

    // TODO: prevent overlapping of those 3 sections
    BinarySection *header = m_image->createSection("$HEADER", Address(0x4000), Address(0x4000) + sizeof(ExeHeader));
    header->setHostAddr(HostAddress(m_header));
    header->setEntrySize(1);

    // The text and data section
    BinarySection *text = m_image->createSection(".text", Address(0x10000), Address(0x10000) + sizeof(m_imageSize));
    text->setCode(true);
    text->setData(true);
    text->setHostAddr(HostAddress(m_loadedImage));
    text->setEntrySize(1);

    BinarySection *reloc = m_image->createSection("$RELOC", Address(0x4000) + sizeof(ExeHeader), Address(0x4000) + sizeof(ExeHeader) + sizeof(DWord) * m_numReloc);
    reloc->setHostAddr(HostAddress(m_relocTable));
    reloc->setEntrySize(sizeof(DWord));
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
    Byte buf[4];
    fl.read(reinterpret_cast<char *>(buf), sizeof(buf));

    if (TESTMAGIC2(buf, 0, 'M', 'Z')) {
        /* DOS-based file */
        return 2;
    }

    return 0;
}


BOOMERANG_LOADER_PLUGIN(ExeBinaryLoader,
                        "DOS Exe loader plugin", BOOMERANG_VERSION, "Boomerang developers")
