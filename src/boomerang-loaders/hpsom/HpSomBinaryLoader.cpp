#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "HpSomBinaryLoader.h"


#include "boomerang/core/IBoomerang.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/util/Log.h"

#include <QBuffer>
#include <QFile>
#include <QString>

#include <cstddef>
#include <cassert>
#include <cstring>


// Macro to convert a pointer to a Big Endian integer into a host integer
#define UINT4(p)        Util::readDWord(p, true)
#define UINT4ADDR(p)    Util::readDWord(p, true)


HpSomBinaryLoader::HpSomBinaryLoader()
    : m_loadedImage(nullptr)
    , m_symbols(nullptr)
    , m_image(nullptr)
{
}


HpSomBinaryLoader::~HpSomBinaryLoader()
{
    delete[] m_loadedImage;
}


void HpSomBinaryLoader::initialize(BinaryImage *image, BinarySymbolTable *symbols)
{
    m_image   = image;
    m_symbols = symbols;
}


int Read4(int *pi)
{
    return Util::readDWord(pi, true);
}


// Read the main symbol table, if any
void HpSomBinaryLoader::processSymbols()
{
    // Find the main symbol table, if it exists

    unsigned numSym = UINT4(m_loadedImage + 0x60);

    HostAddress symPtr       = HostAddress(m_loadedImage) + UINT4(m_loadedImage + 0x5C);
    const char  *symbolNames = reinterpret_cast<const char *>(m_loadedImage + static_cast<int>(UINT4(m_loadedImage + 0x6C)));

#define SYMSIZE    20 // 5 4-byte words per symbol entry
#define SYMBOLNM(idx)     UINT4(symPtr + idx * SYMSIZE +  4)
#define SYMBOLAUX(idx)    UINT4(symPtr + idx * SYMSIZE +  8)
#define SYMBOLVAL(idx)    UINT4(symPtr + idx * SYMSIZE + 16)
#define SYMBOLTY(idx)     ((UINT4(symPtr + idx * SYMSIZE) >> 24) & 0x3f)

    for (unsigned idx = 0; idx < numSym; idx++) {
        unsigned   symbolType  = SYMBOLTY(idx);
        Address    value       = Address(SYMBOLVAL(idx));
        const char *symbolName = symbolNames + SYMBOLNM(idx);

        // Only interested in type 3 (code), 8 (stub), and 12 (millicode)
        if ((symbolType != 3) && (symbolType != 8) && (symbolType != 12)) {
            // 2 - initialized data
            // 4 - primary entry point
            // 5 - secondary entrypoint
            // 6 - any antry code
            // 7 - uninitialized data blocks
            // 9 - MODULE name of source module
            if (m_symbols->findSymbolByAddress(value)) {
                continue;
            }

            if (!m_symbols->findSymbolByName(symbolName)) {
                m_symbols->createSymbol(value, symbolName);
            }

            continue;
        }

        //          if ((symbol_type == 10) || (symbol_type == 11))
        // These are extension entries; not interested
        //              continue;
        // Ignore symbols starting with one $; for example, there are many
        // $CODE$ (but we want to see helper functions like $$remU)
        if ((symbolName[0] == '$') && (symbolName[1] != '$')) {
            continue;
        }

        //          if ((symbol_type >= 3) && (symbol_type <= 8))
        // Addresses of code; remove the privilege bits
        value = Address(value.value() & 3);

        // HP's symbol table is crazy. It seems that imports like printf have entries of type 3 with the wrong
        // value. So we have to check whether the symbol has already been entered (assume first one is correct).
        if ((m_symbols->findSymbolByAddress(value) == nullptr) && (m_symbols->findSymbolByName(symbolName) == nullptr)) {
            m_symbols->createSymbol(value, symbolName);
        }
    }
}


bool HpSomBinaryLoader::loadFromMemory(QByteArray& imgdata)
{
    QBuffer fp(&imgdata);

    fp.open(QBuffer::ReadOnly);

    if (!fp.open(QFile::ReadOnly)) {
        LOG_ERROR("Could not load binary file");
        return false;
    }

    qint64 size = fp.size();

    // Allocate a buffer for the image
    m_loadedImage = new unsigned char[size];

    if (m_loadedImage == nullptr) {
        LOG_ERROR("Could not allocate %1 bytes for image", static_cast<QWord>(size));
        return false;
    }

    if (fp.read(reinterpret_cast<char *>(m_loadedImage), size) != size) {
        LOG_ERROR("Error reading binary file", static_cast<QWord>(size));
        return false;
    }

    // Check type at offset 0x0; should be 0x0210 or 0x20B then
    // 0107, 0108, or 010B
    unsigned magic     = UINT4(m_loadedImage);
    unsigned system_id = magic >> 16;
    unsigned a_magic   = magic & 0xFFFF;

    if (((system_id != 0x210) && (system_id != 0x20B)) ||
        ((a_magic != 0x107) && (a_magic != 0x108) && (a_magic != 0x10B))) {
        LOG_ERROR("File is not a standard PA/RISC executable file, with system ID %1 and magic number %2",
                  system_id, a_magic);
        return false;
    }

    // Find the array of aux headers
    unsigned *auxHeaders = reinterpret_cast<unsigned *>(m_loadedImage + UINT4(m_loadedImage + 0x1c));

    if (auxHeaders == nullptr) {
        LOG_ERROR("Auxilliary header array is not present");
        return false;
    }

    // Get the size of the aux headers
    unsigned sizeAux = UINT4(m_loadedImage + 0x20);
    // Search through the auxillary headers. There should be one of type 4
    // ("Exec Auxilliary Header")
    bool     found   = false;
    unsigned *maxAux = auxHeaders + sizeAux;

    while (auxHeaders < maxAux) {
        if ((UINT4(m_loadedImage + Address::value_type(auxHeaders)) & 0xFFFF) == 0x0004) {
            found = true;
            break;
        }

        // Skip this one; length is at the second word.
        auxHeaders += UINT4(auxHeaders + 1) / sizeof(unsigned);
    }

    if (!found) {
        LOG_ERROR("Exec auxilliary header not found");
        return false;
    }

    // There will be just three sections:
    // one for text, one for initialised data, one for BSS

    // Find the DL Table, if it exists
    // The DL table (Dynamic Link info?) is supposed to be at the start of
    // the $TEXT$ space, but the only way I can presently find that is to
    // assume that the first subspace entry points to it
    const char   *subspaceLoc         = reinterpret_cast<const char *>(m_loadedImage) + UINT4(m_loadedImage + 0x34);
    Address      firstSubspaceFileLoc = Address(UINT4(subspaceLoc + 8));
    const char   *dlTable             = reinterpret_cast<const char *>(m_loadedImage) + firstSubspaceFileLoc.value();
    const char   *dlStrings           = dlTable + UINT4(dlTable + 0x28);
    unsigned     numImports           = UINT4(dlTable + 0x14); // Number of import strings
    unsigned     numExports           = UINT4(dlTable + 0x24); // Number of export strings
    const export_entry *export_list   = reinterpret_cast<const export_entry *>(dlTable + UINT4(dlTable + 0x20));

    // A convenient macro for accessing the fields (0-11) of the auxilliary header
    // Fields 0, 1 are the header (flags, aux header type, and size)
#define AUXHDR(idx)    (UINT4(m_loadedImage + Address::value_type(auxHeaders + idx)))

    // Section 0: text (code)
    BinarySection *text = m_image->createSection("$TEXT$", Address(AUXHDR(3)), Address(AUXHDR(3) + AUXHDR(2)));
    assert(text);
    text->setHostAddr(HostAddress(m_loadedImage) + AUXHDR(4));
    text->setEntrySize(1);
    text->setCode(true);
    text->setData(false);
    text->setBss(false);
    text->setReadOnly(true);
    text->setEndian(0);
    text->addDefinedArea(Address(AUXHDR(3)), Address(AUXHDR(3) + AUXHDR(2)));

    // Section 1: initialised data
    BinarySection *data = m_image->createSection("$DATA$", Address(AUXHDR(6)), Address(AUXHDR(6) + AUXHDR(5)));
    assert(data);
    data->setHostAddr(HostAddress(m_loadedImage) + AUXHDR(7));
    data->setEntrySize(1);
    data->setCode(false);
    data->setData(true);
    data->setBss(false);
    data->setReadOnly(false);
    data->setEndian(0);
    data->addDefinedArea(Address(AUXHDR(6)), Address(AUXHDR(6) + AUXHDR(5)));

    // Section 2: BSS
    // For now, assume that BSS starts at the end of the initialised data
    BinarySection *bss = m_image->createSection("$BSS$", Address(AUXHDR(6) + AUXHDR(5)),
                                                 Address(AUXHDR(6) + AUXHDR(5) + AUXHDR(8)));
    assert(bss);
    bss->setHostAddr(HostAddress::ZERO);
    bss->setEntrySize(1);
    bss->setCode(false);
    bss->setData(false);
    bss->setBss(true);
    bss->setReadOnly(false);
    bss->setEndian(0);

    // Work through the imports, and find those for which there are stubs using that import entry.
    // Add the addresses of any such stubs.
    ptrdiff_t deltaText = (text->getHostAddr() - text->getSourceAddr()).value();

    // The "end of data" where r27 points is not necessarily the same as
    // the end of the $DATA$ space. So we have to call getSubSpaceInfo
    std::pair<Address, int> pr = getSubspaceInfo("$GLOBAL$");

    //  ADDRESS endData = pr.first + pr.second;
    pr = getSubspaceInfo("$PLT$");
    //  int minPLT = pr.first - endData;
    //  int maxPLT = minPLT + pr.second;

    // cout << "Offset limits are " << dec << minPLT << " and " << maxPLT << endl;
    // Note: DLT entries come before PLT entries in the import array, but
    // the $DLT$ subsection is not necessarilly just before the $PLT$
    // subsection in memory.
    int numDLT = UINT4(dlTable + 0x40);

    // For each PLT import table entry, add a symbol
    // u runs through import table; v through $PLT$ subspace
    // There should be a one to one correspondance between (DLT + PLT) entries and import table entries.
    // The DLT entries always come first in the import table
    unsigned u = static_cast<unsigned>(numDLT);
    unsigned v = 0;
//   plt_record *PLTs = (plt_record *)(pltStart + deltaData).value();

    u += numImports;
    v += numImports;

    // Work through the exports, and find main. This isn't main itself,
    // but in fact a call to main.
    for (u = 0; u < numExports; u++) {
        // cout << "Exporting " << (pDlStrings+UINT4(&export_list[u].name)) << " value " << hex <<
        // UINT4(&export_list[u].value) << endl;
        if (strncmp(dlStrings + UINT4(&export_list[u].name), "main", 4) == 0) {
            Address callMainAddr(UINT4ADDR(&export_list[u]));

            // Enter the symbol "_callmain" for this address
            m_symbols->createSymbol(callMainAddr, "_callmain");

            // Found call to main. Extract the offset. See assemble_17
            // in pa-risc 1.1 manual page 5-9
            // +--------+--------+--------+----+------------+-+-+
            // | 3A (6) |  t (5) | w1 (5) |0(3)|   w2 (11)  |n|w|  BL
            // +--------+--------+--------+----+------------+-+-+
            //  31    26|25    21|20    16|1513|12         2|1|0
            // +----------------------+--------+-----+----------+
            // |wwww...              w| w1 (5) |w2lsb| w2 msb's | offset
            // +----------------------+--------+-----+----------+
            //  31                  16|15    11| 10  |9        0

            DWord bincall = *reinterpret_cast<DWord *>(callMainAddr.value() + deltaText);
            int   offset  = ((((bincall & 1) << 31) >> 15) |    // w
                             ((bincall & 0x1f0000) >> 5) |      // w1
                             ((bincall & 4) << 8) |             // w2@10
                             ((bincall & 0x1ff8) >> 3));        // w2@0..9
            // Address of main is st + 8 + offset << 2
            m_symbols->createSymbol(callMainAddr + 8 + (offset << 2), "main")
               ->setAttribute("Export", true);
            break;
        }
    }

    processSymbols();
    m_symbols->findSymbolByName("main")->setAttribute("EntryPoint", true);
    return true;
}


int HpSomBinaryLoader::canLoad(QIODevice& dev) const
{
    unsigned char buf[64];

    dev.read(reinterpret_cast<char *>(buf), sizeof(buf));

    if ((buf[0] == 0x02) && (buf[2] == 0x01) && ((buf[1] == 0x10) || (buf[1] == 0x0B)) &&
        ((buf[3] == 0x07) || (buf[3] == 0x08) || (buf[4] == 0x0B))) {
        /* HP Som binary */
        return 5;
    }

    return 0;
}


void HpSomBinaryLoader::unload()
{
    if (m_loadedImage) {
        delete[] m_loadedImage;
        m_loadedImage = nullptr;
    }
}


Address HpSomBinaryLoader::getEntryPoint()
{
    assert(false); /* FIXME: Someone who understands this file please implement */
    return Address::ZERO;
}


void HpSomBinaryLoader::close()
{
    // Not implemented yet
}


LoadFmt HpSomBinaryLoader::getFormat() const
{
    return LoadFmt::PAR;
}


Machine HpSomBinaryLoader::getMachine() const
{
    return Machine::HPRISC;
}


bool HpSomBinaryLoader::isLibrary() const
{
    int type = UINT4(m_loadedImage) & 0xFFFF;

    return(type == 0x0104 || type == 0x010D || type == 0x010E || type == 0x0619);
}


std::pair<Address, int> HpSomBinaryLoader::getSubspaceInfo(const char *ssname)
{
    std::pair<Address, int> ret(Address::ZERO, 0);
    // Get the start and length of the subspace with the given name
    subspace_dictionary_record *subSpaces = reinterpret_cast<subspace_dictionary_record *>(m_loadedImage + UINT4(m_loadedImage + 0x34));
    unsigned   numSubSpaces  = UINT4(m_loadedImage + 0x38);
    const char *spaceStrings = reinterpret_cast<const char *>(m_loadedImage + UINT4(m_loadedImage + 0x44));

    for (unsigned u = 0; u < numSubSpaces; u++) {
        const char *thisName    = reinterpret_cast<const char *>(spaceStrings + UINT4(&subSpaces[u].name));
        unsigned   thisNameSize = UINT4(spaceStrings + UINT4(&subSpaces[u].name) - 4);

        // cout << "Subspace " << thisName << " starts " << hex << subSpaces[u].subspace_start << " length " <<
        // subSpaces[u].subspace_length << endl;
        if ((thisNameSize == strlen(ssname)) && ((strcmp(thisName, ssname) == 0))) {
            ret.first  = Address(UINT4(&subSpaces[u].subspace_start));
            ret.second = UINT4(&subSpaces[u].subspace_length);
            return ret;
        }
    }

    // Failed. Return the zeroes
    return ret;
}


// Specific to BinaryFile objects that implement a "global pointer"
// Gets a pair of unsigned integers representing the address of %agp
// (first) and the value for GLOBALOFFSET (unused for ra-risc)
// The understanding at present is that the global data pointer (%r27 for
// pa-risc) points just past the end of the $GLOBAL$ subspace.
std::pair<Address, unsigned> HpSomBinaryLoader::getGlobalPointerInfo()
{
    std::pair<Address, unsigned> ret(Address::ZERO, 0);
    // Search the subspace names for "$GLOBAL$
    std::pair<Address, int> info = getSubspaceInfo("$GLOBAL$");
    // We want the end of the $GLOBAL$ section, which is the sum of the start
    // address and the size
    ret.first = info.first + info.second;
    return ret;
}


std::map<Address, const char *> *HpSomBinaryLoader::getDynamicGlobalMap()
{
    // Find the DL Table, if it exists
    // The DL table (Dynamic Link info) is supposed to be at the start of
    // the $TEXT$ space, but the only way I can presently find that is to
    // assume that the first subspace entry points to it
    const char *subspace_location     = reinterpret_cast<const char *>(m_loadedImage) + UINT4(m_loadedImage + 0x34);
    Address    first_subspace_fileloc = Address(UINT4(subspace_location + 8));
    const char *DLTable = reinterpret_cast<const char *>(m_loadedImage) + first_subspace_fileloc.value();

    unsigned numDLT = UINT4(DLTable + 0x40);
    // Offset 0x38 in the DL table has the offset relative to $DATA$ (section 2)
    unsigned *p = reinterpret_cast<unsigned *>((m_image->getSectionByIndex(1)->getHostAddr() + UINT4(DLTable + 0x38)).value());

    // The DLT is paralelled by the first <numDLT> entries in the import table;
    // the import table has the symbolic names
    const import_entry *import_list = reinterpret_cast<const import_entry *>(DLTable + UINT4(DLTable + 0x10));
    // Those names are in the DLT string table
    const char *dlStrings = DLTable + UINT4(DLTable + 0x28);

    std::map<Address, const char *> *ret = new std::map<Address, const char *>;

    for (unsigned u = 0; u < numDLT; u++) {
        // ? Sometimes the names are just -1
        if (import_list[u].name == -1) {
            continue;
        }

        const char *str = dlStrings + import_list[u].name;
        (*ret)[Address(*p++)] = str;
    }

    return ret;
}


Address HpSomBinaryLoader::getMainEntryPoint()
{
    const BinarySymbol *sym = m_symbols->findSymbolByName("main");

    return sym ? sym->getLocation() : Address::INVALID;
}


BOOMERANG_LOADER_PLUGIN(HpSomBinaryLoader,
                        "HpSom binary file loader", BOOMERANG_VERSION, "Boomerang developers")
