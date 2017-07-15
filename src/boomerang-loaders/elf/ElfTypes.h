#pragma once

/** \file ElfTypes.h
 * \brief This file contains the elf format support structures
 */

/// Internal elf info
struct Elf32_Ehdr
{
    Byte  e_ident[4];  ///< Magic number. Should be 0x7F 'E' 'L' 'F'
    Byte  e_class;     ///< Bit format. Must be 1. (32 bit)
    Byte  endianness;  ///< 1 = little endian, 2 = big endian;
    Byte  version;     ///< ELF version. 1 = original version
    Byte  osAbi;       ///< OS ABI. For valid values, see https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
    Byte  abiVersion;  ///< OS ABI version. May be unused.
    Byte  pad[7];      ///< Currently unused.
    SWord e_type;      ///< 1 = relocatable, 2 = executable, 3 = shared, 4 = core
    SWord e_machine;   ///< Instruction set architecture. For valid values, see https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
    Byte  e_version;   ///< See above.
    DWord e_entry;     ///< Address of the entry point.
    DWord e_phoff;     ///< Offset of the Program Header table.
    DWord e_shoff;     ///< Offset of the Section Header table.
    DWord e_flags;     ///< architecture specific flags.
    SWord e_ehsize;    ///< Size of this header. Normally 64 bytes.
    SWord e_phentsize; ///< Size of an entry in the Program Header table.
    SWord e_phnum;     ///< Number of entries in the Program Header table.
    SWord e_shentsize; ///< Size of an entry in the Section Header table.
    SWord e_shnum;     ///< Number of entries in the Section Header table.
    SWord e_shstrndx;  ///< The index of the entry in the Section Header Table containing the section names.
};

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


/// Program header
struct Elf32_Phdr
{
    DWord p_type;   /* entry type */
    DWord p_offset; /* file offset */
    DWord p_vaddr;  /* virtual address */
    DWord p_paddr;  /* physical address */
    DWord p_filesz; /* file size */
    DWord p_memsz;  /* memory size */
    DWord p_flags;  /* entry flags */
    DWord p_align;  /* memory/file alignment */
};

/// Section header
struct Elf32_Shdr
{
    DWord sh_name;
    DWord sh_type;
    DWord sh_flags;
    DWord sh_addr;
    DWord sh_offset;
    DWord sh_size;
    DWord sh_link;
    DWord sh_info;
    DWord sh_addralign;
    DWord sh_entsize;
};

#define SHF_WRITE        0x01 // Writeable
#define SHF_ALLOC        0x02 // Consumes memory in exe
#define SHF_EXECINSTR    0x04 // Executable
#define SHF_MERGE        0x10


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
    DWord st_name;
    DWord st_value;
    DWord st_size;
    Byte  st_info;
    Byte  st_other;
    SWord st_shndx;
};

#pragma pack(pop)

struct Elf32_Rel
{
    DWord r_offset;
    DWord r_info;
};

struct Elf32_Rela
{
    DWord r_offset;
    DWord r_info;
    DWord r_addend;
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
