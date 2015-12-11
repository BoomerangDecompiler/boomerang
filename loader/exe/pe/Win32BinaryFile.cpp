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

/***************************************************************************/ /**
  * \file Win32BinaryFile.cpp
  * This file contains the implementation of the class Win32BinaryFile.
  */

/* Win32 binary file format.
 *    This file implements the class Win32BinaryFile, derived from class
 *    BinaryFile. See Win32BinaryFile.h and BinaryFile.h for details.
 * 25 Jun 02 - Mike: Added code to find WinMain by finding a call within 5
 *                instructions of a call to GetModuleHandleA
 * 07 Jul 02 - Mike: Added a LMMH() so code works on big-endian host
 * 08 Jul 02 - Mike: Changed algorithm to find main; now looks for ordinary
 *                 call up to 10 instructions before an indirect call to exit
 * 24 Jul 05 - Mike: State machine to recognize main in Borland Builder files
 */

#ifdef _WIN32
#include <windows.h>
#ifndef __MINGW32__
namespace dbghelp {
#include <dbghelp.h>
};
#endif
#endif
#include "Win32BinaryFile.h"

#include "BinaryFile.h"
#include "IBinaryImage.h"
#include "IBinarySymbols.h"
#include "IBoomerang.h"
#include "config.h"

#include <cstring>
#include <cstdlib>
#include <cassert>
#include <QString>
#include <QFile>

extern "C" {
int microX86Dis(void *p); // From microX86dis.c
}
namespace {

struct SectionParam {
    QString Name;
    ADDRESS From;
    size_t Size;
    size_t PhysSize;
    ADDRESS ImageAddress;
    bool Bss,Code,Data,ReadOnly;
};

}
#ifndef IMAGE_SCN_CNT_CODE // Assume that if one is not defined, the rest isn't either.
#define IMAGE_SCN_CNT_CODE 0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080
#define IMAGE_SCN_MEM_READ 0x40000000
#define IMAGE_SCN_MEM_WRITE 0x80000000
#endif

Win32BinaryFile::Win32BinaryFile() : mingw_main(false) {
}

Win32BinaryFile::~Win32BinaryFile() {
    if(base)
        free(base);
}
void Win32BinaryFile::initialize(IBoomerang *sys) {
    Image = sys->getImage();
    Symbols = sys->getSymbols();
}
void Win32BinaryFile::Close() { UnLoad(); }

ADDRESS Win32BinaryFile::GetEntryPoint() {
    return ADDRESS::g(LMMH(m_pPEHeader->EntrypointRVA) + LMMH(m_pPEHeader->Imagebase));
}

// This is a bit of a hack, but no more than the rest of Windows :-O  The pattern is to look for an indirect call (FF 15
// opcode) to exit; within 10 instructions before that should be the call to WinMain (with no other calls inbetween).
// This pattern should work for "old style" and "new style" PE executables, as well as console mode PE files.
ADDRESS Win32BinaryFile::GetMainEntryPoint() {
    auto aMain = Symbols->find("main");
    if (aMain)
        return aMain->getLocation();
    aMain = Symbols->find("_main"); // Example: MinGW
    if (aMain)
        return aMain->getLocation();
    aMain = Symbols->find("WinMain"); // Example: MinGW
    if (aMain)
        return aMain->getLocation();

    // Start at program entry point
    unsigned p = LMMH(m_pPEHeader->EntrypointRVA);
    unsigned lim = p + 0x200;
    unsigned char op1, op2;
    ADDRESS addr;
    unsigned lastOrdCall = 0;
    int gap;              // Number of instructions from the last ordinary call
    int borlandState = 0; // State machine for Borland

    IBinarySection *si = Image->GetSectionInfoByName(".text");
    if (si == nullptr)
        si = Image->GetSectionInfoByName("CODE");
    assert(si);
    unsigned textSize = si->size();
    if (textSize < 0x200)
        lim = p + textSize;

    if (m_pPEHeader->Subsystem == 1) // native
        return ADDRESS::g(LMMH(m_pPEHeader->EntrypointRVA) + LMMH(m_pPEHeader->Imagebase));

    gap = 0xF0000000; // Large positive number (in case no ordinary calls)
    while (p < lim) {
        op1 = *(unsigned char *)(p + base);
        op2 = *(unsigned char *)(p + base + 1);
        //        std::cerr << std::hex << "At " << p << ", ops " << (unsigned)op1 << ", " << (unsigned)op2 << std::dec
        //        << "\n";
        switch (op1) {
        case 0xE8: {
            // An ordinary call; this could be to winmain/main
            lastOrdCall = p;
            gap = 0;
            if (borlandState == 1)
                borlandState++;
            else
                borlandState = 0;
            break;
        }
        case 0xFF:
            if (op2 == 0x15) { // Opcode FF 15 is indirect call
                // Get the 4 byte address from the instruction
                addr = LMMH(*(p + base + 2));
                //                    const char *c = dlprocptrs[addr].c_str();
                //                    printf("Checking %x finding %s\n", addr, c);
                auto exit_sym = Symbols->find(addr);
                if (exit_sym && exit_sym->getName() == "exit") {
                    if (gap <= 10) {
                        // This is it. The instruction at lastOrdCall is (win)main
                        addr = LMMH(*(lastOrdCall + base + 1));
                        addr += lastOrdCall + 5; // Addr is dest of call
                        //                            printf("*** MAIN AT 0x%x ***\n", addr);
                        return addr + LMMH(m_pPEHeader->Imagebase);
                    }
                }
            } else
                borlandState = 0;
            break;
        case 0xEB:           // Short relative jump, e.g. Borland
            if (op2 >= 0x80) // Branch backwards?
                break;       // Yes, just ignore it
            // Otherwise, actually follow the branch. May have to modify this some time...
            p += op2 + 2; // +2 for the instruction itself, and op2 for the displacement
            gap++;
            continue;
        case 0x6A:
            if (op2 == 0) { // Push 00
                // Borland pattern: push 0 / call __ExceptInit / pop ecx / push offset mainInfo / push 0
                // Borland state before: 0                1               2            3                4
                if (borlandState == 0)
                    borlandState = 1;
                else if (borlandState == 4) {
                    // Borland pattern succeeds. p-4 has the offset of mainInfo
                    ADDRESS mainInfo = ADDRESS::g(LMMH(*(base + p - 4)));
                    ADDRESS main =
                            ADDRESS::g(Image->readNative4(mainInfo + ADDRESS::g(0x18))); // Address of main is at mainInfo+18
                    return main;
                }
            } else
                borlandState = 0;
            break;
        case 0x59: // Pop ecx
            if (borlandState == 2)
                borlandState = 3;
            else
                borlandState = 0;
            break;
        case 0x68: // Push 4 byte immediate
            if (borlandState == 3)
                borlandState++;
            else
                borlandState = 0;
            break;
        default:
            borlandState = 0;
            break;
        }
        int size = microX86Dis(p + base);
        if (size == 0x40) {
            fprintf(stderr, "Warning! Microdisassembler out of step at offset 0x%x\n", p);
            size = 1;
        }
        p += size;
        gap++;
    }

    // VS.NET release console mode pattern
    p = LMMH(m_pPEHeader->EntrypointRVA);
    if (*(unsigned char *)(p + base + 0x20) == 0xff && *(unsigned char *)(p + base + 0x21) == 0x15) {
        ADDRESS desti = ADDRESS::g(LMMH(*(p + base + 0x22)));
        auto dest_sym = Symbols->find(desti);
        if (dest_sym && dest_sym->getName() == "GetVersionExA") {
            if (*(unsigned char *)(p + base + 0x6d) == 0xff && *(unsigned char *)(p + base + 0x6e) == 0x15) {
                desti = LMMH(*(p + base + 0x6f));
                dest_sym = Symbols->find(desti);
                if (dest_sym && dest_sym->getName() == "GetModuleHandleA") {
                    if (*(unsigned char *)(p + base + 0x16e) == 0xe8) {
                        ADDRESS dest = ADDRESS::g(p + 0x16e + 5 + LMMH(*(p + base + 0x16f)));
                        return dest + LMMH(m_pPEHeader->Imagebase);
                    }
                }
            }
        }
    }

    // For VS.NET, need an old favourite: find a call with three pushes in the first 100 instuctions
    int count = 100;
    int pushes = 0;
    p = LMMH(m_pPEHeader->EntrypointRVA);
    while (count > 0) {
        count--;
        op1 = *(unsigned char *)(p + base);
        if (op1 == 0xE8) { // CALL opcode
            if (pushes == 3) {
                // Get the offset
                int off = LMMH(*(p + base + 1));
                ADDRESS dest = ADDRESS::g((unsigned)p + 5 + off);
                // Check for a jump there
                op1 = *(unsigned char *)(dest.m_value + base);
                if (op1 == 0xE9) {
                    // Follow that jump
                    off = LMMH(*(dest.m_value + base + 1));
                    dest = dest + 5 + off;
                }
                return dest + LMMH(m_pPEHeader->Imagebase);
            } else
                pushes = 0;                    // Assume pushes don't accumulate over calls
        } else if (op1 >= 0x50 && op1 <= 0x57) // PUSH opcode
            pushes++;
        else if (op1 == 0xFF) {
            // FF 35 is push m[K]
            op2 = *(unsigned char *)(p + 1 + base);
            if (op2 == 0x35)
                pushes++;
        } else if (op1 == 0xE9) {
            // Follow the jump
            int off = LMMH(*(p + base + 1));
            p += off + 5;
            continue;
        }

        int size = microX86Dis(p + base);
        if (size == 0x40) {
            fprintf(stderr, "Warning! Microdisassembler out of step at offset 0x%x\n", p);
            size = 1;
        }
        p += size;
        if (p >= textSize)
            break;
    }

    // mingw pattern
    p = LMMH(m_pPEHeader->EntrypointRVA);
    bool in_mingw_CRTStartup = false;
    ADDRESS lastcall = ADDRESS::g(0L), lastlastcall = ADDRESS::g(0L);
    while (1) {
        op1 = *(unsigned char *)(p + base);
        if (in_mingw_CRTStartup && op1 == 0xC3)
            break;
        if (op1 == 0xE8) { // CALL opcode
            unsigned int dest = p + 5 + LMMH(*(p + base + 1));
            if (in_mingw_CRTStartup) {
                op2 = *(unsigned char *)(dest + base);
                unsigned char op2a = *(unsigned char *)(dest + base + 1);
                ADDRESS desti = ADDRESS::g(LMMH(*(dest + base + 2)));
                // skip all the call statements until we hit a call to an indirect call to ExitProcess
                // main is the 2nd call before this one
                if (op2 == 0xff && op2a == 0x25) {
                    auto dest_sym = Symbols->find(desti);
                    if(dest_sym && dest_sym->getName() == "ExitProcess") {
                        mingw_main = true;
                        return lastlastcall + 5 + LMMH(*(lastlastcall.m_value + base + 1)) + LMMH(m_pPEHeader->Imagebase);
                    }
                }
                lastlastcall = lastcall;
                lastcall = p;
            } else {
                p = dest;
                in_mingw_CRTStartup = true;
                continue;
            }
        }

        int size = microX86Dis(p + base);
        if (size == 0x40) {
            fprintf(stderr, "Warning! Microdisassembler out of step at offset 0x%x\n", p);
            size = 1;
        }
        p += size;
        if (p >= textSize)
            break;
    }

    // Microsoft VisualC 2-6/net runtime
    p = LMMH(m_pPEHeader->EntrypointRVA);
    bool gotGMHA = false;
    while (1) {
        op1 = *(unsigned char *)(p + base);
        op2 = *(unsigned char *)(p + base + 1);
        if (op1 == 0xFF && op2 == 0x15) { // indirect CALL opcode
            ADDRESS desti = ADDRESS::g(LMMH(*(p + base + 2)));
            auto dest_sym = Symbols->find(desti);
            if (dest_sym && dest_sym->getName() == "GetModuleHandleA") {
                gotGMHA = true;
            }
        }
        if (op1 == 0xE8 && gotGMHA) { // CALL opcode
            ADDRESS dest = ADDRESS::g(p + 5 + LMMH(*(p + base + 1)));
            Symbols->create(dest + LMMH(m_pPEHeader->Imagebase),"WinMain");
            return dest + LMMH(m_pPEHeader->Imagebase);
        }
        if (op1 == 0xc3) // ret ends search
            break;

        int size = microX86Dis(p + base);
        if (size == 0x40) {
            fprintf(stderr, "Warning! Microdisassembler out of step at offset 0x%x\n", p);
            size = 1;
        }
        p += size;
        if (p >= textSize)
            break;
    }

    return NO_ADDRESS;
}

#if defined(_WIN32) && !defined(__MINGW32__)
BOOL CALLBACK lookforsource(dbghelp::PSOURCEFILE pSourceFile, PVOID UserContext) {
    *(bool *)UserContext = true;
    return FALSE;
}
#endif

void Win32BinaryFile::processIAT()
{
    PEImportDtor *id = (PEImportDtor *)(LMMH(m_pPEHeader->ImportTableRVA) + base);
    if (m_pPEHeader->ImportTableRVA) { // If any import table entry exists
        while (id->name != 0) {
            char *dllName = LMMH(id->name) + base;
            unsigned thunk = id->originalFirstThunk ? id->originalFirstThunk : id->firstThunk;
            unsigned *iat = (unsigned *)(LMMH(thunk) + base);
            unsigned iatEntry = LMMH(*iat);
            ADDRESS paddr = ADDRESS::g(LMMH(id->firstThunk) + LMMH(m_pPEHeader->Imagebase));
            while (iatEntry) {
                if (iatEntry >> 31) {
                    // This is an ordinal number (stupid idea)
                    QString nodots = QString(dllName).replace(".","_"); // Dots can't be in identifiers
                    nodots = QString("%1_%2").arg(nodots).arg(iatEntry & 0x7FFFFFFF);
                    Symbols->create(paddr,nodots).setAttr("Imported",true).setAttr("Function",true);
                } else {
                    // Normal case (IMAGE_IMPORT_BY_NAME). Skip the useless hint (2 bytes)
                    QString name((const char *)(iatEntry + 2 + base));
                    Symbols->create(paddr,name).setAttr("Imported",true).setAttr("Function",true);
                    ADDRESS old_loc = ADDRESS::host_ptr(iat) - ADDRESS::host_ptr(base) + LMMH(m_pPEHeader->Imagebase);
                    if (paddr != old_loc) // add both possibilities
                        Symbols->create(old_loc,QString("old_") + name).setAttr("Imported",true).setAttr("Function",true);
                }
                iat++;
                iatEntry = LMMH(*iat);
                paddr += 4;
            }
            id++;
        }
    }
}
void Win32BinaryFile::readDebugData() {
#if defined(_WIN32) && !defined(__MINGW32__)
    // attempt to load symbols for the exe or dll

    DWORD error;
    HANDLE hProcess;

    hProcess = GetCurrentProcess();
    // hProcess = (HANDLE)processId;

    dbghelp::SymSetOptions(SYMOPT_LOAD_LINES);

    if (dbghelp::SymInitialize(hProcess, nullptr, FALSE)) {
        // SymInitialize returned success
    } else {
        // SymInitialize failed
        error = GetLastError();
        printf("SymInitialize returned error : %d\n", error);
        return true;
    }

    DWORD64 dwBaseAddr = 0;

    if (dwBaseAddr = dbghelp::SymLoadModule64(hProcess, nullptr, (PSTR)sName, nullptr, dwBaseAddr, 0)) {
        assert(dwBaseAddr == m_pPEHeader->Imagebase);
        bool found = false;
        dbghelp::SymEnumSourceFiles(hProcess, dwBaseAddr, 0, lookforsource, &found);
        haveDebugInfo = found;
    } else {
        // SymLoadModule64 failed
        error = GetLastError();
        printf("SymLoadModule64 returned error : %d\n", error);
        return true;
    }
#endif
}
bool Win32BinaryFile::LoadFromArray(QByteArray &arr) {
    const char *data = arr.constData();
    const char *data_end = arr.constData() + arr.size();
    if(arr.size()<int(0x40+sizeof(PEHeader)))
        return false;
    DWord peoffLE, peoff;
    peoffLE = *(DWord *)(data+0x3C); // Note: peoffLE will be in Little Endian
    peoff = LMMH(peoffLE);
    if(data+peoff >= data_end)
        return false;
    PEHeader *tmphdr = (PEHeader *)(data+peoff);

    // Note: all tmphdr fields will be little endian

    base = (char *)malloc(LMMH(tmphdr->ImageSize));

    if (!base) {
        fprintf(stderr, "Cannot allocate memory for copy of image\n");
        return false;
    }
    if(data+LMMH(tmphdr->HeaderSize)>=data_end)
        return false;

    memcpy(base,data,LMMH(tmphdr->HeaderSize));
    m_pHeader = (Header *)base;
    if (m_pHeader->sigLo != 'M' || m_pHeader->sigHi != 'Z') {
        fprintf(stderr, "error loading file %s, bad magic\n", qPrintable(m_pFileName));
        return false;
    }

    m_pPEHeader = (PEHeader *)(base + peoff);
    if (m_pPEHeader->sigLo != 'P' || m_pPEHeader->sigHi != 'E') {
        fprintf(stderr, "error loading file %s, bad PE magic\n", qPrintable(m_pFileName));
        return false;
    }

    // printf("Image Base %08X, real base %p\n", LMMH(m_pPEHeader->Imagebase), base);

    const PEObject *o = (PEObject *)(((char *)m_pPEHeader) + LH(&m_pPEHeader->NtHdrSize) + 24);

    std::vector<SectionParam> params;

    uint32_t numSections = LH(&m_pPEHeader->numObjects);
    //    IBinarySection *reloc = nullptr;
    for (unsigned i = 0; i < numSections; i++, o++) {
        SectionParam sect;
        // TODO: Check for unreadable sections (!IMAGE_SCN_MEM_READ)?
        memset(base + LMMH(o->RVA), 0, LMMH(o->VirtualSize));
        memcpy(base + LMMH(o->RVA), data+LMMH(o->PhysicalOffset), LMMH(o->PhysicalSize));

        sect.Name = QByteArray(o->ObjectName,8);
        sect.From = ADDRESS::g(LMMH(o->RVA) + LMMH(m_pPEHeader->Imagebase));
        sect.ImageAddress = ADDRESS::host_ptr(LMMH(o->RVA) + base);
        sect.Size = LMMH(o->VirtualSize);
        sect.PhysSize = LMMH(o->PhysicalSize);
        DWord Flags = LMMH(o->Flags);
        sect.Bss = (Flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA) ? 1 : 0;
        sect.Code = (Flags & IMAGE_SCN_CNT_CODE) ? 1 : 0;
        sect.Data = (Flags & IMAGE_SCN_CNT_INITIALIZED_DATA) ? 1 : 0;
        sect.ReadOnly = (Flags & IMAGE_SCN_MEM_WRITE) ? 0 : 1;
        params.push_back(sect);
    }
    for(SectionParam par : params) {
        IBinarySection *sect = Image->createSection(par.Name,par.From,par.From+par.Size);
        if(!sect)
            continue;
        sect->setBss(par.Bss)
                .setCode(par.Code)
                .setData(par.Data)
                .setReadOnly(par.ReadOnly)
                .setHostAddr(par.ImageAddress)
                .setEndian(0); // little endian
        if( !(par.Bss || par.From.isZero()) ) {
            sect->addDefinedArea(par.From,par.From+par.PhysSize);
        }
    }

    // Add the Import Address Table entries to the symbol table
    processIAT();

    // Was hoping that _main or main would turn up here for Borland console mode programs. No such luck.
    // I think IDA Pro must find it by a combination of FLIRT and some pattern matching
    // PEExportDtor* eid = (PEExportDtor*)
    //    (LMMH(m_pPEHeader->ExportTableRVA) + base);

    // Give the entry point a symbol
    ADDRESS entry = GetMainEntryPoint();
    if (entry != NO_ADDRESS) {
        if (!Symbols->find(entry))
            Symbols->create(entry,"main").setAttr("Function",true);
    }

    // Give a name to any jumps you find to these import entries
    // NOTE: VERY early MSVC specific!! Temporary till we can think of a better way.
    ADDRESS start = GetEntryPoint();
    findJumps(start);

    readDebugData();
    return true;

}
bool Win32BinaryFile::RealLoad(const QString &sName) {
    m_pFileName = sName;
    QFile fp(sName);
    if(fp.open(QFile::ReadOnly)) {
        QByteArray data = fp.readAll();
        return LoadFromArray(data);
    }
    return false;
}

// Used above for a hack to find jump instructions pointing to IATs.
// Heuristic: start just before the "start" entry point looking for FF 25 opcodes followed by a pointer to an import
// entry.  E.g. FF 25 58 44 40 00  where 00404458 is the IAT for _ftol.
// Note: some are on 0x10 byte boundaries, some on 2 byte boundaries (6 byte jumps packed), and there are often up to
// 0x30 bytes of statically linked library code (e.g. _atexit, __onexit) with sometimes two static libs in a row.
// So keep going until there is about 0x60 bytes with no match.
// Note: slight chance of coming across a misaligned match; probability is about 1/65536 times dozens in 2^32 ~= 10^-13
void Win32BinaryFile::findJumps(ADDRESS curr) {
    int cnt = 0; // Count of bytes with no match
    IBinarySection *sec = Image->GetSectionInfoByName(".text");
    if (sec == nullptr)
        sec = Image->GetSectionInfoByName("CODE");
    assert(sec);
    // Add to native addr to get host:
    ptrdiff_t delta = (sec->hostAddr() - sec->sourceAddr()).m_value;
    while (cnt < 0x60) { // Max of 0x60 bytes without a match
        curr -= 2;       // Has to be on 2-byte boundary
        cnt += 2;
        if(curr<sec->sourceAddr())
            break; // stepped out of section
        if (LH((curr + delta).m_value) != 0xFF + (0x25 << 8))
            continue;
        ADDRESS operand = ADDRESS::g(LMMH2((curr + delta + 2).m_value));
        auto symbol_it = Symbols->find(operand);
        if (nullptr == symbol_it)
            continue;
        QString sym_name = symbol_it->getName();
        if(false == const_cast<IBinarySymbol *>(symbol_it)->rename("__imp_" + sym_name)) {
            continue;
        }
        Symbols->create(curr,sym_name).setAttr("Function",true).setAttr("Imported",true);
        curr -= 4; // Next match is at least 4+2 bytes away
        cnt = 0;
    }
}

// Clean up and unload the binary image
void Win32BinaryFile::UnLoad() {}

bool Win32BinaryFile::PostLoad(void *handle) {
    Q_UNUSED(handle);
    return false;
}

#if defined(_WIN32) && !defined(__MINGW32__)

char *SymTagEnums[] = {"SymTagNull",            "SymTagExe",            "SymTagCompiland",    "SymTagCompilandDetails",
                       "SymTagCompilandEnv",    "SymTagFunction",       "SymTagBlock",        "SymTagData",
                       "SymTagAnnotation",      "SymTagLabel",          "SymTagPublicSymbol", "SymTagUDT",
                       "SymTagEnum",            "SymTagFunctionType",   "SymTagPointerType",  "SymTagArrayType",
                       "SymTagBaseType",        "SymTagTypedef",        "SymTagBaseClass",    "SymTagFriend",
                       "SymTagFunctionArgType", "SymTagFuncDebugStart", "SymTagFuncDebugEnd", "SymTagUsingNamespace",
                       "SymTagVTableShape",     "SymTagVTable",         "SymTagCustom",       "SymTagThunk",
                       "SymTagCustomType",      "SymTagManagedType",    "SymTagDimension"};

enum SymTagEnum {
    SymTagNull,
    SymTagExe,
    SymTagCompiland,
    SymTagCompilandDetails,
    SymTagCompilandEnv,
    SymTagFunction,
    SymTagBlock,
    SymTagData,
    SymTagAnnotation,
    SymTagLabel,
    SymTagPublicSymbol,
    SymTagUDT,
    SymTagEnum,
    SymTagFunctionType,
    SymTagPointerType,
    SymTagArrayType,
    SymTagBaseType,
    SymTagTypedef,
    SymTagBaseClass,
    SymTagFriend,
    SymTagFunctionArgType,
    SymTagFuncDebugStart,
    SymTagFuncDebugEnd,
    SymTagUsingNamespace,
    SymTagVTableShape,
    SymTagVTable,
    SymTagCustom,
    SymTagThunk,
    SymTagCustomType,
    SymTagManagedType,
    SymTagDimension
};

char *basicTypes[] = {
    "notype", "void", "char", "WCHAR", "??", "??", "int", "unsigned int", "float", "bcd", "bool", "??", "??",
    "long"
    "unsigned long",
};

void printType(DWORD index, DWORD64 ImageBase) {
    HANDLE hProcess = GetCurrentProcess();

    int got;
    WCHAR *name;
    got = dbghelp::SymGetTypeInfo(hProcess, ImageBase, index, dbghelp::TI_GET_SYMNAME, &name);
    if (got) {
        char nameA[1024];
        WideCharToMultiByte(CP_ACP, 0, name, -1, nameA, sizeof(nameA), 0, nullptr);
        std::cout << nameA;
        return;
    }

    DWORD d;
    got = dbghelp::SymGetTypeInfo(hProcess, ImageBase, index, dbghelp::TI_GET_SYMTAG, &d);
    assert(got);

    switch (d) {
    case SymTagPointerType: {
        got = dbghelp::SymGetTypeInfo(hProcess, ImageBase, index, dbghelp::TI_GET_TYPE, &d);
        assert(got);
        printType(d, ImageBase);
        std::cout << "*";
    } break;
    case SymTagBaseType:
        got = dbghelp::SymGetTypeInfo(hProcess, ImageBase, index, dbghelp::TI_GET_BASETYPE, &d);
        assert(got);
        std::cout << basicTypes[d];
        break;
    default:
        std::cerr << "unhandled symtag " << SymTagEnums[d] << "\n";
        assert(false);
    }
}

BOOL CALLBACK printem(dbghelp::PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext) {
    HANDLE hProcess = GetCurrentProcess();
    printType(pSymInfo->TypeIndex, pSymInfo->ModBase);
    std::cout << " " << pSymInfo->Name << " flags: ";
    if (pSymInfo->Flags & SYMFLAG_VALUEPRESENT)
        std::cout << "value present, ";
    if (pSymInfo->Flags & SYMFLAG_REGISTER)
        std::cout << "register, ";
    if (pSymInfo->Flags & SYMFLAG_REGREL)
        std::cout << "regrel, ";
    if (pSymInfo->Flags & SYMFLAG_FRAMEREL)
        std::cout << "framerel, ";
    if (pSymInfo->Flags & SYMFLAG_PARAMETER)
        std::cout << "parameter, ";
    if (pSymInfo->Flags & SYMFLAG_LOCAL)
        std::cout << "local, ";
    if (pSymInfo->Flags & SYMFLAG_CONSTANT)
        std::cout << "constant, ";
    if (pSymInfo->Flags & SYMFLAG_EXPORT)
        std::cout << "export, ";
    if (pSymInfo->Flags & SYMFLAG_FORWARDER)
        std::cout << "forwarder, ";
    if (pSymInfo->Flags & SYMFLAG_FUNCTION)
        std::cout << "function, ";
    if (pSymInfo->Flags & SYMFLAG_VIRTUAL)
        std::cout << "virtual, ";
    if (pSymInfo->Flags & SYMFLAG_THUNK)
        std::cout << "thunk, ";
    if (pSymInfo->Flags & SYMFLAG_TLSREL)
        std::cout << "tlsrel, ";
    std::cout << "\n";
    std::cout << "register: " << pSymInfo->Register << " address: " << (int)pSymInfo->Address << "\n";
    return TRUE;
}
#endif


bool Win32BinaryFile::DisplayDetails(const char *fileName, FILE *f /* = stdout */) {
    Q_UNUSED(fileName);
    Q_UNUSED(f);
    return false;
}

int Win32BinaryFile::win32Read2(short *ps) const {
    unsigned char *p = (unsigned char *)ps;
    // Little endian
    int n = (int)(p[0] + (p[1] << 8));
    return n;
}

int Win32BinaryFile::win32Read4(int *pi) const {
    short *p = (short *)pi;
    int n1 = win32Read2(p);
    int n2 = win32Read2(p + 1);
    int n = (int)(n1 | (n2 << 16));
    return n;
}

bool Win32BinaryFile::IsStaticLinkedLibProc(ADDRESS uNative) {
#if defined(_WIN32) && !defined(__MINGW32__)
    HANDLE hProcess = GetCurrentProcess();
    dbghelp::IMAGEHLP_LINE64 line;
    line.SizeOfStruct = sizeof(line);
    line.FileName = nullptr;
    dbghelp::SymGetLineFromAddr64(hProcess, uNative.m_value, 0, &line);
    if (haveDebugInfo && line.FileName == nullptr || line.FileName && *line.FileName == 'f')
        return true;
#endif

    if (IsMinGWsAllocStack(uNative) || IsMinGWsFrameInit(uNative) || IsMinGWsFrameEnd(uNative) ||
            IsMinGWsCleanupSetup(uNative) || IsMinGWsMalloc(uNative)) {
        return true;
    }

    return false;
}

bool Win32BinaryFile::IsMinGWsAllocStack(ADDRESS uNative) {
    if (mingw_main) {
        const IBinarySection * si = Image->getSectionInfoByAddr(uNative);
        if (si) {
            ADDRESS host = si->hostAddr() - si->sourceAddr() + uNative;
            unsigned char pat[] = {0x51, 0x89, 0xE1, 0x83, 0xC1, 0x08, 0x3D, 0x00, 0x10, 0x00, 0x00, 0x72,
                                   0x10, 0x81, 0xE9, 0x00, 0x10, 0x00, 0x00, 0x83, 0x09, 0x00, 0x2D, 0x00,
                                   0x10, 0x00, 0x00, 0xEB, 0xE9, 0x29, 0xC1, 0x83, 0x09, 0x00, 0x89, 0xE0,
                                   0x89, 0xCC, 0x8B, 0x08, 0x8B, 0x40, 0x04, 0xFF, 0xE0};
            if (memcmp((void *)host.m_value, pat, sizeof(pat)) == 0) {
                return true;
            }
        }
    }
    return false;
}

bool Win32BinaryFile::IsMinGWsFrameInit(ADDRESS uNative) {
    if (mingw_main) {
        const IBinarySection * si = Image->getSectionInfoByAddr(uNative);
        if (si) {
            ADDRESS host = si->hostAddr() - si->sourceAddr() + uNative;
            unsigned char pat1[] = {0x55, 0x89, 0xE5, 0x83, 0xEC, 0x18, 0x89, 0x7D, 0xFC,
                                    0x8B, 0x7D, 0x08, 0x89, 0x5D, 0xF4, 0x89, 0x75, 0xF8};
            if (memcmp((void *)host.m_value, pat1, sizeof(pat1)) == 0) {
                unsigned char pat2[] = {0x85, 0xD2, 0x74, 0x24, 0x8B, 0x42, 0x2C, 0x85, 0xC0, 0x78, 0x3D, 0x8B, 0x42,
                                        0x2C, 0x85, 0xC0, 0x75, 0x56, 0x8B, 0x42, 0x28, 0x89, 0x07, 0x89, 0x7A, 0x28,
                                        0x8B, 0x5D, 0xF4, 0x8B, 0x75, 0xF8, 0x8B, 0x7D, 0xFC, 0x89, 0xEC, 0x5D, 0xC3};
                if (memcmp((void *)(host.m_value + sizeof(pat1) + 6), pat2, sizeof(pat2)) == 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Win32BinaryFile::IsMinGWsFrameEnd(ADDRESS uNative) {
    if (mingw_main) {
        const IBinarySection * si = Image->getSectionInfoByAddr(uNative);
        if (si) {
            ADDRESS host = si->hostAddr() - si->sourceAddr() + uNative;
            unsigned char pat1[] = {0x55, 0x89, 0xE5, 0x53, 0x83, 0xEC, 0x14, 0x8B, 0x45, 0x08, 0x8B, 0x18};
            if (memcmp((void *)host.m_value, pat1, sizeof(pat1)) == 0) {
                unsigned char pat2[] = {0x85, 0xC0, 0x74, 0x1B, 0x8B, 0x48, 0x2C, 0x85, 0xC9, 0x78, 0x34, 0x8B, 0x50,
                                        0x2C, 0x85, 0xD2, 0x75, 0x4D, 0x89, 0x58, 0x28, 0x8B, 0x5D, 0xFC, 0xC9, 0xC3};
                if (memcmp((void *)(host.m_value + sizeof(pat1) + 5), pat2, sizeof(pat2)) == 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Win32BinaryFile::IsMinGWsCleanupSetup(ADDRESS uNative) {
    if (mingw_main) {
        const IBinarySection * si = Image->getSectionInfoByAddr(uNative);
        if (si) {
            ADDRESS host = si->hostAddr() - si->sourceAddr() + uNative;
            unsigned char pat1[] = {0x55, 0x89, 0xE5, 0x53, 0x83, 0xEC, 0x04};
            if (memcmp((void *)host.m_value, pat1, sizeof(pat1)) == 0) {
                unsigned char pat2[] = {0x85, 0xDB, 0x75, 0x35};
                if (memcmp((void *)(host.m_value + sizeof(pat1) + 6), pat2, sizeof(pat2)) == 0) {
                    unsigned char pat3[] = {0x83, 0xF8, 0xFF, 0x74, 0x24, 0x85, 0xC0, 0x89,
                                            0xC3, 0x74, 0x0E, 0x8D, 0x74, 0x26, 0x00};
                    if (memcmp((void *)(host.m_value + sizeof(pat1) + 6 + sizeof(pat2) + 16), pat3, sizeof(pat3)) ==
                            0) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Win32BinaryFile::IsMinGWsMalloc(ADDRESS uNative) {
    if (mingw_main) {
        const IBinarySection * si = Image->getSectionInfoByAddr(uNative);
        if (si) {
            ADDRESS host = si->hostAddr() - si->sourceAddr() + uNative;
            unsigned char pat1[] = {0x55, 0x89, 0xE5, 0x8D, 0x45, 0xF4, 0x83, 0xEC, 0x58, 0x89, 0x45, 0xE0, 0x8D, 0x45,
                                    0xC0, 0x89, 0x04, 0x24, 0x89, 0x5D, 0xF4, 0x89, 0x75, 0xF8, 0x89, 0x7D, 0xFC};
            if (memcmp((void *)host.m_value, pat1, sizeof(pat1)) == 0) {
                unsigned char pat2[] = {0x89, 0x65, 0xE8};
                if (memcmp((void *)(host.m_value + sizeof(pat1) + 0x15), pat2, sizeof(pat2)) == 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

ADDRESS Win32BinaryFile::IsJumpToAnotherAddr(ADDRESS uNative) {
    if ((Image->readNative1(uNative) & 0xff) != 0xe9)
        return NO_ADDRESS;
    return ADDRESS::g(Image->readNative4(uNative + 1)) + uNative + 5;
}

LOAD_FMT Win32BinaryFile::GetFormat() const { return LOADFMT_PE; }

MACHINE Win32BinaryFile::getMachine() const { return MACHINE_PENTIUM; }

bool Win32BinaryFile::isLibrary() const { return ((m_pPEHeader->Flags & 0x2000) != 0); }

ADDRESS Win32BinaryFile::getImageBase() { return ADDRESS::g(m_pPEHeader->Imagebase); }

size_t Win32BinaryFile::getImageSize() { return m_pPEHeader->ImageSize; }

DWord Win32BinaryFile::getDelta() {
    // Stupid function anyway: delta depends on section
    // This should work for the header only
    //    return (DWord)base - LMMH(m_pPEHeader->Imagebase);
    return DWord(intptr_t(base)) - (DWord)m_pPEHeader->Imagebase;
}
