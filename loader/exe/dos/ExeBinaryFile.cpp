/*
 * Copyright (C) 1997,2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: ExeBinaryFile.cc
 * Desc: This file contains the implementation of the class ExeBinaryFile.
*/

/* EXE binary file format.
        This file implements the class ExeBinaryFile, derived from class BinaryFile.
        See ExeBinaryFile.h and BinaryFile.h for details
        MVE 08/10/97
 * 21 May 02 - Mike: Slight mod for gcc 3.1
*/
#include "ExeBinaryFile.h"

#include "IBoomerang.h"
#include "IBinaryImage.h"

#include <cassert>

ExeBinaryFile::ExeBinaryFile() {
}
void ExeBinaryFile::initialize(IBoomerang *sys) {
    Image = sys->getImage();
    Symbols = sys->getSymbols();
}

bool ExeBinaryFile::RealLoad(const QString &sName) {
    FILE *fp;
    int i, cb;
    Byte buf[4];
    int fCOM;

    m_pFileName = sName;

    // Always just 3 sections
    m_pHeader = new exeHeader;
    if (m_pHeader == nullptr) {
        fprintf(stderr, "Could not allocate header memory\n");
        return 0;
    }

    /* Open the input file */
    if ((fp = fopen(qPrintable(sName), "rb")) == nullptr) {
        fprintf(stderr, "Could not open file %s\n", qPrintable(sName));
        return 0;
    }

    /* Read in first 2 bytes to check EXE signature */
    if (fread(m_pHeader, 1, 2, fp) != 2) {
        fprintf(stderr, "Cannot read file %s\n", qPrintable(sName));
        return 0;
    }

    // Check for the "MZ" exe header
    if (!(fCOM = (m_pHeader->sigLo != 0x4D || m_pHeader->sigHi != 0x5A))) {
        /* Read rest of m_pHeader */
        fseek(fp, 0, SEEK_SET);
        if (fread(m_pHeader, sizeof(exeHeader), 1, fp) != 1) {
            fprintf(stderr, "Cannot read file %s\n", qPrintable(sName));
            return 0;
        }

        /* This is a typical DOS kludge! */
        if (LH(&m_pHeader->relocTabOffset) == 0x40) {
            fprintf(stderr, "Error - NE format executable\n");
            return 0;
        }

        /* Calculate the load module size.
         * This is the number of pages in the file
         * less the length of the m_pHeader and reloc table
         * less the number of bytes unused on last page
         */
        cb = (DWord)LH(&m_pHeader->numPages) * 512 - (DWord)LH(&m_pHeader->numParaHeader) * 16;
        if (m_pHeader->lastPageSize) {
            cb -= 512 - LH(&m_pHeader->lastPageSize);
        }

        /* We quietly ignore minAlloc and maxAlloc since for our
         * purposes it doesn't really matter where in real memory
         * the m_am would end up.  EXE m_ams can't really rely on
         * their load location so setting the PSP segment to 0 is fine.
         * Certainly m_ams that prod around in DOS or BIOS are going
         * to have to load DS from a constant so it'll be pretty
         * obvious.
        */
        m_cReloc = (SWord)LH(&m_pHeader->numReloc);

        /* Allocate the relocation table */
        if (m_cReloc) {
            m_pRelocTable = new DWord[m_cReloc];
            if (m_pRelocTable == nullptr) {
                fprintf(stderr, "Could not allocate relocation table (%d entries)\n",m_cReloc);
                return 0;
            }
            fseek(fp, LH(&m_pHeader->relocTabOffset), SEEK_SET);

            /* Read in seg:offset pairs and convert to Image ptrs */
            for (i = 0; i < m_cReloc; i++) {
                fread(buf, 1, 4, fp);
                m_pRelocTable[i] = LH(buf) + (((int)LH(buf + 2)) << 4);
            }
        }

        /* Seek to start of image */
        fseek(fp, (int)LH(&m_pHeader->numParaHeader) * 16, SEEK_SET);

        // Initial PC and SP. Note that we fake the seg:offset by putting
        // the segment in the top half, and offset int he bottom
        m_uInitPC = ((LH(&m_pHeader->initCS)) << 16) + LH(&m_pHeader->initIP);
        m_uInitSP = ((LH(&m_pHeader->initSS)) << 16) + LH(&m_pHeader->initSP);
    } else {
        /* COM file
         * In this case the load module size is just the file length
         */
        fseek(fp, 0, SEEK_END);
        cb = ftell(fp);

        /* COM programs start off with an ORG 100H (to leave room for a PSP)
                 * This is also the implied start address so if we load the image
                 * at offset 100H addresses should all line up properly again.
                */
        m_uInitPC = 0x100;
        m_uInitSP = 0xFFFE;
        m_cReloc = 0;

        fseek(fp, 0, SEEK_SET);
    }

    /* Allocate a block of memory for the image. */
    m_cbImage = cb;
    m_pImage = new Byte[m_cbImage];

    if (cb != (int)fread(m_pImage, 1, (size_t)cb, fp)) {
        fprintf(stderr, "Cannot read file %s\n", qPrintable(sName));
        return 0;
    }

    /* Relocate segment constants */
    if (m_cReloc) {
        for (i = 0; i < m_cReloc; i++) {
            Byte *p = &m_pImage[m_pRelocTable[i]];
            SWord w = (SWord)LH(p);
            *p++ = (Byte)(w & 0x00FF);
            *p = (Byte)((w & 0xFF00) >> 8);
        }
    }

    fclose(fp);
    //TODO: prevent overlapping of those 3 sections
    IBinarySection *header = Image->createSection("$HEADER",ADDRESS::n(0x4000),ADDRESS::n(0x4000)+sizeof(exeHeader));
    header->setHostAddr(ADDRESS::host_ptr(m_pHeader))
            .setEntrySize(1);
    // The text and data section
    IBinarySection *text = Image->createSection(".text",ADDRESS::n(0x10000),ADDRESS::n(0x10000)+sizeof(m_cbImage));
    text->setCode(true)
            .setData(true)
            .setHostAddr(ADDRESS::host_ptr(m_pImage))
            .setEntrySize(1);
    IBinarySection *reloc = Image->createSection("$RELOC",ADDRESS::n(0x4000)+sizeof(exeHeader),ADDRESS::n(0x4000)+sizeof(exeHeader)+sizeof(DWord) * m_cReloc);
    reloc->setHostAddr(ADDRESS::host_ptr(m_pRelocTable))
            .setEntrySize(sizeof(DWord));
    return 1;
}

// Clean up and unload the binary image
void ExeBinaryFile::UnLoad() {
    delete m_pHeader;
    delete[] m_pImage;
    delete[] m_pRelocTable;
}

// const char *ExeBinaryFile::SymbolByAddress(ADDRESS dwAddr) {
//    if (dwAddr == GetMainEntryPoint())
//        return const_cast<char *>("main");

//    // No symbol table handled at present
//    return nullptr;
//}
bool ExeBinaryFile::DisplayDetails(const char *fileName, FILE *f
                                   /* = stdout */) {
    Q_UNUSED(fileName);
    Q_UNUSED(f);

    return false;
}

LOAD_FMT ExeBinaryFile::GetFormat() const { return LOADFMT_EXE; }

MACHINE ExeBinaryFile::getMachine() const { return MACHINE_PENTIUM; }

void ExeBinaryFile::Close() {
    // Not implemented yet
    return;
}
bool ExeBinaryFile::PostLoad(void *handle) {
    Q_UNUSED(handle);
    // Not needed: for archives only
    return false;
}


ADDRESS ExeBinaryFile::getImageBase() { return ADDRESS::g(0L); /* FIXME */ }

size_t ExeBinaryFile::getImageSize() { return 0; /* FIXME */ }

// Should be doing a search for this
ADDRESS ExeBinaryFile::GetMainEntryPoint() { return NO_ADDRESS; }

ADDRESS ExeBinaryFile::GetEntryPoint() {
    // Check this...
    return ADDRESS::g((LH(&m_pHeader->initCS) << 4) + LH(&m_pHeader->initIP));
}
