int main(int argc, union { char *[] *; int; } argv);
__size32 __sputc(__size32 param1, union { __size32; __size32 *; } param2, unsigned char param3);

/** address: 0x00002838 */
int main(int argc, union { char *[] *; int; } argv)
{
    unsigned int CR0; 		// r64
    unsigned int CR1; 		// r65
    unsigned int CR2; 		// r66
    unsigned int CR3; 		// r67
    unsigned int CR4; 		// r68
    unsigned int CR5; 		// r69
    unsigned int CR6; 		// r70
    __size32 CR7; 		// r71
    int g0; 		// r0
    union { unsigned int; char; } g0_1; 		// r0{0}
    __size32 g1; 		// r1
    int g3; 		// r3
    __size32 g30; 		// r30
    __size32 g30_1; 		// r30{0}
    char * *g4; 		// r4
    union { __size32; unsigned char *; } g5; 		// r5
    int g9; 		// r9
    int g9_1; 		// r9{0}
    __size32 g9_2; 		// r9{0}
    __size32 g9_3; 		// r9{0}
    int local8; 		// m[g1 - 69]
    __size32 local9; 		// g30{0}

    g30 = g1 - 96;
    if (argc <= 1) {
        if (argc >= 0) {
            if (argc <= 0) {
                g0_1 = *(unsigned char*)g5;
                if ((int) g0_1 == 47) {
bb0x2aec:
                    g30_1 = g30;
                    *(__size32*)(g30 + 64) = 1;
                    local9 = g30_1;
                }
                else {
                    g3 = main(-61, (int) g0);
                    g3 = main(0, g3); /* Warning: also results in g30 */
                    local9 = g30;
                    if (g3 != 0) {
                        goto bb0x2aec;
                    }
                    else {
                    }
                }
            }
            else {
                g3 = main(2, 2); /* Warning: also results in g30 */
                local9 = g30;
                *(int*)(g30 + 64) = g3;
            }
        }
        else {
            if (argc >= -72) {
                if (argc >= -50) {
                    g3 = main((ROTL((CR0 * 0x10000000 + CR1 * 0x1000000 + CR2 * 0x100000 + CR3 * 0x10000 + CR4 * 0x1000 + CR5 * 256 + CR6 * 16 + CR7)) & 0x1) + argc, argv); /* Warning: also results in g30 */
                    local9 = g30;
                    *(int*)(g30 + 64) = g3;
                }
                else {
                    g0 = *(unsigned char*)g5;
                    if (argv != (int) g0) {
                        g3 = main(-65, argv); /* Warning: also results in g30 */
                        local9 = g30;
                        *(int*)(g30 + 64) = g3;
                    }
                    else {
                        g4 = *(/* machine specific */ (int) LR + 0x810);
                        g3 = __sputc(/* machine specific */ (int) LR, g4 + 88, local8); /* Warning: also results in g30 */
                        local9 = g30;
                        *(int*)(g30 + 64) = g3;
                    }
                }
            }
            else {
                g3 = main(argv, argc); /* Warning: also results in g30 */
                local9 = g30;
                *(int*)(g30 + 64) = g3;
            }
        }
    }
    else {
        if (argc <= 2) {
            main(-86, 0);
            main(-87, 1 - g0);
            g30 = main(-79, -13);
        }
        g0 = *(g30 + 120);
        g9_1 = *(g30 + 124);
        if (g0 < g9_1) {
            main(g9_2 + 1, g4);
        }
        g3 = main(-94, g9_3 - 27); /* Warning: also results in g30 */
        local9 = g30;
        local9 = g30;
        if (g3 == 0) {
bb0x2964:
            *(__size32*)(g30 + 64) = 16;
        }
        else {
            g0 = *(g30 + 120);
            if (g0 != 2) {
                goto bb0x2964;
            }
            else {
                g0 = *(g30 + 124);
                if (g0 > 12) {
                    *(__size32*)(g30 + 64) = 9;
                }
                else {
                    g3 = main(2, g9 + 1); /* Warning: also results in g30 */
                    local9 = g30;
                    *(int*)(g30 + 64) = g3;
                }
            }
        }
    }
    g30 = local9;
    g0 = *(g30 + 64);
    return g0;
}

/** address: 0x00002b10 */
__size32 __sputc(__size32 param1, union { __size32; __size32 *; } param2, unsigned char param3)
{
    int g0; 		// r0
    __size32 g1; 		// r1
    int g11; 		// r11
    int g3; 		// r3
    int g9; 		// r9
    int local0; 		// m[g1 - 32]

    g9 = *(param2 + 8);
    *(int*)(param2 + 8) = g9 - 1;
    if (g9 >= 1) {
bb0x2b74:
        g11 = *param2;
        g0 = (param3);
        *(__size8*)g11 = (char) g0;
        *(__size32*)param2 = g11 + 1;
        local0 = ROTL(g0) & 0xff;
    }
    else {
        g9 = *(param2 + 8);
        g0 = *(param2 + 24);
        if (g9 < g0) {
bb0x2b9c:
            g3 = __swbuf();
            local0 = g3;
        }
        else {
            if ((int) (param3) != 10) {
                goto bb0x2b74;
            }
            else {
                goto bb0x2b9c;
            }
        }
    }
    return param1; /* WARNING: Also returning: g3 := local0, g30 := (g1 - 96) */
}

