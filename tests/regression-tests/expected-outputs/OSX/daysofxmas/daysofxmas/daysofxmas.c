int main(union { int; char *[] *; } argc, union { int; char *[] *; } argv);
__size32 __sputc(void **param1, unsigned char param2);

__size32 global_0x00003060;// 4 bytes

/** address: 0x00002838 */
int main(union { int; char *[] *; } argc, union { int; char *[] *; } argv)
{
    int CR0; 		// r100
    int CR1; 		// r101
    int CR2; 		// r102
    int CR3; 		// r103
    int CR4; 		// r104
    int CR5; 		// r105
    int CR6; 		// r106
    __size32 CR7; 		// r107
    unsigned int g0; 		// r0
    unsigned int g0_1; 		// r0{38}
    int g3; 		// r3
    char * *g3_1; 		// r3
    unsigned char *g5; 		// r5
    int local0; 		// m[g1 - 32]
    int local1; 		// m[g1 - 69]

    if (argc <= 1) {
        if (argc >= 0) {
            if (argc <= 0) {
                g0_1 = *(unsigned char*)g5;
                if ((int) g0_1 == 47) {
bb0x2aec:
                    local0 = 1;
                }
                else {
                    g3_1 = main(-61, (int) g0);
                    g3 = main(0, g3_1);
                    if (g3 != 0) {
                        goto bb0x2aec;
                    }
                }
            }
            else {
                g3 = main(2, 2);
                local0 = g3;
            }
        }
        else {
            if (argc >= -72) {
                if (argc >= -50) {
                    g3 = main((ROTL(((CR0 << 28) + (CR1 << 24) + (CR2 << 20) + (CR3 << 16) + (CR4 << 12) + (CR5 << 8) + (CR6 << 4) + CR7), 31) & 0x1) + argc, argv);
                    local0 = g3;
                }
                else {
                    g0 = *(unsigned char*)g5;
                    if (argv != (int) g0) {
                        g3 = main(-65, argv);
                        local0 = g3;
                    }
                    else {
                        g3 = __sputc(global_0x00003060 + 88, local1);
                        local0 = g3;
                    }
                }
            }
            else {
                g3 = main(argv, argc);
                local0 = g3;
            }
        }
    }
    else {
        if (argc <= 2) {
            main(-86, 0);
            main(-87, 1 - argv);
            main(-79, -13);
        }
        if (argc < argv) {
            main(argc + 1, argv);
        }
        g3 = main(-94, argc - 27);
        if (g3 == 0 || argc != 2) {
            local0 = 16;
        }
        else {
            if (argv > 12) {
                local0 = 9;
            }
            else {
                g3 = main(2, argv + 1);
                local0 = g3;
            }
        }
    }
    return local0;
}

/** address: 0x00002b10 */
__size32 __sputc(void **param1, unsigned char param2)
{
    int g0; 		// r0
    __size8 *g11; 		// r11
    __size32 g3; 		// r3
    void **g4; 		// r4
    int g9; 		// r9
    __size32 local0; 		// m[g1 - 32]
    void **local2; 		// param1{17}

    local2 = param1;
    g9 = *(param1 + 8);
    *(int*)(param1 + 8) = g9 - 1;
    if (g9 >= 1) {
bb0x2b74:
        g11 = *param1;
        g0 = (param2);
        *(__size8*)g11 = (char) g0;
        *(void **)param1 = g11 + 1;
        local0 = g0 & 0xff;
    }
    else {
        g9 = *(param1 + 8);
        g0 = *(param1 + 24);
        if (g9 >= g0 && (int) (param2) != 10) {
            goto bb0x2b74;
        }
        else {
            g3 = __swbuf(); /* Warning: also results in g4, g9, g11 */
            local2 = g4;
            local0 = g3;
        }
    }
    param1 = local2;
    return local0;
}

