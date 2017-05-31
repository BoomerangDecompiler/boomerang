#pragma once

/** \file ElfTypes.h
 * \brief This file contains the elf format support structures
 */

// Internal elf info
typedef struct
{
	char  e_ident[4];
	char  e_class;
	char  endianness;
	char  version;
	char  osAbi;
	char  pad[8];
	short e_type;
	short e_machine;
	int   e_version;
	int   e_entry;
	int   e_phoff;
	int   e_shoff;
	int   e_flags;
	short e_ehsize;
	short e_phentsize;
	short e_phnum;
	short e_shentsize;
	short e_shnum;
	short e_shstrndx;
} Elf32_Ehdr;

#define EM_SPARC          2    // Sun SPARC
#define EM_386            3    // Intel 80386 or higher
#define EM_68K            4    // Motorola 68000
#define EM_MIPS           8    // MIPS
#define EM_PA_RISC        15   // HP PA-RISC
#define EM_SPARC32PLUS    18   // Sun SPARC 32+
#define EM_PPC            20   // PowerPC
#define EM_X86_64         62
#define EM_ST20           0xa8 // ST20 (made up... there is no official value?)

#define ET_DYN            3    // Elf type (dynamic library)

enum ElfRelocKind
{
	R_386_32        = 1,
	R_386_PC32      = 2,
	R_386_GOT32     = 3,
	R_386_PLT32     = 4,
	R_386_COPY      = 5,
	R_386_GLOB_DAT  = 6,
	R_386_JUMP_SLOT = 7,
	R_386_RELATIVE  = 8,
	R_386_GOTOFF    = 9,
	R_386_GOTPC     = 10
};

#define R_SPARC_NONE        0
#define R_SPARC_8           1
#define R_SPARC_16          2
#define R_SPARC_32          3
#define R_SPARC_DISP8       4
#define R_SPARC_DISP16      5
#define R_SPARC_DISP32      6
#define R_SPARC_WDISP30     7
#define R_SPARC_WDISP22     8
#define R_SPARC_HI22        9
#define R_SPARC_22          10
#define R_SPARC_13          11
#define R_SPARC_LO10        12
#define R_SPARC_GOT10       13
#define R_SPARC_GOT13       14
#define R_SPARC_GOT22       15
#define R_SPARC_PC10        16
#define R_SPARC_PC22        17
#define R_SPARC_WPLT30      18
#define R_SPARC_COPY        19
#define R_SPARC_GLOB_DAT    20
#define R_SPARC_JMP_SLOT    21
#define R_SPARC_RELATIVE    22
#define R_SPARC_UA32        23
#define R_SPARC_PLT32       24
#define R_SPARC_HIPLT22     25
#define R_SPARC_LOPLT10     26
#define R_SPARC_PCPLT32     27
#define R_SPARC_PCPLT22     28
#define R_SPARC_PCPLT10     29
#define R_SPARC_10          30
#define R_SPARC_11          31
#define R_SPARC_64          32
#define R_SPARC_OLO10       33
#define R_SPARC_HH22        34
#define R_SPARC_HM10        35
#define R_SPARC_LM22        36
#define R_SPARC_PC_HH22     37
#define R_SPARC_PC_HM10     38
#define R_SPARC_PC_LM22     39
#define R_SPARC_WDISP16     40
#define R_SPARC_WDISP19     41
#define R_SPARC_GLOB_JMP    42
#define R_SPARC_7           43
#define R_SPARC_5           44
#define R_SPARC_6           45
#define R_SPARC_DISP64      46
#define R_SPARC_PLT64       47
#define R_SPARC_HIX22       48
#define R_SPARC_LOX10       49
#define R_SPARC_H44         50
#define R_SPARC_M44         51
#define R_SPARC_L44         52
#define R_SPARC_REGISTER    53
#define R_SPARC_UA64        54
#define R_SPARC_UA16        55

// Program header
struct Elf32_Phdr
{
	int p_type;   /* entry type */
	int p_offset; /* file offset */
	int p_vaddr;  /* virtual address */
	int p_paddr;  /* physical address */
	int p_filesz; /* file size */
	int p_memsz;  /* memory size */
	int p_flags;  /* entry flags */
	int p_align;  /* memory/file alignment */
};

// Section header
struct Elf32_Shdr
{
	int sh_name;
	int sh_type;
	int sh_flags;
	int sh_addr;
	int sh_offset;
	int sh_size;
	int sh_link;
	int sh_info;
	int sh_addralign;
	int sh_entsize;
};

#define SHF_WRITE        1 // Writeable
#define SHF_ALLOC        2 // Consumes memory in exe
#define SHF_EXECINSTR    4 // Executable
enum ElfSectionTypes
{
	SHT_NULL     = 0,
	SHT_PROGBITS = 1,
	SHT_SYMTAB   = 2, // Symbol table
	SHT_STRTAB   = 3,
	SHT_RELA     = 4, // Relocation table (with addend, e.g. RISC)
	SHT_NOBITS   = 8, // Bss
	SHT_REL      = 9, // Relocation table (no addend)
	SHT_DYNSYM   = 11 // Dynamic symbol table
};

#pragma pack(push,1)
struct Elf32_Sym
{
	int           st_name;
	unsigned      st_value;
	int           st_size;
	unsigned char st_info;
	unsigned char st_other;
	short         st_shndx;
};

#pragma pack(pop)

struct Elf32_Rel
{
	unsigned r_offset;
	int      r_info;
};

struct Elf32_Rela
{
	unsigned r_offset;
	int      r_info;
	int      r_addend;
};

#define ELF32_R_SYM(info)         ((info) >> 8)
#define ELF32_ST_BIND(i)          ((ElfSymBinding)((i) >> 4))
#define ELF32_ST_TYPE(i)          ((ElfSymType)((i) & 0xf))
#define ELF32_ST_INFO(b, t)       (((b) << 4) + ((t) & 0xf))
#define ELF32_ST_VISIBILITY(o)    ((ElfSymVisibility)((o) & 0x3))
enum ElfSymType
{
	STT_NOTYPE  = 0, // Symbol table type: none
	STT_OBJECT  = 1,
	STT_FUNC    = 2, // Symbol table type: function
	STT_SECTION = 3,
	STT_FILE    = 4,
	STT_COMMON  = 5
};

enum ElfSymBinding
{
	STB_LOCAL  = 0,
	STB_GLOBAL = 1,
	STB_WEAK   = 2
};

enum ElfSymVisibility
{
	STV_DEFAULT   = 0,
	STV_INTERNAL  = 1,
	STV_HIDDEN    = 2,
	STV_PROTECTED = 3
};

typedef struct
{
	short d_tag; /* how to interpret value */
	union
	{
		int d_val;
		int d_ptr;
		int d_off;
	}     d_un;
} Elf32_Dyn;

// Tag values
#define DT_NULL      0 // Last entry in list
#define DT_STRTAB    5 // String table
#define DT_NEEDED    1 // A needed link-type object

#define E_REL        1 // Relocatable file type
