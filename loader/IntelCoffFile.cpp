#include "IntelCoffFile.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "config.h"

struct struc_coff_sect          // segment information
{
        char  sch_sectname[8];
        ulong sch_physaddr;
        ulong sch_virtaddr;
        ulong sch_sectsize;
        ulong sch_sectptr;
        ulong sch_relptr;
        ulong sch_lineno_ptr;
        ushort sch_nreloc;
        ushort sch_nlineno;
        ulong sch_flags;
} PACKED;       // 40 bytes

struct coff_symbol      // symbol information
{
        union
        {
                struct {
                        ulong zeros;
                        ulong offset;
                } e;
                char name[8];
        } e;
#define csym_name       e.name
#define csym_zeros      e.e.zeros
#define csym_offset     e.e.offset

        ulong csym_value;
        ushort csym_sectnum;
#define N_UNDEF 0

        ushort csym_type;
#define T_FUNC  0x20

        unsigned char csym_loadclass;
        unsigned char csym_numaux;
} PACKED;       // 18 bytes

struct struct_coff_rel
{
        ulong   r_vaddr;
        ulong   r_symndx;
        ushort  r_type;
#define RELOC_ADDR32    6
#define RELOC_REL32     20
} PACKED;


PSectionInfo IntelCoffFile::AddSection(PSectionInfo psi)
{
	int idxSect = m_iNumSections++;
	PSectionInfo ps = new SectionInfo[m_iNumSections];
	for ( int i = 0; i < idxSect; i++ )
		ps[i] = m_pSections[i];
	ps[idxSect] = *psi;
	delete[] m_pSections;
	m_pSections = ps;
	return ps + idxSect;
}

IntelCoffFile::IntelCoffFile() : BinaryFile(false)
{
	m_pFilename = NULL;
	m_fd = -1;
}

IntelCoffFile::~IntelCoffFile()
{
	if ( m_fd != -1 ) close(m_fd);
}

bool IntelCoffFile::Open(const char *sName)
{
	printf("IntelCoffFile::Open called\n");
	return false;
}

bool IntelCoffFile::RealLoad(const char *sName)
{
	printf("IntelCoffFile::RealLoad('%s') called\n", sName);

	m_pFilename = sName;
	m_fd = open (sName, O_RDWR);
	if (m_fd == -1) return 0;

	printf("IntelCoffFile opened successful.\n");

	if ( sizeof m_Header != read(m_fd, &m_Header, sizeof m_Header) )
		return false;

	printf("Read COFF header\n");

	// Skip the optional header, if present
	if ( m_Header.coff_opthead_size )
	{
		printf("Skipping optional header of %d bytes.\n", (int)m_Header.coff_opthead_size);
		if ( (off_t)-1 == lseek(m_fd, m_Header.coff_opthead_size, SEEK_CUR) )
			return false;
	}

	struct struc_coff_sect *psh = (struct struc_coff_sect *)malloc(sizeof *psh * m_Header.coff_sections);
	if ( !psh )
		return false;

	if ( static_cast<signed long>(sizeof *psh * m_Header.coff_sections) != read(m_fd, psh, sizeof *psh * m_Header.coff_sections) )
	{
		free(psh);
		return false;
	}
	for ( int iSection = 0; iSection < m_Header.coff_sections; iSection++ )
	{
//		assert(0 == psh[iSection].sch_virtaddr);
//		assert(0 == psh[iSection].sch_physaddr);

		char sectname[sizeof psh->sch_sectname + 1];
		strncpy(sectname, psh[iSection].sch_sectname, sizeof psh->sch_sectname);
		sectname[sizeof psh->sch_sectname] = '\0';

		PSectionInfo psi = NULL;
		int sidx = GetSectionIndexByName(sectname);
		if ( -1 == sidx )
		{
			SectionInfo si;
			si.bCode = 0 != (psh[iSection].sch_flags & 0x20);
			si.bData = 0 != (psh[iSection].sch_flags & 0x40);
			si.bBss = 0 != (psh[iSection].sch_flags & 0x80);
			si.bReadOnly = 0 != (psh[iSection].sch_flags & 0x1000);
			si.pSectionName = strdup(sectname);

			sidx = m_iNumSections;
			psi = AddSection(&si);
		}
		else
		{
			psi = GetSectionInfo(sidx);
		}

		psh[iSection].sch_virtaddr = psi->uSectionSize;
		psh[iSection].sch_physaddr = sidx;

		psi->uSectionSize += psh[iSection].sch_sectsize;
	}
	printf("Loaded %d section headers\n", (int)m_Header.coff_sections);

	ADDRESS a = 0x40000000;
	for ( int sidx = 0; sidx < m_iNumSections; sidx++ )
	{
		PSectionInfo psi = GetSectionInfo(sidx);
		if ( psi->uSectionSize > 0 )
		{
			void *pData = malloc(psi->uSectionSize);
			if ( !pData )
				return false;
			psi->uHostAddr = (ADDRESS)pData;
			psi->uNativeAddr = a;
			a += psi->uSectionSize;
		}
	}
	printf("Allocated %d segments. a=%08x", m_iNumSections, a);
	
	for ( int iSection = 0; iSection < m_Header.coff_sections; iSection++ )
	{
		printf("Loading section %d of %hd\n", iSection+1, m_Header.coff_sections);

		PSectionInfo psi = GetSectionInfo(psh[iSection].sch_physaddr);

		if ( (off_t)psh[iSection].sch_sectptr != lseek(m_fd, (off_t)psh[iSection].sch_sectptr, SEEK_SET) )
			return false;

		char *pData = (char*)psi->uHostAddr + psh[iSection].sch_virtaddr;
		if ( !(psh[iSection].sch_flags & 0x80) )
		{
			if ( static_cast<signed long>(psh[iSection].sch_sectsize) != read(m_fd, pData, psh[iSection].sch_sectsize) )
				return false;
		}
	}

	// Load the symbol table
	printf("Load symbol table\n");
	if ( static_cast<signed long>(m_Header.coff_symtab_ofs) != lseek(m_fd, m_Header.coff_symtab_ofs, SEEK_SET) )
		return false;
	struct coff_symbol *pSymbols = (struct coff_symbol *)malloc(m_Header.coff_num_syment * sizeof (struct coff_symbol));
	if ( !pSymbols )
		return false;
	if ( static_cast<signed long>(m_Header.coff_num_syment * sizeof (struct coff_symbol)) != read(m_fd, pSymbols, m_Header.coff_num_syment * sizeof (struct coff_symbol)) )
		return false;

	// TODO: Groesse des Abschnittes vorher bestimmen
	char *pStrings = (char*)malloc(0x8000);
	read(m_fd, pStrings, 0x8000);
	

	// Run the symbol table
	ADDRESS fakeForImport = (ADDRESS)0xfffe0000;
	 
printf("Size of one symbol: %u\n", sizeof pSymbols[0]);
	for (unsigned int iSym = 0; iSym < m_Header.coff_num_syment; iSym += pSymbols[iSym].csym_numaux+1)
	{
		char tmp_name[9]; tmp_name[8] = 0;
		char* name = tmp_name;
		if ( pSymbols[iSym].csym_zeros == 0 )
		{
			// TODO: the symbol is found in a string table behind the symbol table at offset csym_offset
//			snprintf(tmp_name, 8, "n%07lx", pSymbols[iSym].csym_offset);
			name = pStrings + pSymbols[iSym].csym_offset;
		}
		else
			memcpy(tmp_name, pSymbols[iSym].csym_name, 8);

		if ( !(pSymbols[iSym].csym_loadclass & 0x60) && (pSymbols[iSym].csym_sectnum <= m_Header.coff_sections) )
		{
			if ( pSymbols[iSym].csym_sectnum > 0 )
			{
				PSectionInfo psi = GetSectionInfo(psh[pSymbols[iSym].csym_sectnum-1].sch_physaddr);
				pSymbols[iSym].csym_value += psh[pSymbols[iSym].csym_sectnum-1].sch_virtaddr + psi->uNativeAddr;
				if ( strcmp(name, ".strip.") )
					m_Symbols.Add(pSymbols[iSym].csym_value, name);
				if ( pSymbols[iSym].csym_type & 0x20 && psi->bCode )
				{
					PSectionInfo si = new SectionInfo();
					*si = *psi;
					si->uNativeAddr = pSymbols[iSym].csym_value;
					si->uHostAddr = psi->uHostAddr + psh[pSymbols[iSym].csym_sectnum-1].sch_virtaddr;
//					si->uSectionSize -= pSymbols[iSym].csym_value - psi->uNativeAddr;
//					si->uSectionSize = 1;
					si->uSectionSize = 0x10000;
					m_EntryPoints.push_back(si);
//					printf("Made '%s' an entry point.\n", name);
				}
			}
			else
			{
				if ( pSymbols[iSym].csym_type & 0x20 )
				{
					pSymbols[iSym].csym_value = fakeForImport; // TODO: external reference
					fakeForImport -= 0x10000;
					m_Symbols.Add(pSymbols[iSym].csym_value, name);
				}
				else if ( pSymbols[iSym].csym_value != 0 )
					assert(false); //	pSymbols[iSym].csym_value = ield_1C->SetName(var_8, 0, this, field_4[var_4].csym_value);
				else
				{
					pSymbols[iSym].csym_value = fakeForImport; // TODO: external reference
					fakeForImport -= 0x10000;
					m_Symbols.Add(pSymbols[iSym].csym_value, name);
				}
			}

		}
		printf("Symbol %d: %s %08lx\n", iSym, name, pSymbols[iSym].csym_value); 
	}

	for ( int iSection = 0; iSection < m_Header.coff_sections; iSection++ )
	{
//		printf("Relocating section %d of %hd\n", iSection+1, m_Header.coff_sections);
		PSectionInfo psi = GetSectionInfo(psh[iSection].sch_physaddr);
		char *pData = (char*)psi->uHostAddr + psh[iSection].sch_virtaddr;

		if ( !psh[iSection].sch_nreloc ) continue;

//printf("Relocation table at %08lx\n", psh[iSection].sch_relptr);
		if (static_cast<signed long>(psh[iSection].sch_relptr) != lseek(m_fd, psh[iSection].sch_relptr, SEEK_SET) )
			return false;

		struct struct_coff_rel *pRel = (struct struct_coff_rel *)malloc(sizeof (struct struct_coff_rel) * psh[iSection].sch_nreloc);
		if ( !pRel )
			return false;

		if (static_cast<signed long>(sizeof (struct struct_coff_rel) * psh[iSection].sch_nreloc) != read(m_fd, pRel, sizeof (struct struct_coff_rel) * psh[iSection].sch_nreloc) )
			return false;

		for ( int iReloc = 0; iReloc < psh[iSection].sch_nreloc; iReloc++ )
		{
			struct struct_coff_rel *tRel = pRel + iReloc;
                        struct coff_symbol* pSym = pSymbols+tRel->r_symndx;
			ulong *pPatch = (ulong*)(pData + tRel->r_vaddr);
//printf("Relocating at %08lx: type %d, dest %08lx\n", tRel->r_vaddr + psi->uNativeAddr + psh[iSection].sch_virtaddr, (int)tRel->r_type, pSym->csym_value);

			switch ( tRel->r_type )
			{
			case RELOC_ADDR32:
			case RELOC_ADDR32+1:
				// TODO: Handle external references
//printf("Relocating at %08lx absulute to %08lx\n", tRel->r_vaddr + psi->uNativeAddr + psh[iSection].sch_virtaddr, pSym->csym_value);
				*pPatch += pSym->csym_value;
				m_Relocations.push_back(tRel->r_vaddr);
				break;

			case RELOC_REL32:
				// TODO: Handle external references
//printf("Relocating at %08lx relative to %08lx\n", tRel->r_vaddr + psi->uNativeAddr + psh[iSection].sch_virtaddr, pSym->csym_value);
//printf("Value before relocation: %08lx\n", *pPatch);
				*pPatch += pSym->csym_value - (unsigned long)(tRel->r_vaddr + psi->uNativeAddr + psh[iSection].sch_virtaddr + 4);
//printf("Value after relocation: %08lx\n", *pPatch);
				m_Relocations.push_back(tRel->r_vaddr);
				break;
			}
		}

		free(pRel);

/*
		if ( iSection == 0 )
		{
			for ( int i = 0; i < psh[iSection].sch_sectsize ; i += 8 )
			{
				printf("%08x", i);
				for ( int j=0; j < 8; j++ )
					printf(" %02x", pData[i+j] & 0xff);
				printf("\n");
			}
		}
*/
	}

	// TODO: Perform relocation
	// TODO: Define symbols (internal, exported, imported)

	return true;
}

bool IntelCoffFile::PostLoad(void*)
{
	// There seems to be no need to implement this since one file is loaded ever.
	printf("IntelCoffFile::PostLoad called\n");
	return false;
}

void IntelCoffFile::Close()
{
	printf("IntelCoffFile::Close called\n");
}

LOAD_FMT IntelCoffFile::GetFormat() const
{
	return LOADFMT_COFF;
}

MACHINE IntelCoffFile::GetMachine() const
{
	return MACHINE_PENTIUM;
}

const char *IntelCoffFile::getFilename() const
{
	return m_pFilename;
}

bool IntelCoffFile::isLibrary() const
{
	printf("IntelCoffFile::isLibrary called\n");
	return false;
}

void IntelCoffFile::UnLoad()
{
	printf("IntelCoffFile::Unload called\n");
	// TODO: Implement when we know what is going on.
}

ADDRESS IntelCoffFile::getImageBase()
{
	// TODO: Do they really always start at 0?
	return (ADDRESS)0;
}

size_t IntelCoffFile::getImageSize()
{
	printf("IntelCoffFile::getImageSize called\n");
	// TODO: Implement it. We will have to load complete before knowing the size
	return 0;
}

ADDRESS IntelCoffFile::GetMainEntryPoint()
{
	printf("IntelCoffFile::GetMainEntryPoint called\n");
	// There is no such thing, but we need to deliver one since the first entry point might
	// be zero and this is skipped when returned by GetEntryPoint().
	//	return NO_ADDRESS;
	return GetEntryPoint();
}

ADDRESS IntelCoffFile::GetEntryPoint()
{
	printf("IntelCoffFile::GetEntryPoint called\n");
	// There is no such thing, but we have to deliver one
	if ( m_EntryPoints.empty() )
		return NO_ADDRESS;

	printf("IntelCoffFile::GetEntryPoint atleast one entry point exists\n");
	std::list<SectionInfo*>::iterator it = m_EntryPoints.begin();
	if ( !*it )
		return NO_ADDRESS;

	printf("IntelCoffFile::GetEntryPoint returning %08x\n", (*it)->uNativeAddr);
	return (*it)->uNativeAddr;
}

std::list<SectionInfo*>& IntelCoffFile::GetEntryPoints(const char* pEntry)
{
//	unused(pEntry);
	printf("IntelCoffFile::GetEntryPoints called\n");
	// TODO: Provide a list of all code exported public. We can return the list already
	//       but it needs to be filled while loading!
	return m_EntryPoints;
}

std::list<const char *> IntelCoffFile::getDependencyList()
{
	std::list<const char *> dummy;
	return dummy;	// TODO: How ever return this is ought to work out
}

extern "C" {
#ifdef _WIN32
        __declspec(dllexport)
#endif
	BinaryFile* construct()
	{
		return new IntelCoffFile();
	}
}

const char* IntelCoffFile::SymbolByAddress(const ADDRESS dwAddr) {
	return m_Symbols.find(dwAddr);
}

bool IntelCoffFile::IsDynamicLinkedProc(ADDRESS uNative) {
        if (uNative >= (unsigned)0xc0000000)
	                return true;                                                            // Say yes for fake library functions
	return false;
}

bool IntelCoffFile::IsRelocationAt(ADDRESS uNative)
{
	for ( std::list<ADDRESS>::iterator it = m_Relocations.begin(); it != m_Relocations.end(); it++ )
	{
		if ( *it == uNative ) return true;
	}
	return false;
}

std::map<ADDRESS, std::string>& IntelCoffFile::getSymbols()
{
	return m_Symbols.getAll();
}

unsigned char* IntelCoffFile::getAddrPtr(ADDRESS a, ADDRESS range)
{
	for ( int iSection = 0; iSection < m_iNumSections; iSection++ )
	{
		PSectionInfo psi = GetSectionInfo(iSection);
		if ( a >= psi->uNativeAddr && (a+range) < (psi->uNativeAddr + psi->uSectionSize) )
		{
			return (unsigned char*)(psi->uHostAddr + (a-psi->uNativeAddr));
		}
	}
	return 0;
}
int IntelCoffFile::readNative(ADDRESS a, unsigned short n)
{
	unsigned char *buf = getAddrPtr(a, (ADDRESS)n);
	if ( !a ) return 0;

	unsigned long tmp = 0;
	unsigned long mult = 1;
	for ( unsigned short o = 0; o < n; o++ )
	{
		tmp += (unsigned long)(*buf++) * mult;
		mult *= 256;
	}
	return tmp;
}

int IntelCoffFile::readNative4(ADDRESS a)
{
	return readNative(a, 4);
/*
	for ( int iSection = 0; iSection < m_iNumSections; iSection++ )
	{
		PSectionInfo psi = GetSectionInfo(iSection);
		if ( a >= psi->uNativeAddr && (a+3) < (psi->uNativeAddr + psi->uSectionSize) )
		{
			unsigned long tmp;
			unsigned char *buf = (unsigned char*)(psi->uHostAddr + (a-psi->uNativeAddr));
			tmp = *buf++;
			tmp += (unsigned long)*buf++ * 256;
			tmp += (unsigned long)*buf++ * 256 * 256;
			tmp += (unsigned long)*buf++ * 256 * 256 * 256;
			return (int)tmp;
		}
	}
	return 0;
*/
}

int IntelCoffFile::readNative2(ADDRESS a)
{
	return readNative(a, 2);
}

int IntelCoffFile::readNative1(ADDRESS a)
{
	return readNative(a, 1);
}
