#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DOS4GWBinaryLoader.h"


#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/util/Log.h"

#include <QBuffer>
#include <QFile>

#include <cassert>
#include <cstring>
#include <cstdlib>


namespace
{
struct SectionParam
{
    QString     Name;
    Address     from;
    size_t      Size;
    HostAddress ImageAddress;
    bool        Bss, Code, Data, ReadOnly;
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


void DOS4GWBinaryLoader::initialize(BinaryImage *image, BinarySymbolTable *symbols)
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
    return Address((LMMH(m_LXObjects[LMMH(m_LXHeader->eipobjectnum)].RelocBaseAddr) + LMMH(m_LXHeader->eip)));
}


Address DOS4GWBinaryLoader::getMainEntryPoint()
{
    const BinarySymbol *sym = m_symbols->findSymbolByName("main");

    if (sym) {
        return sym->getLocation();
    }

    sym = m_symbols->findSymbolByName("__CMain");

    if (sym) {
        return sym->getLocation();
    }

    // Search with this crude pattern: call, sub ebp, ebp, call __Cmain in the first 0x300 bytes
    // Start at program entry point
    unsigned      p = LMMH(m_LXHeader->eip);
    unsigned      lim = p + 0x300;
    Address       addr;

    // unsigned lastOrdCall = 0; //TODO: identify the point of setting this variable
    bool gotSubEbp   = false;                               // True if see sub ebp, ebp
    bool lastWasCall = false;                               // True if the last instruction was a call

    BinarySection *si = m_image->getSectionByName("seg0"); // Assume the first section is text

    if (si == nullptr) {
        si = m_image->getSectionByName(".text");
    }

    if (si == nullptr) {
        si = m_image->getSectionByName("CODE");
    }

    assert(si);
    Address  nativeOrigin = si->getSourceAddr();
    unsigned textSize     = si->getSize();

    if (textSize < 0x300) {
        lim = p + textSize;
    }

    while (p < lim) {
        const Byte op1 = *reinterpret_cast<const Byte *>(base + p + 0);
        const Byte op2 = *reinterpret_cast<const Byte *>(base + p + 1);

        switch (op1)
        {
        case 0xE8:

            // An ordinary call
            if (gotSubEbp) {
                // This is the call we want. Get the offset from the call instruction
                addr = nativeOrigin + p + 5 + LMMH2(base + p + 1);
                LOG_VERBOSE("Found __CMain at address %1", addr);
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
            LOG_WARN("Microdisassembler out of step at offset %1", p);
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

    buf.read(reinterpret_cast<char *>(&lxoffLE), 4); // Note: peoffLE will be in Little Endian
    lxoff = LMMH(lxoffLE);

    if (!buf.seek(lxoff)) {
        return false;
    }

    m_LXHeader = new LXHeader;

    if (!buf.read(reinterpret_cast<char *>(m_LXHeader), sizeof(LXHeader))) {
        return false;
    }

    if ((m_LXHeader->sigLo != 'L') || ((m_LXHeader->sigHi != 'X') && (m_LXHeader->sigHi != 'E'))) {
        LOG_ERROR("Error loading file: bad LE/LX magic");
        return false;
    }

    if (!buf.seek(lxoff + LMMH(m_LXHeader->objtbloffset))) {
        return false;
    }

    m_LXObjects = new LXObject[LMMH(m_LXHeader->numobjsinmodule)];
    buf.read(reinterpret_cast<char *>(m_LXObjects), sizeof(LXObject) * LMMH(m_LXHeader->numobjsinmodule));

    unsigned npages = 0;
    m_cbImage = 0;

    for (unsigned n = 0; n < LMMH(m_LXHeader->numobjsinmodule); n++) {
        if (LMMH(m_LXObjects[n].ObjectFlags) & 0x40) {
            if (LMMH(m_LXObjects[n].PageTblIdx) + LMMH(m_LXObjects[n].NumPageTblEntries) - 1 > npages) {
                npages = LMMH(m_LXObjects[n].PageTblIdx) + LMMH(m_LXObjects[n].NumPageTblEntries) - 1;
            }

            m_cbImage = LMMH(m_LXObjects[n].RelocBaseAddr) + LMMH(m_LXObjects[n].VirtualSize);
        }
    }

    m_cbImage -= LMMH(m_LXObjects[0].RelocBaseAddr);

    base = reinterpret_cast<char *>(malloc(m_cbImage));

    uint32_t numSections = LMMH(m_LXHeader->numobjsinmodule);
    std::vector<SectionParam> params;

    for (unsigned n = 0; n < numSections; n++) {
        if (LMMH(m_LXObjects[n].ObjectFlags) & 0x40) {
            printf("vsize %x reloc %x flags %x page %u npage %u\n", LMMH(m_LXObjects[n].VirtualSize),
                   LMMH(m_LXObjects[n].RelocBaseAddr), LMMH(m_LXObjects[n].ObjectFlags),
                   LMMH(m_LXObjects[n].PageTblIdx), LMMH(m_LXObjects[n].NumPageTblEntries));

            SectionParam sect;
            DWord        Flags = LMMH(m_LXObjects[n].ObjectFlags);

            sect.Name         = QString("seg%i").arg(n); // no section names in LX
            sect.from         = Address(LMMH(m_LXObjects[n].RelocBaseAddr));
            sect.ImageAddress = HostAddress(base) + (sect.from - params.front().from).value();
            sect.Size         = LMMH(m_LXObjects[n].VirtualSize);
            sect.Bss          = 0; // TODO
            sect.Code         = (Flags & 0x4) ? true  : false;
            sect.Data         = (Flags & 0x4) ? false : true;
            sect.ReadOnly     = (Flags & 0x1) ? false : true;

            buf.seek(
                m_LXHeader->datapagesoffset + (LMMH(m_LXObjects[n].PageTblIdx) - 1) * LMMH(m_LXHeader->pagesize)
                );
            char *p = base + LMMH(m_LXObjects[n].RelocBaseAddr) - LMMH(m_LXObjects[0].RelocBaseAddr);
            buf.read(p, LMMH(m_LXObjects[n].NumPageTblEntries) * LMMH(m_LXHeader->pagesize));
        }
    }

    for (SectionParam par : params) {
        BinarySection *sect = m_image->createSection(par.Name, par.from, par.from + par.Size);

        if (sect) {
            sect->setBss(par.Bss);
            sect->setCode(par.Code);
            sect->setData(par.Data);
            sect->setReadOnly(par.ReadOnly);
            sect->setHostAddr(par.ImageAddress);
        }
    }

    // TODO: decode entry tables

    // fixups
    if (!buf.seek(LMMH(m_LXHeader->fixuppagetbloffset) + lxoff)) {
        return false;
    }

    unsigned int *fixuppagetbl = new unsigned int[npages + 1];
    buf.read(reinterpret_cast<char *>(fixuppagetbl), sizeof(unsigned int) * (npages + 1));

    // for (unsigned n = 0; n < npages; n++)
    //    printf("offset for page %i: %x\n", n + 1, fixuppagetbl[n]);
    // printf("offset to end of fixup rec: %x\n", fixuppagetbl[npages]);

    buf.seek(LMMH(m_LXHeader->fixuprecordtbloffset) + lxoff);
    LXFixup  fixup;
    unsigned srcpage = 0;

    do {
        buf.read(reinterpret_cast<char *>(&fixup), sizeof(fixup));

        if ((fixup.src != 7) || (fixup.flags & ~0x50)) {
            LOG_WARN("Unknown fixup type %1 %2",
                     QString("%1").arg(fixup.src,   2, 16, QChar('0')),
                     QString("%1").arg(fixup.flags, 2, 16, QChar('0')));

            return false;
        }

        // printf("srcpage = %i srcoff = %x object = %02x trgoff = %x\n", srcpage + 1, fixup.srcoff, fixup.object,
        // fixup.trgoff);
        unsigned long  src    = srcpage * LMMH(m_LXHeader->pagesize) + LMMHw(fixup.srcoff);
        unsigned short object = 0;

        if (fixup.flags & 0x40) {
            buf.read(reinterpret_cast<char *>(&object), 2);
        }
        else {
            buf.read(reinterpret_cast<char *>(&object), 1);
        }

        unsigned int trgoff = 0;

        if (fixup.flags & 0x10) {
            buf.read(reinterpret_cast<char *>(&trgoff), 4);
        }
        else {
            buf.read(reinterpret_cast<char *>(&trgoff), 2);
        }

        unsigned long target = LMMH(m_LXObjects[object - 1].RelocBaseAddr) + LMMHw(trgoff);
        //        printf("relocate dword at %x to point to %x\n", src, target);
        Util::writeDWord(base + src, target, Endian::Little);

        while (buf.pos() - (LMMH(m_LXHeader->fixuprecordtbloffset) + lxoff) >= LMMH(fixuppagetbl[srcpage + 1])) {
            srcpage++;
        }
    } while (srcpage < npages);

    return true;
}


#define TESTMAGIC2(buf, off, a, b)    (buf[off] == a && buf[off + 1] == b)

int DOS4GWBinaryLoader::canLoad(QIODevice& fl) const
{
    unsigned char buf[64];

    fl.read(reinterpret_cast<char *>(buf), sizeof(buf));

    if (TESTMAGIC2(buf, 0, 'M', 'Z')) { /* DOS-based file */
        int peoff = LMMH(buf[0x3C]);

        if ((peoff != 0) && fl.seek(peoff)) {
            fl.read(reinterpret_cast<char *>(buf), 4);

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


SWord DOS4GWBinaryLoader::dos4gwRead2(const void *src) const
{
    return Util::readWord(src, Endian::Little);
}


DWord DOS4GWBinaryLoader::dos4gwRead4(const void *src) const
{
    return Util::readDWord(src, Endian::Little);
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
    //    return (DWord)base - LMMH(m_peHeader->Imagebase);
    return intptr_t(base) - m_LXObjects[0].RelocBaseAddr;
}


BOOMERANG_LOADER_PLUGIN(DOS4GWBinaryLoader,
                        "DOS4GW binary loader plugin", BOOMERANG_VERSION, "Boomerang developers")
