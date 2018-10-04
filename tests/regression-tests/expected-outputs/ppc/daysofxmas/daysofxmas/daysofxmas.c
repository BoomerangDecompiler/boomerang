int main(int argc, union { char *[] *; int; } argv);

/** address: 0x10000418 */
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
    unsigned int g0; 		// r0
    unsigned int g0_1; 		// r0{0}
    int g3; 		// r3
    int g5; 		// r5
    int local6; 		// m[g1 - 28]

    if (argc <= 1) {
        if (argc >= 0) {
            if (argc <= 0) {
                local6 = 0;
                g0_1 = *(unsigned char*)g5;
                if ((ROTL(g0_1) & 0xff) == 47) {
bb0x100006e8:
                    local6 = 1;
                }
                else {
                    g3 = main(-61, ROTL(g0) & 0xff);
                    g3 = main(0, g3);
                    if (g3 != 0) {
                        goto bb0x100006e8;
                    }
                }
            }
            else {
                g3 = main(2, 2);
                local6 = g3;
            }
        }
        else {
            if (argc >= -72) {
                if (argc >= -50) {
                    g3 = main((ROTL((CR0 * 0x10000000 + CR1 * 0x1000000 + CR2 * 0x100000 + CR3 * 0x10000 + CR4 * 0x1000 + CR5 * 256 + CR6 * 16 + CR7)) & 0x1) + argc, argv);
                    local6 = g3;
                }
                else {
                    g0 = *(unsigned char*)g5;
                    if (argv != (ROTL(g0) & 0xff)) {
                        g3 = main(-65, argv);
                        local6 = g3;
                    }
                    else {
                        g3 = putchar();
                        local6 = g3;
                    }
                }
            }
            else {
                g3 = main(argv, argc);
                local6 = g3;
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
            local6 = 16;
        }
        else {
            if (argv > 12) {
                local6 = 9;
            }
            else {
                g3 = main(2, argv + 1);
                local6 = g3;
            }
        }
    }
    return local6;
}

