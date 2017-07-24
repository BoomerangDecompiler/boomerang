/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/**
 * \file DOS4GWBinaryLoader.cpp
 * Desc: This file contains the implementation of the class DOS4GWBinaryLoader.
 * See DOS4GWBinaryLoader.h and BinaryFile.h for details.
 * 24 Jan 05 - Trent: created.
 */

#include "DOS4GWBinaryLoader.h"

#include "boomerang/core/BinaryFileFactory.h"
#include "boomerang/core/IBoomerang.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySymbols.h"
#include "boomerang/db/IBinarySection.h"

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <QBuffer>
#include <QFile>
#include <QDebug>


namespace
{
struct SectionParam
{
    QString Name;
    Address from;
    size_t  Size;
    HostAddress ImageAddress;
    bool    Bss, Code, Data, ReadOnly;
};
}
extern "C" {
int microX86Dis(void *p); // From microX86dis.c
}

DOS4GWBinaryLoader::DOS4GWBinaryLoader()
{
}


DOS4GWBinaryLoader::~DOS4GWBinaryLoader()
{
}


void DOS4GWBinaryLoader::initialize(IBinaryImage *image, IBinarySymbolTable *symbols)
{
    m_image   = image;
    m_symbols = symbols;
}


void DOS4GWBinaryLoader::close()
{
    unload();
}


Address DOS4GWBinaryLoader::getEntryPoint()
{
    return Address((LMMH(m_pLXObjects[LMMH(m_pLXHeader->eipobjectnum)].RelocBaseAddr) + LMMH(m_pLXHeader->eip)));
}


Address DOS4GWBinaryLoader::getMainEntryPoint()
{
    const IBinarySymbol *sym = m_symbols->find("main");

    if (sym) {
        return sym->getLocation();
    }

    sym = m_symbols->find("__CMain");

    if (sym) {
        return sym->getLocation();
    }

    // Search with this crude pattern: call, sub ebp, ebp, call __Cmain in the first 0x300 bytes
    // Start at program entry point
    unsigned      p = LMMH(m_pLXHeader->eip);
    unsigned      lim = p + 0x300;
    unsigned char op1, op2;
       Address       addr;
    // unsigned lastOrdCall = 0; //TODO: identify the point of setting this variable
    bool gotSubEbp   = false;                                   // True if see sub ebp, ebp
    bool lastWasCall = false;                                   // True if the last instruction was a call

    IBinarySection *si = m_image->getSectionInfoByName("seg0"); // Assume the first section is text

    if (si == nullptr) {
        si = m_image->getSectionInfoByName(".text");
    }

    if (si == nullptr) {
        si = m_image->getSectionInfoByName("CODE");
    }

    assert(si);
       Address  nativeOrigin = si->getSourceAddr();
    unsigned textSize     = si->getSize();

    if (textSize < 0x300) {
        lim = p + textSize;
    }

    while (p < lim) {
        op1 = *(unsigned char *)(p + base);
        op2 = *(unsigned char *)(p + base + 1);

        // std::cerr << std::hex << "At " << p << ", ops " << (unsigned)op1 << ", " << (unsigned)op2 << std::dec <<
        // "\n";
        switch (op1)
        {
        case 0xE8:

            // An ordinary call
            if (gotSubEbp) {
                // This is the call we want. Get the offset from the call instruction
                addr = nativeOrigin + p + 5 + LMMH(*(p + base + 1));
                // std::cerr << "__CMain at " << std::hex << addr << "\n";
                return addr;
            }

            // lastOrdCall = p;
            lastWasCall = true;
            break;

        case 0x2B: // 0x2B 0xED is sub ebp,ebp

            if ((op2 == 0xED) && lastWasCall) {
                gotSubEbp = true;
            }

            lastWasCall = false;
            break;

        default:
            gotSubEbp   = false;
            lastWasCall = false;
            break;

        case 0xEB:             // Short relative jump

            if (op2 >= 0x80) { // Branch backwards?
                break;         // Yes, just ignore it
            }

            // Otherwise, actually follow the branch. May have to modify this some time...
            p += op2 + 2; // +2 for the instruction itself, and op2 for the displacement
            continue;     // Don't break, we have the new "pc" set already
        }

        int size = microX86Dis(p + base);

        if (size == 0x40) {
            fprintf(stderr, "Warning! Microdisassembler out of step at offset 0x%x\n", p);
            size = 1;
        }

        p += size;
    }

    return Address::INVALID;
}


bool DOS4GWBinaryLoader::loadFromMemory(QByteArray& data)
{
    QBuffer buf(&data);

    buf.open(QBuffer::ReadOnly);

    DWord lxoffLE, lxoff;

    if (!buf.seek(0x3c)) {
        return false;
    }

    buf.read((char *)&lxoffLE, 4); // Note: peoffLE will be in Little Endian
    lxoff = LMMH(lxoffLE);

    if (!buf.seek(lxoff)) {
        return false;
    }

    m_pLXHeader = new LXHeader;

    if (!buf.read((char *)m_pLXHeader, sizeof(LXHeader))) {
        return false;
    }

    if ((m_pLXHeader->sigLo != 'L') || ((m_pLXHeader->sigHi != 'X') && (m_pLXHeader->sigHi != 'E'))) {
        qWarning() << "error loading file bad LE/LX magic";
        return false;
    }

    if (!buf.seek(lxoff + LMMH(m_pLXHeader->objtbloffset))) {
        return false;
    }

    m_pLXObjects = new LXObject[LMMH(m_pLXHeader->numobjsinmodule)];
    buf.read((char *)m_pLXObjects, sizeof(LXObject) * LMMH(m_pLXHeader->numobjsinmodule));

    unsigned npages = 0;
    m_cbImage = 0;

    for (unsigned n = 0; n < LMMH(m_pLXHeader->numobjsinmodule); n++) {
        if (LMMH(m_pLXObjects[n].ObjectFlags) & 0x40) {
            if (LMMH(m_pLXObjects[n].PageTblIdx) + LMMH(m_pLXObjects[n].NumPageTblEntries) - 1 > npages) {
                npages = LMMH(m_pLXObjects[n].PageTblIdx) + LMMH(m_pLXObjects[n].NumPageTblEntries) - 1;
            }

            m_cbImage = LMMH(m_pLXObjects[n].RelocBaseAddr) + LMMH(m_pLXObjects[n].VirtualSize);
        }
    }

    m_cbImage -= LMMH(m_pLXObjects[0].RelocBaseAddr);

    base = (char *)malloc(m_cbImage);

    uint32_t numSections = LMMH(m_pLXHeader->numobjsinmodule);
    std::vector<SectionParam> params;

    for (unsigned n = 0; n < numSections; n++) {
        if (LMMH(m_pLXObjects[n].ObjectFlags) & 0x40) {
            printf("vsize %x reloc %x flags %x page %u npage %u\n", LMMH(m_pLXObjects[n].VirtualSize),
                   LMMH(m_pLXObjects[n].RelocBaseAddr), LMMH(m_pLXObjects[n].ObjectFlags),
                   LMMH(m_pLXObjects[n].PageTblIdx), LMMH(m_pLXObjects[n].NumPageTblEntries));

            SectionParam sect;
            DWord Flags = LMMH(m_pLXObjects[n].ObjectFlags);

            sect.Name         = QString("seg%i").arg(n); // no section names in LX
            sect.from         = Address(LMMH(m_pLXObjects[n].RelocBaseAddr));
            sect.ImageAddress = HostAddress(base) + (sect.from - params.front().from).value();
            sect.Size         = LMMH(m_pLXObjects[n].VirtualSize);
            sect.Bss      = 0; // TODO
            sect.Code     = Flags & 0x4 ? 1 : 0;
            sect.Data     = Flags & 0x4 ? 0 : 1;
            sect.ReadOnly = Flags & 0x1 ? 0 : 1;
            buf.seek(
                m_pLXHeader->datapagesoffset + (LMMH(m_pLXObjects[n].PageTblIdx) - 1) * LMMH(m_pLXHeader->pagesize)
                );
            char *p = base + LMMH(m_pLXObjects[n].RelocBaseAddr) - LMMH(m_pLXObjects[0].RelocBaseAddr);
            buf.read(p, LMMH(m_pLXObjects[n].NumPageTblEntries) * LMMH(m_pLXHeader->pagesize));
        }
    }

    for (SectionParam par : params) {
        IBinarySection *sect = m_image->createSection(par.Name, par.from, par.from + par.Size);

        if (sect) {
            sect->setBss(par.Bss)
               .setCode(par.Code)
               .setData(par.Data)
               .setReadOnly(par.ReadOnly)
               .setHostAddr(par.ImageAddress);
        }
    }

    // TODO: decode entry tables

    // fixups
    if (!buf.seek(LMMH(m_pLXHeader->fixuppagetbloffset) + lxoff)) {
        return false;
    }

    unsigned int *fixuppagetbl = new unsigned int[npages + 1];
    buf.read((char *)fixuppagetbl, sizeof(unsigned int) * (npages + 1));

    // for (unsigned n = 0; n < npages; n++)
    //    printf("offset for page %i: %x\n", n + 1, fixuppagetbl[n]);
    // printf("offset to end of fixup rec: %x\n", fixuppagetbl[npages]);

    buf.seek(LMMH(m_pLXHeader->fixuprecordtbloffset) + lxoff);
    LXFixup  fixup;
    unsigned srcpage = 0;

    do {
        buf.read((char *)&fixup, sizeof(fixup));

        if ((fixup.src != 7) || (fixup.flags & ~0x50)) {
            qWarning() << QString("unknown fixup type %1 %2").arg(fixup.src, 2, 16, QChar('0'))
                .arg(fixup.flags, 2, 16, QChar('0'));
            return false;
        }

        // printf("srcpage = %i srcoff = %x object = %02x trgoff = %x\n", srcpage + 1, fixup.srcoff, fixup.object,
        // fixup.trgoff);
        unsigned long  src    = srcpage * LMMH(m_pLXHeader->pagesize) + (short)LMMHw(fixup.srcoff);
        unsigned short object = 0;

        if (fixup.flags & 0x40) {
            buf.read((char *)&object, 2);
        }
        else {
            buf.read((char *)&object, 1);
        }

        unsigned int trgoff = 0;

        if (fixup.flags & 0x10) {
            buf.read((char *)&trgoff, 4);
        }
        else {
            buf.read((char *)&trgoff, 2);
        }

        unsigned long target = LMMH(m_pLXObjects[object - 1].RelocBaseAddr) + LMMHw(trgoff);
        //        printf("relocate dword at %x to point to %x\n", src, target);
        *(unsigned int *)(base + src) = target;

        while (buf.pos() - (LMMH(m_pLXHeader->fixuprecordtbloffset) + lxoff) >= LMMH(fixuppagetbl[srcpage + 1])) {
            srcpage++;
        }
    } while (srcpage < npages);

    return true;
}


#define TESTMAGIC2(buf, off, a, b)          (buf[off] == a && buf[off + 1] == b)

int DOS4GWBinaryLoader::canLoad(QIODevice& fl) const
{
    unsigned char buf[64];

    fl.read((char *)buf, sizeof(buf));

    if (TESTMAGIC2(buf, 0, 'M', 'Z')) { /* DOS-based file */
        int peoff = LMMH(buf[0x3C]);

        if ((peoff != 0) && fl.seek(peoff)) {
            fl.read((char *)buf, 4);

            if (TESTMAGIC2(buf, 0, 'L', 'E')) {
                /* Win32 VxD (Linear Executable) or DOS4GW app */
                return 2 + 4 + 2;
            }
        }
    }

    return 0;
}


// Clean up and unload the binary image
void DOS4GWBinaryLoader::unload()
{
}


SWord DOS4GWBinaryLoader::dos4gwRead2(const void* src) const
{
    return Util::readWord(src, false);
}


DWord DOS4GWBinaryLoader::dos4gwRead4(const void* src) const
{
    return Util::readDWord(src, false);
}


LoadFmt DOS4GWBinaryLoader::getFormat() const
{
    return LoadFmt::LX;
}


Machine DOS4GWBinaryLoader::getMachine() const
{
    return Machine::PENTIUM;
}


DWord DOS4GWBinaryLoader::getDelta()
{
    // Stupid function anyway: delta depends on section
    // This should work for the header only
    //    return (DWord)base - LMMH(m_pPEHeader->Imagebase);
    return intptr_t(base) - m_pLXObjects[0].RelocBaseAddr;
}


BOOMERANG_LOADER_PLUGIN(DOS4GWBinaryLoader,
    "DOS4GW binary loader plugin", BOOMERANG_VERSION, "Boomerang developers")
