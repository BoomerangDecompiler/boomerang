/* Tiny X86 disassembler, capable only of finding the number of bytes of
 * a given instruction
 * In other words, a surface engine
 * Assumes a seg32 segment; ignores the address size override
 * Note that the function could return Not Handled (NH, 0x40)
 */

#define MODRM 0x10
#define OPSIZE 0x20
#define NH 0x40

static unsigned char opmap[256] = {
    /* 00-07 */
    MODRM+1, MODRM+1, MODRM+1, MODRM+1, 2, OPSIZE+1, 1, 1,
    /* 08-0F */
    MODRM+1, MODRM+1, MODRM+1, MODRM+1, 2, OPSIZE+1, 1, 0,
    /* 10-17 */
    MODRM+1, MODRM+1, MODRM+1, MODRM+1, 2, OPSIZE+1, 1, 1,
    /* 18-1F */
    MODRM+1, MODRM+1, MODRM+1, MODRM+1, 2, OPSIZE+1, 1, 1,
    /* 20-27 */
    MODRM+1, MODRM+1, MODRM+1, MODRM+1, 2, OPSIZE+1, 1, 1,
    /* 28-2F */
    MODRM+1, MODRM+1, MODRM+1, MODRM+1, 2, OPSIZE+1, 1, 1,
    /* 30-37 */
    MODRM+1, MODRM+1, MODRM+1, MODRM+1, 2, OPSIZE+1, 1, 1,
    /* 38-3F */
    MODRM+1, MODRM+1, MODRM+1, MODRM+1, 2, OPSIZE+1, 1, 1,
    /* 40-4F. Note for X86-64, these are special escapes! */
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    /* 50-57 */
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    /* 60-67 */
    1, 1, MODRM+1, MODRM+1, 1, 1, 0, NH,
    /* 68-6F */
    OPSIZE+1, MODRM+OPSIZE+1, 2, MODRM+2, 1+1, OPSIZE+1, 1+1, OPSIZE+1,
    /* 70-7F */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
    /* 80-87 */
    MODRM+1+1, MODRM+OPSIZE+1, 1+1, MODRM+1+1, MODRM+1, MODRM+1,MODRM+1,MODRM+1,
    /* 88-8F */
    MODRM+1,MODRM+1,MODRM+1,MODRM+1, MODRM+1,MODRM+1,MODRM+1,MODRM+1,
    /* 90-97 */
    1,1,1,1,1,1,1,1,
    /* 98-9F */
    1, 1, OPSIZE+3, 1, 1, 1, 1, 1,
    /* A0-A7 */
    2, OPSIZE+1, 2, OPSIZE+1, 1, 1, 1, 1,
    /* A8-AF */
    2, OPSIZE+1, 2, OPSIZE+1, 1, 1, 1, 1,
    /* B0-B7 */
    2,2,2,2,2,2,2,2,
    /* B8-BF */
    OPSIZE+1,OPSIZE+1,OPSIZE+1,OPSIZE+1,OPSIZE+1,OPSIZE+1,OPSIZE+1,OPSIZE+1,
    /* C0-C7 */
    MODRM+2, MODRM+2, 3, 1, MODRM+OPSIZE+3, MODRM+OPSIZE+3, MODRM+2, MODRM+OPSIZE+1,
    /* C8-CF */
    4, 1, 3, 1, 1, 2, 1, 1,
    /* D0-D7 */
    MODRM+1, MODRM+1, MODRM+1, MODRM+1, 1, 1, NH, 1,
    /* D8-DF (floating point) */
    MODRM+1,MODRM+1,MODRM+1,MODRM+1,MODRM+1,MODRM+1,MODRM+1,MODRM+1,
    /* E0-E7 */
    2,2,2,2, 2,2,2,2,
    /* E8-EF */
    OPSIZE+1, OPSIZE+1, OPSIZE+3, 2, 1, 1, 1, 1,
    /* F0-F7 */
    1, NH, 1, 1, 1, 1, MODRM+1, MODRM+1,
    /* F8-FF */
    1, 1, 1, 1, 1, 1, MODRM+1, MODRM+1
};

/* Secondary map for when first opcode is 0F */
    static unsigned char op0Fmap[256] = {
    /* 00-07 */
    MODRM+1, MODRM+1, MODRM+1, MODRM+1, NH, NH, 2, NH,
    /* 08-0F */
    2, 2, NH, NH, NH,NH,NH,NH,
    /* 10-1F */
    NH, NH, NH, NH, NH, NH, NH, NH,
    NH, NH, NH, NH, NH, NH, NH, NH,
    /* 20-27 */
    2, 2, 2, 2, 2, NH, 2, NH,
    /* 28-2F */
    NH, NH, NH, NH, NH, NH, NH, NH,
    /* 30-37 */
    2, 2, 2, NH, NH, NH, NH, NH,
    /* 38-3F */
    NH, NH, NH, NH, NH, NH, NH, NH,
    /* 40-4F */
    NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH,
    /* 50-5F */
    NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH,
    /* 60-6F */
    NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH,
    /* 70-7F */
    NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH,
    /* 80-8F */
    OPSIZE+2,OPSIZE+2,OPSIZE+2,OPSIZE+2,
    OPSIZE+2,OPSIZE+2,OPSIZE+2,OPSIZE+2,
    OPSIZE+2,OPSIZE+2,OPSIZE+2,OPSIZE+2,
    OPSIZE+2,OPSIZE+2,OPSIZE+2,OPSIZE+2,
    /* 90-97 */
    2,2,2,2,2,2,2,2,
    /* 98-9F */
    MODRM+2,MODRM+2,MODRM+2,MODRM+2,MODRM+2,MODRM+2,MODRM+2,MODRM+2,
    /* A0-A7 */
    2,2,2, MODRM+2, MODRM+3, MODRM+2, NH, NH,
    /* A8-AF */
    2,2,2, MODRM+2, MODRM+3, MODRM+2, NH, MODRM+2,
    /* B0-B7 */
    MODRM+2, MODRM+2, MODRM+OPSIZE+2, MODRM+2, MODRM+OPSIZE+2, MODRM+OPSIZE+2,
      MODRM+2, MODRM+2,
    /* B8-BF */
    NH, NH, MODRM+3, MODRM+2, MODRM+2, MODRM+2, MODRM+2, MODRM+2,
    /* C0-C7 */
    MODRM+2, MODRM+2, NH,NH,NH,NH,NH,NH,
    /* C8-CF */
    2,2,2,2,2,2,2,2,
    /* D0-FF */
    NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH,
    NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH,
    NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH, NH
};

int microX86Dis(unsigned char* pCode) {
    int opsize = 4;             /* Operand size override will change to 2 */
    int size = 0;
    unsigned char modrm, mod, op2, sib;    
    unsigned char op = *pCode++; 
    int prefix = 1;

    while (prefix) {
        switch (op) {
            case 0x66:
                /* Operand size override */
                opsize = 2;
                op = *pCode++; 
                size += 1;              /* Count the 0x66 */
                break;
            case 0xF0: case 0xF2: case 0xF3:
            case 0x26: case 0x2E:
            case 0x36: case 0x3E:
            case 0x64: case 0x65:
                /* Prefixes (Lock/repne/rep, segment overrides);
                  count these as part of the instruction rather than
                  returning 1 byte.
                  Easier to compare output with disassembly */
                op = *pCode++; 
                size += 1;              /* Count the prefix */
                break;
            default:
                prefix = 0;
        }
    }

    if (op == 0x0F) {
        /* Two byte escape */
        op2 = *pCode++;
        size += op0Fmap[op2];
    }
    else
        /* Normal case: look up the main table */
        size += opmap[op];
    if (size & MODRM) {
        size &= ~MODRM;     /* Remove flag from size */
        size++;             /* Count the mod/rm itself */
        modrm = *pCode++;
        mod = modrm >> 6;
        if ((mod != 3) && ((modrm & 0x7) == 4)) {
            /* SIB also present */
            size++;     /* Count the SIB itself */
            sib = *pCode++;
            if ((mod == 0) && ((sib & 0x7) == 0x5)) {
                /* ds:d32 with scale */
                size += 4;
            }
        }
        /* Regardless of whether a SIB is present or not... */
        if (mod == 1) size++;           /* d8 */
        else if (mod == 2) size += 4;   /* d32 */
        /* ds:d32 is a special case */
        if ((mod == 0) && ((modrm & 0x7) == 5))
            size += 4;                  /* ds:d32 */
    }
    if (size & OPSIZE) {
        /* This means add on the current op size */
        size &= ~OPSIZE;
        size += opsize;
    }
    if (op == 0xF6) {
        /* Group 3, byte */
        if (((modrm & 0x38) >> 3) == 0) {
            // There is an immediate byte as well
            size++;
        }
    }
    if (op == 0xF7) {
        /* Group 3, word */
        if (((modrm & 0x38) >> 3) == 0) {
            // There is an immediate word as well
            size += opsize;
        }
    }
            
    return size;
}
