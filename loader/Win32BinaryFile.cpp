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

/* File: Win32BinaryFile.cc
 * $Revision$
 * Desc: This file contains the implementation of the class Win32BinaryFile.
 */

/* Win32 binary file format.
 *	This file implements the class Win32BinaryFile, derived from class
 *	BinaryFile. See Win32BinaryFile.h and BinaryFile.h for details.
 * 25 Jun 02 - Mike: Added code to find WinMain by finding a call within 5
 *				instructions of a call to GetModuleHandleA
 * 07 Jul 02 - Mike: Added a LMMH() so code works on big-endian host
 * 08 Jul 02 - Mike: Changed algorithm to find main; now looks for ordinary
 *				 call up to 10 instructions before an indirect call to exit
 * 24 Jul 05 - Mike: State machine to recognise main in Borland Builder files
 */

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "BinaryFile.h"
#include "Win32BinaryFile.h"
#include "config.h"
#include <iostream>
#include <sstream>
#include <assert.h>

extern "C" {
	int microX86Dis(void* p);			// From microX86dis.c
}


#ifndef IMAGE_SCN_CNT_CODE // Assume that if one is not defined, the rest isn't either.
#define IMAGE_SCN_CNT_CODE               0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA   0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080
#define IMAGE_SCN_MEM_READ               0x40000000
#define IMAGE_SCN_MEM_WRITE              0x80000000
#endif


namespace {

// Due to the current rigid design, where BinaryFile holds a C-style array of
// SectionInfo's, we can't extend a subclass of SectionInfo with the data required
// to express the semantics of a PE section. We therefore need this external mapping
// from SectionInfo's to PEObject's, that contain the info we need.
// TODO: Refactor BinaryFile to not expose its private parts in public. Design both
// a protected (for subclasses) and public (for users) interface.
typedef std::map<const class PESectionInfo*, const PEObject*> SectionObjectMap;

SectionObjectMap s_sectionObjects;


// Note that PESectionInfo currently must be the exact same size as
// SectionInfo due to the already mentioned array held by BinaryFile.
class PESectionInfo : public SectionInfo
{
	virtual bool isAddressBss(ADDRESS a) const
	{
		if (a < uNativeAddr || a >= uNativeAddr + uSectionSize) {
			return false; // not even within this section
		}
		if (bBss) {
			return true; // obvious
		}
		if (bReadOnly) {
			return false; // R/O BSS makes no sense.
		}
		// Don't check for bData here. So long as the section has slack at end, that space can contain BSS.
		const SectionObjectMap::iterator it = s_sectionObjects.find(this);
		assert(it != s_sectionObjects.end());
		assert(it->second);
		assert(this == it->first);
		const PEObject* sectionHeader = it->second;
		const bool has_slack = LMMH(sectionHeader->VirtualSize) > LMMH(sectionHeader->PhysicalSize);
		if (!has_slack) {
			return false; // BSS not possible.
		}
		if (a >= uNativeAddr + LMMH(sectionHeader->PhysicalSize)) {
			return true;
		}
		return false;
	}
};

// attempt at a compile-time assert for the size requirement.
// If the sizes differs, this statement will try to define a zero-sized array, which is invalid.
typedef char ct_failure[sizeof(SectionInfo) == sizeof(PESectionInfo)];

}


Win32BinaryFile::Win32BinaryFile() : m_pFileName(0)
{ }

Win32BinaryFile::~Win32BinaryFile()
{
	for (int i=0; i < m_iNumSections; i++) {
		if (m_pSections[i].pSectionName)
			delete [] m_pSections[i].pSectionName;
	}
	if (m_pSections) delete [] m_pSections;
}

bool Win32BinaryFile::Open(const char* sName) {
	//return Load(sName) != 0;
	return false;
}

void Win32BinaryFile::Close() {
	UnLoad();
}

std::list<SectionInfo*>& Win32BinaryFile::GetEntryPoints(
	const char* pEntry)
{
	fprintf(stderr,"really don't know how to implement GetEntryPoints\n");
	exit(0);
	static std::list<SectionInfo*> l;
	return l;
}

ADDRESS Win32BinaryFile::GetEntryPoint()
{
	return (ADDRESS)(LMMH(m_pPEHeader->EntrypointRVA) +
					 LMMH(m_pPEHeader->Imagebase));
}

// This is a bit of a hack, but no more than the rest of Windows :-O  The pattern is to look for an indirect call (FF 15
// opcode) to exit; within 10 instructions before that should be the call to WinMain (with no other calls inbetween).
// This pattern should work for "old style" and "new style" PE executables, as well as console mode PE files.
ADDRESS Win32BinaryFile::GetMainEntryPoint() {
	ADDRESS aMain = GetAddressByName ("main", true);
	if (aMain != NO_ADDRESS)
		return aMain;
	aMain = GetAddressByName ("_main", true);		// Example: MinGW
	if (aMain != NO_ADDRESS)
		return aMain;

	// Start at program entry point
	unsigned p = LMMH(m_pPEHeader->EntrypointRVA);
	unsigned lim = p + 0x200;
	unsigned char op1, op2;
	unsigned addr, lastOrdCall = 0;
	int gap;				// Number of instructions from the last ordinary call
	int borlandState = 0;	// State machine for Borland

	SectionInfo* si = GetSectionInfoByName(".text");
	if (si == NULL) si = GetSectionInfoByName("CODE");
	assert(si);
	unsigned textSize = si->uSectionSize;
	if (textSize < 0x200)
		lim = p + textSize;

	if (m_pPEHeader->Subsystem == 1) 	// native
		return LMMH(m_pPEHeader->EntrypointRVA) + LMMH(m_pPEHeader->Imagebase);

	gap = 0xF0000000;	// Large positive number (in case no ordinary calls)
	while (p < lim) {
		op1 = *(unsigned char*)(p + base);
		op2 = *(unsigned char*)(p + base + 1);
//		std::cerr << std::hex << "At " << p << ", ops " << (unsigned)op1 << ", " << (unsigned)op2 << std::dec << "\n";
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
				if (op2 == 0x15) { 			// Opcode FF 15 is indirect call
					// Get the 4 byte address from the instruction
					addr = LMMH(*(p + base + 2));
//					const char *c = dlprocptrs[addr].c_str();
//					printf("Checking %x finding %s\n", addr, c);
					if (dlprocptrs[addr] == "exit") {
						if (gap <= 10) {
							// This is it. The instruction at lastOrdCall is (win)main
							addr = LMMH(*(lastOrdCall + base + 1));
							addr += lastOrdCall + 5;	// Addr is dest of call
//							printf("*** MAIN AT 0x%x ***\n", addr);
							return addr + LMMH(m_pPEHeader->Imagebase);
						}
					}
				} else
					borlandState = 0;
				break;
			case 0xEB: 					// Short relative jump, e.g. Borland
				if (op2 >= 0x80)		// Branch backwards?
					break;				// Yes, just ignore it
				// Otherwise, actually follow the branch. May have to modify this some time...
				p += op2+2;				// +2 for the instruction itself, and op2 for the displacement
				gap++;
				continue;
			case 0x6A:
				if (op2 == 0) {			// Push 00
					// Borland pattern: push 0 / call __ExceptInit / pop ecx / push offset mainInfo / push 0
					// Borland state before: 0				1			   2			3				4
					if (borlandState == 0)
						borlandState = 1;
					else if (borlandState == 4) {
						// Borland pattern succeeds. p-4 has the offset of mainInfo
						ADDRESS mainInfo = LMMH(*(base + p-4));
						ADDRESS main = readNative4(mainInfo+0x18);		// Address of main is at mainInfo+18
						return main;
					}
				} else
					borlandState = 0;
				break;
			case 0x59:					// Pop ecx
				if (borlandState == 2)
					borlandState = 3;
				else
					borlandState = 0;
				break;
			case 0x68: 					// Push 4 byte immediate
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

	// For VS.NET, need an old favourite: find a call with three pushes in the first 100 instuctions
	int count = 100;
	int pushes = 0;
	p = LMMH(m_pPEHeader->EntrypointRVA);
	while (count > 0) {
		op1 = *(unsigned char*)(p + base);
		if (op1 == 0xE8) {			// CALL opcode
			if (pushes == 3) {
				// Get the offset
				int off = LMMH(*(p + base + 1));
				unsigned dest = (unsigned)p + 5 + off;
				// Check for a jump there
				op1 = *(unsigned char*)(dest + base);
				if (op1 == 0xE9) {
					// Follow that jump
					off = LMMH(*(dest + base + 1));
					dest = dest + 5 + off;
				}
				return dest + LMMH(m_pPEHeader->Imagebase);
			} else
				pushes = 0;			// Assume pushes don't accumulate over calls
		}
		else if (op1 >= 0x50 && op1 <= 0x57)	// PUSH opcode
			pushes++;
		else if (op1 == 0xFF) {
			// FF 35 is push m[K]
			op2 = *(unsigned char*)(p + 1 + base);
			if (op2 == 0x35)
				pushes++;
		}
		else if (op1 == 0xE9) {
			// Follow the jump
			int off = LMMH(*(p + base + 1));
			p += off+5;
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
	
	return NO_ADDRESS;
}


bool Win32BinaryFile::RealLoad(const char* sName)
{
	m_pFileName = sName;
	FILE *fp = fopen(sName,"rb");

	DWord peoffLE, peoff;
	fseek(fp, 0x3c, SEEK_SET);
	fread(&peoffLE, 4, 1, fp);		// Note: peoffLE will be in Little Endian
	peoff = LMMH(peoffLE);

	PEHeader tmphdr;

	fseek(fp, peoff, SEEK_SET);
	fread(&tmphdr, sizeof(tmphdr), 1, fp);
	// Note: all tmphdr fields will be little endian

	base = (char *)malloc(LMMH(tmphdr.ImageSize));

	if (!base) {
		fprintf(stderr,"Cannot allocate memory for copy of image\n");
		return false;
	}

	fseek(fp, 0, SEEK_SET);

	fread(base, LMMH(tmphdr.HeaderSize), 1, fp);

	m_pHeader = (Header *)base;
	if (m_pHeader->sigLo!='M' || m_pHeader->sigHi!='Z') {
		fprintf(stderr,"error loading file %s, bad magic\n", sName);
		return false;
	}

	m_pPEHeader = (PEHeader *)(base+peoff);
	if (m_pPEHeader->sigLo!='P' || m_pPEHeader->sigHi!='E') {
		fprintf(stderr,"error loading file %s, bad PE magic\n", sName);
		return false;
	}

//printf("Image Base %08X, real base %p\n", LMMH(m_pPEHeader->Imagebase), base);

	const PEObject *o = (PEObject *)(((char *)m_pPEHeader) + LH(&m_pPEHeader->NtHdrSize) + 24);
	m_iNumSections = LH(&m_pPEHeader->numObjects);
	m_pSections = new PESectionInfo[m_iNumSections];
//	SectionInfo *reloc = NULL;
	for (int i=0; i<m_iNumSections; i++, o++) {
		SectionInfo& sect = m_pSections[i];
		//	printf("%.8s RVA=%08X Offset=%08X size=%08X\n", (char*)o->ObjectName, LMMH(o->RVA), LMMH(o->PhysicalOffset),
		//	  LMMH(o->VirtualSize));
		sect.pSectionName = new char[9];
		strncpy(sect.pSectionName, o->ObjectName, 8);
//		if (!strcmp(sect.pSectionName, ".reloc"))
//			reloc = &sect;
		sect.uNativeAddr=(ADDRESS)(LMMH(o->RVA) + LMMH(m_pPEHeader->Imagebase));
		sect.uHostAddr=(ADDRESS)(LMMH(o->RVA) + base);
		sect.uSectionSize=LMMH(o->VirtualSize);
		DWord Flags = LMMH(o->Flags);
		sect.bBss      = (Flags&IMAGE_SCN_CNT_UNINITIALIZED_DATA)?1:0;
		sect.bCode     = (Flags&IMAGE_SCN_CNT_CODE)?1:0;
		sect.bData     = (Flags&IMAGE_SCN_CNT_INITIALIZED_DATA)?1:0;
		sect.bReadOnly = (Flags&IMAGE_SCN_MEM_WRITE)?0:1;
		// TODO: Check for unreadable sections (!IMAGE_SCN_MEM_READ)?
		fseek(fp, LMMH(o->PhysicalOffset), SEEK_SET);
		memset(base + LMMH(o->RVA), 0, LMMH(o->VirtualSize));
		fread(base + LMMH(o->RVA), LMMH(o->PhysicalSize), 1, fp);
		s_sectionObjects[static_cast<const PESectionInfo*>(&sect)] = o;
	}

	// Add the Import Address Table entries to the symbol table
	PEImportDtor* id = (PEImportDtor*) (LMMH(m_pPEHeader->ImportTableRVA) + base);
	if (m_pPEHeader->ImportTableRVA) {			// If any import table entry exists
		while (id->name != 0) {
			char* dllName = LMMH(id->name) + base;
			unsigned thunk = id->originalFirstThunk ? id->originalFirstThunk : id->firstThunk;
			unsigned* iat = (unsigned*)(LMMH(thunk) + base);
			unsigned iatEntry = LMMH(*iat);
			ADDRESS paddr = LMMH(id->firstThunk) + LMMH(m_pPEHeader->Imagebase);
			while (iatEntry) {
				if (iatEntry >> 31) {
					// This is an ordinal number (stupid idea)
					std::ostringstream ost;
					std::string nodots(dllName);
					int len = nodots.size();
					for (int j=0; j < len; j++)
						if (nodots[j] == '.')
							nodots[j] = '_';	// Dots can't be in identifiers
					ost << nodots << "_" << (iatEntry & 0x7FFFFFFF);				
					dlprocptrs[paddr] = ost.str();
					// printf("Added symbol %s value %x\n", ost.str().c_str(), paddr);
				} else {
					// Normal case (IMAGE_IMPORT_BY_NAME). Skip the useless hint (2 bytes)
					std::string name((const char*)(iatEntry+2+base));
					dlprocptrs[paddr] = name;
					if ((unsigned)paddr != (unsigned)iat - (unsigned)base + LMMH(m_pPEHeader->Imagebase))
						dlprocptrs[(unsigned)iat - (unsigned)base + LMMH(m_pPEHeader->Imagebase)]
							= std::string("old_") + name; // add both possibilities
					// printf("Added symbol %s value %x\n", name.c_str(), paddr);
					// printf("Also added old_%s value %x\n", name.c_str(), (int)iat - (int)base +
					// 		LMMH(m_pPEHeader->Imagebase));
				}
				iat++;
				iatEntry = LMMH(*iat);
				paddr+=4;
			}
			id++;
		}
	}

	// Was hoping that _main or main would turn up here for Borland console mode programs. No such luck.
	// I think IDA Pro must find it by a combination of FLIRT and some pattern matching
	//PEExportDtor* eid = (PEExportDtor*)
	//	(LMMH(m_pPEHeader->ExportTableRVA) + base);


	// Give the entry point a symbol
	ADDRESS entry = GetMainEntryPoint();
	if (entry != NO_ADDRESS) {
		std::map<ADDRESS, std::string>::iterator it = dlprocptrs.find(entry);
		if (it == dlprocptrs.end())
			dlprocptrs[entry] = "main";
	}

	// Give a name to any jumps you find to these import entries
	// NOTE: VERY early MSVC specific!! Temporary till we can think of a better way.
	ADDRESS start = GetEntryPoint();
	findJumps(start);

	fclose(fp);
	return true;
}

// Used above for a hack to find jump instructions pointing to IATs.
// Heuristic: start just before the "start" entry point looking for FF 25 opcodes followed by a pointer to an import
// entry.  E.g. FF 25 58 44 40 00  where 00404458 is the IAT for _ftol.
// Note: some are on 0x10 byte boundaries, some on 2 byte boundaries (6 byte jumps packed), and there are often up to
// 0x30 bytes of statically linked library code (e.g. _atexit, __onexit) with sometimes two static libs in a row.
// So keep going until there is about 0x60 bytes with no match.
// Note: slight chance of coming across a misaligned match; probability is about 1/65536 times dozens in 2^32 ~= 10^-13
void Win32BinaryFile::findJumps(ADDRESS curr) {
	int cnt = 0;			// Count of bytes with no match
	SectionInfo* sec = GetSectionInfoByName(".text");
	if (sec == NULL) sec = GetSectionInfoByName("CODE");
	assert(sec);
	// Add to native addr to get host:
	int delta = sec->uHostAddr - sec->uNativeAddr;
	while (cnt < 0x60) {	// Max of 0x60 bytes without a match
		curr -= 2;			// Has to be on 2-byte boundary
		cnt += 2;
		if (LH(delta+curr) != 0xFF + (0x25<<8)) continue;
		ADDRESS operand = LMMH2(delta+curr+2);
		std::map<ADDRESS, std::string>::iterator it;
		it = dlprocptrs.find(operand);
		if (it == dlprocptrs.end()) continue;
		std::string sym = it->second;
		dlprocptrs[operand] = "__imp_" + sym;
		dlprocptrs[curr] = sym;		 // Add new entry
		// std::cerr << "Added " << sym << " at 0x" << std::hex << curr << "\n";
		curr -= 4;					// Next match is at least 4+2 bytes away
		cnt = 0;
	}
}

// Clean up and unload the binary image
void Win32BinaryFile::UnLoad()
{
} 

bool Win32BinaryFile::PostLoad(void* handle)
{
	return false;
}

const char* Win32BinaryFile::SymbolByAddress(ADDRESS dwAddr)
{
	if (m_pPEHeader->Subsystem == 1 &&				// native
			LMMH(m_pPEHeader->EntrypointRVA) + LMMH(m_pPEHeader->Imagebase) == dwAddr)
		return "DriverEntry";

	std::map<ADDRESS, std::string>::iterator it = dlprocptrs.find(dwAddr);
	if (it == dlprocptrs.end())
		return 0;
	return (char*) it->second.c_str();
}

ADDRESS Win32BinaryFile::GetAddressByName(const char* pName,
	bool bNoTypeOK /* = false */) {
	// This is "looking up the wrong way" and hopefully is uncommon.  Use linear search
	std::map<ADDRESS, std::string>::iterator it = dlprocptrs.begin();
	while (it != dlprocptrs.end()) {
		// std::cerr << "Symbol: " << it->second.c_str() << " at 0x" << std::hex << it->first << "\n";
		if (strcmp(it->second.c_str(), pName) == 0)
			return it->first;
		it++;
	}
	return NO_ADDRESS;
}

void Win32BinaryFile::AddSymbol(ADDRESS uNative, const char *pName)
{
	dlprocptrs[uNative] = pName;
}

bool Win32BinaryFile::DisplayDetails(const char* fileName, FILE* f
	 /* = stdout */)
{
	return false;
}

int Win32BinaryFile::win32Read2(short* ps) const {
	unsigned char* p = (unsigned char*)ps;
	// Little endian
	int n = (int)(p[0] + (p[1] << 8));
	return n;
}

int Win32BinaryFile::win32Read4(int* pi) const{
	short* p = (short*)pi;
	int n1 = win32Read2(p);
	int n2 = win32Read2(p+1);
	int n = (int) (n1 | (n2 << 16));
	return n;
}

// Read 2 bytes from given native address
int Win32BinaryFile::readNative1(ADDRESS nat) {
	PSectionInfo si = GetSectionInfoByAddr(nat);
	if (si == 0) 
		si = GetSectionInfo(0);
	ADDRESS host = si->uHostAddr - si->uNativeAddr + nat;
	return *(char*)host;
}

// Read 2 bytes from given native address
int Win32BinaryFile::readNative2(ADDRESS nat) {
	PSectionInfo si = GetSectionInfoByAddr(nat);
	if (si == 0) return 0;
	ADDRESS host = si->uHostAddr - si->uNativeAddr + nat;
	int n = win32Read2((short*)host);
	return n;
}

// Read 4 bytes from given native address
int Win32BinaryFile::readNative4(ADDRESS nat) {
	PSectionInfo si = GetSectionInfoByAddr(nat);
	if (si == 0) return 0;
	ADDRESS host = si->uHostAddr - si->uNativeAddr + nat;
	int n = win32Read4((int*)host);
	return n;
}

// Read 8 bytes from given native address
QWord Win32BinaryFile::readNative8(ADDRESS nat) {
	int raw[2];
#ifdef WORDS_BIGENDIAN		// This tests the host machine
	// Source and host are different endianness
	raw[1] = readNative4(nat);
	raw[0] = readNative4(nat+4);
#else
	// Source and host are same endianness
	raw[0] = readNative4(nat);
	raw[1] = readNative4(nat+4);
#endif
	return *(QWord*)raw;
}

// Read 4 bytes as a float
float Win32BinaryFile::readNativeFloat4(ADDRESS nat) {
	int raw = readNative4(nat);
	// Ugh! gcc says that reinterpreting from int to float is invalid!!
	//return reinterpret_cast<float>(raw);		// Note: cast, not convert!!
	return *(float*)&raw;						// Note: cast, not convert
}

// Read 8 bytes as a float
double Win32BinaryFile::readNativeFloat8(ADDRESS nat) {
	int raw[2];
#ifdef WORDS_BIGENDIAN		// This tests the host machine
	// Source and host are different endianness
	raw[1] = readNative4(nat);
	raw[0] = readNative4(nat+4);
#else
	// Source and host are same endianness
	raw[0] = readNative4(nat);
	raw[1] = readNative4(nat+4);
#endif
	//return reinterpret_cast<double>(*raw);	// Note: cast, not convert!!
	return *(double*)raw;
}

bool Win32BinaryFile::IsDynamicLinkedProcPointer(ADDRESS uNative)
{
	if (dlprocptrs.find(uNative) != dlprocptrs.end())
		return true;
	return false;
}

const char *Win32BinaryFile::GetDynamicProcName(ADDRESS uNative)
{
	return dlprocptrs[uNative].c_str();
}

LOAD_FMT Win32BinaryFile::GetFormat() const
{
	return LOADFMT_PE;
}

MACHINE Win32BinaryFile::GetMachine() const
{
	return MACHINE_PENTIUM;
}

bool Win32BinaryFile::isLibrary() const
{
	return ( (m_pPEHeader->Flags & 0x2000) != 0 );
}

ADDRESS Win32BinaryFile::getImageBase()
{
	return m_pPEHeader->Imagebase;
}

size_t Win32BinaryFile::getImageSize()
{
	return m_pPEHeader->ImageSize;
}

std::list<const char *> Win32BinaryFile::getDependencyList()
{
	return std::list<const char *>(); /* FIXME */
}

DWord Win32BinaryFile::getDelta() {
	// Stupid function anyway: delta depends on section
	// This should work for the header only
	//	return (DWord)base - LMMH(m_pPEHeader->Imagebase); 
	return (DWord)base - (DWord)m_pPEHeader->Imagebase; 
}

// This function is called via dlopen/dlsym; it returns a new BinaryFile derived concrete object. After this object is
// returned, the virtual function call mechanism will call the rest of the code in this library.  It needs to be C
// linkage so that it its name is not mangled
extern "C" {
#ifdef _WIN32
	__declspec(dllexport)
#endif
	BinaryFile* construct()
	{
		return new Win32BinaryFile;
	}	 
}

void Win32BinaryFile::dumpSymbols() {
	std::map<ADDRESS, std::string>::iterator it;
	std::cerr << std::hex;
	for (it = dlprocptrs.begin(); it != dlprocptrs.end(); ++it)
		std::cerr << "0x" << it->first << " " << it->second << "        ";
	std::cerr << std::dec << "\n";
}

