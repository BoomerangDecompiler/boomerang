/*
 * Copyright (C) 2000-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:        HpSomBinaryFile.cc
 * OVERVIEW:    This file contains the implementation of the class
 *              HpSomBinaryFile, for decoding PA/RISC SOM executable files.
 *              Derived from class BinaryFile
 *============================================================================*/

/*
 * $Revision$
 *
 * 22 Jun 00 - Mike: Initial version
 * 15 May 02 - Mike: Fixed several UINT4(&...) that were needed for endianness
*/

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#include "exp.h"		// For MSVC 5.00
#endif

#include "HpSomBinaryFile.h"
#include <cstring>

// Macro to convert a pointer to a Big Endian integer into a host integer
#define UC(p) ((unsigned char*)p)
#define UINT4(p) ((UC(p)[0] << 24) + (UC(p)[1] << 16) + (UC(p)[2] << 8) + \
    UC(p)[3])

HpSomBinaryFile::HpSomBinaryFile()
    : m_pImage(0)
{
}

HpSomBinaryFile::~HpSomBinaryFile()
{
    if (m_pImage) {
        delete m_pImage;
    }
}

// Functions to recognise various instruction patterns
// Note: these are not presently used. May be needed again if it turns out
// that addresses in the PLT do not always point to the BOR (Bind On Reference,
// a kind of stub)
#if 0
bool isLDW(unsigned instr, int& offset, unsigned dest)
{
    if (((instr >> 26) == 0x12) &&              // Opcode
        (instr & 1) &&                          // Offset is neg
        (((instr >> 21) & 0x1f) == 27) &&       // register b 
        (((instr >> 16) & 0x1f) == dest)) {     // register t
        offset = ((((int)instr << 31) >> 18) |
          ((instr & 0x3ffe) >> 1));
        return true;
    } else
        return false;
}

bool isLDSID(unsigned instr) {
    // Looking for LDSID       (%s0,%r21),%r1
    return (instr == 0x02a010a1);
}

bool isMSTP(unsigned instr) {
    // Looking for MTSP        %r1,%s0
    return (instr == 0x00011820);
}

bool isBE(unsigned instr) {
    // Looking for BE          0(%s0,%r21)
    return (instr == 0xe2a00000);
}

bool isSTW(unsigned instr) {
    // Looking for STW         %r2,-24(%s0,%r30)
    return (instr == 0x6bc23fd1);
}

bool isStub(ADDRESS hostAddr, int& offset) {
    // Looking for this pattern:
    // 2600: 4b753f91  LDW         -56(%s0,%r27),%r21
    // 2604: 4b733f99  LDW         -52(%s0,%r27),%r19
    // 2608: 02a010a1  LDSID       (%s0,%r21),%r1
    // 260c: 00011820  MTSP        %r1,%s0
    // 2610: e2a00000  BE          0(%s0,%r21)
    // 2614: 6bc23fd1  STW         %r2,-24(%s0,%r30)
    // Where the only things that vary are the first two offsets (here -56 and
    // -52)
    unsigned instr;
    int offset1, offset2;
    instr = *((unsigned*)hostAddr); hostAddr += 4;
    if (!isLDW(instr, offset1, 21)) return false;
    instr = *((unsigned*)hostAddr); hostAddr += 4;
    if (!isLDW(instr, offset2, 19)) return false;
    instr = *((unsigned*)hostAddr); hostAddr += 4;
    if (!isLDSID(instr)) return false;
    instr = *((unsigned*)hostAddr); hostAddr += 4;
    if (!isMSTP(instr)) return false;
    instr = *((unsigned*)hostAddr); hostAddr += 4;
    if (!isBE(instr)) return false;
    instr = *((unsigned*)hostAddr);
    if (!isSTW(instr)) return false;
    if ((offset2 - offset1) != 4) return false;
    offset = offset1;
    return true;
}
#endif


bool HpSomBinaryFile::RealLoad(const char* sName) {
    FILE    *fp;

    m_pFileName = sName;

    if ((fp = fopen(sName, "rb")) == NULL) {
        fprintf(stderr, "Could not open binary file %s\n", sName);
        return false;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    // Allocate a buffer for the image
    m_pImage = new unsigned char[size];
    if (m_pImage == 0) {
        fprintf(stderr, "Could not allocate %ld bytes for image\n", size);
        return false;
    }
    memset(m_pImage, size, 0);

    fseek(fp, 0, SEEK_SET);
    if (fread(m_pImage, 1, size, fp) != (unsigned)size) {
        fprintf(stderr, "Error reading binary file %s\n", sName);
        return false;
    }

    // Check type at offset 0x0; should be 0x0210 or 0x20B then
    // 0107, 0108, or 010B
    unsigned magic = UINT4(m_pImage);
    unsigned system_id = magic >> 16;
    unsigned a_magic = magic & 0xFFFF;
    if (((system_id != 0x210) && (system_id != 0x20B)) || 
      ((a_magic != 0x107) && (a_magic != 0x108) && (a_magic != 0x10B))) {
        fprintf(stderr, "%s is not a standard PA/RISC executable file, with "
            "system ID %X and magic number %X\n", sName, system_id, a_magic);
        return false;
    }

    // Find the array of aux headers
    unsigned* auxHeaders = (unsigned*)UINT4(m_pImage + 0x1c);
    if (auxHeaders == 0) {
        fprintf(stderr, "Error: auxilliary header array is not present\n");
        return false;
    }
    // Get the size of the aux headers
    unsigned sizeAux = UINT4(m_pImage + 0x20);
    // Search through the auxillary headers. There should be one of type 4
    // ("Exec Auxilliary Header")
    bool found = false;
    unsigned* maxAux = auxHeaders + sizeAux;
    while (auxHeaders < maxAux) {
        if ((UINT4(m_pImage + (ADDRESS) auxHeaders) & 0xFFFF) == 0x0004) {
            found = true;
            break;
        }
        // Skip this one; length is at the second word. Rightshift by 2 for
        // sizeof(unsigned).
        auxHeaders += (UINT4((UC(auxHeaders+1)))) >> 2;
    }
    if (!found) {
        fprintf(stderr, "Error: Exec auxilliary header not found\n");
        return false;
    }

    // Allocate the section information. There will be just four entries:
    // one for the header, one for text, one for initialised data, one for BSS
    m_pSections = new SectionInfo[4];
    m_iNumSections = 4;
    if (m_pSections == 0) {
        fprintf(stderr, "Could not allocate section info array of 4 items\n");
        if (m_pImage) {
            delete m_pImage;
            m_pImage = 0;
        }
        return false;
    }

    // Find the main symbol table, if it exists
    ADDRESS symPtr = (ADDRESS)m_pImage + UINT4(m_pImage + 0x5C);
    unsigned numSym = UINT4(m_pImage + 0x60);

    // Find the DL Table, if it exists
    // The DL table (Dynamic Link info?) is supposed to be at the start of
    // the $TEXT$ space, but the only way I can presently find that is to
    // assume that the first subspace entry points to it
    char* subspace_location = (char*)m_pImage + UINT4(m_pImage + 0x34);
    ADDRESS first_subspace_fileloc = UINT4(subspace_location + 8);
    char* DLTable = (char*)m_pImage + first_subspace_fileloc;
    char* pDlStrings = DLTable + UINT4(DLTable + 0x28);
    unsigned numImports = UINT4(DLTable + 0x14);    // Number of import strings
    import_entry* import_list = (import_entry*)(DLTable +
      UINT4(DLTable + 0x10));
    unsigned numExports = UINT4(DLTable + 0x24);    // Number of export strings
    export_entry* export_list = (export_entry*)(DLTable +
      UINT4(DLTable + 0x20));

// A convenient macro for accessing the fields (0-11) of the auxilliary header
// Fields 0, 1 are the header (flags, aux header type, and size)
#define AUXHDR(idx) (UINT4(m_pImage + (ADDRESS)(auxHeaders+idx)))

    // Section 0: header
    m_pSections[0].pSectionName = const_cast<char *>("$HEADER$");
    m_pSections[0].uNativeAddr = 0;         // Not applicable
    m_pSections[0].uHostAddr = (ADDRESS)m_pImage;
//  m_pSections[0].uSectionSize = AUXHDR(4);
    // There is nothing that appears in memory space here; to give this a size
    // is to invite GetSectionInfoByAddr to return this section!
    m_pSections[0].uSectionSize = 0;

    m_pSections[0].uSectionEntrySize = 1;   // Not applicable
    m_pSections[0].bCode = 0;
    m_pSections[0].bData = 0;
    m_pSections[0].bBss = 0;
    m_pSections[0].bReadOnly = 0;

    // Section 1: text (code)
    m_pSections[1].pSectionName = const_cast<char *>("$TEXT$");
    m_pSections[1].uNativeAddr = AUXHDR(3);
    m_pSections[1].uHostAddr = (ADDRESS)m_pImage + AUXHDR(4);
    m_pSections[1].uSectionSize = AUXHDR(2);
    m_pSections[1].uSectionEntrySize = 1;   // Not applicable
    m_pSections[1].bCode = 1;
    m_pSections[1].bData = 0;
    m_pSections[1].bBss = 0;
    m_pSections[1].bReadOnly = 1;

    // Section 2: initialised data
    m_pSections[2].pSectionName = const_cast<char *>("$DATA$");
    m_pSections[2].uNativeAddr = AUXHDR(6);
    m_pSections[2].uHostAddr = (ADDRESS)m_pImage + AUXHDR(7);
    m_pSections[2].uSectionSize = AUXHDR(5);
    m_pSections[2].uSectionEntrySize = 1;   // Not applicable
    m_pSections[2].bCode = 0;
    m_pSections[2].bData = 1;
    m_pSections[2].bBss = 0;
    m_pSections[2].bReadOnly = 0;

    // Section 3: BSS
    m_pSections[3].pSectionName = const_cast<char *>("$BSS$");
    // For now, assume that BSS starts at the end of the initialised data
    m_pSections[3].uNativeAddr = AUXHDR(6) + AUXHDR(5);
    m_pSections[3].uHostAddr = 0;           // Not applicable
    m_pSections[3].uSectionSize = AUXHDR(8);
    m_pSections[3].uSectionEntrySize = 1;   // Not applicable
    m_pSections[3].bCode = 0;
    m_pSections[3].bData = 0;
    m_pSections[3].bBss = 1;
    m_pSections[3].bReadOnly = 0;

    // Work through the imports, and find those for which there are stubs using that import entry.
	// Add the addresses of any such stubs.
    int deltaText = m_pSections[1].uHostAddr - m_pSections[1].uNativeAddr;
    int deltaData = m_pSections[2].uHostAddr - m_pSections[2].uNativeAddr;
    // The "end of data" where r27 points is not necessarily the same as
    // the end of the $DATA$ space. So we have to call getSubSpaceInfo
    std::pair<unsigned, int> pr = getSubspaceInfo("$GLOBAL$");
//  ADDRESS endData = pr.first + pr.second;
    pr = getSubspaceInfo("$PLT$");
//  int minPLT = pr.first - endData;
//  int maxPLT = minPLT + pr.second;
    ADDRESS pltStart = pr.first;
//cout << "Offset limits are " << dec << minPLT << " and " << maxPLT << endl;
    // Note: DLT entries come before PLT entries in the import array, but
    // the $DLT$ subsection is not necessarilly just before the $PLT$
    // subsection in memory.
    int numDLT = UINT4(DLTable + 0x40);

    // This code was for pattern patching the BOR (Bind On Reference, or library call stub) routines. It appears to be
	// unnecessary, since as they appear in the file, the PLT entries point to the BORs
#if 0
    ADDRESS startText = m_pSections[1].uHostAddr;
    ADDRESS endText = startText + m_pSections[1].uSectionSize - 0x10;
    ADDRESS host;
    for (host = startText; host != endText; host += 4) {
        // Test this location for a BOR (library stub)
        int offset;
        if (isStub(host, offset)) {
cout << "Found a stub with offset " << dec << offset << endl;
            if ((offset >= minPLT) && (offset < maxPLT)) {
                // This stub corresponds with an import entry
                u = (offset - minPLT) / sizeof(plt_record);
                // Add an offset for the DLT entries
                u += numDLT;
                symbols[import_list[u].name + pDlStrings] = host - deltaText;
cout << "Added sym " << (import_list[u].name + pDlStrings) << ", value " << hex << (host - deltaText) << endl;
            }
        }
    }
#endif

    // For each PLT import table entry, add a symbol
    // u runs through import table; v through $PLT$ subspace
    // There should be a one to one correspondance between (DLT + PLT) entries and import table entries.
	// The DLT entries always come first in the import table
    unsigned u = (unsigned)numDLT, v = 0;
    plt_record* PLTs = (plt_record*)(pltStart + deltaData);
    for (; u < numImports; u++, v++) {
//cout << "Importing " << (pDlStrings+import_list[u].name) << endl;
        symbols.Add(PLTs[v].value, pDlStrings + UINT4(&import_list[u].name));
        // Add it to the set of imports; needed by IsDynamicLinkedProc()
        imports.insert(PLTs[v].value);
//cout << "Added import sym " << (import_list[u].name + pDlStrings) << ", value " << hex << PLTs[v].value << endl;
    }
    // Work through the exports, and find main. This isn't main itself,
    // but in fact a call to main.
    for (u=0; u < numExports; u++) {
//cout << "Exporting " << (pDlStrings+UINT4(&export_list[u].name)) << " value " << hex << UINT4(&export_list[u].value) << endl;
        if (strncmp(pDlStrings+UINT4(&export_list[u].name), "main", 4) == 0) {
            // Enter the symbol "_callmain" for this address
            symbols.Add(UINT4(&export_list[u].value), const_cast<char *>("_callmain"));
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

            unsigned bincall = *(unsigned*) (UINT4(&export_list[u].value) + deltaText);
            int offset = ((((bincall & 1) << 31) >> 15) |     // w
                           ((bincall & 0x1f0000) >> 5) |      // w1
                           ((bincall &        4) << 8) |      // w2@10
                           ((bincall &   0x1ff8) >> 3));      // w2@0..9
            // Address of main is st + 8 + offset << 2
            symbols.Add(UINT4(&export_list[u].value) + 8 + (offset << 2), const_cast<char *>("main"));
            break;
        }
    }

    // Read the main symbol table, if any
    if (numSym) {
        char* pNames = (char*) (m_pImage+(int)UINT4(m_pImage+0x6C));
#define SYMSIZE 20              // 5 4-byte words per symbol entry
#define SYMBOLNM(idx)  (UINT4(symPtr + idx*SYMSIZE + 4))
#define SYMBOLAUX(idx) (UINT4(symPtr + idx*SYMSIZE + 8))
#define SYMBOLVAL(idx) (UINT4(symPtr + idx*SYMSIZE + 16))
#define SYMBOLTY(idx)  ((UINT4(symPtr + idx*SYMSIZE) >> 24) & 0x3f)
        for (u=0; u < numSym; u++) {
// cout << "Symbol " << pNames+SYMBOLNM(u) << ", type " << SYMBOLTY(u) << ", value " << hex << SYMBOLVAL(u) << ", aux " << SYMBOLAUX(u) << endl; 
            unsigned symbol_type = SYMBOLTY(u);
            // Only interested in type 3 (code), 8 (stub), and 12 (millicode)
            if ((symbol_type != 3) && (symbol_type != 8) && (symbol_type != 12))
                continue;
//          if ((symbol_type == 10) || (symbol_type == 11))
                // These are extension entries; not interested
//              continue;
            char* pSymName = pNames + SYMBOLNM(u);
            // Ignore symbols starting with one $; for example, there are many
            // $CODE$ (but we want to see helper functions like $$remU)
            if ((pSymName[0] == '$') && (pSymName[1] != '$')) continue;
//          if ((symbol_type == 6) && (strcmp("main", pSymName) == 0))
                // Entry point for main. Make sure to ignore this entry, else it
                // ends up being the main entry point
//              continue;
            ADDRESS value = SYMBOLVAL(u);
//          if ((symbol_type >= 3) && (symbol_type <= 8))
                // Addresses of code; remove the privilege bits
                value &= ~3;
//if (strcmp("main", pNames+SYMBOLNM(u)) == 0) {    // HACK!
//  cout << "main at " << hex << value << " has type " << SYMBOLTY(u) << endl;}
            // HP's symbol table is crazy. It seems that imports like printf have entries of type 3 with the wrong
			// value. So we have to check whether the symbol has already been entered (assume first one is correct).
			if (symbols.find(pSymName) == NO_ADDRESS)
				symbols.Add(value, pSymName);
//cout << "Symbol " << pNames+SYMBOLNM(u) << ", type " << SYMBOLTY(u) << ", value " << hex << value << ", aux " << SYMBOLAUX(u) << endl;  // HACK!
        }
    }       // if (numSym)

    return true;
}

void HpSomBinaryFile::UnLoad()
{
    if (m_pImage) {
        delete [] m_pImage;
        m_pImage = 0;
    }
}

ADDRESS HpSomBinaryFile::GetEntryPoint()
{
    assert(0); /* FIXME: Someone who understands this file please implement */
    return 0;
}

// This is provided for completeness only...
std::list<SectionInfo*>& HpSomBinaryFile::GetEntryPoints(const char* pEntry
 /* = "main" */) {
    std::list<SectionInfo*>* ret = new std::list<SectionInfo*>;
    SectionInfo* pSect = GetSectionInfoByName("code1");
    if (pSect == 0)
        return *ret;               // Failed
    ret->push_back(pSect);
    return *ret;
}


bool HpSomBinaryFile::Open(const char* sName)
{
    // Not implemented yet
    return false;
}
void HpSomBinaryFile::Close()
{
    // Not implemented yet
    return; 
}
bool HpSomBinaryFile::PostLoad(void* handle)
{
    // Not needed: for archives only
    return false;
}

LOAD_FMT HpSomBinaryFile::GetFormat() const
{
    return LOADFMT_PAR;
}

MACHINE HpSomBinaryFile::GetMachine() const
{
    return MACHINE_HPRISC;
}

bool HpSomBinaryFile::isLibrary() const
{
    int type =  UINT4(m_pImage)&0xFFFF;
    return ( type == 0x0104 || type == 0x010D ||
             type == 0x010E || type == 0x0619 );
}

std::list<const char *> HpSomBinaryFile::getDependencyList()
{
    return std::list<const char *>(); /* FIXME */
}

ADDRESS HpSomBinaryFile::getImageBase()
{
    return 0; /* FIXME */
}

size_t HpSomBinaryFile::getImageSize()
{
    return UINT4(m_pImage + 0x24);
}

// We at least need to be able to name the main function and system calls
const char* HpSomBinaryFile::SymbolByAddress(ADDRESS a) {
    return symbols.find(a);
}

ADDRESS HpSomBinaryFile::GetAddressByName(char* pName, bool bNoTypeOK /* = false */)
{
    // For now, we ignore the symbol table and do a linear search of our
    // SymTab table
    ADDRESS res = symbols.find(pName);
    if (res == NO_ADDRESS)
        return 0;           // Till the failure return value is fixed
    return res;
}

bool HpSomBinaryFile::IsDynamicLinkedProc(ADDRESS uNative)
{
    // Look up the address in the set of imports
    return imports.find(uNative) != imports.end();
}

std::pair<ADDRESS, int> HpSomBinaryFile::getSubspaceInfo(const char* ssname)
{
    std::pair<ADDRESS, int> ret(0, 0);
    // Get the start and length of the subspace with the given name
    struct subspace_dictionary_record* subSpaces =
      (struct subspace_dictionary_record*)(m_pImage + UINT4(m_pImage + 0x34));
    unsigned numSubSpaces = UINT4(m_pImage + 0x38);
    const char* spaceStrings = (const char*)
      (m_pImage + UINT4(m_pImage + 0x44));
    for (unsigned u=0; u < numSubSpaces; u++) {
        char* thisName = (char*)(spaceStrings + UINT4(&subSpaces[u].name));
        unsigned thisNameSize = UINT4(spaceStrings + UINT4(&subSpaces[u].name) - 4);
//cout << "Subspace " << thisName << " starts " << hex << subSpaces[u].subspace_start << " length " << subSpaces[u].subspace_length << endl;
        if ((thisNameSize == strlen(ssname)) &&
          ((strcmp(thisName, ssname) == 0))) {
            ret.first = UINT4(&subSpaces[u].subspace_start);
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
std::pair<unsigned,unsigned> HpSomBinaryFile::GetGlobalPointerInfo()
{
    std::pair<unsigned, unsigned> ret(0, 0);
    // Search the subspace names for "$GLOBAL$
    std::pair<ADDRESS, int> info = getSubspaceInfo("$GLOBAL$");
    // We want the end of the $GLOBAL$ section, which is the sum of the start
    // address and the size
    ret.first = info.first + info.second;
    return ret;
}

/*==============================================================================
 * FUNCTION:    HpSomBinaryFile::GetDynamicGlobalMap
 * OVERVIEW:    Get a map from ADDRESS to const char*. This map contains the
 *                native addresses and symbolic names of global data items
 *                (if any) which are shared with dynamically linked libraries.
 *                Example: __iob (basis for stdout). The ADDRESS is the native
 *                address of a pointer to the real dynamic data object.
 * NOTE:        The caller should delete the returned map.
 * PARAMETERS:  None
 * RETURNS:     Pointer to a new map with the info
 *============================================================================*/
std::map<ADDRESS, const char*>* HpSomBinaryFile::GetDynamicGlobalMap()
{
    // Find the DL Table, if it exists
    // The DL table (Dynamic Link info) is supposed to be at the start of
    // the $TEXT$ space, but the only way I can presently find that is to
    // assume that the first subspace entry points to it
    const char* subspace_location = (char*)m_pImage + UINT4(m_pImage + 0x34);
    ADDRESS first_subspace_fileloc = UINT4(subspace_location + 8);
    const char* DLTable = (char*)m_pImage + first_subspace_fileloc;

    unsigned numDLT = UINT4(DLTable + 0x40);
    // Offset 0x38 in the DL table has the offset relative to $DATA$ (section 2)
    unsigned* p = (unsigned*)(UINT4(DLTable + 0x38) + m_pSections[2].uHostAddr);

    // The DLT is paralelled by the first <numDLT> entries in the import table;
    // the import table has the symbolic names
    const import_entry* import_list = (import_entry*)(DLTable +
      UINT4(DLTable + 0x10));
    // Those names are in the DLT string table
    const char* pDlStrings = DLTable + UINT4(DLTable + 0x28);

    std::map<ADDRESS, const char*>* ret = new std::map<ADDRESS, const char*>;
    for (unsigned u=0; u < numDLT; u++) {
        // ? Sometimes the names are just -1
        if (import_list[u].name == -1)
            continue;
        const char* str = pDlStrings + import_list[u].name;
        (*ret)[*p++] = str;
    }
    return ret;
}

ADDRESS HpSomBinaryFile::GetMainEntryPoint()
{
    return symbols.find("main");
#if 0
    if (mainExport == 0) {
        // This means we didn't find an export table entry for main
        // didn't load the file
        fprintf(stderr, "Did not find export entry for `main'\n");
        return 0;
    }
    // Expect a bl <main>, rp instruction
    unsigned instr = UINT4(m_pSections[1].uHostAddr + mainExport -
        m_pSections[1].uNativeAddr);
    int disp;
    // Standard form: sub-opcode 0, target register = 2
    if ((instr >> 26 == 0x3A) && (((instr >> 21) & 0x1F) == 2) &&
      (((instr >> 13) & 0x7) == 0)) {
        disp = (instr & 1) << 16 |          // w
            ((instr >> 16) & 0x1F) << 11 |  // w1
            ((instr >> 2) & 1) << 10 |      // w2{10}
            ((instr >> 3) & 0x3FF);         // w2{0..9}
        // Sign extend
        disp <<= 15; disp >>= 15;
    }
    // Alternate (v2 only?) form: sub-opcode 5, t field becomes w3
    // (extra 5 bits of address range)
    else if ((instr >> 26 == 0x3A) && (((instr >> 13) & 0x7) == 5)) {
        disp = (instr & 1) << 21 |          // w
            ((instr >> 21) & 0x1F) << 16 |  // w3
            ((instr >> 16) & 0x1F) << 11 |  // w1
            ((instr >> 2) & 1) << 10 |      // w2{10}
            ((instr >> 3) & 0x3FF);         // w2{0..9}
        // Sign extend
        disp <<= 10; disp >>= 10;
    }
    else {
        fprintf(stderr, "Error: expected BL instruction at %X, found %X\n",
            mainExport, instr);
        return 0;
    }
    // Return the effective destination address
    return mainExport + (disp << 2) + 8;
#endif
}

// This function is called via dlopen/dlsym; it returns a new BinaryFile
// derived concrete object. After this object is returned, the virtual function
// call mechanism will call the rest of the code in this library
// It needs to be C linkage so that it its name is not mangled
extern "C" {
#ifdef _WIN32
     __declspec(dllexport)
#endif
    BinaryFile* construct()
    {
        return new HpSomBinaryFile;
    }    
}
