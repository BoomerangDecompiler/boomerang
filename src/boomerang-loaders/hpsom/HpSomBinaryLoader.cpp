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


#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/util/log/Log.h"

#include <QBuffer>
#include <QFile>
#include <QString>


// Macro to convert a pointer to a Big Endian integer into a host integer
#define UINT4(p)        Util::readDWord(p, Endian::Big)
#define UINT4ADDR(p)    Util::readDWord(p, Endian::Big)

struct sys_clock
{
    unsigned int secs;
    unsigned int nanosecs;
};

struct header {
    short int system_id;        /* magic number - system        */
    short int a_magic;          /* magic number - file type     */
    unsigned int version_id;    /* version id; format=YYMMDDHH */
    struct sys_clock file_time; /* system clock- zero if unused */
    unsigned int entry_space;   /* index of space containing entry point  */
    unsigned int entry_subspace;/* index of subspace for entry point      */
    unsigned int entry_offset;  /* offset of entry point        */
    unsigned int aux_header_location; /* auxiliary header location    */
    unsigned int aux_header_size; /* auxiliary header size        */
    unsigned int som_length;    /* length in bytes of entire som*/
    unsigned int presumed_dp;   /* DP value assumed during compilation                  */
    unsigned int space_location;/* location in file of space dictionary                   */
    unsigned int space_total;   /* number of space entries      */
    unsigned int subspace_location; /* location of subspace entries */
    unsigned int subspace_total; /* number of subspace entries   */
    unsigned int loader_fixup_location; /* MPE/iX loader fixup */
    unsigned int loader_fixup_total; /* number of loader fixup records */
    unsigned int space_strings_location; /* file location of string area for space and subspace names */
    unsigned int space_strings_size; /* size of string area for space and subspace names          */
    unsigned int init_array_location; /* reserved for use by system */
    unsigned int init_array_total; /* reserved for use by system */
    unsigned int compiler_location; /* location in file of module dictionary                   */
    unsigned int compiler_total;    /* number of modules            */
    unsigned int symbol_location;   /* location in file of symbol dictionary                   */
    unsigned int symbol_total;      /* number of symbol records     */
    unsigned int fixup_request_location; /* location in file of fix_up requests                     */
    unsigned int fixup_request_total; /* number of fixup requests     */
    unsigned int symbol_strings_location;   /* file location of string area for module and symbol names */
    unsigned int symbol_strings_size; /* size of string area for module and symbol names      */
    unsigned int unloadable_sp_location;    /* byte offset of first byte of data for unloadable spaces   */
    unsigned int unloadable_sp_size; /* byte length of data for unloadable spaces            */
    unsigned int checksum;
};

/* values for system_id */
#define SOM_SID_PARISC_1_0      0x020b
#define SOM_SID_PARISC_1_1      0x0210
#define SOM_SID_PARISC_2_0      0x0214

/* values for a_magic */
#define EXECLIBMAGIC 0x104
#define RELOC_MAGIC  0x106
#define EXEC_MAGIC   0x107
#define SHARE_MAGIC  0x108
#define DEMAND_MAGIC 0x10b
#define DL_MAGIC     0x10d
#define SHL_MAGIC    0x10e
#define LIBMAGIC     0x0619

/* values for version_id. */
#define VERSION_ID     85082112
#define NEW_VERSION_ID 87102412


struct aux_id {
    unsigned short mandatory  : 1;
    unsigned short copy       : 1;
    unsigned short append     : 1;
    unsigned short ignore     : 1;
    unsigned short reserved   : 12;
    unsigned short type;
    unsigned int length;
};


struct som_exec_auxhdr {
    struct aux_id som_auxhdr;   /* som auxiliary header  */
    uint32 exec_tsize;          /* text size in bytes */
    uint32 exec_tmem;           /* offset of text in memory    */
    uint32 exec_tfile;          /* location of text in file  */
    uint32 exec_dsize;          /* initialized data */
    uint32 exec_dmem;           /* offset of data in memory */
    uint32 exec_dfile;          /* location of data in file */
    uint32 exec_bsize;          /* uninitialized data (bss) */
    uint32 exec_entry;          /* offset of entrypoint */
    uint32 exec_flags;          /* loader flags */
    uint32 exec_bfill;          /* bss initialization value */
};


struct dl_header {
    int hdr_version;        /* header version number */
    int ltptr_value;        /* data offset of LT pointer (R19) */
    int shlib_list_loc;     /* text offset of shlib list */
    int shlib_list_count;   /* count of items in shlib list */
    int import_list_loc;    /* text offset of import list */
    int import_list_count;  /* count of items in import list */
    int hash_table_loc;     /* text offset of export hash table */
    int hash_table_size;    /* count of slots in export hash table */
    int export_list_loc;    /* text offset of export list */
    int export_list_count;  /* count of items in export list */
    int string_table_loc;   /* text offset of string table */
    int string_table_size;  /* length in bytes of string table */
    int dreloc_loc;         /* text offset of dynamic reloc records */
    int dreloc_count;       /* number of dynamic relocation records */
    int dlt_loc;            /* data offset of data linkage table */
    int plt_loc;            /* data offset of procedure linkage table */
    int dlt_count;          /* number of dlt entries in linkage table */
    int plt_count;          /* number of plt entries in linkage table */
    short highwater_mark;   /* highest version number seen in lib or in shlib list*/
    short flags;            /* various flags */
    int export_ext_loc;     /* text offset of export extension tbl */
    int module_loc;         /* text offset of module table*/
    int module_count;       /* number of module entries */
    int elaborator;         /* import index of elaborator */
    int initializer;        /* import index of initializer */
    int embedded_path;      /* index into string table for search path */
                            /* index must be > 0 to be valid */
    int initializer_count;  /* number of initializers declared*/
    int reserved3;          /* currently initialized to 0 */
    int reserved4;          /* currently initialized to 0 */
};


HpSomBinaryLoader::HpSomBinaryLoader()
    : m_image(nullptr)
    , m_symbols(nullptr)
    , m_header(nullptr)
{
}


HpSomBinaryLoader::~HpSomBinaryLoader()
{
}


void HpSomBinaryLoader::initialize(BinaryImage *image, BinarySymbolTable *symbols)
{
    m_image   = image;
    m_symbols = symbols;
}


int Read4(int *pi)
{
    return Util::readDWord(pi, Endian::Big);
}


// Read the main symbol table, if any
void HpSomBinaryLoader::processSymbols(const QByteArray& imgdata)
{
    // Find the main symbol table, if it exists

    const unsigned numSym = UINT4(&m_header->symbol_total);

    const char *symPtr      = imgdata.data() + UINT4(&m_header->symbol_location);
    const char *symbolNames = imgdata.data() + UINT4(&m_header->symbol_strings_location);

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
    m_header = reinterpret_cast<header *>(imgdata.data() + 0);

    switch (Util::readWord(&m_header->system_id, Endian::Big)) {
        case SOM_SID_PARISC_1_0:
        case SOM_SID_PARISC_1_1:
        case SOM_SID_PARISC_2_0:
            break; // recognized
        default:
            LOG_ERROR("File is not a standard PA/RISC executable, with system ID %1", m_header->system_id);
            return false;
    }

    switch (Util::readWord(&m_header->a_magic, Endian::Big)) {
        case EXEC_MAGIC:
        case SHARE_MAGIC:
        case DEMAND_MAGIC:
            break; // supported

        case EXECLIBMAGIC:
        case RELOC_MAGIC:
        case DL_MAGIC:
        case SHL_MAGIC:
        case LIBMAGIC:
            LOG_WARN("Recognized but unsupported PA/RISC magic %1 (trying to load file anyway)", m_header->a_magic);
            break;

        default:
            LOG_ERROR("File is not a standard PA/RISC executable, with magic %1", m_header->a_magic);
            return false;
    }

    // Find the array of aux headers
    const unsigned int auxHeaderOffset = UINT4(&m_header->aux_header_location);
    const unsigned int auxHeaderSize   = UINT4(&m_header->aux_header_size);

    if (auxHeaderOffset == 0 || auxHeaderSize == 0) {
        LOG_ERROR("Auxiliary header array not present");
        return false;
    }
    else if (auxHeaderOffset >= (unsigned int)imgdata.length() ||
             auxHeaderOffset + auxHeaderSize > (unsigned int)imgdata.length()) {
        LOG_ERROR("Auxiliary header extends beyond image size");
        return false;
    }

    const aux_id *auxid    = reinterpret_cast<aux_id *>(imgdata.data() + auxHeaderOffset);
    const aux_id *auxidEnd = reinterpret_cast<aux_id *>(imgdata.data() + auxHeaderOffset + auxHeaderSize);
    bool found = false;

    while (auxid < auxidEnd) {
        const unsigned short type = Util::readWord(&auxid->type, Endian::Big);
        if (type == 0x0004) {
            found = true;
            break;
        }

        // according to the spec the header is always aligned.
        // the only exceptions are the first aux header (which may be unaligned)
        // and the second aux header if it is merged and the first header is unaligned.
        // However, for now we always assume the first header is aligned.
        const unsigned int offset = sizeof(auxid) + UINT4(&auxid->length);
        auxid = reinterpret_cast<const aux_id *>((const char *)auxid + offset);
    }

    if (!found) {
        LOG_ERROR("Executable auxiliary header not found");
        return false;
    }

    // There will be just three sections:
    // one for text, one for initialised data, one for BSS

    // Find the DL Table, if it exists
    // The DL table (Dynamic Link info?) is supposed to be at the start of
    // the $TEXT$ space, but the only way I can presently find that is to
    // assume that the first subspace entry points to it

    unsigned int subspaceOffset       = Util::readDWord(&m_header->subspace_location, Endian::Big);
    const char   *dlTable             = imgdata.data() + UINT4(imgdata.data() + subspaceOffset + 8);
    const char   *dlStrings           = dlTable + UINT4(dlTable + 0x28);
    unsigned     numImports           = UINT4(dlTable + 0x14); // Number of import strings
    unsigned     numExports           = UINT4(dlTable + 0x24); // Number of export strings
    const export_entry *export_list   = reinterpret_cast<const export_entry *>(dlTable + UINT4(dlTable + 0x20));

    const som_exec_auxhdr *execAuxHeader = reinterpret_cast<const som_exec_auxhdr *>(auxid);

    // Section 0: text (code)
    BinarySection *text = m_image->createSection("$TEXT$", Address(UINT4(&execAuxHeader->exec_tmem)),
        Address(UINT4(&execAuxHeader->exec_tmem) + UINT4(&execAuxHeader->exec_tsize)));
    assert(text);

    text->setHostAddr(HostAddress(imgdata.data()) + UINT4(&execAuxHeader->exec_tfile));
    text->setEntrySize(1);
    text->setCode(true);
    text->setData(false);
    text->setBss(false);
    text->setReadOnly(true);
    text->setEndian(Endian::Little);
    text->addDefinedArea(Address(UINT4(&execAuxHeader->exec_tmem)),
        Address(UINT4(&execAuxHeader->exec_tmem) + UINT4(&execAuxHeader->exec_tsize)));

    // Section 1: initialised data
    BinarySection *data = m_image->createSection("$DATA$", Address(UINT4(&execAuxHeader->exec_dmem)),
        Address(UINT4(&execAuxHeader->exec_dmem) + UINT4(&execAuxHeader->exec_dsize)));
    assert(data);

    data->setHostAddr(HostAddress(imgdata.data()) + UINT4(&execAuxHeader->exec_dfile));
    data->setEntrySize(1);
    data->setCode(false);
    data->setData(true);
    data->setBss(false);
    data->setReadOnly(false);
    data->setEndian(Endian::Little);
    data->addDefinedArea(Address(UINT4(&execAuxHeader->exec_dmem)),
        Address(UINT4(&execAuxHeader->exec_dmem) + UINT4(&execAuxHeader->exec_dsize)));

    // Section 2: BSS
    // For now, assume that BSS starts at the end of the initialised data
    const Address bssStart = data->getSourceAddr() + data->getSize();
    BinarySection *bss = m_image->createSection("$BSS$", bssStart,
        bssStart + UINT4(&execAuxHeader->exec_bsize));
    assert(bss);

    bss->setHostAddr(HostAddress::ZERO);
    bss->setEntrySize(1);
    bss->setCode(false);
    bss->setData(false);
    bss->setBss(true);
    bss->setReadOnly(false);
    bss->setEndian(Endian::Little);

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
        // cout << "Exporting " << (dlStrings+UINT4(&export_list[u].name)) << " value " << hex <<
        // UINT4(&export_list[u].value) << endl;
        if (strncmp(dlStrings + UINT4(&export_list[u].name), "main", 4) == 0) {
            Address callMainAddr(UINT4ADDR(&export_list[u].value));

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

    processSymbols(imgdata);
    m_symbols->findSymbolByName("main")->setAttribute("EntryPoint", true);
    return true;
}


int HpSomBinaryLoader::canLoad(QIODevice& dev) const
{
    header h;

    if ((unsigned long)dev.read(reinterpret_cast<char *>(&h), sizeof(header)) < sizeof(header)) {
        return 0;
    }

    switch (Util::readWord(&h.system_id, Endian::Big)) {
        case SOM_SID_PARISC_1_0:
        case SOM_SID_PARISC_1_1:
        case SOM_SID_PARISC_2_0:
            break; // recognized
        default:
            return 0;
    }

    switch (Util::readWord(&h.a_magic, Endian::Big)) {
        case EXEC_MAGIC:
        case SHARE_MAGIC:
        case DEMAND_MAGIC:
            return 4; // supported

        case EXECLIBMAGIC:
        case RELOC_MAGIC:
        case DL_MAGIC:
        case SHL_MAGIC:
        case LIBMAGIC:
            return 2; // not officially supported, but try to load it anyway

        default:
            return 0;
    }
}


void HpSomBinaryLoader::unload()
{
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
    const unsigned short magic = Util::readWord(&m_header->a_magic, Endian::Big);
    return
        magic == EXECLIBMAGIC ||
        magic == DL_MAGIC ||
        magic == SHL_MAGIC ||
        magic == LIBMAGIC;
}


std::pair<Address, int> HpSomBinaryLoader::getSubspaceInfo(const char *ssname)
{
    std::pair<Address, int> ret(Address::ZERO, 0);
    // Get the start and length of the subspace with the given name
    const subspace_dictionary_record *subSpaces = reinterpret_cast<const subspace_dictionary_record *>(
        (const char *)m_header + UINT4(&m_header->subspace_location));

    const unsigned int numSubSpaces  = Util::readDWord(&m_header->subspace_total, Endian::Big);
    const char *spaceStrings = reinterpret_cast<const char *>(m_header) + Util::readDWord(&m_header->space_strings_location, Endian::Big);

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
    const char *subspace_location     = reinterpret_cast<const char *>(m_header) + UINT4(&m_header->subspace_location);
    Address    first_subspace_fileloc = Address(UINT4(subspace_location + 8));
    const char *DLTable = reinterpret_cast<const char *>(m_header) + first_subspace_fileloc.value();

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
