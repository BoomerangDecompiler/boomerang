int main(int argc, char *argv[]);

union { unsigned int; void *[]; } glyphs;

/** address: 0x00002894 */
int main(int argc, char *argv[])
{
    __size32 CTR; 		// r301
    int XERCA; 		// r202
    union { int; void *; } g1; 		// r1
    int g10; 		// r10
    int g11; 		// r11
    union { int; void *; } g2; 		// r2
    __size32 g26; 		// r26
    __size32 g26_1; 		// r26{13}
    __size32 g26_2; 		// r26{62}
    union { int; char *; } *g28; 		// r28
    unsigned int g29; 		// r29
    unsigned int g29_1; 		// r29{23}
    unsigned int g29_2; 		// r29{57}
    int g3; 		// r3
    union { int; char *; } g30; 		// r30
    union { int; char *; } *g3_1; 		// r3
    int g4; 		// r4
    int g7; 		// r7
    union { unsigned int; char *; } g7_1; 		// r7{28}
    union { unsigned int; char *; } g7_2; 		// r7{43}
    union { int; void *; } g7_4; 		// r7{51}
    union { int; void *; } g7_5; 		// r7{55}
    unsigned int g9_2; 		// r9{29}
    unsigned char *g9_4; 		// r9{35}
    unsigned char *g9_7; 		// r9{36}
    unsigned char *g9_8; 		// r9{39}
    char local0; 		// m[g1 - 112]
    __size32 local1; 		// g26_1{13}
    unsigned int local2; 		// g29_1{23}
    union { unsigned int; char *; } local3; 		// g7_1{28}
    unsigned char *local4; 		// g9_7{36}
    union { int; void *; } local5; 		// g7_4{51}

    g26 = 1;
    local1 = g26;
    g3_1 = malloc(12);
    g2 = 0x28a4;
    g28 = g3_1;
    *(__size32*)(g3_1 + 4) = 0x3ff4;
    do {
        g26_1 = local1;
        g3 = *g28;
        g3 = strlen(g3);
        g30 = g3;
        if (g3 > 10) {
            g30 = 10;
        }
        g29 = 0;
        local2 = g29;
        do {
            g29_1 = local2;
            g7 = 0;
            local3 = g7;
            if (0 < g30) {
                g3 = *g28;
                do {
                    g7_1 = local3;
                    g9_2 = *(unsigned char*)(g3 + g7_1);
                    g11 = (int) g9_2 - 32;
                    if (!ADDFLAGSX0((int) g9_2 - 32, (int) g9_2, -32)) {
                        g11 = 0;
                    }
                    g10 = 0;
                    CTR = 7;
                    g9_4 = (g11 - ((g11 >> 3) + XERCA) * 8) * 7 + glyphs[(((g11 >> 3) + XERCA) * 7 + g29_1)];
                    local4 = g9_4;
                    do {
                        g9_7 = local4;
                        g11 = *(unsigned char*)g9_7;
                        g9_8 = g9_7 + 1;
                        local4 = g9_8;
                        *(__size8*)(g7_1 * 8 + g1 + g10 - 112) = (char) g11;
                        g10++;
                    } while (ADDFLAGSX0((int) g9_2 - 32, (int) g9_2, -32));
                    g7_2 = g7_1 + 1;
                    local3 = g7_2;
                    g2 = g7_1 * 8 + g1 - 112;
                    *(__size8*)(g2 + 7) = 32;
                } while (g7_1 + 1 < g30);
            }
            g7 = g30 * 8 - 1;
            local5 = g7;
            if (!ADDFLAGSX0(g30 * 8 - 1, g30 * 8, -1)) {
                do {
                    g7_4 = local5;
                    g4 = *(unsigned char*)(g1 + g7_4 - 112);
                    if (g4 == 32) {
                        *(__size8*)(g1 + g7_4 - 112) = 0;
                        g7_5 = g7_4 - 1;
                        local5 = g7_5;
                    }
                } while (ADDFLAGSX0(g7_4 - 1, g7_4 - 1, -1));
            }
            g29_2 = g29_1 + 1;
            local2 = g29_2;
            puts(&local0);
        } while (g29_1 + 1 <= 6);
        g28++;
        puts("");
        g26_2 = g26_1 - 1;
        local1 = g26_2;
    } while (ADDFLAGSX0(g26_1 - 1, g26_1 - 1, -1));
    return 0;
}

