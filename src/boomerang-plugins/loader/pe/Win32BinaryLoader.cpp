#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Win32BinaryLoader.h"


/* Win32 binary file format.
 *    This file implements the class Win32BinaryLoader, derived from class
 *    IFileLoader. See Win32BinaryLoader.h and IFileLoader.h for details.
 *
 * 25 Jun 02 - Mike: Added code to find WinMain by finding a call within 5
 *                instructions of a call to GetModuleHandleA
 * 07 Jul 02 - Mike: Added a LMMH() so code works on big-endian host
 * 08 Jul 02 - Mike: Changed algorithm to find main; now looks for ordinary
 *                 call up to 10 instructions before an indirect call to exit
 * 24 Jul 05 - Mike: State machine to recognize main in Borland Builder files
 */

#if defined(_WIN32) && !defined(__MINGW32__) && !defined(__MINGW64__)
#    define NOMINMAX
#    include <windows.h>
namespace dbghelp
{
#    include <dbghelp.h>
}
#endif

#include "Win32BinaryLoader.h"

#include "boomerang/core/plugin/Plugin.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinarySymbol.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/Log.h"

#include <QFile>
#include <QString>

#include <stdexcept>


extern "C"
{
    int microX86Dis(void *p); // From microX86dis.c
}

namespace
{
struct SectionParam
{
    QString Name;
    Address From;
    size_t Size;
    size_t PhysSize;
    HostAddress ImageAddress;
    bool Bss, Code, Data, ReadOnly;
};
}

// clang-format off
#ifndef IMAGE_SCN_CNT_CODE // Assume that if one is not defined, the rest isn't either.
#define IMAGE_SCN_CNT_CODE                  0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA      0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA    0x00000080
#define IMAGE_SCN_MEM_READ                  0x40000000
#define IMAGE_SCN_MEM_WRITE                 0x80000000
#endif
// clang-format on


Win32BinaryLoader::Win32BinaryLoader(Project *project)
    : IFileLoader(project)
    , m_image(nullptr)
    , m_imageSize(0)
    , m_header(nullptr)
    , m_peHeader(nullptr)
    , m_numRelocs(0)
    , m_hasDebugInfo(false)
    , m_mingwMain(false)
    , m_binaryImage(nullptr)
    , m_symbols(nullptr)
{
}


Win32BinaryLoader::~Win32BinaryLoader()
{
    Win32BinaryLoader::unload();
}


void Win32BinaryLoader::initialize(BinaryFile *file, BinarySymbolTable *symbols)
{
    unload();
    m_binaryImage = file->getImage();
    m_symbols     = symbols;

    file->setBitness(32);
}


void Win32BinaryLoader::close()
{
    unload();
}


Address Win32BinaryLoader::getEntryPoint()
{
    return Address(READ4_LE(m_peHeader->Imagebase) + READ4_LE(m_peHeader->EntrypointRVA));
}


Address Win32BinaryLoader::getMainEntryPoint()
{
    const BinarySymbol *mainSymbol = m_symbols->findSymbolByName("main");

    if (mainSymbol) {
        return mainSymbol->getLocation();
    }

    mainSymbol = m_symbols->findSymbolByName("_main"); // Example: MinGW

    if (mainSymbol) {
        return mainSymbol->getLocation();
    }

    mainSymbol = m_symbols->findSymbolByName("WinMain"); // Example: MinGW

    if (mainSymbol) {
        return mainSymbol->getLocation();
    }

    // This is a bit of a hack, but no more than the rest of Windows :-O  The pattern is to look for
    // an indirect call (opcode FF 15) to exit; within 10 instructions before that should be the
    // call to WinMain (with no other calls inbetween). This pattern should work for "old style" and
    // "new style" PE executables, as well as console mode PE files.

    // Start at program entry point
    const Address imageBase = Address(READ4_LE(m_peHeader->Imagebase));
    unsigned rva            = READ4_LE(m_peHeader->EntrypointRVA);
    Address addr;
    unsigned lastOrdCall = 0;
    int gap;              // Number of instructions from the last ordinary call
    int borlandState = 0; // State machine for Borland

    BinarySection *section = m_binaryImage->getSectionByName(".text");

    if (section == nullptr) {
        section = m_binaryImage->getSectionByName("CODE");

        if (section == nullptr) {
            LOG_ERROR("Cannot find a section containing code!");
            return Address::INVALID;
        }
    }

#define MAIN_RANGE (0x200U) // number of bytes to look for main/WinMain from start of entry point

    const unsigned int textSize    = section->getSize();
    const unsigned int searchLimit = rva + std::min(MAIN_RANGE, textSize);

    if (m_peHeader->Subsystem == 1) {
        // native -> _start == main
        return imageBase + READ4_LE(m_peHeader->EntrypointRVA);
    }

    gap = 0xF0000000; // Large positive number (in case no ordinary calls)

    while (rva + 1 < searchLimit) { // make sure not to read past the end of the section
        const Byte op1 = Util::readByte(m_image + rva + 0);
        const Byte op2 = Util::readByte(m_image + rva + 1);

        LOG_VERBOSE("At %1, ops 0x%2, 0x%3", QString::number(rva, 16), QString::number(op1, 16),
                    QString::number(op2, 16));

        switch (op1) {
        case 0xE8:
            // An ordinary call; this could be to winmain/main
            lastOrdCall = rva;
            gap         = 0;

            if (borlandState == 1) {
                borlandState++;
            }
            else {
                borlandState = 0;
            }

            break;

        case 0xFF:
            if (op2 == 0x15) { // Opcode FF 15 is indirect call
                // Get the 4 byte address from the instruction
                addr = Address(READ4_LE_P(m_image + rva + 2));
                //                    const char *c = dlprocptrs[addr].c_str();
                //                    printf("Checking %x finding %s\n", addr, c);
                const BinarySymbol *calleeSym = m_symbols->findSymbolByAddress(addr);

                if (calleeSym && (calleeSym->getName() == "exit")) {
                    if (gap <= 10) {
                        // This is it. The instruction at lastOrdCall is (win)main
                        addr = Address(READ4_LE_P(m_image + lastOrdCall + 1));
                        addr += lastOrdCall + 5; // Addr is dest of call
                        return imageBase + addr;
                    }
                }
            }
            else {
                borlandState = 0;
            }

            break;

            // Short relative jump, e.g. Borland
        case 0xEB:
            if (op2 >= 0x80) { // Branch backwards?
                break;         // Yes, just ignore it
            }

            // Otherwise, actually follow the branch. May have to modify this some time...
            rva += op2 + 2; // +2 for the instruction itself, and op2 for the displacement
            gap++;
            continue;

        case 0x6A:
            if (op2 == 0) { // Push 00
                // Borland pattern:             Borland state before:
                //     push 0                   0
                //     call __ExceptInit        1
                //     pop ecx                  2
                //     push offset mainInfo     3
                //     push 0                   4
                if (borlandState == 0) {
                    borlandState = 1;
                }
                else if (borlandState == 4) {
                    // Borland pattern succeeds. p-4 has the offset of mainInfo
                    Address mainInfo = Address(READ4_LE(*(m_image + rva - 4)));

                    // Address of main is at mainInfo+0x18
                    Address main = Address::INVALID;
                    if (m_binaryImage->readNativeAddr4(mainInfo + 0x18, main)) {
                        return main;
                    }
                    else {
                        return Address::INVALID;
                    }
                }
            }
            else {
                borlandState = 0;
            }

            break;

        case 0x59: // Pop ecx
            if (borlandState == 2) {
                borlandState = 3;
            }
            else {
                borlandState = 0;
            }

            break;

        case 0x68: // Push 4 byte immediate
            if (borlandState == 3) {
                borlandState++;
            }
            else {
                borlandState = 0;
            }

            break;

        default: borlandState = 0; break;
        }

        const int size = microX86Dis(rva + m_image);

        if (size == 0x40) {
            LOG_WARN("Microdisassembler out of step at offset %1", rva);
            break;
        }

        rva += size;
        gap++;
    }

    // VS.NET release console mode pattern
    rva = READ4_LE(m_peHeader->EntrypointRVA);

    if ((Util::readByte(m_image + rva + 0x20) == 0xff) &&
        (Util::readByte(m_image + rva + 0x21) == 0x15)) {
        Address desti                = Address(READ4_LE_P(m_image + rva + 0x22));
        const BinarySymbol *dest_sym = m_symbols->findSymbolByAddress(desti);

        if (dest_sym && (dest_sym->getName() == "GetVersionExA")) {
            if ((Util::readByte(m_image + rva + 0x6d) == 0xff) &&
                (Util::readByte(m_image + rva + 0x6e) == 0x15)) {
                desti    = Address(READ4_LE_P(m_image + rva + 0x6f));
                dest_sym = m_symbols->findSymbolByAddress(desti);

                if (dest_sym && (dest_sym->getName() == "GetModuleHandleA")) {
                    if (Util::readByte(m_image + rva + 0x16e) == 0xe8) {
                        Address dest = Address(rva + 0x16e + 5 + READ4_LE_P(rva + m_image + 0x16f));
                        return imageBase + dest;
                    }
                }
            }
        }
    }

    // For VS.NET, need an old favourite: find a call with three pushes in the first 100 instuctions
    int count     = 100;
    int numPushes = 0;
    rva           = READ4_LE(m_peHeader->EntrypointRVA);

    while (count > 0 && rva + 1 < m_imageSize) {
        count--;
        const Byte op1 = Util::readByte(m_image + rva + 0);
        const Byte op2 = Util::readByte(m_image + rva + 1);

        if (op1 == 0xE8) { // CALL opcode
            if (numPushes == 3) {
                // Get the offset
                int off      = READ4_LE(*(m_image + rva + 1));
                Address dest = Address(rva + 5 + off);

                // Check for a jump there
                const Byte destOp = *reinterpret_cast<Byte *>(m_image + dest.value());

                if (destOp == 0xE9) {
                    // Follow that jump
                    off = READ4_LE(*(m_image + dest.value() + 1));
                    dest += off + 5;
                }

                return dest + READ4_LE(m_peHeader->Imagebase);
            }
            else {
                numPushes = 0; // Assume pushes don't accumulate over calls
            }
        }
        else if ((op1 >= 0x50) && (op1 <= 0x57)) { // PUSH opcode
            numPushes++;
        }
        else if (op1 == 0xFF) {
            // FF 35 is push m[K]

            if (op2 == 0x35) {
                numPushes++;
            }
        }
        else if (op1 == 0xE9) {
            // Follow the jump
            const int off = READ4_LE(*(m_image + rva + 1));
            rva += off + 5;
            continue;
        }

        int size = microX86Dis(rva + m_image);

        if (size == 0x40) {
            LOG_WARN("Microdisassembler out of step at offset %1", rva);
            break;
        }

        rva += size;

        if (rva >= textSize) {
            break;
        }
    }

    // mingw pattern
    rva                      = READ4_LE(m_peHeader->EntrypointRVA);
    bool in_mingw_CRTStartup = false;
    Address lastcall         = Address::ZERO;
    Address lastlastcall     = Address::ZERO;

    while (true) {
        const Byte op1 = Util::readByte(m_image + rva);

        if (in_mingw_CRTStartup && op1 == 0xC3) {
            break;
        }

        if (op1 == 0xE8) { // CALL opcode
            unsigned int dest = rva + 5 + READ4_LE_P(m_image + rva + 1);
            if (Util::inRange(dest, 0U, m_imageSize)) {
                const Byte op2 = Util::readByte(m_image + dest);

                if (in_mingw_CRTStartup) {
                    const Byte op2a = Util::readByte(m_image + dest + 1);
                    Address desti   = Address(READ4_LE_P(m_image + dest + 2));

                    // skip all the call statements until we hit a call to an indirect call to
                    // ExitProcess; main is the 2nd call before this one
                    if (op2 == 0xff && op2a == 0x25) {
                        const BinarySymbol *dest_sym = m_symbols->findSymbolByAddress(desti);

                        if (dest_sym && (dest_sym->getName() == "ExitProcess")) {
                            m_mingwMain = true;
                            return Address(READ4_LE(m_peHeader->Imagebase)) + lastlastcall + 5 +
                                   READ4_LE_P(m_image + lastlastcall.value() + 1);
                        }
                    }

                    lastlastcall = lastcall;
                    lastcall     = Address(rva);
                }
                else {
                    rva                 = dest;
                    in_mingw_CRTStartup = true;
                    continue;
                }
            }
        }

        int size = microX86Dis(m_image + rva);

        if (size == 0x40) {
            LOG_WARN("Microdisassembler out of step at offset %1", rva);
            break;
        }

        rva += size;

        if (rva >= textSize) {
            break;
        }
    }

    // Microsoft VisualC 2-6/net runtime
    rva          = READ4_LE(m_peHeader->EntrypointRVA);
    bool gotGMHA = false; // has GetModuleHandleA been found?

    while (rva < textSize) {
        const Byte op1 = Util::readByte(m_image + rva + 0);
        const Byte op2 = Util::readByte(m_image + rva + 1);

        if (op1 == 0xFF && op2 == 0x15) { // indirect CALL opcode
            const Address destAddr      = Address(READ4_LE_P(m_image + rva + 2));
            const BinarySymbol *destSym = m_symbols->findSymbolByAddress(destAddr);

            if (destSym && (destSym->getName() == "GetModuleHandleA")) {
                gotGMHA = true;
            }
        }

        if (op1 == 0xE8 && gotGMHA) { // CALL opcode
            Address dest = Address(rva + 5 + READ4_LE(*(m_image + rva + 1)));
            m_symbols->createSymbol(dest + READ4_LE(m_peHeader->Imagebase), "WinMain");
            return dest + READ4_LE(m_peHeader->Imagebase);
        }

        if (op1 == 0xc3) { // ret ends search
            break;
        }

        int size = microX86Dis(rva + m_image);

        if (size == 0x40) {
            LOG_WARN("Microdisassembler out of step at offset %1", rva);
            break;
        }

        rva += size;
    }

    return Address::INVALID;
}


#if defined(_WIN32) && !defined(__MINGW32__)
BOOL CALLBACK lookforsource(dbghelp::PSOURCEFILE /*SourceFile*/, PVOID UserContext)
{
    *(bool *)UserContext = true;
    return FALSE;
}
#endif

void Win32BinaryLoader::processIAT()
{
    PEImportDtor *id = reinterpret_cast<PEImportDtor *>(
        (m_image + READ4_LE(m_peHeader->ImportTableRVA)));

    // If any import table entry exists
    if (m_peHeader->ImportTableRVA == 0 || id->name == 0) {
        return;
    }

    do {
        const DWord nameOffset = READ4_LE(id->name);
        const char *dllName    = m_image + nameOffset;
        if (!Util::inRange(nameOffset, 0U, m_imageSize)) {
            LOG_WARN("Cannot read IAT entry: name offset out or range");
            continue;
        }

        const DWord originalFirstThunk = READ4_LE(id->originalFirstThunk);
        const DWord firstThunk         = READ4_LE(id->firstThunk);

        if (!Util::inRange(originalFirstThunk, 0U, m_imageSize) ||
            !Util::inRange(firstThunk, 0U, m_imageSize)) {
            LOG_WARN("Cannot read IAT entry: thunk offset out of range");
            continue;
        }

        const DWord thunk = (originalFirstThunk != 0) ? originalFirstThunk : firstThunk;

        const DWord *iat = reinterpret_cast<const DWord *>(m_image + thunk);
        DWord iatEntry   = Util::readDWord(iat, Endian::Little);
        Address paddr    = Address(READ4_LE(m_peHeader->Imagebase) + firstThunk);

        while (iatEntry != 0) {
            if (reinterpret_cast<const char *>(iat) > m_image + m_imageSize) {
                LOG_WARN("Cannot read IAT entry: entry extends past file size");
                break;
            }

            if ((iatEntry >> 31) != 0) {
                // This is an ordinal number (stupid idea)
                // Dots can't be in identifiers
                QString nodots    = QString(dllName).replace(".", "_");
                nodots            = QString("%1_%2").arg(nodots).arg(iatEntry & ~(1 << 31));
                BinarySymbol *sym = m_symbols->createSymbol(paddr, nodots);
                sym->setAttribute("Imported", true);
                sym->setAttribute("Function", true);
            }
            else {
                // Normal case (IMAGE_IMPORT_BY_NAME). Skip the useless hint (2 bytes)
                if (!Util::inRange(iatEntry + 2, 0U, m_imageSize)) {
                    LOG_WARN("Cannot read IAT entry: entry name offset out of range");
                    break;
                }

                QString name = m_image + iatEntry + 2;

                BinarySymbol *sym = m_symbols->createSymbol(paddr, name);
                sym->setAttribute("Imported", true);
                sym->setAttribute("Function", true);
                Address old_loc = Address(HostAddress(iat).value() - HostAddress(m_image).value() +
                                          READ4_LE(m_peHeader->Imagebase));

                if (paddr != old_loc) { // add both possibilities
                    BinarySymbol *symbol = m_symbols->createSymbol(old_loc, QString("old_") + name);
                    symbol->setAttribute("Imported", true);
                    symbol->setAttribute("Function", true);
                }
            }

            iat++;
            iatEntry = READ4_LE_P(iat);
            paddr += 4;
        }
    } while ((++id)->name != 0);
}


void Win32BinaryLoader::readDebugData(QString exename)
{
#if defined(_WIN32) && !defined(__MINGW32__)
    // attempt to load symbols for the exe or dll

    DWORD error;
    HANDLE currProcess = GetCurrentProcess();

    dbghelp::SymSetOptions(SYMOPT_LOAD_LINES);

    if (!dbghelp::SymInitialize(currProcess, nullptr, FALSE)) {
        error = GetLastError();
        printf("SymInitialize returned error : %d\n", error);
        return;
    }

    DWORD64 dwBaseAddr = 0;
    dwBaseAddr = dbghelp::SymLoadModule64(currProcess, nullptr, qPrintable(exename), nullptr,
                                          dwBaseAddr, 0);

    if (dwBaseAddr != 0) {
        assert(dwBaseAddr == m_peHeader->Imagebase);
        bool found = false;
        dbghelp::SymEnumSourceFiles(currProcess, dwBaseAddr, 0, lookforsource, &found);
        m_hasDebugInfo = found;
    }
    else {
        // SymLoadModule64 failed
        error = GetLastError();
        printf("SymLoadModule64 returned error : %d\n", error);
        return;
    }
#else
    Q_UNUSED(exename);
    LOG_WARN("Loading PE debug information is only available on Windows!");
#endif
}


#define DOS_HEADER_SIZE 0x3C

bool Win32BinaryLoader::loadFromMemory(QByteArray &arr)
{
    const char *fileData = arr.constData();
    const DWord fileSize = arr.size();

    if (DOS_HEADER_SIZE + 4 /* ptr to PE header */ + sizeof(PEHeader) > fileSize) {
        LOG_ERROR("Invalid PE: File size too small");
        return false;
    }

    DWord peHeaderOffset = Util::readDWord(fileData + DOS_HEADER_SIZE, Endian::Little);
    if (peHeaderOffset + sizeof(PEHeader) > fileSize) {
        LOG_ERROR("Invalid PE: PE header extends past file boundary");
        return false;
    }

    const PEHeader *peHdr = reinterpret_cast<const PEHeader *>(fileData + peHeaderOffset);

    try {
        const DWord imageSize = READ4_LE(peHdr->ImageSize);
        m_image               = new char[imageSize];
        m_imageSize           = imageSize;
    }
    catch (const std::bad_alloc &) {
        LOG_ERROR("Cannot allocate memory for copy of image");
        return false;
    }

    const DWord dosHeaderSize = READ4_LE(peHdr->HeaderSize);
    if (dosHeaderSize >= fileSize) {
        LOG_ERROR("Invalid PE: DOS header extends past file boundary");
        return false;
    }

    memcpy(m_image, fileData, dosHeaderSize);
    m_header = reinterpret_cast<Header *>(m_image);

    if (!Util::testMagic((Byte *)m_header, { 'M', 'Z' })) {
        LOG_ERROR("Invalid PE: Bad magic");
        return false;
    }

    m_peHeader = reinterpret_cast<PEHeader *>(m_image + peHeaderOffset);
    if (!Util::testMagic((Byte *)m_peHeader, { 'P', 'E' })) {
        LOG_ERROR("Invalid PE: Bad PE magic");
        return false;
    }

    const SWord ntHeaderSize = Util::readWord(&m_peHeader->NtHdrSize, Endian::Little);
    const PEObject *o = reinterpret_cast<const PEObject *>((Byte *)(m_peHeader) + ntHeaderSize +
                                                           24);

    std::vector<SectionParam> params;

    const DWord numSections = Util::readWord(&m_peHeader->numObjects, Endian::Little);

    for (DWord i = 0; i < numSections; i++, o++) {
        const DWord rva      = READ4_LE(o->RVA);
        const DWord size     = READ4_LE(o->VirtualSize);
        const DWord physOff  = READ4_LE(o->PhysicalOffset);
        const DWord physSize = READ4_LE(o->PhysicalSize);

        SectionParam sect;
        // TODO: Check for unreadable sections (!IMAGE_SCN_MEM_READ)?
        // FIXME Using std::min fixes the crash but does not solve the root issue.
        // This needs further consideration.
        memset(m_image + rva, 0, size);
        memcpy(m_image + rva, fileData + physOff, std::min(physSize, size));

        sect.Name         = QByteArray(o->ObjectName, 8);
        sect.From         = Address(READ4_LE(m_peHeader->Imagebase) + rva);
        sect.ImageAddress = HostAddress(m_image) + rva;
        sect.Size         = size;
        sect.PhysSize     = physSize;

        // clang-format off
        const DWord peFlags = READ4_LE(o->Flags);
        sect.Bss            = (peFlags & IMAGE_SCN_CNT_UNINITIALIZED_DATA) ? true  : false;
        sect.Code           = (peFlags & IMAGE_SCN_CNT_CODE)               ? true  : false;
        sect.Data           = (peFlags & IMAGE_SCN_CNT_INITIALIZED_DATA)   ? true  : false;
        sect.ReadOnly       = (peFlags & IMAGE_SCN_MEM_WRITE)              ? false : true;
        // clang-fomat on

        params.push_back(sect);
    }

    for (SectionParam par : params) {
        BinarySection *sect = m_binaryImage->createSection(par.Name, par.From, par.From + par.Size);

        if (!sect) {
            LOG_WARN("Cannot create PE section '%1'", par.Name);
            continue;
        }

        sect->setBss(par.Bss);
        sect->setCode(par.Code);
        sect->setData(par.Data);
        sect->setReadOnly(par.ReadOnly);
        sect->setHostAddr(par.ImageAddress);
        sect->setEndian(Endian::Little); // little endian

        if (!(par.Bss || par.From.isZero())) {
            sect->addDefinedArea(par.From, par.From + par.PhysSize);
        }
    }

    // Add the Import Address Table entries to the symbol table
    processIAT();

    // Was hoping that _main or main would turn up here for Borland console mode programs. No such
    // luck. I think IDA Pro must find it by a combination of FLIRT and some pattern matching
    // PEExportDtor* eid = (PEExportDtor*)
    //    (LMMH(m_peHeader->ExportTableRVA) + base);

    // Give the entry point a symbol
    Address entry = getMainEntryPoint();

    if (entry != Address::INVALID) {
        if (!m_symbols->findSymbolByAddress(entry)) {
            m_symbols->createSymbol(entry, "main")->setAttribute("Function", true);
        }
    }

    // Give a name to any jumps you find to these import entries
    // NOTE: VERY early MSVC specific!! Temporary till we can think of a better way.
    Address start = getEntryPoint();
    findJumps(start);

    // TODO: loading debuging data should be an optional step, decision should be made 'upstream'
    // readDebugData();
    return true;
}


int Win32BinaryLoader::canLoad(QIODevice &fl) const
{
    unsigned char buf[64];

    fl.read(reinterpret_cast<char *>(buf), sizeof(buf));

    if (Util::testMagic(buf, { 'M', 'Z' })) { /* DOS-based file */
        int peoff = READ4_LE(buf[0x3C]);

        if ((peoff != 0) && fl.seek(peoff)) {
            fl.read(reinterpret_cast<char *>(buf), 4);

            if (Util::testMagic(buf, { 'P', 'E', 0, 0 })) {
                /* Win32 Binary */
                return 2 + 4 + 4;
            }
        }
    }

    return 0;
}


/**
 * \internal Used above for a hack to find jump instructions pointing to IATs.
 * Heuristic: start just before the "start" entry point looking for FF 25 opcodes followed by a
 * pointer to an import entry.  E.g. FF 25 58 44 40 00  where 00404458 is the IAT for _ftol. Note:
 * some are on 0x10 byte boundaries, some on 2 byte boundaries (6 byte jumps packed), and there are
 * often up to 0x30 bytes of statically linked library code (e.g. _atexit, __onexit) with sometimes
 * two static libs in a row. So keep going until there is about 0x60 bytes with no match. Note:
 * slight chance of coming across a misaligned match; probability is about 1/65536 times dozens in
 * 2^32 ~= 10^-13
 */
void Win32BinaryLoader::findJumps(Address curr)
{
    BinarySection *section = m_binaryImage->getSectionByName(".text");
    if (section == nullptr) {
        section = m_binaryImage->getSectionByName("CODE");
        if (section == nullptr) {
            return;
        }
    }

    // Add to native addr to get host:
    ptrdiff_t delta = (section->getHostAddr() - section->getSourceAddr()).value();

    int cnt = 0;         // Count of bytes with no match
    while (cnt < 0x60) { // Max of 0x60 bytes without a match
        curr -= 2;       // Has to be on 2-byte boundary
        cnt += 2;

        if (curr < section->getSourceAddr()) {
            break; // stepped out of section
        }

        const HostAddress instrPtr(curr, delta);
        const Byte *opcode = reinterpret_cast<const Byte *>(instrPtr.value());
        if (opcode[0] != 0xFF || opcode[1] != 0x25) { // only check FF 25 jumps
            continue;
        }

        Address operand            = Address(READ4_LE_P(HostAddress(curr, delta + 2)));
        const BinarySymbol *symbol = m_symbols->findSymbolByAddress(operand);

        if (symbol == nullptr) {
            continue;
        }

        // try to rename symbol
        const QString oldName = symbol->getName();
        if (m_symbols->renameSymbol(oldName, "__imp_" + oldName) == false) {
            continue;
        }

        BinarySymbol *sym = m_symbols->createSymbol(curr, oldName);
        sym->setAttribute("Function", true);
        sym->setAttribute("Imported", true);
        curr -= 4; // Next match is at least 4+2 bytes away
        cnt = 0;
    }
}


void Win32BinaryLoader::unload()
{
    m_imageSize = 0;
    m_numRelocs = 0;

    delete[] m_image;
    m_image = nullptr;
}


#if defined(_WIN32) && !defined(__MINGW32__)

// clang-format off
char *SymTagEnums[] =
{
    "SymTagNull",               "SymTagExe",                "SymTagCompiland",
    "SymTagCompilandDetails",   "SymTagCompilandEnv",       "SymTagFunction",
    "SymTagBlock",              "SymTagData",               "SymTagAnnotation",
    "SymTagLabel",              "SymTagPublicSymbol",       "SymTagUDT",
    "SymTagEnum",               "SymTagFunctionType",       "SymTagPointerType",
    "SymTagArrayType",          "SymTagBaseType",           "SymTagTypedef",
    "SymTagBaseClass",          "SymTagFriend",             "SymTagFunctionArgType",
    "SymTagFuncDebugStart",     "SymTagFuncDebugEnd",       "SymTagUsingNamespace",
    "SymTagVTableShape",        "SymTagVTable",             "SymTagCustom",
    "SymTagThunk",              "SymTagCustomType",         "SymTagManagedType",
    "SymTagDimension"
};

enum SymTagEnum
{
    SymTagNull,                 SymTagExe,                  SymTagCompiland,
    SymTagCompilandDetails,     SymTagCompilandEnv,         SymTagFunction,
    SymTagBlock,                SymTagData,                 SymTagAnnotation,
    SymTagLabel,                SymTagPublicSymbol,         SymTagUDT,
    SymTagEnum,                 SymTagFunctionType,         SymTagPointerType,
    SymTagArrayType,            SymTagBaseType,             SymTagTypedef,
    SymTagBaseClass,            SymTagFriend,               SymTagFunctionArgType,
    SymTagFuncDebugStart,       SymTagFuncDebugEnd,         SymTagUsingNamespace,
    SymTagVTableShape,          SymTagVTable,               SymTagCustom,
    SymTagThunk,                SymTagCustomType,           SymTagManagedType,
    SymTagDimension
};

char *basicTypes[] =
{
    "notype",
    "void",
    "char",
    "WCHAR",
    "??",
    "??",
    "int",
    "unsigned int",
    "float",
    "bcd",
    "bool",
    "??",
    "??",
    "long"
    "unsigned long",
};
// clang-format on

void printType(DWORD index, DWORD64 ImageBase)
{
    HANDLE hProcess = GetCurrentProcess();

    int got;
    WCHAR *name;

    got = dbghelp::SymGetTypeInfo(hProcess, ImageBase, index, dbghelp::TI_GET_SYMNAME, &name);

    if (got) {
        char nameA[1024];
        WideCharToMultiByte(CP_ACP, 0, name, -1, nameA, sizeof(nameA), 0, nullptr);
        LOG_VERBOSE("Found type info: %1", nameA);
        return;
    }

    DWORD d;
    got = dbghelp::SymGetTypeInfo(hProcess, ImageBase, index, dbghelp::TI_GET_SYMTAG, &d);
    assert(got);

    switch (d) {
    case SymTagPointerType:
        got = dbghelp::SymGetTypeInfo(hProcess, ImageBase, index, dbghelp::TI_GET_TYPE, &d);
        assert(got);
        printType(d, ImageBase);
        LOG_VERBOSE("*");
        break;

    case SymTagBaseType:
        got = dbghelp::SymGetTypeInfo(hProcess, ImageBase, index, dbghelp::TI_GET_BASETYPE, &d);
        assert(got);
        LOG_VERBOSE("%1", basicTypes[d]);
        break;

    default: LOG_FATAL("unhandled symtag %1", SymTagEnums[d]);
    }
}


BOOL CALLBACK printem(dbghelp::PSYMBOL_INFO symInfo, ULONG /*SymbolSize*/, PVOID /*UserContext*/)
{
    printType(symInfo->TypeIndex, symInfo->ModBase);

    QString flagsStr;
    OStream ost(&flagsStr);

    ost << " " << symInfo->Name << " flags: ";

    if (symInfo->Flags & SYMFLAG_VALUEPRESENT) {
        ost << "value present, ";
    }

    if (symInfo->Flags & SYMFLAG_REGISTER) {
        ost << "register, ";
    }

    if (symInfo->Flags & SYMFLAG_REGREL) {
        ost << "regrel, ";
    }

    if (symInfo->Flags & SYMFLAG_FRAMEREL) {
        ost << "framerel, ";
    }

    if (symInfo->Flags & SYMFLAG_PARAMETER) {
        ost << "parameter, ";
    }

    if (symInfo->Flags & SYMFLAG_LOCAL) {
        ost << "local, ";
    }

    if (symInfo->Flags & SYMFLAG_CONSTANT) {
        ost << "constant, ";
    }

    if (symInfo->Flags & SYMFLAG_EXPORT) {
        ost << "export, ";
    }

    if (symInfo->Flags & SYMFLAG_FORWARDER) {
        ost << "forwarder, ";
    }

    if (symInfo->Flags & SYMFLAG_FUNCTION) {
        ost << "function, ";
    }

    if (symInfo->Flags & SYMFLAG_VIRTUAL) {
        ost << "virtual, ";
    }

    if (symInfo->Flags & SYMFLAG_THUNK) {
        ost << "thunk, ";
    }

    if (symInfo->Flags & SYMFLAG_TLSREL) {
        ost << "tlsrel, ";
    }

    LOG_VERBOSE(flagsStr);
    LOG_VERBOSE("register: %1, address: %2", (int)symInfo->Register, Address(symInfo->Address));
    return TRUE;
}


#endif


SWord Win32BinaryLoader::win32Read2(const void *src) const
{
    return Util::readWord(src, Endian::Little);
}


DWord Win32BinaryLoader::win32Read4(const void *src) const
{
    return Util::readDWord(src, Endian::Little);
}


bool Win32BinaryLoader::isStaticLinkedLibProc(Address addr) const
{
#if defined(_WIN32) && !defined(__MINGW32__)
    HANDLE hProcess = GetCurrentProcess();
    dbghelp::IMAGEHLP_LINE64 line;
    line.SizeOfStruct = sizeof(line);
    line.FileName     = nullptr;
    dbghelp::SymGetLineFromAddr64(hProcess, addr.value(), 0, &line);

    if (m_hasDebugInfo && (line.FileName == nullptr) || line.FileName && (*line.FileName == 'f')) {
        return true;
    }
#endif

    return (isMinGWsAllocStack(addr) || isMinGWsFrameInit(addr) || isMinGWsFrameEnd(addr) ||
            isMinGWsCleanupSetup(addr) || isMinGWsMalloc(addr));
}


bool Win32BinaryLoader::isMinGWsAllocStack(Address addr) const
{
    if (m_mingwMain) {
        const BinarySection *section = m_binaryImage->getSectionByAddr(addr);

        if (section) {
            HostAddress hostAddr    = section->getHostAddr() - section->getSourceAddr() + addr;
            unsigned char pattern[] = { 0x51, 0x89, 0xE1, 0x83, 0xC1, 0x08, 0x3D, 0x00, 0x10,
                                        0x00, 0x00, 0x72, 0x10, 0x81, 0xE9, 0x00, 0x10, 0x00,
                                        0x00, 0x83, 0x09, 0x00, 0x2D, 0x00, 0x10, 0x00, 0x00,
                                        0xEB, 0xE9, 0x29, 0xC1, 0x83, 0x09, 0x00, 0x89, 0xE0,
                                        0x89, 0xCC, 0x8B, 0x08, 0x8B, 0x40, 0x04, 0xFF, 0xE0 };

            if (memcmp(hostAddr, pattern, sizeof(pattern)) == 0) {
                return true;
            }
        }
    }

    return false;
}


bool Win32BinaryLoader::isMinGWsFrameInit(Address addr) const
{
    if (!m_mingwMain) {
        return false;
    }

    const BinarySection *section = m_binaryImage->getSectionByAddr(addr);

    if (!section) {
        return false;
    }

    HostAddress hostAddr = section->getHostAddr() - section->getSourceAddr() + addr;
    unsigned char pat1[] = { 0x55, 0x89, 0xE5, 0x83, 0xEC, 0x18, 0x89, 0x7D, 0xFC,
                             0x8B, 0x7D, 0x08, 0x89, 0x5D, 0xF4, 0x89, 0x75, 0xF8 };

    if (memcmp(hostAddr, pat1, sizeof(pat1)) != 0) {
        return false;
    }

    unsigned char pat2[] = { 0x85, 0xD2, 0x74, 0x24, 0x8B, 0x42, 0x2C, 0x85, 0xC0, 0x78,
                             0x3D, 0x8B, 0x42, 0x2C, 0x85, 0xC0, 0x75, 0x56, 0x8B, 0x42,
                             0x28, 0x89, 0x07, 0x89, 0x7A, 0x28, 0x8B, 0x5D, 0xF4, 0x8B,
                             0x75, 0xF8, 0x8B, 0x7D, 0xFC, 0x89, 0xEC, 0x5D, 0xC3 };
    return memcmp(hostAddr + sizeof(pat1) + 6, pat2, sizeof(pat2)) == 0;
}


bool Win32BinaryLoader::isMinGWsFrameEnd(Address addr) const
{
    if (!m_mingwMain) {
        return false;
    }

    const BinarySection *section = m_binaryImage->getSectionByAddr(addr);

    if (!section) {
        return false;
    }

    HostAddress host     = section->getHostAddr() - section->getSourceAddr() + addr;
    unsigned char pat1[] = {
        0x55, 0x89, 0xE5, 0x53, 0x83, 0xEC, 0x14, 0x8B, 0x45, 0x08, 0x8B, 0x18
    };

    if (memcmp(host, pat1, sizeof(pat1)) != 0) {
        return false;
    }

    unsigned char pat2[] = { 0x85, 0xC0, 0x74, 0x1B, 0x8B, 0x48, 0x2C, 0x85, 0xC9,
                             0x78, 0x34, 0x8B, 0x50, 0x2C, 0x85, 0xD2, 0x75, 0x4D,
                             0x89, 0x58, 0x28, 0x8B, 0x5D, 0xFC, 0xC9, 0xC3 };
    return memcmp(host + sizeof(pat1) + 5, pat2, sizeof(pat2)) == 0;
}


bool Win32BinaryLoader::isMinGWsCleanupSetup(Address addr) const
{
    if (!m_mingwMain) {
        return false;
    }

    const BinarySection *section = m_binaryImage->getSectionByAddr(addr);

    if (!section) {
        return false;
    }

    HostAddress host     = section->getHostAddr() - section->getSourceAddr() + addr;
    unsigned char pat1[] = { 0x55, 0x89, 0xE5, 0x53, 0x83, 0xEC, 0x04 };

    if (memcmp(host, pat1, sizeof(pat1)) != 0) {
        return false;
    }

    unsigned char pat2[] = { 0x85, 0xDB, 0x75, 0x35 };

    if (memcmp(host + sizeof(pat1) + 6, pat2, sizeof(pat2)) != 0) {
        return false;
    }

    unsigned char pat3[] = { 0x83, 0xF8, 0xFF, 0x74, 0x24, 0x85, 0xC0, 0x89,
                             0xC3, 0x74, 0x0E, 0x8D, 0x74, 0x26, 0x00 };
    return memcmp(host + sizeof(pat1) + 6 + sizeof(pat2) + 16, pat3, sizeof(pat3)) == 0;
}


bool Win32BinaryLoader::isMinGWsMalloc(Address addr) const
{
    if (!m_mingwMain) {
        return false;
    }

    const BinarySection *section = m_binaryImage->getSectionByAddr(addr);

    if (!section) {
        return false;
    }

    HostAddress host     = section->getHostAddr() - section->getSourceAddr() + addr;
    unsigned char pat1[] = { 0x55, 0x89, 0xE5, 0x8D, 0x45, 0xF4, 0x83, 0xEC, 0x58,
                             0x89, 0x45, 0xE0, 0x8D, 0x45, 0xC0, 0x89, 0x04, 0x24,
                             0x89, 0x5D, 0xF4, 0x89, 0x75, 0xF8, 0x89, 0x7D, 0xFC };

    if (memcmp(host, pat1, sizeof(pat1)) != 0) {
        return false;
    }

    unsigned char pat2[] = { 0x89, 0x65, 0xE8 };
    return memcmp((host + sizeof(pat1) + 0x15), pat2, sizeof(pat2)) == 0;
}


Address Win32BinaryLoader::getJumpTarget(Address addr) const
{
    Byte opcode = 0;
    if (!m_binaryImage->readNative1(addr, opcode)) {
        return Address::INVALID;
    }
    else if (opcode != 0xE9) {
        return Address::INVALID;
    }

    DWord disp = 0;
    if (!m_binaryImage->readNative4(addr + 1, disp)) {
        return Address::INVALID;
    }

    // Note: for backwards jumps, this wraps around since we have unsigned integers
    return Address(addr + 5 + disp);
}


LoadFmt Win32BinaryLoader::getFormat() const
{
    return LoadFmt::PE;
}


Machine Win32BinaryLoader::getMachine() const
{
    return Machine::X86;
}


bool Win32BinaryLoader::isLibrary() const
{
    return (m_peHeader->Flags & 0x2000) != 0;
}


BOOMERANG_DEFINE_PLUGIN(PluginType::FileLoader, Win32BinaryLoader, "Win32 PE loader plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
