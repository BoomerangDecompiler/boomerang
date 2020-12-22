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

#include "boomerang/core/plugin/Plugin.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinarySymbol.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/Log.h"

#include <QBuffer>
#include <QFile>

#include <stdexcept>


namespace
{
struct SectionParam
{
    QString Name;
    Address from;
    size_t Size;
    HostAddress ImageAddress;
    bool Bss, Code, Data, ReadOnly;
};
}

extern "C" int microX86Dis(void *p); // From microX86dis.c

DOS4GWBinaryLoader::DOS4GWBinaryLoader(Project *project)
    : IFileLoader(project)
{
}


DOS4GWBinaryLoader::~DOS4GWBinaryLoader()
{
}


void DOS4GWBinaryLoader::initialize(BinaryFile *file, BinarySymbolTable *symbols)
{
    m_image   = file->getImage();
    m_symbols = symbols;

    file->setBitness(16);
}


void DOS4GWBinaryLoader::close()
{
    unload();
}


Address DOS4GWBinaryLoader::getEntryPoint()
{
    return Address((READ4_LE(m_LXObjects[READ4_LE(m_LXHeader.eipobjectnum)].RelocBaseAddr) +
                    READ4_LE(m_LXHeader.eip)));
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
    unsigned p   = READ4_LE(m_LXHeader.eip);
    unsigned lim = p + 0x300;
    Address addr;

    // unsigned lastOrdCall = 0; //TODO: identify the point of setting this variable
    bool gotSubEbp   = false; // True if see sub ebp, ebp
    bool lastWasCall = false; // True if the last instruction was a call

    BinarySection *textSection = m_image->getSectionByName(
        "seg0"); // Assume the first section is text

    if (textSection == nullptr) {
        textSection = m_image->getSectionByName(".text");
    }

    if (textSection == nullptr) {
        textSection = m_image->getSectionByName("CODE");
    }

    if (textSection == nullptr) {
        LOG_ERROR("Could not find text (code) section!");
        return Address::INVALID;
    }

    const Address nativeOrigin = textSection->getSourceAddr();
    const unsigned textSize    = textSection->getSize();

    if (textSize < 0x300) {
        lim = p + textSize;
    }

    while (p < lim) {
        const Byte op1 = Util::readByte(&m_imageBase[p + 0]);
        const Byte op2 = Util::readByte(&m_imageBase[p + 1]);

        switch (op1) {
        case 0xE8:

            // An ordinary call
            if (gotSubEbp) {
                // This is the call we want. Get the offset from the call instruction
                addr = nativeOrigin + p + 5 + READ4_LE_P(&m_imageBase[p + 1]);
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

        case 0xEB: // Short relative jump

            if (op2 >= 0x80) { // Branch backwards?
                break;         // Yes, just ignore it
            }

            // Otherwise, actually follow the branch. May have to modify this some time...
            p += op2 + 2; // +2 for the instruction itself, and op2 for the displacement
            continue;     // Don't break, we have the new "pc" set already
        }

        int size = microX86Dis(&m_imageBase[p + 0]);

        if (size == 0x40) {
            LOG_WARN("Microdisassembler out of step at offset %1", p);
            size = 1;
        }

        p += size;
    }

    return Address::INVALID;
}


bool DOS4GWBinaryLoader::loadFromMemory(QByteArray &data)
{
    QBuffer buf(&data);

    if (!buf.open(QBuffer::ReadOnly)) {
        return false;
    }

    DWord lxoffLE, lxoff;

    if (!buf.seek(0x3c)) {
        return false;
    }

    buf.read(reinterpret_cast<char *>(&lxoffLE), 4); // Note: peoffLE will be in Little Endian
    lxoff = READ4_LE(lxoffLE);

    if (!buf.seek(lxoff)) {
        return false;
    }

    if (!buf.read(reinterpret_cast<char *>(&m_LXHeader), sizeof(LXHeader))) {
        return false;
    }

    if (!Util::testMagic(&m_LXHeader.sigLo, { 'L', 'X' }) &&
        !Util::testMagic(&m_LXHeader.sigLo, { 'L', 'E' })) {
        LOG_ERROR("Error loading file: bad LE/LX magic");
        return false;
    }

    const DWord objTableOffset = READ4_LE(m_LXHeader.objtbloffset);
    if (!Util::inRange(lxoff, sizeof(LXHeader), buf.size()) ||
        !Util::inRange(objTableOffset, 0U, (DWord)buf.size()) ||
        !Util::inRange(lxoff + objTableOffset, sizeof(LXHeader), buf.size())) {
        LOG_ERROR("Cannot read LX file: Object table extends past file boundary");
        return false;
    }
    else if (!buf.seek(lxoff + READ4_LE(m_LXHeader.objtbloffset))) {
        LOG_ERROR("Cannot read LX object table");
        return false;
    }

    const DWord numObjsInModule = READ4_LE(m_LXHeader.numobjsinmodule);
    if (!Util::inRange(numObjsInModule, 0UL,
                       (buf.size() - lxoff - objTableOffset) / sizeof(LXObject))) {
        LOG_ERROR("Cannot read LX file: Object table extends past file boundary");
        return false;
    }

    m_LXObjects.resize(numObjsInModule);

    buf.read(reinterpret_cast<char *>(&m_LXObjects[0]), numObjsInModule * sizeof(LXObject));

    unsigned npages = 0;
    int cbImage     = 0;

    for (unsigned n = 0; n < numObjsInModule; n++) {
        if (READ4_LE(m_LXObjects[n].ObjectFlags) & 0x40) {
            if (READ4_LE(m_LXObjects[n].PageTblIdx) + READ4_LE(m_LXObjects[n].NumPageTblEntries) -
                    1 >
                npages) {
                npages = READ4_LE(m_LXObjects[n].PageTblIdx) +
                         READ4_LE(m_LXObjects[n].NumPageTblEntries) - 1;
            }

            cbImage = READ4_LE(m_LXObjects[n].RelocBaseAddr) + READ4_LE(m_LXObjects[n].VirtualSize);
        }
    }

    if (numObjsInModule > 0) {
        cbImage -= READ4_LE(m_LXObjects[0].RelocBaseAddr);
    }

    if (!Util::inRange(cbImage, 0, buf.size())) {
        LOG_ERROR("Cannot allocate %1 bytes for copy of LX image", cbImage);
        return false;
    }

    m_imageBase.resize(cbImage);

    uint32_t numSections = READ4_LE(m_LXHeader.numobjsinmodule);
    std::vector<SectionParam> params;

    for (unsigned n = 0; n < numSections; n++) {
        if (READ4_LE(m_LXObjects[n].ObjectFlags) & 0x40) {
            printf("vsize %x reloc %x flags %x page %u npage %u\n",
                   READ4_LE(m_LXObjects[n].VirtualSize), READ4_LE(m_LXObjects[n].RelocBaseAddr),
                   READ4_LE(m_LXObjects[n].ObjectFlags), READ4_LE(m_LXObjects[n].PageTblIdx),
                   READ4_LE(m_LXObjects[n].NumPageTblEntries));

            SectionParam sect;
            DWord Flags = READ4_LE(m_LXObjects[n].ObjectFlags);

            sect.Name         = QString("seg%1").arg(n); // no section names in LX
            sect.from         = Address(READ4_LE(m_LXObjects[n].RelocBaseAddr));
            sect.ImageAddress = HostAddress(&m_imageBase[0]) +
                                (sect.from - params.front().from).value();
            sect.Size     = READ4_LE(m_LXObjects[n].VirtualSize);
            sect.Bss      = 0; // TODO
            sect.Code     = (Flags & 0x4) ? true : false;
            sect.Data     = (Flags & 0x4) ? false : true;
            sect.ReadOnly = (Flags & 0x1) ? false : true;

            buf.seek(m_LXHeader.datapagesoffset +
                     (READ4_LE(m_LXObjects[n].PageTblIdx) - 1) * READ4_LE(m_LXHeader.pagesize));
            char *p = &m_imageBase[0] + READ4_LE(m_LXObjects[n].RelocBaseAddr) -
                      READ4_LE(m_LXObjects[0].RelocBaseAddr);
            buf.read(p, READ4_LE(m_LXObjects[n].NumPageTblEntries) * READ4_LE(m_LXHeader.pagesize));
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
    if (!buf.seek(READ4_LE(m_LXHeader.fixuppagetbloffset) + lxoff)) {
        return false;
    }

    unsigned int *fixuppagetbl = new unsigned int[npages + 1];
    buf.read(reinterpret_cast<char *>(fixuppagetbl), sizeof(unsigned int) * (npages + 1));

    // for (unsigned n = 0; n < npages; n++)
    //    printf("offset for page %i: %x\n", n + 1, fixuppagetbl[n]);
    // printf("offset to end of fixup rec: %x\n", fixuppagetbl[npages]);

    buf.seek(READ4_LE(m_LXHeader.fixuprecordtbloffset) + lxoff);
    LXFixup fixup;
    unsigned srcpage = 0;

    do {
        buf.read(reinterpret_cast<char *>(&fixup), sizeof(fixup));

        if ((fixup.src != 7) || (fixup.flags & ~0x50)) {
            LOG_WARN("Unknown fixup type %1 %2", QString("%1").arg(fixup.src, 2, 16, QChar('0')),
                     QString("%1").arg(fixup.flags, 2, 16, QChar('0')));

            return false;
        }

        // printf("srcpage = %i srcoff = %x object = %02x trgoff = %x\n", srcpage + 1, fixup.srcoff,
        // fixup.object, fixup.trgoff);
        unsigned long src     = srcpage * READ4_LE(m_LXHeader.pagesize) + READ2_LE(fixup.srcoff);
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

        unsigned long target = READ4_LE(m_LXObjects[object - 1].RelocBaseAddr) + READ2_LE(trgoff);
        //        printf("relocate dword at %x to point to %x\n", src, target);
        Util::writeDWord(&m_imageBase[src], target, Endian::Little);

        while (buf.pos() - (READ4_LE(m_LXHeader.fixuprecordtbloffset) + lxoff) >=
               READ4_LE(fixuppagetbl[srcpage + 1])) {
            srcpage++;
        }
    } while (srcpage < npages);

    return true;
}


int DOS4GWBinaryLoader::canLoad(QIODevice &fl) const
{
    LXHeader testHeader;

    if (fl.read(reinterpret_cast<char *>(&testHeader), sizeof(testHeader)) != sizeof(LXHeader)) {
        return 0; // file too small
    }
    else if (!Util::testMagic(&testHeader.sigLo, { 'M', 'Z' })) {
        return 0; // not a DOS-based file
    }

    // read offset 0x3C from beginning of file (pointer to PE header)
    const DWord peOffset = READ4_LE(testHeader.loadersectionchksum);
    if (!fl.seek(peOffset)) {
        return 0; // file corrupted / too small
    }

    Byte buf[2];
    if (fl.read(reinterpret_cast<char *>(buf), 2) != 2) {
        return 0;
    }

    if (Util::testMagic(buf, { 'L', 'E' })) {
        return 2 + 4 + 2; // Win32 VxD (Linear Executable) or DOS4GW app
    }

    return 0; // unrecognized file format
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
    return Machine::X86;
}


DWord DOS4GWBinaryLoader::getDelta()
{
    // Stupid function anyway: delta depends on section
    // This should work for the header only
    //    return (DWord)base - LMMH(m_peHeader->Imagebase);
    return intptr_t(&m_imageBase[0]) - m_LXObjects[0].RelocBaseAddr;
}


BOOMERANG_DEFINE_PLUGIN(PluginType::FileLoader, DOS4GWBinaryLoader, "DOS4GW loader plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
