// Needed by both signature.h and frontend.h
#ifndef __SIGENUM__
#define __SIGENUM__

enum platform {
    PLAT_PENTIUM,
    PLAT_SPARC,
    PLAT_M68K,
    PLAT_PARISC,
    PLAT_PPC,
    PLAT_MIPS
};

enum callconv {
    CONV_C,         // Standard C, no callee pop
    CONV_PASCAL,    // callee pop
    CONV_THISCALL   // MSVC "thiscall": one parameter in register ecx
};

#endif  // __SIGENUM__
