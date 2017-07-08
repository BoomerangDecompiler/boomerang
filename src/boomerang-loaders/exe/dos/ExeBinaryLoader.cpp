/*
 * Copyright (C) 1997,2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/**
 * \file ExeBinaryLoader.cpp
 * Desc: This file contains the implementation of the class ExeBinaryLoader.
 *       See ExeBinaryLoader.h and BinaryLoader.h for details
 *       MVE 08/10/97
 * 21 May 02 - Mike: Slight mod for gcc 3.1
 */

#include "ExeBinaryLoader.h"

#include "boomerang/include/IBoomerang.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/loader/IFileLoader.h"
#include "boomerang/db/IBinarySection.h"

#include <QBuffer>
#include <QFile>
#include <cassert>
#include <QDebug>


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
		qWarning() << "Cannot read file ";
		return false;
	}

	// Check for the "MZ" exe header
	if (!(fCOM = ((m_header->sigLo != 0x4D) || (m_header->sigHi != 0x5A)))) {
		/* Read rest of m_pHeader */
		fp.seek(0);

		if (fp.read((char *)m_header, sizeof(ExeHeader)) != sizeof(ExeHeader)) {
			qWarning() << "Cannot read file ";
			return false;
		}

		/* This is a typical DOS kludge! */
		if (LH(&m_header->relocTabOffset) == 0x40) {
			qWarning() << "Error - NE format executable";
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
		qWarning() << "Cannot read file ";
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
	IBinarySection *header = m_image->createSection("$HEADER", Address::n(0x4000), Address::n(0x4000) + sizeof(ExeHeader));
	header->setHostAddr(Address::host_ptr(m_header))
	   .setEntrySize(1);
	// The text and data section
	IBinarySection *text = m_image->createSection(".text", Address::n(0x10000), Address::n(0x10000) + sizeof(m_imageSize));
	text->setCode(true)
	   .setData(true)
	   .setHostAddr(Address::host_ptr(m_loadedImage))
	   .setEntrySize(1);
	IBinarySection *reloc = m_image->createSection("$RELOC", Address::n(0x4000) + sizeof(ExeHeader), Address::n(0x4000) + sizeof(ExeHeader) + sizeof(DWord) * m_numReloc);
	reloc->setHostAddr(Address::host_ptr(m_relocTable))
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


// const char *ExeBinaryFile::SymbolByAddress(ADDRESS dwAddr) {
//    if (dwAddr == GetMainEntryPoint())
//        return const_cast<char *>("main");

//    // No symbol table handled at present
//    return nullptr;
// }
bool ExeBinaryLoader::displayDetails(const char *fileName, FILE *f
                                     /* = stdout */)
{
	Q_UNUSED(fileName);
	Q_UNUSED(f);

	return false;
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


bool ExeBinaryLoader::postLoad(void *handle)
{
	Q_UNUSED(handle);
	// Not needed: for archives only
	return false;
}


Address ExeBinaryLoader::getImageBase()
{
	return Address::g(0L);                                     /* FIXME */
}


size_t ExeBinaryLoader::getImageSize()
{
	return 0;                                    /* FIXME */
}


// Should be doing a search for this
Address ExeBinaryLoader::getMainEntryPoint()
{
	return NO_ADDRESS;
}


Address ExeBinaryLoader::getEntryPoint()
{
	// Check this...
	return Address::g((LH(&m_header->initCS) << 4) + LH(&m_header->initIP));
}


#define TESTMAGIC2(buf, off, a, b)    (buf[off] == a && buf[off + 1] == b)
#define LMMH(x)																								  \
	((unsigned)((Byte *)(&x))[0] + ((unsigned)((Byte *)(&x))[1] << 8) + ((unsigned)((Byte *)(&x))[2] << 16) + \
	 ((unsigned)((Byte *)(&x))[3] << 24))

int ExeBinaryLoader::canLoad(QIODevice& fl) const
{
	unsigned char buf[4];

	fl.read((char *)buf, sizeof(buf));

	if (TESTMAGIC2(buf, 0, 'M', 'Z')) { /* DOS-based file */
		return 2;
	}

	return 0;
}


DEFINE_PLUGIN(PluginType::Loader, IFileLoader, ExeBinaryLoader,
			  "DOS Exe loader plugin", "0.4.0", "Boomerang developers")
