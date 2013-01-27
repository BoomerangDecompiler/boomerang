// address: 0x10000418
int main(int argc, char *argv[], union { char *[] * x5; unsigned char * x6; } envp) {
    unsigned int CR0; 		// r64
    unsigned int CR1; 		// r65
    unsigned int CR2; 		// r66
    unsigned int CR3; 		// r67
    unsigned int CR4; 		// r68
    unsigned int CR5; 		// r69
    unsigned int CR6; 		// r70
    __size32 CR7; 		// r71
    unsigned int g0; 		// r0
    unsigned int g0_1; 		// r0{184}
    int g3; 		// r3
    char * *g3_1; 		// r3
    int local6; 		// m[g1 - 28]

    if (argc <= 1) {
        if (argc >= 0) {
            if (argc <= 0) {
                local6 = 0;
                g0_1 = *(unsigned char*)envp;
                if ((ROTL(g0_1) & 0xff) == 47) {
L1:
                    local6 = 1;
                } else {
                    g3_1 = main(-61, ROTL(g0) & 0xff, 0x10000c5c);
                    g3 = main(0, g3_1, envp + 1);
                    if (g3 != 0) {
                        goto L1;
                    }
                }
            } else {
                g3 = main(2, 2, 0x10000c58);
                local6 = g3;
            }
        } else {
            if (argc >= -72) {
                if (argc >= -50) {
                    g3 = main((ROTL((CR0 * 0x10000000 + CR1 * 0x1000000 + CR2 * 0x100000 + CR3 * 0x10000 + CR4 * 0x1000 + CR5 * 256 + CR6 * 16 + CR7)) & 0x1) + argc, argv, envp + 1);
                    local6 = g3;
                } else {
                    g0 = *(unsigned char*)envp;
                    if (argv != (ROTL(g0) & 0xff)) {
                        g3 = main(-65, argv, envp + 1);
                        local6 = g3;
                    } else {
                        putchar();
                        local6 = g3;
                    }
                }
            } else {
                g3 = main(argv, argc, 0x10000abc);
                local6 = g3;
            }
        }
    } else {
        if (argc <= 2) {
            g3 = main(-86, 0, envp + 1);
            g3 = main(-87, 1 - argv, g3 + envp);
            main(-79, -13, g3 + envp);
        }
        if (argc < argv) {
            main(argc + 1, argv, envp);
        }
        g3 = main(-94, argc - 27, envp);
        if (g3 == 0 || argc != 2) {
            local6 = 16;
        } else {
            if (argv > 12) {
                local6 = 9;
            } else {
                g3 = main(2, argv + 1, 0x10000ab0);
                local6 = g3;
            }
        }
    }
    return local6;
}

