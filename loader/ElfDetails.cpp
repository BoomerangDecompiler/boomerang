#if 0
/*
 * Copyright (C) 1996-1998, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: ElfDetails.cc
 * Desc: This file implements most of the verbose decoding of the
 *       ELF file contents in a Solaris ELF environment. 
 * Note: Due to system differences between Solaris and Linux, this 
 *       file does not implement decoding of Linux ELF content. 
*/

/* Jan 1996 - Cristina
 *	 changed name to verbose.c; this file outputs all the
 *		data to stdout.
 * 26 Feb 97 - Cristina
 *      the Elf32_Dyn definition is in <sys/link.h> as per
 *              linker & libraries guide, Nov 1995.
 *	changed disassemble() for decode_instr() with appropriate
 *		parameters for fetch_word().
 *	dumpPTL() takes section header as argument (for decode_instr()).
 * 7 Mar 97 - Mike
 *	Removed dumpText() (was identical with function in load_elf)
 *	Removed verboseShdrs() (integrated with readShdrs())
 * 28 Jan 98 - Mike
 *  Removed a dependancy on Sparc (the sparc_dis.h file)
 * 3 Feb 98 - Cristina
 *	Added RTL dictionary argument to dumpPLT(), decode_instr(),
 *		dumpText() and dumpShdr().
 * 19 Feb 98 - Cristina
 *	Added include of Frontend.h and removed inlined prototype for
 *		dumpText() -- we should be using the header files.
 *	Updated dumpText() function call for SPARC only.
 * 3 Mar 98 - Cristina
 *  replaced ADDR for ADDRESS.
 * 24 Mar 98 - Cristina
 *  replaced driver include to global.h. 
 * 2 Jun 98 - Mike
 *	Loader->BinaryFile; renamed from Verbose.cc; integrated into BinaryFile
 * 14 Jan 98 - Mike
 *  Updated handling of odd section header types and tag values, e.g. 0x6ffffffe
 *	Also muted some warnings about long printf-style format strings
 *	Also proper decoding of the SUNW_verXXX sections.
 * 6 Apr 01 - Cristina
 *  Added <sys/link.h> and ElfBinaryFile.h as include files (for standalone 
 *      compilation of a loader). 
 * 17 Apr 01 - Mike: sys/link.h is not #included if LINUX is defined
 * 02 May 01 - Nathan: #include <sys/link.h> moved out to global.h
 * 29 May 01 - Cristina: displays program, section and symbol table 
 *		information in hex notation
 * 30 May 01 - Cristina: removed printing of empty strings in dumpStrTab.
 * 21 Feb 02 - Mike: Fix segfault for machine name > 8 (e.g. new Sparcs)
 */

#include "ElfBinaryFile.h"

#ifndef SHT_SUNW_verdef
#ifdef SHT_GNU_verdef
#define SHT_SUNW_verdef SHT_GNU_verdef
#define SHT_SUNW_verneed SHT_GNU_verneed
#define SHT_SUNW_versym SHT_GNU_versym
#endif
#endif

/* Names for different sections of ELF files */
char *ElfKindNames[ELF_K_NUM] =
        {"ELF_K_NONE", "ELF_K_AR", "ELF_K_COFF", "ELF_K_ELF"};
 
char *ElfCmdNames[ELF_C_NUM] =
        {"ELF_C_NULL", "ELF_C_READ", "ELF_C_WRITE", "ELF_C_CLR",
         "ELF_C_SET", "ELF_C_FDDONE", "ELF_C_FDREAD", "ELF_C_RDWR"};

/*		// Why aren't these used?
static char *ElfTypeNames[ELF_T_NUM] =
	{"ELF_T_BYTE", "ELF_T_ADDR", "ELF_T_DYN", "ELF_T_EHDR",
	 "ELF_T_HALF", "ELF_T_OFF", "ELF_T_PHDR", "ELF_T_RELA",
	 "ELF_T_REL", "ELF_T_SHDR", "ELF_T_SWORD", "ELF_T_SYM",
	 "ELF_T_WORD"};
*/

static char *E_identIndexNames[8] =
	{"EI_MAG0", "EI_MAG1", "EI_MAG2", "EI_MAG3",
	 "EI_CLASS", "EI_DATA", "EI_VERSION", "EI_PAD"};

static char *EI_ClassNames[4] =
	{"ELFCLASSNONE", "ELFCLASS32", "ELFCLASS64", "ELFCLASSNUM"};

static char *EI_DataNames[4] = 
	{"ELFDATANONE", "ELFDATA2LSB", "ELFDATA2MSB", "ELFDATANUM"};

static char *E_TypeNames[ET_NUM] =
	{"ET_NONE", "ET_REL", "ET_EXEC", "ET_DYN", "ET_CORE"};

static char *E_MachineNames[EM_NUM] =
	{"EM_NONE", "EM_M32", "EM_SPARC", "EM_386", 
	 "EM_68K", "EM_88K", "EM_486", "EM_860"};

static char *E_VersionNames[EV_NUM] =
	{"EV_NONE", "EV_CURRENT"};

static char *PTypeNames[PT_NUM] =
	{"PT_NULL", "PT_LOAD", "PT_DYNAMIC", "PT_INTERP",
	 "PT_NOTE", "PT_SHLIB", "PT_PHDR"};

static char *PFlags[8] =
	{"Nil", "PF_X", "PF_W", "PF_X | PF_W", "PF_R", 
	 "PF_X | PF_R", "PF_W | PF_R", "PF_X | PF_W | PF_R"};

static char *ShTypeNames[SHT_NUM] =
	{"SHT_NULL", "SHT_PROGBITS", "SHT_SYMTAB", "SHT_STRTAB",
	 "SHT_RELA", "SHT_HASH", "SHT_DYNAMIC", "SHT_NOTE",
	 "SHT_NOBITS", "SHT_REL", "SHT_SHLIB", "SHT_DYNSYM"};

static char *SFlags[8] =
	{"Nil", "SHF_WRITE", "SHF_ALLOC", "SHF_WRITE | SHF_ALLOC",
	 "SHF_EXECINSTR", "SHF_WRITE | SHF_EXECINSTR", 
	 "SHF_ALLOC | SHF_EXECINSTR", "SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR"};

static char *Elf32BindName[16] =
	{"STB_LOCAL", "STB_GLOBAL", "STB_WEAK", "STB_NUM",
	 "", "", "", "", "", "", "", "",
	"STB_LOPROC", "", "STB_HIPROC"};

static char *Elf32TypeName[16] =
	{"STT_NOTYPE", "STT_OBJECT", "STT_FUNC", "STT_SECTION",
	 "STT_FILE", "STT_NUM", "", "", "", "", "", "", "",
	 "STT_LOPROC", "", "STT_HIPROC"};

static char *Elf32DynTagName[25] =
	{"DT_NULL", "DT_NEEDED", "DT_PLTRELSZ", "DT_PLTGOT",
	 "DT_HASH", "DT_STRTAB", "DT_SYMTAB", "DT_RELA",
	 "DT_RELASZ", "DT_RELAENT", "DT_STRSZ", "DT_SYMENT", 
	 "DT_INIT", "DT_FINI", "DT_SONAME", "DT_RPATH", 
	 "DT_SYMBOLIC", "DT_REL", "DT_RELSZ", "DT_RELENT", 
	 "DT_PLTREL", "DT_DEBUG", "DT_TEXTREL", "DT_JMPREL", "DT_FILTER"};

static char *SparcRelocName[25] =
	{"R_SPARC_NONE", "R_SPARC_8", "R_SPARC_16",
	 "R_SPARC_32", "R_SPARC_DISP8", "R_SPARC_DISP16",
	 "R_SPARC_DISP32", "R_SPARC_WDISP30", "R_SPARC_WDISP22",
	 "R_SPARC_HI22", "R_SPARC_22", "R_SPARC_13",
	 "R_SPARC_LO10", "R_SPARC_GOT10", "R_SPARC_GOT13",
	 "R_SPARC_GOT22", "R_SPARC_PC10", "R_SPARC_PC22",
	 "R_SPARC_WPLT30", "R_SPARC_COPY", "R_SPARC_GLOB_DAT",
	 "R_SPARC_JMP_SLOT", "R_SPARC_RELATIVE", "R_SPARC_UA32",
	 "R_SPARC_NUM"};


void dumpElf32 (Elf32_Ehdr *ehdr, const char *fileName, FILE* f)
{ int i;

	fprintf (f, "Elf32 header for %s\n", fileName);
	fprintf (f, "\tIdent bytes =\n");
	for (i = 0; i < 8; i++)
	    switch (i) {
	    case EI_MAG0 : 
		fprintf (f, "\t\t%s = %X\n", E_identIndexNames[i], ehdr->e_ident[i]); 
		break;
	    case EI_MAG1:
	    case EI_MAG2:
	    case EI_MAG3:
		fprintf (f, "\t\t%s = %c\n", E_identIndexNames[i], ehdr->e_ident[i]); 
		break;
	    case EI_CLASS:
		fprintf (f, "\t\t%s = %s\n", E_identIndexNames[i], 
			EI_ClassNames[ehdr->e_ident[i]]); 
		break;
	    case EI_DATA:
		fprintf (f, "\t\t%s = %s\n", E_identIndexNames[i], 
			EI_DataNames[ehdr->e_ident[i]]); 
		break;
	    case EI_VERSION:
	    case EI_PAD:
		fprintf (f, "\t\t%s = %d\n", E_identIndexNames[i], ehdr->e_ident[i]); 
		break;
	    }
	fprintf (f, "\tFile type = %s\n", E_TypeNames[ehdr->e_type]);
    fprintf (f, "\tTarget machine = %s\n", ehdr->e_machine > 8 ? "unknown" :
        E_MachineNames[ehdr->e_machine]);
//	fprintf (f, "\tTarget machine = %s\n", E_MachineNames[ehdr->e_machine]);
	fprintf (f, "\tFile version = %s\n", E_VersionNames[ehdr->e_version]);
	fprintf (f, "\tStart address = %ld (0x%08lX)\n", ehdr->e_entry,
		ehdr->e_entry);
	fprintf (f, "\tPhdr file offset = %ld (0x%08lX)\n", ehdr->e_phoff,
		ehdr->e_phoff);
	fprintf (f, "\tShdr file offset = %ld (0x%08lX)\n", ehdr->e_shoff,
		ehdr->e_shoff);
	fprintf (f, "\tFile flags = %lX\n", ehdr->e_flags);
	fprintf (f, "\tSizeof ehdr = %d\n", ehdr->e_ehsize);
	fprintf (f, "\tSizeof phdr = %d\n", ehdr->e_phentsize);
	fprintf (f, "\tNumber phdrs = %d\n", ehdr->e_phnum);
	fprintf (f, "\tSizeof shdr = %d\n", ehdr->e_shentsize);
	fprintf (f, "\tNumber shdrs = %d\n", ehdr->e_shnum);
	fprintf (f, "\tshdr string index = %d\n\n", ehdr->e_shstrndx);
}


void dumpPhdr (Elf32_Phdr *phdr, FILE* f)
{
	fprintf (f, "Program header\n");
	fprintf (f, "\tEntry type = %s\n", PTypeNames[phdr->p_type]);
	fprintf (f, "\tFile offset = %ld\n", phdr->p_offset);
	fprintf (f, "\tVirtual address = %ld (0x%08lX)\n", phdr->p_vaddr,
		phdr->p_vaddr);
	fprintf (f, "\tPhysical address = %ld\n", phdr->p_paddr);
	fprintf (f, "\tFile size = %ld\t(=> 0x%08lX..0x%08lX)\n", phdr->p_filesz, 
		phdr->p_vaddr, phdr->p_vaddr == 0 ? phdr->p_vaddr : 
		phdr->p_vaddr + phdr->p_filesz - 1);
	fprintf (f, "\tMemory size = %ld (0x%08lX)\n", phdr->p_memsz,phdr->p_memsz);
	fprintf (f, "\tEntry flags = %s\n", PFlags[phdr->p_flags]);
	fprintf (f, "\tMemory/file alignment = %ld (0x%08lX)\n\n", phdr->p_align,
		phdr->p_align);
}


void ElfBinaryFile::dumpSymtab (char* sSymName, char* sStrName,
	Elf32_Shdr* pShdr, FILE* f)
{
	Elf32_Word i;
	Elf32_Sym sym;	/* symtab structure */
	PSectionInfo pScn = GetSectionInfoByName(sSymName);
	if (pScn == 0) return;
	Elf32_Sym* pSym = (Elf32_Sym*) pScn->uHostAddr;
	unsigned nSyms = (unsigned) pScn->uSectionSize / pScn->uSectionEntrySize;
	int idxStr = GetSectionIndexByName(sStrName);
  
	fprintf (f, "\t%s Information\n", sSymName);
	//for (i = pShdr->sh_info; i < nSyms; i++)
	for (i = 0; i < nSyms; i++)			// Sometimes local symbols are useful
	{
		memcpy (&sym, &pSym[i], sizeof(Elf32_Sym));	
		fprintf (f, "\tName[%ld] = %s\n", i, sym.st_name == 0 ? "No name" :
			GetStrPtr(idxStr, sym.st_name));
		if (ELF32_ST_TYPE(sym.st_info) == STT_FUNC) 
		   fprintf (f,
				"\t\tAddress = 0x%08lX\tSize = %ld\tScn hdr index = %d\n",
				sym.st_value, sym.st_size > 0 ? sym.st_size : 0, sym.st_shndx);
		else
		   fprintf (f, "\t\tValue = %ld (0x%08lX)\t\tSize = %ld\tScn hdr index = %d\n", 
			sym.st_value, sym.st_value, sym.st_size > 0 ? sym.st_size : 0, 
			sym.st_shndx);
		fprintf (f, "\t\tBind = %s\tType = %s\n",
			Elf32BindName[ELF32_ST_BIND(sym.st_info)],
			Elf32TypeName[ELF32_ST_TYPE(sym.st_info)]);
	}			
	fprintf (f, "\n");
}


void dumpHashTab (Elf_Scn *scn, FILE* f)
{ Elf32_Word i;                 /* counter */
  Elf_Data *d = NULL;
  Elf32_Word nBucket=0, nChain=0;  /* size of the hash table and chain table */
 
	fprintf (f, "\tHash Table Information\n");
	d = elf_getdata (scn, d);
	for (i = 0; i < d->d_size / sizeof(Elf32_Word); i++)
	{
	    if (i == 0)	
	    {
			nBucket = *((Elf32_Word *)(d->d_buf));
			fprintf (f, "\tnBuckets = %ld\n", nBucket);
	    }
	    else if (i == 1)
	    {
			nChain = *((Elf32_Word *)(d->d_buf) + 1);
			fprintf (f, "\tnChain = %ld\n", nChain);
	    }
	    else	/* i > 1 */ 
	    {
			if (i < (nBucket + 2))
		   		fprintf (f, "\tBucket[%ld] = %ld\n", i - 2, 
					*((Elf32_Word *)(d->d_buf) + i));
		else
		   fprintf (f, "\tChain[%ld] = %ld\n", i - nBucket - 2,
				*((Elf32_Word *)(d->d_buf) + i)); 
	    }
	}
	fprintf (f, "\n");
}


void ElfBinaryFile::dumpDynTab (Elf_Scn *scn, const char* name, FILE* f)
{ Elf32_Dyn e;
  Elf32_Word i = 0;
  const char* pStr;

	fprintf (f, "\tDynamic Section Table Information\n");
	PSectionInfo pSect = GetSectionInfoByName(name);
	char* pBuf = (char*) pSect->uHostAddr;
	int idxStr = GetSectionIndexByName(".dynstr");
	memcpy (&e, pBuf, sizeof(Elf32_Dyn)); 
	while (e.d_tag != DT_NULL) 
	{
		if (e.d_tag < 25)
			fprintf (f, "\t\tTag = %s\t\t", Elf32DynTagName[e.d_tag]);
		else
			fprintf(f, "\t\tTag = (unknown) %lX\n", e.d_tag);
		switch (e.d_tag) {
		  case DT_PLTRELSZ:
		  case DT_RELASZ: case DT_RELAENT:
		  case DT_STRSZ:  case DT_SYMENT:
		  case DT_SONAME: case DT_RPATH:
		  case DT_RELSZ:  case DT_RELENT:
		  case DT_PLTREL: 	
			fprintf (f, "Value = %ld\n", e.d_un.d_val);
			break;
		  case DT_NEEDED:
			fprintf (f, "Value = %ld\n", e.d_un.d_val);
			pStr = GetStrPtr(idxStr, e.d_un.d_val);
			if (pStr)
				fprintf (f, "\t\t\t\t\t%s\n", pStr);
			else fprintf (f, "\t\t\t\t\t*NULL!!*\n");
#if 0
			Elf32_Shdr* shdr = elf32_getshdr (scn);
			Elf_Scn* linkScn = elf_getscn (elf, shdr->sh_link); 
			dlink = elf_getdata (linkScn, NULL);
			if (dlink)
				fprintf (f, "\t\t\t\t\t%s\n", 
					(char *)((char *)dlink->d_buf+e.d_un.d_val)); 
			else fprintf (f, "\t\t\t\t\t*NULL!!*\n");
#endif
			break;
		  case DT_PLTGOT: case DT_HASH:
		  case DT_STRTAB: case DT_SYMTAB:
		  case DT_RELA:   case DT_INIT:
		  case DT_FINI:   case DT_REL:
		  case DT_DEBUG:  case DT_JMPREL:
		  case DT_FILTER:
			fprintf (f, "Address = 0x%08lX\n", e.d_un.d_ptr); 
		}
		i++;
		memcpy (&e, (Elf32_Dyn *)pBuf + i, sizeof(Elf32_Dyn)); 
	}
	fprintf (f, "\n");
}


void dumpInterp (Elf_Scn *scn, FILE* f)
{ Elf_Data *d = NULL;

	d = elf_getdata (scn, d);
	fprintf (f, "\tPath = %s\n\n", (char*) d->d_buf);
}


void dumpStrTab (Elf_Scn *scn, const char* name, FILE* f)
{ Elf_Data *d = NULL;
  Elf32_Word len, i = 1;

	fprintf (f, "\tStrings in Table %s\n", name);
	d = elf_getdata (scn, d);
	len = d->d_size;	
	while (i < len)
	{
		if (((char *)d->d_buf)[i] == '\0') {	// empty byte/string
			i++;
		}
		else {
			fprintf (f, "\t\t%s\n", &((char *)d->d_buf)[i]);
			i = i + strlen (&((char *)d->d_buf)[i]) + 1;
		}
	}
	fprintf (f, "\n");
}


void ElfBinaryFile::dumpPLT (Elf_Scn *scn, Elf32_Shdr *shdr,
	 Elf32_Addr sh_addr, FILE* f)
{
	Elf_Data *d = NULL;
	const char* pName;

	fprintf (f, "\tProcedure Linkage Table Information (.plt)\n");
	d = elf_getdata (scn, d);

	// Find how many entries in the table
	int n = shdr->sh_size / shdr->sh_entsize;
	ADDRESS uHostAddr = (ADDRESS)d->d_buf;
	for (int i=0; i < n; i++)
	{
		// Mike: may need changes for non Sparc!
		fprintf (f, "\t%08lX (PLT%d)\n", sh_addr, i);
		for (unsigned j=0; j < shdr->sh_entsize/4; j++)
		{
			pName = SymbolByAddress(sh_addr);
			if (pName)
				fprintf(f, "%s:\n", pName);	// Symbol

			fprintf(f, "%08lX: %08X\n", sh_addr, *(ADDRESS*)uHostAddr);
			uHostAddr += 4;
			sh_addr += 4;
		}
	}
		
	fprintf (f, "\n");
}


void dumpRela (Elf_Scn *scn, const char* scn_name, FILE* f)
{ Elf_Data *d = NULL;
  Elf32_Word i;		/* idx in # of Elf32_Rela structures */
  Elf32_Word tabSize;	/* number of entries in the table */
  Elf32_Word info;

	fprintf (f, "\tRelocation Section Table %s\n", scn_name);
	d = elf_getdata (scn, d);

	tabSize = d->d_size / sizeof(Elf32_Rela);
	for (i = 0; i < tabSize; i++)
	{
		fprintf (f, "\tAddress = %08lX\n", 
			((Elf32_Rela *)d->d_buf)[i].r_offset);
		info = ((Elf32_Rela *)d->d_buf)[i].r_info;
		fprintf (f, "\t\tInfo = %08lX\tAddend = %08lX\n", info, 
			((Elf32_Rela *)d->d_buf)[i].r_addend);
		fprintf (f, "\t\tSym = %ld\tType = %s\n", ELF32_R_SYM(info),
			SparcRelocName[ELF32_R_TYPE(info)]);
	}		
	fprintf (f, "\n");
}


void dumpRel (Elf_Scn *scn, const char* scn_name, FILE* f)
{ Elf_Data *d = NULL;
  Elf32_Word i;		/* idx in # of Elf32_Rel structures */
  Elf32_Word tabSize;	/* number of entries in the table */
  Elf32_Word info;

	fprintf (f, "\tRelocation Section Table %s\n", scn_name);
	d = elf_getdata (scn, d);

	tabSize = d->d_size / sizeof(Elf32_Rel);
	for (i = 0; i < tabSize; i++)
	{
		fprintf (f, "\tAddress = %08lX\n", 
			((Elf32_Rel *)d->d_buf)[i].r_offset);
		info = ((Elf32_Rel *)d->d_buf)[i].r_info;
		fprintf (f, "\t\tInfo = %08lX\n", info);
		fprintf (f, "\t\tSym = %ld\tType = %s\n", ELF32_R_SYM(info),
			SparcRelocName[ELF32_R_TYPE(info)]);
	}		
	fprintf (f, "\n");
}

// Must be a member of ElfBinaryFile because GetStrPtr() is also a member of
// that class
void ElfBinaryFile::dumpVerdef(Elf_Scn *scn, const char* scn_name,
	Elf32_Shdr* pShdr, FILE* f)
{
	Elf_Data *d = NULL;
	Elf32_Word i;
	d = elf_getdata (scn, d);
	Elf32_Verdef* pDef = (Elf32_Verdef*) d->d_buf;
	for (i=0; i < pShdr->sh_info; i++)
	{
		fprintf(f, "\tvd_version = %d\n", pDef->vd_version);
		fprintf(f, "\tvd_flags = %04X\n", pDef->vd_flags);
		fprintf(f, "\tvd_ndx   = %04X\n", pDef->vd_ndx);
		fprintf(f, "\tNumber of aux entries = %d\n", pDef->vd_cnt);
		fprintf(f, "\tvd_hash = %08lX\n", pDef->vd_hash);
		Elf32_Verdaux* pAux = (Elf32_Verdaux*)((char*)pDef + pDef->vd_aux);
		for (int j=0; j < pDef->vd_cnt; j++)
		{
			fprintf(f, "\t\tvda_name = %s\n",
				GetStrPtr((int)pShdr->sh_link, pAux->vda_name));
		}
		fprintf(f, "\n");
		pDef = (Elf32_Verdef*)((char*)pDef + pDef->vd_next);
	}
}

// Must be a member of ElfBinaryFile because GetStrPtr() is also a member of
// that class
void ElfBinaryFile::dumpVerneed(Elf_Scn *scn, const char* scn_name,
	Elf32_Shdr* pShdr, FILE* f)
{
	Elf_Data *d = NULL;
	Elf32_Word i;
	d = elf_getdata (scn, d);
	Elf32_Verneed* pNeed = (Elf32_Verneed*) d->d_buf;
	for (i=0; i < pShdr->sh_info; i++)
	{
		fprintf(f, "\tvn_version = %d\n", pNeed->vn_version);
		fprintf(f, "\tNumber of aux entries = %d\n", pNeed->vn_cnt);
		fprintf(f, "\tNeeded file = %s\n",
			GetStrPtr((int)pShdr->sh_link, pNeed->vn_file));
		Elf32_Vernaux* pAux = (Elf32_Vernaux*)((char*)pNeed + pNeed->vn_aux);
		for (int j=0; j < pNeed->vn_cnt; j++)
		{
			fprintf(f, "\t\tvna_hash = %08lX\n", pAux->vna_hash);
			fprintf(f, "\t\tvna_flags = %04X\n", pAux->vna_flags);
			fprintf(f, "\t\tvna_other = %04X\n", pAux->vna_other);
			fprintf(f, "\t\tvna_name = %s\n\n",
				GetStrPtr((int)pShdr->sh_link, pAux->vna_name));
			pAux = (Elf32_Vernaux*)((char*)pAux + pAux->vna_next);
		}
		pNeed = (Elf32_Verneed*)((char*)pNeed + pNeed->vn_next);
	}
}


//
// Must be a member of ElfBinaryFile because GetStrPtr() is also a member of
// that class
//
void ElfBinaryFile::dumpVersym(Elf_Scn *scn, const char* scn_name,
	Elf32_Shdr* pShdr, FILE* f)
{
	fprintf(f, "SUNW_versym section refers to symbol table in section %s\n",
		m_pSections[pShdr->sh_link].pSectionName);
	
	Elf_Data *d = NULL;
	d = elf_getdata (scn, d);
	Elf32_Versym* pSym = (Elf32_Versym*) d->d_buf;
	Elf32_Word n = pShdr->sh_size / sizeof(Elf32_Versym);
	Elf32_Word i;
	for (i=0; i < n; i++)
	{
		fprintf(f, "\tVersion Depencency Index [%ld] = %04X\n", i, pSym[i]);
	}
}


//
// Dumps the contents of a section header, as well as invokes the 
// relevant function that dumps information about the section itself. 
//
void ElfBinaryFile::dumpShdr (Elf32_Shdr *pShdr, int idxElf, FILE* f)
{
	int idxStr = GetSectionIndexByName(".shstrtab");
	// Note: some elf files combine into the strtab section
	if (idxStr == -1) 
		idxStr = GetSectionIndexByName(".strtab");
	fprintf (f, "Section header %i\n", idxElf);
	fprintf (f, "\tSection name = %s\n",  GetStrPtr(idxStr, pShdr->sh_name));
	fprintf (f, "\tSection type = ");
	if (pShdr->sh_type > SHT_NUM)
	{
		switch (pShdr->sh_type)
		{
		case SHT_SUNW_verdef:	fprintf(f, "%s\n", "SHT_SUNW_verdef"); break;
		case SHT_SUNW_verneed:	fprintf(f, "%s\n", "SHT_SUNW_verneed"); break;
		case SHT_SUNW_versym:	fprintf(f, "%s\n", "SHT_SUNW_versym"); break;
		default:
			if ((pShdr->sh_type >= SHT_LOPROC) && 
				(pShdr->sh_type <= SHT_HIPROC))
				fprintf(f, "SHT_PROC %lX\n", pShdr->sh_type);
			else if (pShdr->sh_type >= SHT_LOUSER)
				fprintf(f, "SHT_USER %lX\n", pShdr->sh_type);
			else fprintf(f, "unknown type %lX\n", pShdr->sh_type);
		}
	}
	else
		fprintf (f, "%s\n", ShTypeNames[pShdr->sh_type]);
	fprintf (f, "\tFlags = %s\n", SFlags[pShdr->sh_flags]);
	fprintf (f, "\tVirtual address = %ld (0x%08lX)\n",
		pShdr->sh_addr, pShdr->sh_addr);
	fprintf (f, "\tFile offset = %ld (0x%08lX)\n", pShdr->sh_offset,
		pShdr->sh_offset);
	fprintf (f, "\tSection size = %ld (=> 0x%08lX..0x%08lX)\n", pShdr->sh_size,
		pShdr->sh_addr, pShdr->sh_size == 0 ? pShdr->sh_addr : 
 		pShdr->sh_addr + pShdr->sh_size - 1);
	fprintf (f, "\tSection link = %ld\n", pShdr->sh_link);
	fprintf (f, "\tSection info = %ld\n", pShdr->sh_info);
	fprintf (f, "\tMemory alignment = %ld\n", pShdr->sh_addralign);
	fprintf (f, "\tEntry size if table = %ld\n\n", pShdr->sh_entsize);

	std::string name(GetStrPtr(idxStr, pShdr->sh_name));
	Elf_Scn* scn = GetElfScn(idxElf);

	switch (pShdr->sh_type)
	{
	case SHT_SYMTAB:							/* symtab section */
		dumpSymtab(".symtab", ".strtab", pShdr, f);
		break;
	case SHT_DYNSYM:							/* dyn symtab scn */
		dumpSymtab(".dynsym", ".dynstr", pShdr, f);
		break;
	case SHT_HASH:							 	/* hash table scn */
		dumpHashTab (scn, f);
		break;
	case SHT_DYNAMIC:							/* dynamic scn */
		dumpDynTab (scn, name.c_str(), f);
		break;
	case SHT_RELA:							 	/* reloc table */
		dumpRela (scn, name.c_str(), f);
		break;
	case SHT_REL:								/* reloc table, no addend */
		dumpRel (scn, name.c_str(), f);
		break;
	case SHT_STRTAB:							/* string table */
		dumpStrTab (scn, (const char*) name.c_str(), f);
		break;
	case SHT_SUNW_verdef:						/* Sun version definition */
		dumpVerdef(scn, name.c_str(), pShdr, f);
        break;
	case SHT_SUNW_verneed:
		dumpVerneed(scn, name.c_str(), pShdr, f);
		break;
	case SHT_SUNW_versym:
		dumpVerdef(scn, name.c_str(), pShdr, f);
        break;

		dumpStrTab (GetElfScn ((int) pShdr->sh_link), name.c_str(), f);
		break;
	default:
		break;
	}
	
    if (name == ".plt")      					/* PLT scn */
        dumpPLT (scn, pShdr, pShdr->sh_addr, f);
	else if (name == ".interp")
		dumpInterp(scn, f);
  	else if (name == ".comment")
       	dumpStrTab(scn, name.c_str(), f);
}

#endif
