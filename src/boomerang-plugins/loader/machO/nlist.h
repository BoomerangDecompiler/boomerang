#include <cstdint>


struct nlist
{
    union
    {
        uint32_t n_name; /* for use when in-core */
        uint32_t n_strx; /* index into file string table */
    } n_un;

    unsigned char n_type; /* type flag; see below */
    unsigned char n_sect; /* section number or NO_SECT */
    short n_desc;         /* see the header file stab.h */
    unsigned n_value;     /* value of this symbol table entry
                           * (or stab offset) */
};


// clang-format off
#define N_STAB      0xe0 /* if any bits are set, this is a symbolic debugging entry */
#define N_TYPE      0x1e /* mask for the type bits */
#define N_EXT       0x01 /* external symbol bit; set for external symbols */
#define N_UNDF      0x0  /* undefined; n_sect == NO_SECT */
#define N_ABS       0x2  /* absolute; n_sect == NO_SECT */
#define N_SECT      0xe  /* defined in section number n_sect */
#define N_INDR      0xa  /* indirect */
#define NO_SECT     0    /* the symbol isn't in any section */
#define MAX_SECT    255  /* 1 through 255 inclusive */
// clang-format on
