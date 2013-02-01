__size32 __sputc(void **param1, unsigned char param2);

// address: 2838
int main(int argc, char *argv[], union { char *[] * x41; unsigned char * x42; } envp) {
    unsigned int CR0; 		// r64
    unsigned int CR1; 		// r65
    unsigned int CR2; 		// r66
    unsigned int CR3; 		// r67
    unsigned int CR4; 		// r68
    unsigned int CR5; 		// r69
    unsigned int CR6; 		// r70
    __size32 CR7; 		// r71
    int g0; 		// r0
    __size32 g0_1; 		// r0{90}
    union { unsigned int x319; char x320; char x306; char x292; char x278; char x264; char x250; char x236; char x222; char x208; char x194; char x180; char x166; char x152; char x138; char x124; char x110; char x96; char x82; char x68; char x52; } g0_2; 		// r0{239}
    void *g1; 		// r1
    int g3; 		// r3
    union { void * x326; int x327; } g30; 		// r30
    __size32 g31; 		// r31
    char * *g3_1; 		// r3
    char * *g4; 		// r4
    union { char *[] * x41; unsigned char * x42; } g5; 		// r5
    char * *g5_1; 		// r5{118}
    int g9; 		// r9
    void *g9_1; 		// r9
    int g9_2; 		// r9{111}
    int local8; 		// m[g1 - 69]

    g30 = g1 - 96;
    if (argc <= 1) {
        if (argc >= 0) {
            if (argc <= 0) {
                g0_2 = *(unsigned char*)envp;
                if ((int) g0_2 == 47) {
L1:
                    *(__size32*)(g30 + 64) = 1;
                } else {
                    g3_1 = main(-61, (int) g0, /* machine specific */ (int) LR + 1904);
                    g3 = main(0, g3_1, g9 + 1); /* Warning: also results in g30 */
                    if (g3 != 0) {
                        goto L1;
                    }
                }
            } else {
                g3 = main(2, 2, /* machine specific */ (int) LR + 1900); /* Warning: also results in g30 */
                *(int*)(g30 + 64) = g3;
            }
        } else {
            if (argc >= -72) {
                if (argc >= -50) {
                    g3 = main((ROTL((CR0 * 0x10000000 + CR1 * 0x1000000 + CR2 * 0x100000 + CR3 * 0x10000 + CR4 * 0x1000 + CR5 * 256 + CR6 * 16 + CR7)) & 0x1) + argc, argv, envp + 1); /* Warning: also results in g30 */
                    *(int*)(g30 + 64) = g3;
                } else {
                    g0 = *(unsigned char*)envp;
                    if (argv != (int) g0) {
                        g3 = main(-65, argv, envp + 1); /* Warning: also results in g30 */
                        *(int*)(g30 + 64) = g3;
                    } else {
                        g4 = *(/* machine specific */ (int) LR + 0x810);
                        g3 = __sputc(g4 + 88, local8); /* Warning: also results in g30 */
                        *(int*)(g30 + 64) = g3;
                    }
                }
            } else {
                g3 = main(argv, argc, /* machine specific */ (int) LR + 1488); /* Warning: also results in g30 */
                *(int*)(g30 + 64) = g3;
            }
        }
    } else {
        if (argc <= 2) {
            g3 = main(-86, 0, envp + 1);
            g3 = main(-87, 1 - g0, g3 + g0_1);
            g30 = main(-79, -13, g3 + g0);
        }
        g0 = *(g30 + 120);
        g9_2 = *(g30 + 124);
        if (g0 < g9_2) {
            main(g9 + 1, g4, g5_1);
        }
        g3 = main(-94, g9_1 - 27, g5); /* Warning: also results in g30, g31 */
        if (g3 == 0) {
L21:
            *(__size32*)(g30 + 64) = 16;
        } else {
            g0 = *(g30 + 120);
            if (g0 != 2) {
                goto L21;
            } else {
                g0 = *(g30 + 124);
                if (g0 > 12) {
                    *(__size32*)(g30 + 64) = 9;
                } else {
                    g3 = main(2, g9 + 1, g31 + 1476); /* Warning: also results in g30 */
                    *(int*)(g30 + 64) = g3;
                }
            }
        }
    }
    g0 = *(g30 + 64);
    return g0;
}

// address: 2b10
__size32 __sputc(void **param1, unsigned char param2) {
    int g0; 		// r0
    void *g1; 		// r1
    void *g11; 		// r11
    int g3; 		// r3
    __size32 g30; 		// r30
    int g9; 		// r9
    int g9_1; 		// r9{74}
    int local0; 		// m[g1 - 32]

    g9_1 = *(param1 + 8);
    *(int*)(param1 + 8) = g9_1 - 1;
    if (g9_1 - 1 >= 0) {
L1:
        g11 = *param1;
        g0 = (param2);
        *(__size8*)g11 = (char) g0;
        *(void **)param1 = g11 + 1;
        local0 = ROTL(g0) & 0xff;
    } else {
        g9 = *(param1 + 8);
        g0 = *(param1 + 24);
        if (g9 < g0) {
L3:
            __swbuf();
            local0 = g3;
        } else {
            g0 = (param2);
            if ((int) g0 != 10) {
                goto L1;
            } else {
                goto L3;
            }
        }
    }
    return g30; /* WARNING: Also returning: g3 := local0, g30 := (g1 - 96) */
}

