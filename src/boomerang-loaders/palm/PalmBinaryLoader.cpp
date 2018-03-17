#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PalmBinaryLoader.h"


#include "palmsystraps.h"

#include "boomerang/core/IBoomerang.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/util/Log.h"

#include <cassert>
#include <cstring>
#include <cstdlib>


// Macro to convert a pointer to a Big Endian integer into a host integer
#define UINT4(p)        Util::readDWord(p, true)
#define UINT4ADDR(p)    Util::readDWord(reinterpret_cast<const void *>((p).value()), true)


PalmBinaryLoader::PalmBinaryLoader()
    : m_image(nullptr)
    , m_data(nullptr)
{
}


PalmBinaryLoader::~PalmBinaryLoader()
{
    m_image = nullptr;
    delete[] m_data;
}


void PalmBinaryLoader::initialize(BinaryImage *image, BinarySymbolTable *symbols)
{
    m_binaryImage = image;
    m_symbols     = symbols;
}


namespace
{
struct SectionParams
{
    QString     name;
    Address     from, to;
    HostAddress hostAddr;
};
}

bool PalmBinaryLoader::loadFromMemory(QByteArray& img)
{
    long size = img.size();

    m_image = reinterpret_cast<uint8_t *>(img.data());

    // Check type at offset 0x3C; should be "appl" (or "palm"; ugh!)
    if ((strncmp(img.data() + 0x3C, "appl", 4) != 0) &&
        (strncmp(img.data() + 0x3C, "panl", 4) != 0) &&
        (strncmp(img.data() + 0x3C, "libr", 4) != 0)) {
            LOG_ERROR("This is not a standard .prc file");
            return false;
    }

    addTrapSymbols();
    // Get the number of resource headers (one section per resource)

    uint32_t numSections = (m_image[0x4C] << 8) + m_image[0x4D];

    // Iterate through the resource headers (generating section info structs)
    unsigned char              *p  = m_image + 0x4E; // First resource header
    unsigned                   off = 0;
    std::vector<SectionParams> params;

    for (unsigned i = 0; i < numSections; i++) {
        // Now get the identifier (2 byte binary)
        unsigned   id = (p[4] << 8) + p[5];
        QByteArray qba(reinterpret_cast<char *>(p), 4);
        // First the name (4 alphanumeric characters from p to p+3)
        // Join the id to the name, e.g. code0, data12
        QString name = QString("%1%2").arg(QString(qba)).arg(id);

        p  += 4 + 2;
        off = UINT4(p);
        p  += 4;

        Address start_addr(off);

        // Guess the length
        if (i > 0) {
            params.back().to = start_addr;
        }

        params.push_back({ name, start_addr, Address::INVALID, HostAddress(m_image + off) }); // Address::INVALID will be overwritten
    }

    // Set the length for the last section
    params.back().to = params.back().from + size - off;

    for (SectionParams param : params) {
        assert(param.to != Address::INVALID);
        BinarySection *sect = m_binaryImage->createSection(param.name, param.from, param.to);

        if (sect) {
            // Decide if code or data; note that code0 is a special case (not code)
            sect->setHostAddr(param.hostAddr);
            sect->setCode((param.name != "code0") && (param.name.startsWith("code")));
            sect->setData(param.name.startsWith("data"));
            sect->setEndian(0);                          // little endian
            sect->setEntrySize(1);                       // No info available
            sect->addDefinedArea(param.from, param.to); // no BSS
        }
    }

    // Create a separate, uncompressed, initialised data section
    BinarySection *dataSection = m_binaryImage->getSectionByName("data0");

    if (dataSection == nullptr) {
        LOG_ERROR("No data section found!");
        return false;
    }

    const BinarySection *code0Section = m_binaryImage->getSectionByName("code0");

    if (code0Section == nullptr) {
        LOG_ERROR("No code 0 section found!");
        return false;
    }

    // When the info is all boiled down, the two things we need from the
    // code 0 section are at offset 0, the size of data above a5, and at
    // offset 4, the size below. Save the size below as a member variable
    m_sizeBelowA5 = UINT4ADDR(code0Section->getHostAddr() + 4);

    // Total size is this plus the amount above (>=) a5
    unsigned sizeData = m_sizeBelowA5 + UINT4ADDR(code0Section->getHostAddr());

    // Allocate a new data section
    m_data = new unsigned char[sizeData];

    if (m_data == nullptr) {
        LOG_FATAL("Could not allocate %1 bytes for data section", sizeData);
    }

    // Uncompress the data. Skip first long (offset of CODE1 "xrefs")
    p = reinterpret_cast<unsigned char *>((dataSection->getHostAddr() + 4).value());
    int start = static_cast<int>(UINT4(p));
    p += 4;
    unsigned char *q   = (m_data + m_sizeBelowA5 + start);
    bool          done = false;

    while (!done && (p < reinterpret_cast<unsigned char *>((dataSection->getHostAddr() + dataSection->getSize()).value()))) {
        unsigned char rle = *p++;

        if (rle == 0) {
            done = true;
            break;
        }
        else if (rle == 1) {
            // 0x01 b_0 b_1
            // => 0x00 0x00 0x00 0x00 0xFF 0xFF b_0 b_1
            *q++ = 0x00;
            *q++ = 0x00;
            *q++ = 0x00;
            *q++ = 0x00;
            *q++ = 0xFF;
            *q++ = 0xFF;
            *q++ = *p++;
            *q++ = *p++;
        }
        else if (rle == 2) {
            // 0x02 b_0 b_1 b_2
            // => 0x00 0x00 0x00 0x00 0xFF b_0 b_1 b_2
            *q++ = 0x00;
            *q++ = 0x00;
            *q++ = 0x00;
            *q++ = 0x00;
            *q++ = 0xFF;
            *q++ = *p++;
            *q++ = *p++;
            *q++ = *p++;
        }
        else if (rle == 3) {
            // 0x03 b_0 b_1 b_2
            // => 0xA9 0xF0 0x00 0x00 b_0 b_1 0x00 b_2
            *q++ = 0xA9;
            *q++ = 0xF0;
            *q++ = 0x00;
            *q++ = 0x00;
            *q++ = *p++;
            *q++ = *p++;
            *q++ = 0x00;
            *q++ = *p++;
        }
        else if (rle == 4) {
            // 0x04 b_0 b_1 b_2 b_3
            // => 0xA9 axF0 0x00 b_0 b_1 b_3 0x00 b_3
            *q++ = 0xA9;
            *q++ = 0xF0;
            *q++ = 0x00;
            *q++ = *p++;
            *q++ = *p++;
            *q++ = *p++;
            *q++ = 0x00;
            *q++ = *p++;
        }
        else if (rle < 0x10) {
            // 5-0xF are invalid.
            assert(false);
        }
        else if (rle >= 0x80) {
            // n+1 bytes of literal data
            for (int k = 0; k <= (rle - 0x80); k++) {
                *q++ = *p++;
            }
        }
        else if (rle >= 40) {
            // n+1 repetitions of 0
            for (int k = 0; k <= (rle - 0x40); k++) {
                *q++ = 0x00;
            }
        }
        else if (rle >= 20) {
            // n+2 repetitions of b
            unsigned char b = *p++;

            for (int k = 0; k < (rle - 0x20 + 2); k++) {
                *q++ = b;
            }
        }
        else {
            // 0x10: n+1 repetitions of 0xFF
            for (int k = 0; k <= (rle - 0x10); k++) {
                *q++ = 0xFF;
            }
        }
    }

    if (!done) {
        LOG_WARN("Compressed data section premature end");
    }

    LOG_VERBOSE("Used %1 bytes of %2 in decompressing data section",
                p - reinterpret_cast<unsigned char *>(dataSection->getHostAddr().value()), dataSection->getSize());

    // Replace the data pointer and size with the uncompressed versions

    dataSection->setHostAddr(HostAddress(m_data));
    dataSection->resize(sizeData);

    m_symbols->createSymbol(getMainEntryPoint(), "PilotMain")->setAttribute("EntryPoint", true);
    return true;
}


#define TESTMAGIC4(buf, off, a, b, c, d)    (buf[off] == a && buf[off + 1] == b && buf[off + 2] == c && buf[off + 3] == d)
int PalmBinaryLoader::canLoad(QIODevice& dev) const
{
    unsigned char buf[64];

    dev.read(reinterpret_cast<char *>(buf), sizeof(buf));

    if (TESTMAGIC4(buf, 0x3C, 'a', 'p', 'p', 'l') || TESTMAGIC4(buf, 0x3C, 'p', 'a', 'n', 'l')) {
        /* PRC Palm-pilot binary */
        return 8;
    }

    return 0;
}


void PalmBinaryLoader::unload()
{
}


Address PalmBinaryLoader::getEntryPoint()
{
    assert(0); /* FIXME: Need to be implemented */
    return Address::INVALID;
}


void PalmBinaryLoader::close()
{
    // Not implemented yet
}


LoadFmt PalmBinaryLoader::getFormat() const
{
    return LoadFmt::PALM;
}


Machine PalmBinaryLoader::getMachine() const
{
    return Machine::PALM;
}


bool PalmBinaryLoader::isLibrary() const
{
    return(strncmp(reinterpret_cast<char *>(m_image + 0x3C), "libr", 4) == 0);
}


void PalmBinaryLoader::addTrapSymbols()
{
    for (uint32_t loc = 0xAAAAA000; loc <= 0xAAAAAFFF; ++loc) {
        // This is the convention used to indicate an A-line system call
        unsigned offset = loc & 0xFFF;

        if (offset < numTrapStrings) {
            m_symbols->createSymbol(Address(loc), trapNames[offset]);
        }
    }
}


// Specific to BinaryFile objects that implement a "global pointer"
// Gets a pair of unsigned integers representing the address of %agp,
// and the value for GLOBALOFFSET. For Palm, the latter is the amount of
// space allocated below %a5, i.e. the difference between %a5 and %agp
// (%agp points to the bottom of the global data area).
std::pair<Address, unsigned> PalmBinaryLoader::getGlobalPointerInfo()
{
    Address              agp = Address::ZERO;
    const BinarySection *ps = m_binaryImage->getSectionByName("data0");

    if (ps) {
        agp = ps->getSourceAddr();
    }

    std::pair<Address, unsigned> ret(agp, m_sizeBelowA5);
    return ret;
}


//  //  //  //  //  //  //
//  Specific for Palm   //
//  //  //  //  //  //  //

int PalmBinaryLoader::getAppID() const
{
    // The answer is in the header. Return 0 if file not loaded
    if (m_image == nullptr) {
        return 0;
    }

// Beware the endianness (large)
#define OFFSET_ID    0x40
    return (m_image[OFFSET_ID] << 24) + (m_image[OFFSET_ID + 1] << 16) + (m_image[OFFSET_ID + 2] << 8) +
           (m_image[OFFSET_ID + 3]);
}


// Patterns for Code Warrior
#define WILD    0x4AFC

static SWord CWFirstJump[] =
{
    0x0,     0x1,                                 // ? All Pilot programs seem to start with this
    0x487a,  0x4,                                 // pea 4(pc)
    0x0697, WILD, WILD,                           // addil #number, (a7)
    0x4e75
};                                                // rts
static SWord CWCallMain[] =
{
    0x487a,   14,                                 // pea 14(pc)
    0x487a,    4,                                 // pea 4(pc)
    0x0697, WILD, WILD,                           // addil #number, (a7)
    0x4e75
};                                                // rts
static SWord GccCallMain[] =
{
    0x3F04,                                       // movew d4, -(a7)
    0x6100, WILD,                                 // bsr xxxx
    0x3F04,                                       // movew d4, -(a7)
    0x2F05,                                       // movel d5, -(a7)
    0x3F06,                                       // movew d6, -(a7)
    0x6100, WILD
};                                                // bsr PilotMain

/**
 * Find a byte pattern corresponding to \p patt;
 * \p pat may include a wildcard (16 bit WILD, 0x4AFC)
 *
 * \param start    pointer to code to start searching
 * \param size     max number of SWords to search
 * \param patt     pattern to look for
 * \param pattSize size of the pattern (in SWords)
 *
 * \returns pointer to start of match if found, or nullptr if not found.
 */
const SWord *findPattern(const SWord *start, int size, const SWord *patt, int pattSize)
{
    if (pattSize <= 0 || pattSize > size) {
        return nullptr; // no pattern to find
    }

    const SWord *last = start + size;

    while (start + pattSize <= last) {
        bool allMatched = true;
        for (int i = 0; i < pattSize; i++) {
            const SWord curr = patt[i];
            if (curr == WILD) {
                continue;
            }

            const SWord val  = Util::readWord(start + i, true);
            if (curr != val) {
                // Mismatch
                allMatched = false;
                break;
            }
        }

        if (allMatched) {
            // All parts of the pattern matched
            return start;
        }

        start++;
    }

    // Each start position failed
    return nullptr;
}


// Find the native address for the start of the main entry function.
// For Palm binaries, this is PilotMain.
Address PalmBinaryLoader::getMainEntryPoint()
{
    BinarySection *psect = m_binaryImage->getSectionByName("code1");

    if (psect == nullptr) {
        return Address::ZERO; // Failed
    }

    // Return the start of the code1 section
    SWord *startCode = reinterpret_cast<SWord *>(psect->getHostAddr().value());
    int      delta   = (psect->getHostAddr() - psect->getSourceAddr()).value();

    // First try the CW first jump pattern
    const SWord *res = findPattern(startCode, 1, CWFirstJump, sizeof(CWFirstJump) / sizeof(SWord));

    if (res) {
        // We have the code warrior first jump. Get the addil operand
        const int addilOp      = Util::readDWord((startCode + 5), true);
        SWord     *startupCode = reinterpret_cast<SWord *>((HostAddress(startCode) + 10 + addilOp).value());
        // Now check the next 60 SWords for the call to PilotMain
        res = findPattern(startupCode, 60, CWCallMain, sizeof(CWCallMain) / sizeof(SWord));

        if (res) {
            // Get the addil operand
            const int _addilOp = Util::readDWord((res + 5), true);

            // That operand plus the address of that operand is PilotMain
            Address offset_loc = Address(reinterpret_cast<const Byte *>(res) - reinterpret_cast<const Byte *>(startCode) + 5);
            return offset_loc + _addilOp; // ADDRESS::host_ptr(res) + 10 + addilOp - delta;
        }
        else {
            fprintf(stderr, "Could not find call to PilotMain in CW app\n");
            return Address::ZERO;
        }
    }

    // Check for gcc call to main
    res = findPattern(startCode, 75, GccCallMain, sizeof(GccCallMain) / sizeof(SWord));

    if (res) {
        // Get the operand to the bsr
        SWord bsrOp = res[7];
        return Address((HostAddress(res) - delta).value() + 14 + bsrOp);
    }

    fprintf(stderr, "Cannot find call to PilotMain\n");
    return Address::ZERO;
}


void PalmBinaryLoader::generateBinFiles(const QString& path) const
{
    for (const BinarySection *si : *m_binaryImage) {
        const BinarySection& psect(*si);

        if (psect.getName().startsWith("code") || psect.getName().startsWith("data")) {
            continue;
        }

        // Save this section in a file
        // First construct the file name
        int     sect_num = psect.getName().mid(4).toInt();
        QString name     = QString("%1%2.bin").arg(psect.getName().left(4)).arg(sect_num, 4, 16, QChar('0'));
        QString fullName(path);
        fullName += name;
        // Create the file
        FILE *f = fopen(qPrintable(fullName), "w");

        if (f == nullptr) {
            fprintf(stderr, "Could not open %s for writing binary file\n", qPrintable(fullName));
            return;
        }

        fwrite(psect.getHostAddr(), psect.getSize(), 1, f);
        fclose(f);
    }
}


BOOMERANG_LOADER_PLUGIN(PalmBinaryLoader,
                        "Palm OS binary file loader", BOOMERANG_VERSION, "Boomerang developers")
