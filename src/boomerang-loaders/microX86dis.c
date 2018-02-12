#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


#define MODRM      0x10 /*< instruction size is dependent on modrm byte */
#define OPSIZE     0x20 /*< Add current operand size to instruction size */
#define INVALID    0x40 /*< Invalid / not recognized instruction */

/* *INDENT-OFF* */
static unsigned char opmap[256] =
{
    /* Note for X86-64, 0x40-0x47 are special escapes! */

    /*          0               1                   2               3               4                   5                   6                   7 */
    /* 00-07 */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      2,                  OPSIZE + 1,         1,                  1,
    /* 08-0F */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      2,                  OPSIZE + 1,         1,                  0,
    /* 10-17 */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      2,                  OPSIZE + 1,         1,                  1,
    /* 18-1F */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      2,                  OPSIZE + 1,         1,                  1,
    /* 20-27 */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      2,                  OPSIZE + 1,         1,                  1,
    /* 28-2F */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      2,                  OPSIZE + 1,         1,                  1,
    /* 30-37 */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      2,                  OPSIZE + 1,         1,                  1,
    /* 38-3F */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      2,                  OPSIZE + 1,         1,                  1,
    /* 40-47 */ 1,              1,                  1,              1,              1,                  1,                  1,                  1,
    /* 48-4F */ 1,              1,                  1,              1,              1,                  1,                  1,                  1,
    /* 50-57 */ 1,              1,                  1,              1,              1,                  1,                  1,                  1,
    /* 58-5F */ 1,              1,                  1,              1,              1,                  1,                  1,                  1,
    /* 60-67 */ 1,              1,                  MODRM + 1,      MODRM + 1,      1,                  1,                  0,                  INVALID,
    /* 68-6F */ OPSIZE + 1,     MODRM + OPSIZE + 1, 2,              MODRM + 2,      1 + 1,              OPSIZE + 1,         1 + 1,              OPSIZE + 1,
    /* 70-77 */ 2,              2,                  2,              2,              2,                  2,                  2,                  2,
    /* 78-7F */ 2,              2,                  2,              2,              2,                  2,                  2,                  2,
    /* 80-87 */ MODRM + 1 + 1,  MODRM + OPSIZE + 1, 1 + 1,          MODRM + 1 + 1,  MODRM + 1,          MODRM + 1,          MODRM + 1,          MODRM + 1,
    /* 88-8F */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      MODRM + 1,          MODRM + 1,          MODRM + 1,          MODRM + 1,
    /* 90-97 */ 1,              1,                  1,              1,              1,                  1,                  1,                  1,
    /* 98-9F */ 1,              1,                  OPSIZE + 3,     1,              1,                  1,                  1,                  1,
    /* A0-A7 */ 2,              OPSIZE + 1,         2,              OPSIZE + 1,     1,                  1,                  1,                  1,
    /* A8-AF */ 2,              OPSIZE + 1,         1,              1,              1,                  1,                  1,                  1,
    /* B0-B7 */ 2,              2,                  2,              2,              2,                  2,                  2,                  2,
    /* B8-BF */ OPSIZE + 1,     OPSIZE + 1,         OPSIZE + 1,     OPSIZE + 1,     OPSIZE + 1,         OPSIZE + 1,         OPSIZE + 1,         OPSIZE + 1,
    /* C0-C7 */ MODRM + 2,      MODRM + 2,          3,              1,              MODRM + OPSIZE + 3, MODRM + OPSIZE + 3, MODRM + 2,          MODRM + OPSIZE + 1,
    /* C8-CF */ 4,              1,                  3,              1,              1,                  2,                  1,                  1,
    /* D0-D7 */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      1,                  1,                  INVALID,            1,

    /* FP instructions */
    /* D8-DF */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      MODRM + 1,          MODRM + 1,          MODRM + 1,          MODRM + 1,
    /* E0-E7 */ 2,              2,                  2,              2,              2,                  2,                  2,                  2,
    /* E8-EF */ OPSIZE + 1,     OPSIZE + 1,         OPSIZE + 3,     2,              1,                  1,                  1,                  1,
    /* F0-F7 */ 1,              INVALID,            1,              1,              1,                  1,                  MODRM + 1,          MODRM + 1,
    /* F8-FF */ 1,              1,                  1,              1,              1,                  1,                  MODRM + 1,          MODRM + 1
};

/* Secondary map for when first opcode is 0F */
static unsigned char op0Fmap[256] =
{
    /*          0               1                   2               3               4                   5                   6                   7 */
    /* 00-07 */ MODRM + 1,      MODRM + 1,          MODRM + 1,      MODRM + 1,      INVALID,            INVALID,            2,                  INVALID,
    /* 08-0F */ 2,              2,                  INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 10-17 */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 18-1F */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 20-27 */ 2,              2,                  2,              2,              2,                  INVALID,            2,                  INVALID,
    /* 28-2F */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 30-37 */ 2,              2,                  2,              INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 38-3F */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 40-47 */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 48-4F */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 50-57 */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 58-5F */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 60-67 */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 68-6F */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 70-77 */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 78-7F */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* 80-87 */ OPSIZE + 2,     OPSIZE + 2,         OPSIZE + 2,     OPSIZE + 2,     OPSIZE + 2,         OPSIZE + 2,         OPSIZE + 2,         OPSIZE + 2,
    /* 88-8F */ OPSIZE + 2,     OPSIZE + 2,         OPSIZE + 2,     OPSIZE + 2,     OPSIZE + 2,         OPSIZE + 2,         OPSIZE + 2,         OPSIZE + 2,
    /* 90-97 */ MODRM + 2,      MODRM + 2,          MODRM + 2,      MODRM + 2,      MODRM + 2,          MODRM + 2,          MODRM + 2,          MODRM + 2,
    /* 98-9F */ MODRM + 2,      MODRM + 2,          MODRM + 2,      MODRM + 2,      MODRM + 2,          MODRM + 2,          MODRM + 2,          MODRM + 2,
    /* A0-A7 */ 2,              2,                  2,              MODRM + 2,      MODRM + 3,          MODRM + 2,          INVALID,            INVALID,
    /* A8-AF */ 2,              2,                  2,              MODRM + 2,      MODRM + 3,          MODRM + 2,          INVALID,            MODRM + 2,
    /* B0-B7 */ MODRM + 2,      MODRM + 2,          MODRM + OPSIZE + 2, MODRM + 2,  MODRM + OPSIZE + 2, MODRM + OPSIZE + 2, MODRM + 2,          MODRM + 2,
    /* B8-BF */ INVALID,        INVALID,            MODRM + 3,      MODRM + 2,      MODRM + 2,          MODRM + 2,          MODRM + 2,          MODRM + 2,
    /* C0-C7 */ MODRM + 2,      MODRM + 2,          INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* C8-CF */ 2,              2,                  2,              2,              2,                  2,                  2,                  2,
    /* D0-D7 */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* D8-DF */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* E0-E7 */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* E8-EF */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* F0-F7 */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID,
    /* F8-FF */ INVALID,        INVALID,            INVALID,        INVALID,        INVALID,            INVALID,            INVALID,            INVALID
};
/* *INDENT-ON* */


/**
 * Tiny X86 disassembler, capable only of finding the number of bytes of a given instruction
 * In other words, a surface engine
 * Assumes a seg32 segment; ignores the address size override
 * Note that the function could return Not Handled (NH, 0x40)
 */
int microX86Dis(const unsigned char *instruction)
{
    const unsigned char *p = instruction;
    int                 opsize = 4; /* Operand size override will change to 2 */
    int                 size = 0;
    unsigned char       modrm;
    unsigned char       op     = *p++;
    int                 prefix = 1;

    while (prefix) {
        switch (op)
        {
        case 0x66:
            /* Operand size override */
            opsize = 2;
            op     = *p++;
            size  += 1;                 /* Count the 0x66 */
            break;

        case 0xF0:
        case 0xF2:
        case 0xF3:
        case 0x26:
        case 0x2E:
        case 0x36:
        case 0x3E:
        case 0x64:
        case 0x65:
            /* Prefixes (Lock/repne/rep, segment overrides);
             * Count these as part of the instruction rather than
             * returning 1 byte.
             * Easier to compare output with disassembly */
            op    = *p++;
            size += 1;                  /* Count the prefix */
            break;

        default:
            prefix = 0;
        }
    }

    if (op == 0x0F) {
        /* Two byte escape */
        unsigned char op2   = *p++;
        size += op0Fmap[op2];
    }
    else {
        /* Normal case: look up the main table */
        size += opmap[op];
    }

    if (size & MODRM) {
        size &= ~MODRM;     /* Remove flag from size */
        size++;             /* Count the mod/rm itself */
        modrm = *p++;
        unsigned char mod   = modrm >> 6;

        if ((mod != 3) && ((modrm & 0x7) == 4)) {
            /* SIB also present */
            size++;     /* Count the SIB itself */
            unsigned char sib = *p++;

            if ((mod == 0) && ((sib & 0x7) == 0x5)) {
                /* ds:d32 with scale */
                size += 4;
            }
        }

        /* Regardless of whether a SIB is present or not... */
        if (mod == 1) {
            size++;                     /* d8 */
        }
        else if (mod == 2) {
            size += 4;                  /* d32 */
        }

        /* ds:d32 is a special case */
        if ((mod == 0) && ((modrm & 0x7) == 5)) {
            size += 4;                  /* ds:d32 */
        }
    }
    else {
        modrm = 0;
    }

    if (size & OPSIZE) {
        /* This means add on the current op size */
        size &= ~OPSIZE;
        size += opsize;
    }

    if (op == 0xF6) {
        /* Group 3, byte */
        if (((modrm & 0x38) >> 3) == 0) {
            /** There is an immediate byte as well */
            size++;
        }
    }

    if (op == 0xF7) {
        /* Group 3, word */
        if (((modrm & 0x38) >> 3) == 0) {
            /* There is an immediate word as well */
            size += opsize;
        }
    }

    return size;
}
