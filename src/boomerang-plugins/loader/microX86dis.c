#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


/* clang-format off */
#define INS_MODRM      0x10 /*< instruction size is dependent on modrm byte */
#define INS_OPSIZE     0x20 /*< Add current operand size to instruction size */
#define INS_INVALID    0x40 /*< Invalid / not recognized instruction */

static const unsigned char opmap[256] =
{
    /* Note that for X86-64, 0x40-0x47 are special escapes! */

    /*          0                   1                           2                   3                   4                           5                           6                   7 */
    /* 00-07 */ INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,      2,                          INS_OPSIZE + 1,             1,                  1,
    /* 08-0F */ INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,      2,                          INS_OPSIZE + 1,             1,                  0,
    /* 10-17 */ INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,      2,                          INS_OPSIZE + 1,             1,                  1,
    /* 18-1F */ INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,      2,                          INS_OPSIZE + 1,             1,                  1,
    /* 20-27 */ INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,      2,                          INS_OPSIZE + 1,             1,                  1,
    /* 28-2F */ INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,      2,                          INS_OPSIZE + 1,             1,                  1,
    /* 30-37 */ INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,      2,                          INS_OPSIZE + 1,             1,                  1,
    /* 38-3F */ INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,      2,                          INS_OPSIZE + 1,             1,                  1,
    /* 40-47 */ 1,                  1,                          1,                  1,                  1,                          1,                          1,                  1,
    /* 48-4F */ 1,                  1,                          1,                  1,                  1,                          1,                          1,                  1,
    /* 50-57 */ 1,                  1,                          1,                  1,                  1,                          1,                          1,                  1,
    /* 58-5F */ 1,                  1,                          1,                  1,                  1,                          1,                          1,                  1,
    /* 60-67 */ 1,                  1,                          INS_MODRM + 1,      INS_MODRM + 1,      1,                          1,                          0,                  INS_INVALID,
    /* 68-6F */ INS_OPSIZE + 1,     INS_MODRM + INS_OPSIZE + 1, 2,                  INS_MODRM + 2,      1 + 1,                      INS_OPSIZE + 1,             1 + 1,              INS_OPSIZE + 1,
    /* 70-77 */ 2,                  2,                          2,                  2,                  2,                          2,                          2,                  2,
    /* 78-7F */ 2,                  2,                          2,                  2,                  2,                          2,                          2,                  2,
    /* 80-87 */ INS_MODRM + 1 + 1,  INS_MODRM + INS_OPSIZE + 1, 1 + 1,              INS_MODRM + 1 + 1,  INS_MODRM + 1,              INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,
    /* 88-8F */ INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,
    /* 90-97 */ 1,                  1,                          1,                  1,                  1,                          1,                          1,                  1,
    /* 98-9F */ 1,                  1,                          INS_OPSIZE + 3,     1,                  1,                          1,                          1,                  1,
    /* A0-A7 */ 2,                  INS_OPSIZE + 1,             2,                  INS_OPSIZE + 1,     1,                          1,                          1,                  1,
    /* A8-AF */ 2,                  INS_OPSIZE + 1,             1,                  1,                  1,                          1,                          1,                  1,
    /* B0-B7 */ 2,                  2,                          2,                  2,                  2,                          2,                          2,                  2,
    /* B8-BF */ INS_OPSIZE + 1,     INS_OPSIZE + 1,             INS_OPSIZE + 1,     INS_OPSIZE + 1,     INS_OPSIZE + 1,             INS_OPSIZE + 1,             INS_OPSIZE + 1,     INS_OPSIZE + 1,
    /* C0-C7 */ INS_MODRM + 2,      INS_MODRM + 2,              3,                  1,                  INS_MODRM + INS_OPSIZE + 3, INS_MODRM + INS_OPSIZE + 3, INS_MODRM + 2,      INS_MODRM + INS_OPSIZE + 1,
    /* C8-CF */ 4,                  1,                          3,                  1,                  1,                          2,                          1,                  1,
    /* D0-D7 */ INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,      1,                          1,                          INS_INVALID,        1,

    /* FP instructions */
    /* D8-DF */ INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,      INS_MODRM + 1,              INS_MODRM + 1,              INS_MODRM + 1,      INS_MODRM + 1,
    /* E0-E7 */ 2,                  2,                          2,                  2,                  2,                          2,                          2,                  2,
    /* E8-EF */ INS_OPSIZE + 1,     INS_OPSIZE + 1,             INS_OPSIZE + 3,     2,                  1,                          1,                          1,                  1,
    /* F0-F7 */ 1,                  INS_INVALID,                1,                  1,                  1,                          1,                          INS_MODRM + 1,      INS_MODRM + 1,
    /* F8-FF */ 1,                  1,                          1,                  1,                  1,                          1,                          INS_MODRM + 1,      INS_MODRM + 1
};

/* Secondary map for when first opcode is 0F */
static const unsigned char op0Fmap[256] =
{
    /*          0               1               2                           3               4                           5                           6                       7 */
    /* 00-07 */ INS_MODRM + 1,  INS_MODRM + 1,  INS_MODRM + 1,              INS_MODRM + 1,  INS_INVALID,                INS_INVALID,                2,                      INS_INVALID,
    /* 08-0F */ 2,              2,              INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 10-17 */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 18-1F */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 20-27 */ 2,              2,              2,                          2,              2,                          INS_INVALID,                2,                      INS_INVALID,
    /* 28-2F */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 30-37 */ 2,              2,              2,                          INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 38-3F */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 40-47 */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 48-4F */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 50-57 */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 58-5F */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 60-67 */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 68-6F */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 70-77 */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 78-7F */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* 80-87 */ INS_OPSIZE + 2, INS_OPSIZE + 2, INS_OPSIZE + 2,             INS_OPSIZE + 2, INS_OPSIZE + 2,             INS_OPSIZE + 2,             INS_OPSIZE + 2,         INS_OPSIZE + 2,
    /* 88-8F */ INS_OPSIZE + 2, INS_OPSIZE + 2, INS_OPSIZE + 2,             INS_OPSIZE + 2, INS_OPSIZE + 2,             INS_OPSIZE + 2,             INS_OPSIZE + 2,         INS_OPSIZE + 2,
    /* 90-97 */ INS_MODRM + 2,  INS_MODRM + 2,  INS_MODRM + 2,              INS_MODRM + 2,  INS_MODRM + 2,              INS_MODRM + 2,              INS_MODRM + 2,          INS_MODRM + 2,
    /* 98-9F */ INS_MODRM + 2,  INS_MODRM + 2,  INS_MODRM + 2,              INS_MODRM + 2,  INS_MODRM + 2,              INS_MODRM + 2,              INS_MODRM + 2,          INS_MODRM + 2,
    /* A0-A7 */ 2,              2,              2,                          INS_MODRM + 2,  INS_MODRM + 3,              INS_MODRM + 2,              INS_INVALID,            INS_INVALID,
    /* A8-AF */ 2,              2,              2,                          INS_MODRM + 2,  INS_MODRM + 3,              INS_MODRM + 2,              INS_INVALID,            INS_MODRM + 2,
    /* B0-B7 */ INS_MODRM + 2,  INS_MODRM + 2,  INS_MODRM + INS_OPSIZE + 2, INS_MODRM + 2,  INS_MODRM + INS_OPSIZE + 2, INS_MODRM + INS_OPSIZE + 2, INS_MODRM + 2,          INS_MODRM + 2,
    /* B8-BF */ INS_INVALID,    INS_INVALID,    INS_MODRM + 3,              INS_MODRM + 2,  INS_MODRM + 2,              INS_MODRM + 2,              INS_MODRM + 2,          INS_MODRM + 2,
    /* C0-C7 */ INS_MODRM + 2,  INS_MODRM + 2,  INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* C8-CF */ 2,              2,              2,                          2,              2,                          2,                          2,                      2,
    /* D0-D7 */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* D8-DF */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* E0-E7 */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* E8-EF */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* F0-F7 */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID,
    /* F8-FF */ INS_INVALID,    INS_INVALID,    INS_INVALID,                INS_INVALID,    INS_INVALID,                INS_INVALID,                INS_INVALID,            INS_INVALID
};
/* clang-format on */


/**
 * Tiny X86 disassembler, capable only of finding the number of bytes of a given instruction
 * In other words, a surface engine
 * Assumes a seg32 segment; ignores the address size override
 * Note that the function could return Not Handled (NH, 0x40)
 */
int microX86Dis(const unsigned char *instruction)
{
    const unsigned char *p = instruction;
    int opsize             = 4; /* Operand size override will change to 2 */
    int size               = 0;
    unsigned char modrm;
    unsigned char op = *p++;
    int prefix       = 1;

    while (prefix) {
        switch (op) {
        case 0x66:
            /* Operand size override */
            opsize = 2;
            op     = *p++;
            size += 1; /* Count the 0x66 */
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
            op = *p++;
            size += 1; /* Count the prefix */
            break;

        default: prefix = 0;
        }
    }

    if (op == 0x0F) {
        /* Two byte escape */
        unsigned char op2 = *p++;
        size += op0Fmap[op2];
    }
    else {
        /* Normal case: look up the main table */
        size += opmap[op];
    }

    if (size & INS_MODRM) {
        size &= ~INS_MODRM; /* Remove flag from size */
        size++;             /* Count the mod/rm itself */
        modrm             = *p++;
        unsigned char mod = modrm >> 6;

        if ((mod != 3) && ((modrm & 0x7) == 4)) {
            /* SIB also present */
            size++; /* Count the SIB itself */
            unsigned char sib = *p++;

            if ((mod == 0) && ((sib & 0x7) == 0x5)) {
                /* ds:d32 with scale */
                size += 4;
            }
        }

        /* Regardless of whether a SIB is present or not... */
        if (mod == 1) {
            size++; /* d8 */
        }
        else if (mod == 2) {
            size += 4; /* d32 */
        }

        /* ds:d32 is a special case */
        if ((mod == 0) && ((modrm & 0x7) == 5)) {
            size += 4; /* ds:d32 */
        }
    }
    else {
        modrm = 0;
    }

    if (size & INS_OPSIZE) {
        /* This means add on the current op size */
        size &= ~INS_OPSIZE;
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
