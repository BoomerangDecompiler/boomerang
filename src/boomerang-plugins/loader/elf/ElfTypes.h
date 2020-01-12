#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include <cstdint>


/// \file ElfTypes.h This file contains the elf format support structures
/// \sa http://docs.oracle.com/cd/E18752_01/pdf/817-1984.pdf

// Data types for 32 bit ELF files
typedef uint8_t Elf32_Byte;
typedef uint16_t Elf32_Half;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;

typedef uint8_t Elf64_Byte;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Sword;
typedef int32_t Elf64_Word;
typedef int64_t Elf64_Sxword;
typedef uint64_t Elf64_Xword;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;


// number of bytes for ELF file type identification
#define EI_NIDENT 16

#pragma pack(push, 1)
/// Internal elf info
struct Elf32_Ehdr
{
    Elf32_Byte e_ident[EI_NIDENT]; ///< ELF file identification bytes

    Elf32_Half e_type;      ///< ELF file type
    Elf32_Half e_machine;   ///< Instruction set architecture.
    Elf32_Word e_version;   ///< See above.
    Elf32_Addr e_entry;     ///< Address of the entry point.
    Elf32_Off e_phoff;      ///< Offset of the Program Header table.
    Elf32_Off e_shoff;      ///< Offset of the Section Header table.
    Elf32_Word e_flags;     ///< architecture specific flags.
    Elf32_Half e_ehsize;    ///< Size of this header. Normally 64 bytes.
    Elf32_Half e_phentsize; ///< Size of an entry in the Program Header table.
    Elf32_Half e_phnum;     ///< Number of entries in the Program Header table.
    Elf32_Half e_shentsize; ///< Size of an entry in the Section Header table.
    Elf32_Half e_shnum;     ///< Number of entries in the Section Header table.

    /// The index of the entry in the Section Header Table containing the section names.
    Elf32_Half e_shstrndx;
};

struct Elf64_Ehdr
{
    Elf64_Byte e_ident[EI_NIDENT];
    Elf64_Half e_type;
    Elf64_Half e_machine;
    Elf64_Word e_version;
    Elf64_Addr e_entry;
    Elf64_Off e_phoff;
    Elf64_Off e_shoff;
    Elf64_Word e_flags;
    Elf64_Half e_ehsize;
    Elf64_Half e_phentsize;
    Elf64_Half e_phnum;
    Elf64_Half e_shentsize;
    Elf64_Half e_shnum;
    Elf64_Half e_shstrndx;
};
#pragma pack(pop)

// clang-format off
#define EI_MAGO              0
#define EI_MAG1              1
#define EI_MAG2              2
#define EI_MAG3              3
#define EI_CLASS             4
#define EI_DATA              5
#define EI_VERSION           6
#define EI_OSABI             7
#define EI_ABIVERSION        8
#define EI_PAD               9

// EI_MAG0-3
#define ELFMAG0              0x7F
#define ELFMAG1              'E'
#define ELFMAG2              'L'
#define ELFMAG3              'F'

// EI_CLASS
#define ELFCLASSNONE         0
#define ELFCLASS32           1
#define ELFCLASS64           2

// EI_DATA
#define ELFDATANONE          0
#define ELFDATA2LSB          1
#define ELFDATA2MSB          2

// EI_VERSION
#define EV_NONE              0
#define EV_CURRENT           1

// e_type
#define ET_NONE              0
#define ET_REL               1
#define ET_EXEC              2
#define ET_DYN               3
#define ET_CORE              4
#define ET_LOPROC            0xFF00
#define ET_HIPROC            0xFFFF

// e_machine
#define EM_NONE              0
#define EM_SPARC             2    ///< Sun SPARC
#define EM_386               3    ///< Intel 80386 or higher (x86-32)
#define EM_68K               4    ///< Motorola 68000
#define EM_MIPS              8    ///< MIPS
#define EM_PA_RISC           15   ///< HP PA-RISC
#define EM_SPARC32PLUS       18   ///< Sun SPARC 32+
#define EM_PPC               20   ///< PowerPC
#define EM_SPARCV9           43   ///< SPARC V9
#define EM_AMD64             62   ///< x86-64
#define EM_ST20              0xa8 // ST20 (made up... there is no official value?)

// e_flags
#define EF_SPARC_EXT_MASK    0xffff00 ///< Vendor Extension mask
#define EF_SPARC_32PLUS      0x000100 ///< Generic V8+ features
#define EF_SPARC_SUN_US1     0x000200 ///< Sun UltraSPARC 1 Extensions
#define EF_SPARC_HAL_R1      0x000400 ///< HAL R1 Extensions
#define EF_SPARC_SUN_US3     0x000800 ///< Sun UltraSPARC 3 Extensions
#define EF_SPARCV9_MM        0x3      ///< Mask for Memory Model
#define EF_SPARCV9_TSO       0x0      ///< Total Store Ordering
#define EF_SPARCV9_PSO       0x1      ///< Partial Store Ordering
#define EF_SPARCV9_RMO       0x2      ///< Relaxed Memory Ordering

// e_phnum
/**>
 * The actual number of program header table enties is in the
 * sh_info field of the section header at index 0.
 */
#define PN_XNUM    0xFFFF


// sections

// special section indexes
#define SHN_UNDEF            0
#define SHN_LORESERVE        0xff00
#define SHN_LOPROC           0xff00
#define SHN_BEFORE           0xff00
#define SHN_AFTER            0xff01
#define SHN_AMD64_LCOMMON    0xff02
#define SHN_HIPROC           0xff1f
#define SHN_LOOS             0xff20
#define SHN_LOSUNW           0xff3f
#define SHN_SUNW_IGNORE      0xff3f
#define SHN_HISUNW           0xff3f
#define SHN_HIOS             0xff3f
#define SHN_ABS              0xfff1
#define SHN_COMMON           0xfff2
#define SHN_XINDEX           0xffff
#define SHN_HIRESERVE        0xffff
// clang-format on


#pragma pack(push, 1)
struct Elf32_Shdr
{
    Elf32_Word sh_name;      ///< section name.
    Elf32_Word sh_type;      ///< section type. See SHT_* values.
    Elf32_Word sh_flags;     ///< section flags See SHF_* values.
    Elf32_Addr sh_addr;      ///< section base address in memory.
    Elf32_Off sh_offset;     ///< byte offset of the section data from the beginning of the file
    Elf32_Word sh_size;      ///< section data size in bytes.
    Elf32_Word sh_link;      ///< Interpretation depends on the section type.
    Elf32_Word sh_info;      ///< Extra information; Interpretation depends on section type.
    Elf32_Word sh_addralign; ///< Force alignment to x bytes; 0 = no alignment
    Elf32_Word sh_entsize;   ///< Entry size for fixed-entrysize tables (symbol tables etc.)
};


struct Elf64_Shdr
{
    Elf64_Word sh_name;
    Elf64_Word sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr sh_addr;
    Elf64_Off sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word sh_link;
    Elf64_Word sh_info;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
};
#pragma pack(pop)


// sh_type
enum ElfSectionTypes
{
    SHT_NULL           = 0,
    SHT_PROGBITS       = 1,
    SHT_SYMTAB         = 2, ///< Symbol table
    SHT_STRTAB         = 3,
    SHT_RELA           = 4, ///< Relocation table (with addend, e.g. RISC)
    SHT_HASH           = 5,
    SHT_DYNAMIC        = 6,
    SHT_NOTE           = 7,
    SHT_NOBITS         = 8, ///< Bss
    SHT_REL            = 9, ///< Relocation table (no addend)
    SHT_SHLIB          = 10,
    SHT_DYNSYM         = 11, ///< Dynamic symbol table
    SHT_INIT_ARRAY     = 14,
    SHT_FINI_ARRAY     = 15,
    SHT_PREINIT_ARRAY  = 16,
    SHT_GROUP          = 17,
    SHT_SYMTAB_SHNDX   = 18,
    SHT_LOOS           = 0x60000000,
    SHT_LOSUNW         = 0x6fffffef,
    SHT_SUNW_capchain  = 0x6fffffef,
    SHT_SUNW_capinfo   = 0x6ffffff0,
    SHT_SUNW_symsort   = 0x6ffffff1,
    SHT_SUNW_tlssort   = 0x6ffffff2,
    SHT_SUNW_LDYNSYM   = 0x6ffffff3,
    SHT_SUNW_dof       = 0x6ffffff4,
    SHT_SUNW_cap       = 0x6ffffff5,
    SHT_SUNW_SIGNATURE = 0x6ffffff6,
    SHT_SUNW_ANNOTATE  = 0x6ffffff7,
    SHT_SUNW_DEBUGSTR  = 0x6ffffff8,
    SHT_SUNW_DEBUG     = 0x6ffffff9,
    SHT_SUNW_move      = 0x6ffffffa,
    SHT_SUNW_COMDAT    = 0x6ffffffb,
    SHT_SUNW_syminfo   = 0x6ffffffc,
    SHT_SUNW_verdef    = 0x6ffffffd,
    SHT_SUNW_verneed   = 0x6ffffffe,
    SHT_SUNW_versym    = 0x6fffffff,
    SHT_HISUNW         = 0x6fffffff,
    SHT_HIOS           = 0x6fffffff,
    SHT_LOPROC         = 0x70000000,
    SHT_SPARC_GOTDATA  = 0x70000000,
    SHT_AMD64_UNWIND   = 0x70000001,
    SHT_HIPROC         = 0x7fffffff,
    SHT_LOUSER         = 0x80000000,
    SHT_HIUSER         = 0xffffffff
};

// clang-format off
// sh_flags
#define SHF_WRITE               (1 << 0) ///< Writeable
#define SHF_ALLOC               (1 << 1) ///< Consumes memory in exe
#define SHF_EXECINSTR           (1 << 2) ///< Executable
#define SHF_MERGE               (1 << 4)
#define SHF_STRINGS             (1 << 5)
#define SHF_INFO_LINK           (1 << 6)
#define SHF_LINK_ORDER          (1 << 7)
#define SHF_OS_NONCONFORMING    (1 << 8)
#define SHF_GROUP               (1 << 9)
#define SHF_TLS                 (1 << 10)
#define SHF_MASKOS              0x0ff00000
#define SHF_AMD64_LARGE         0x10000000
#define SHF_ORDERED             0x40000000
#define SHF_EXCLUDE             0x80000000
#define SHF_MASKPROC            0xf0000000

#define ELF32_C_SYM(info)         ((info) >> 8)
#define ELF32_C_GROUP(info)       ((unsigned char)(info))
#define ELF32_C_INFO(sym, grp)    (((sym) << 8) + (unsigned char)(grp))
#define ELF64_C_SYM(info)         ((info) >> 32)
#define ELF64_C_GROUP(info)       ((Elf64_Word)(info))
#define ELF64_C_INFO(sym, grp)    (((Elf64_Xword)(sym) << 32) + (Elf64_Xword)
// clang-format on

#pragma pack(push, 1)
// Relocation
struct Elf32_Rel
{
    Elf32_Addr r_offset;
    Elf32_Word r_info;
};

struct Elf32_Rela
{
    Elf32_Addr r_offset;
    Elf32_Word r_info;
    Elf32_Sword r_addend;
};

struct Elf64_Rel
{
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
};

struct Elf64_Rela
{
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
    Elf64_Sxword r_addend;
};
#pragma pack(pop)

// clang-format off
#define ELF32_R_SYM(info)                ((info) >> 8)
#define ELF32_R_TYPE(info)               (static_cast<unsigned char>(info))
#define ELF32_R_INFO(sym, type)          (((sym) << 8) + (unsigned char)(type))
#define ELF64_R_SYM(info)                ((info) >> 32)
#define ELF64_R_TYPE(info)               ((Elf64_Word)(info))
#define ELF64_R_INFO(sym, type)          (((Elf64_Xword)(sym) << 32) + (Elf64_Xword)(type))

#define ELF64_R_TYPE_DATA(info)          (((Elf64_Xword)(info) << 32) >> 40)
#define ELF64_R_TYPE_ID(info)            (((Elf64_Xword)(info) << 56) >> 56)
#define ELF64_R_TYPE_INFO(data, type)    (((Elf64_Xword)(data) << 8) + (Elf64_Xword)(type))


/**
 * Abbreviations for relocations:
 * A    The addend used to compute the value of the relocatable field.
 * B    The base address at which a shared object is loaded into memory during execution.
 *      Generally, a shared object file is built with a base virtual address of 0.
 *      However, the execution address of the shared object is different. See
 *      “Program Header” on page 282.
 * G    The offset into the global offset table at which the address of the relocation entry's
 *      symbol resides during execution. See “Global Offset Table (Processor-Specific)” on
 *      page 309.
 * GOT  The address of the global offset table. See “Global Offset Table (Processor-Specific)”
 *      on page 309.
 * L    The section offset or address of the procedure linkage table entry for a symbol. See
 *      “Procedure Linkage Table (Processor-Specific)” on page 310.
 * O    The secondary addend used to compute the value of the relocation field. This
 *      addend is extracted from the r_info field by applying the ELF64_R_TYPE_DATA macro.
 *      (64-bit SPARC only)
 * P    The section offset or address of the storage unit being relocated, computed using
 *      r_offset.
 * S    The value of the symbol whose index resides in the relocation entry.
 * Z    The size of the symbol whose index resides in the relocation entry.
 */


// Relocation types for x86-32
#define R_386_NONE           0  ///< None       None
#define R_386_32             1  ///< word32     S + A
#define R_386_PC32           2  ///< word32     S+A-P
#define R_386_GOT32          3  ///< word32     G + A
#define R_386_PLT32          4  ///< word32     L+A-P
#define R_386_COPY           5  ///< None
#define R_386_GLOB_DAT       6  ///< word32     S
#define R_386_JMP_SLOT       7  ///< word32     S
#define R_386_RELATIVE       8  ///< word32     B + A
#define R_386_GOTOFF         9  ///< word32     S+A-GOT
#define R_386_GOTPC          10 ///< word32     GOT+A-P
#define R_386_32PLT          11 ///< word32     L + A
#define R_386_16             20 ///< word16     S + A
#define R_386_PC16           21 ///< word16     S+A-P
#define R_386_8              22 ///< word8      S + A
#define R_386_PC8            23 ///< word8      S+A-P
#define R_386_SIZE32         38 ///< word32     Z + A

// Relocation types for x86-64
#define R_AMD64_NONE         0  ///< None       None
#define R_AMD64_64           1  ///< word64     S + A
#define R_AMD64_PC32         2  ///< word32     S+A-P
#define R_AMD64_GOT32        3  ///< word32     G + A
#define R_AMD64_PLT32        4  ///< word32     L+A-P
#define R_AMD64_COPY         5  ///< None
#define R_AMD64_GLOB_DAT     6  ///< word64     S
#define R_AMD64_JUMP_SLOT    7  ///< word64     S
#define R_AMD64_RELATIVE     8  ///< word64     B + A
#define R_AMD64_GOTPCREL     9  ///< word32     G + GOT+A-P
#define R_AMD64_32           10 ///< word32     S + A
#define R_AMD64_32S          11 ///< word32     S + A
#define R_AMD64_16           12 ///< word16     S + A
#define R_AMD64_PC16         13 ///< word16     S+A-P
#define R_AMD64_8            14 ///< word8      S + A
#define R_AMD64_PC8          15 ///< word8      S+A-P
#define R_AMD64_PC64         24 ///< word64     S+A-P
#define R_AMD64_GOTOFF64     25 ///< word64     S+A-GOT
#define R_AMD64_GOTPC32      26 ///< word32     GOT+A+P
#define R_AMD64_SIZE32       32 ///< word32     Z + A
#define R_AMD64_SIZE64       33 ///< word64     Z + A


/// SPARC relocation types
#define R_SPARC_NONE                0  ///< None       None
#define R_SPARC_8                   1  ///< V-byte8    S + A
#define R_SPARC_16                  2  ///< V-half16   S + A
#define R_SPARC_32                  3  ///< V-word32   S + A
#define R_SPARC_DISP8               4  ///< V-byte8    S+A-P
#define R_SPARC_DISP16              5  ///< V-half16   S+A-P
#define R_SPARC_DISP32              6  ///< V-disp32   S+A-P
#define R_SPARC_WDISP30             7  ///< V-disp30   (S+A-P)>>2
#define R_SPARC_WDISP22             8  ///< V-disp22   (S+A-P)>>2
#define R_SPARC_HI22                9  ///< T-imm22    (S + A) >> 10
#define R_SPARC_22                  10 ///< V-imm22    S + A
#define R_SPARC_13                  11 ///< V-simm13   S + A
#define R_SPARC_LO10                12 ///< T-simm13   (S + A) & 0x3ff
#define R_SPARC_GOT10               13 ///< T-simm13   G & 0x3ff
#define R_SPARC_GOT13               14 ///< V-simm13   G
#define R_SPARC_GOT22               15 ///< T-simm22   G >> 10
#define R_SPARC_PC10                16 ///< T-simm13   (S+A-P) & 0x3ff
#define R_SPARC_PC22                17 ///< V-disp22   (S+A-P) >> 10
#define R_SPARC_WPLT30              18 ///< V-disp30   (L+A-P) >> 2
#define R_SPARC_COPY                19 ///< None
#define R_SPARC_GLOB_DAT            20 ///< V-word32   S + A
#define R_SPARC_JMP_SLOT            21 ///< None
#define R_SPARC_RELATIVE            22 ///< V-word32   B + A
#define R_SPARC_UA32                23 ///< V-word32   S + A
#define R_SPARC_PLT32               24 ///< V-word32   L + A
#define R_SPARC_HIPLT22             25 ///< T-imm22    (L + A) >> 10
#define R_SPARC_LOPLT10             26 ///< T-simm13   (L + A) & 0x3ff
#define R_SPARC_PCPLT32             27 ///< V-word32   L+A-P
#define R_SPARC_PCPLT22             28 ///< V-disp22   (L+A-P)>>10
#define R_SPARC_PCPLT10             29 ///< V-simm13   (L+A-P) & 0x3ff
#define R_SPARC_10                  30 ///< V-simm10   S + A
#define R_SPARC_11                  31 ///< V-simm11   S + A
#define R_SPARC_HH22                34 ///< V-imm22    (S + A) >> 42
#define R_SPARC_HM10                35 ///< T-simm13   ((S + A) >> 32) & 0x3ff
#define R_SPARC_LM22                36 ///< T-imm22    (S + A) >> 10
#define R_SPARC_PC_HH22             37 ///< V-imm22    (S+A-P)>>42
#define R_SPARC_PC_HM10             38 ///< T-simm13   ((S+A-P)>>32) & 0x3ff
#define R_SPARC_PC_LM22             39 ///< T-imm22    (S+A-P)>>10
#define R_SPARC_WDISP16             40 ///< V-d2/disp14 (S+A-P)>>2
#define R_SPARC_WDISP19             41 ///< V-disp19   (S+A-P)>>2
#define R_SPARC_7                   43 ///< V-imm7     S + A
#define R_SPARC_5                   44 ///< V-imm5     S + A
#define R_SPARC_6                   45 ///< V-imm6     S + A
#define R_SPARC_HIX22               48 ///< V-imm22    ((S + A) ^ 0xffffffffffffffff) >> 10
#define R_SPARC_LOX10               49 ///< T-simm13   ((S + A) & 0x3ff) | 0x1c00
#define R_SPARC_H44                 50 ///< V-imm22    (S + A) >> 22
#define R_SPARC_M44                 51 ///< T-imm10    ((S + A) >> 12) & 0x3ff
#define R_SPARC_L44                 52 ///< T-imm13    (S + A) & 0xfff
#define R_SPARC_REGISTER            53 ///< V-word32   S + A
#define R_SPARC_UA16                55 ///< V-half16   S + A
#define R_SPARC_GOTDATA_HIX22       80 ///< V-imm22   ((S+A-GOT)>>10)^((S+A-GOT)>> 31)
#define R_SPARC_GOTDATA_LOX10       81 ///< T-imm13   ((S+A-GOT)&0x3ff)|(((S+A-GOT) >> 31) & 0x1c00)
#define R_SPARC_GOTDATA_OP_HIX22    82 ///< T-imm22    (G >> 10) ^ (G >> 31)
#define R_SPARC_GOTDATA_OP_LOX10    83 ///< T-imm13    (G & 0x3ff) | ((G >> 31) & 0x1c00)
#define R_SPARC_GOTDATA_OP          84 ///< Word32
#define R_SPARC_SIZE32              86 ///< V-word32   Z + A

// 64-bit SPARC
#define R_SPARC_HI22                9  ///< V-imm22    (S + A) >> 10
#define R_SPARC_GLOB_DAT            20 ///< V-xword64  S + A
#define R_SPARC_RELATIVE            22 ///< V-xword64  B + A
#define R_SPARC_64                  32 ///< V-xword64  S + A
#define R_SPARC_OLO10               33 ///< V-simm13   ((S + A) & 0x3ff) + O
#define R_SPARC_DISP64              46 ///< V-xword64  S+A-P
#define R_SPARC_PLT64               47 ///< V-xword64  L + A
#define R_SPARC_REGISTER            53 ///< V-xword64  S + A
#define R_SPARC_UA64                54 ///< V-xword64  S + A
#define R_SPARC_H34                 85 ///< V-imm22    (S + A) >> 12
#define R_SPARC_SIZE64              87 ///< V-xword64  Z + A
// clang-format on


#pragma pack(push, 1)

/// Program header
struct Elf32_Phdr
{
    Elf32_Word p_type;   /* entry type */
    Elf32_Off p_offset;  /* file offset */
    Elf32_Addr p_vaddr;  /* virtual address */
    Elf32_Addr p_paddr;  /* physical address */
    Elf32_Word p_filesz; /* file size */
    Elf32_Word p_memsz;  /* memory size */
    Elf32_Word p_flags;  /* entry flags */
    Elf32_Word p_align;  /* memory/file alignment */
};

struct Elf64_Phdr
{
    Elf64_Word p_type;
    Elf64_Word p_flags;
    Elf64_Off p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
};
#pragma pack(pop)

// clang-format off
// p_type
#define PT_NULL             0
#define PT_LOAD             1
#define PT_DYNAMIC          2
#define PT_INTERP           3
#define PT_NOTE             4
#define PT_SHLIB            5
#define PT_PHDR             6
#define PT_TLS              7
#define PT_LOOS             0x60000000
#define PT_SUNW_UNWIND      0x6464e550
#define PT_SUNW_EH_FRAME    0x6474e550
#define PT_LOSUNW           0x6ffffffa
#define PT_SUNWBSS          0x6ffffffa
#define PT_SUNWSTACK        0x6ffffffb
#define PT_SUNWDTRACE       0x6ffffffc
#define PT_SUNWCAP          0x6ffffffd
#define PT_HISUNW           0x6fffffff
#define PT_HIOS             0x6fffffff
#define PT_LOPROC           0x70000000
#define PT_HIPROC           0x7fffffff

// p_flags
#define PF_X                0x1 ///< Execute
#define PF_W                0x2 ///< Write
#define PF_R                0x4 ///< Read
#define PF_MASKPROC         0xf0000000
// clang-format on

// ELF symbols
#pragma pack(push, 1)
struct Elf32_Sym
{
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    Elf32_Byte st_info;
    Elf32_Byte st_other;
    Elf32_Half st_shndx;
};

struct Elf64_Sym
{
    Elf64_Word st_name;
    Elf64_Byte st_info;
    Elf64_Byte st_other;
    Elf64_Half st_shndx;
    Elf64_Addr st_value;
    Elf64_Xword st_size;
};
#pragma pack(pop)

// clang-format off
#define ELF32_ST_BIND(info)          static_cast<ElfSymBinding>((info) >> 4)
#define ELF32_ST_TYPE(info)          static_cast<ElfSymType>((info) & 0xf)
#define ELF32_ST_INFO(bind, type)    (((bind) << 4) + ((type) & 0xf))
#define ELF64_ST_BIND(info)          ((info) >> 4)
#define ELF64_ST_TYPE(info)          ((info) & 0xf)
#define ELF64_ST_INFO(bind, type)    (((bind) << 4) + ((type) & 0xf))

#define ELF32_ST_VISIBILITY(o)       static_cast<ElfSymVisibility>((o) & 0x3)
#define ELF64_ST_VISIBILITY(o)       static_cast<ElfSymVisibility>((o) & 0x3)
// clang-format on

enum ElfSymBinding
{
    STB_LOCAL  = 0,
    STB_GLOBAL = 1,
    STB_WEAK   = 2,
    STB_LOOS   = 10,
    STB_HIOS   = 12,
    STB_LOPROC = 13,
    STB_HIPROC = 15
};

enum ElfSymType
{
    STT_NOTYPE         = 0, ///< Symbol table type: none
    STT_OBJECT         = 1,
    STT_FUNC           = 2, ///< Symbol table type: function
    STT_SECTION        = 3,
    STT_FILE           = 4,
    STT_COMMON         = 5,
    STT_TLS            = 6, ///< Thread Local Storage
    STT_LOOS           = 10,
    STT_HIOS           = 12,
    STT_LOPROC         = 13,
    STT_SPARC_REGISTER = 14,
    STT_HIPROC         = 15
};

enum ElfSymVisibility
{
    STV_DEFAULT   = 0,
    STV_INTERNAL  = 1,
    STV_HIDDEN    = 2,
    STV_PROTECTED = 3,
    STV_EXPORTED  = 4,
    STV_SINGLETON = 5,
    STV_ELIMINATE = 6
};


// dynamic section data

#pragma pack(push, 1)
struct Elf32_Dyn
{
    Elf32_Sword d_tag; /* how to interpret value */
    union
    {
        Elf32_Word d_val;
        Elf32_Addr d_ptr;
        Elf32_Off d_off;
    } d_un;
};

struct Elf64_Dyn
{
    Elf64_Xword d_tag;
    union
    {
        Elf64_Xword d_val;
        Elf64_Addr d_ptr;
    } d_un;
};
#pragma pack(pop)

// clang-format off
// Tag values
#define DT_NULL                0 ///< Last entry in list
#define DT_NEEDED              1 ///< A needed link-type object
#define DT_PLTRELSZ            2
#define DT_PLTGOT              3
#define DT_HASH                4
#define DT_STRTAB              5 ///< String table
#define DT_SYMTAB              6
#define DT_RELA                7
#define DT_RELASZ              8
#define DT_RELAENT             9
#define DT_STRSZ               10
#define DT_SYMENT              11
#define DT_INIT                12
#define DT_FINI                13
#define DT_SONAME              14
#define DT_RPATH               15
#define DT_SYMBOLIC            16
#define DT_REL                 17
#define DT_RELSZ               18
#define DT_RELENT              19
#define DT_PLTREL              20
#define DT_DEBUG               21
#define DT_TEXTREL             22
#define DT_JMPREL              23
#define DT_BIND_NOW            24
#define DT_INIT_ARRAY          25
#define DT_FINI_ARRAY          26
#define DT_INIT_ARRAYSZ        27
#define DT_FINI_ARRAYSZ        28
#define DT_RUNPATH             29
#define DT_FLAGS               30
#define DT_ENCODING            32 ///< 31???
#define DT_PREINIT_ARRAY       32
#define DT_PREINIT_ARRAYSZ     33
#define DT_MAXPOSTAGS          34
#define DT_LOOS                0x6000000d
#define DT_SUNW_AUXILIARY      0x6000000d
#define DT_SUNW_RTLDINF        0x6000000e
#define DT_SUNW_FILTER         0x6000000e
#define DT_SUNW_CAP            0x60000010
#define DT_SUNW_SYMTAB         0x60000011
#define DT_SUNW_SYMSZ          0x60000012
#define DT_SUNW_ENCODING       0x60000013
#define DT_SUNW_SORTENT        0x60000013
#define DT_SUNW_SYMSORT        0x60000014
#define DT_SUNW_SYMSORTSZ      0x60000015
#define DT_SUNW_TLSSORT        0x60000016
#define DT_SUNW_TLSSORTSZ      0x60000017
#define DT_SUNW_CAPINFO        0x60000018
#define DT_SUNW_STRPAD         0x60000019
#define DT_SUNW_CAPCHAIN       0x6000001a
#define DT_SUNW_LDMACH         0x6000001b
#define DT_SUNW_CAPCHAINENT    0x6000001d
#define DT_SUNW_CAPCHAINSZ     0x6000001f
#define DT_HIOS                0x6ffff000
#define DT_VALRNGLO            0x6ffffd00
#define DT_CHECKSUM            0x6ffffdf8
#define DT_PLTPADSZ            0x6ffffdf9
#define DT_MOVEENT             0x6ffffdfa
#define DT_MOVESZ              0x6ffffdfb
#define DT_POSFLAG_1           0x6ffffdfd
#define DT_SYMINSZ             0x6ffffdfe
#define DT_SYMINENT            0x6ffffdff
#define DT_VALRNGHI            0x6ffffdff
#define DT_ADDRRNGLO           0x6ffffe00
#define DT_CONFIG              0x6ffffefa
#define DT_DEPAUDIT            0x6ffffefb
#define DT_AUDIT               0x6ffffefc
#define DT_PLTPAD              0x6ffffefd
#define DT_MOVETAB             0x6ffffefe
#define DT_SYMINFO             0x6ffffeff
#define DT_ADDRRNGHI           0x6ffffeff
#define DT_RELACOUNT           0x6ffffff9
#define DT_RELCOUNT            0x6ffffffa
#define DT_FLAGS_1             0x6ffffffb
#define DT_VERDEF              0x6ffffffc
#define DT_VERDEFNUM           0x6ffffffd
#define DT_VERNEED             0x6ffffffe
#define DT_VERNEEDNUM          0x6fffffff
#define DT_LOPROC              0x70000000
#define DT_SPARC_REGISTER      0x70000001
#define DT_AUXILIARY           0x7ffffffd
#define DT_USED                0x7ffffffe
#define DT_FILTER              0x7fffffff
#define DT_HIPROC              0x7fffffff
// clang-format on
