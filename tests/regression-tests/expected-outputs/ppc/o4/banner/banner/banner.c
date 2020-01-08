int main(int argc, char *argv[]);

union { unsigned int; void *; } glyphs[84];

/** address: 0x10000468 */
int main(int argc, char *argv[])
{
    __size4 CR1; 		// r101
    __size4 CR6; 		// r106
    __size32 CTR; 		// r301
    int XERCA; 		// r202
    union { unsigned int; __size32 *; } g0; 		// r0
    char *g0_1; 		// r0
    union { unsigned int; void *; } g1; 		// r1
    int g10; 		// r10
    union { unsigned int; void *; } g10_1; 		// r10{39}
    union { unsigned int; void *; } g10_2; 		// r10{41}
    int g11; 		// r11
    __size32 g26; 		// r26
    __size32 g26_1; 		// r26{14}
    __size32 g26_2; 		// r26{64}
    union { unsigned int; __size32 *; } g29; 		// r29
    union { unsigned int; __size32 *; } g29_1; 		// r29{24}
    union { unsigned int; __size32 *; } g29_2; 		// r29{59}
    int g3; 		// r3
    union { unsigned int; char **; } g30; 		// r30
    int g31; 		// r31
    char *g3_1; 		// r3
    int g4; 		// r4
    int g5; 		// r5
    unsigned int g5_1; 		// r5{31}
    unsigned int g5_2; 		// r5{44}
    int g7; 		// r7
    int g8; 		// r8
    int g9; 		// r9
    union { unsigned int; void *; } g9_1; 		// r9{53}
    union { unsigned int; void *; } g9_2; 		// r9{54}
    char local0; 		// m[g1 - 112]
    __size32 local2; 		// g26_1{14}
    union { unsigned int; __size32 *; } local3; 		// g29_1{24}
    unsigned int local4; 		// g5_1{31}
    union { unsigned int; void *; } local5; 		// g10_1{39}
    union { unsigned int; void *; } local6; 		// g9_1{53}

    g26 = 1;
    local2 = g26;
    g3 = malloc(12);
    *(__size32*)(g3 + 4) = 0x10000990;
    g0 = g3 + 4;
    g30 = g3;
    do {
        g26_1 = local2;
        g3_1 = *g30;
        g3 = strlen(g3_1);
        g31 = g3;
        if (g3 > 10) {
            g31 = 10;
        }
        g29 = 0;
        local3 = g29;
        do {
            g29_1 = local3;
            g5 = 0;
            local4 = g5;
            flags = SUBFLAGSNS(0, g31, CR1);
            if (0 < g31) {
                g4 = (g1 - 120);
                do {
                    g5_1 = local4;
                    g0_1 = *g30;
                    g10 = 0;
                    local5 = g10;
                    g7 = *(unsigned char*)(g5_1 + g0_1);
                    g8 = g7 - 32 & ~(g7 - 32) >> 31;
                    g0 = ((g8 >> 3) + XERCA) * 7;
                    CTR = 7;
                    do {
                        g10_1 = local5;
                        g11 = *(unsigned char*)(glyphs[(g0 + g29_1)] + (g8 - ((g8 >> 3) + XERCA) * 8) * 7 + g10_1);
                        g10_2 = g10_1 + 1;
                        local5 = g10_2;
                        *(__size8*)(g4 + g10_1 + 8) = (char) g11;
                    } while (flags);
                    g5_2 = g5_1 + 1;
                    local4 = g5_2;
                    *(__size8*)(g4 + 15) = 32;
                    flags = SUBFLAGSNS(g5_1 + 1, g31, CR6);
                    g4 += 8;
                } while (g5_1 + 1 < g31);
            }
            g9 = g31 * 8 - 1;
            local6 = g9;
            if (!ADDFLAGSX0(g31 * 8 - 1, g31 * 8, -1)) {
                do {
                    g9_1 = local6;
                    g9_2 = g9_1 - 1;
                    local6 = g9_2;
                    g5 = *(unsigned char*)(g1 + g9_1 - 112);
                    if (g5 == 32) {
                        *(__size8*)(g1 + g9_1 - 112) = 0;
                    }
                } while ((int)g5 >= 32);
            }
            g29_2 = g29_1 + 1;
            local3 = g29_2;
            puts(&local0);
        } while (g29_1 + 1 <= 6);
        g30 += 4;
        puts("");
        g26_2 = g26_1 - 1;
        local2 = g26_2;
    } while (ADDFLAGSX0(g26_1 - 1, g26_1 - 1, -1));
    return 0;
}

