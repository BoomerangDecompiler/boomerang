int main(union { int; unsigned char *x82; char *[] *; } argc, union { int; unsigned char *x68; char *[] *; } argv);

__size32 global_0x00002060;// 4 bytes

/** address: 0x000019b8 */
int main(union { int; unsigned char *x82; char *[] *; } argc, union { int; unsigned char *x68; char *[] *; } argv)
{
    int g0; 		// r0
    int g12; 		// r12
    unsigned int g2; 		// r2
    union { int; unsigned char *x2; char *[] *; } g29; 		// r29
    int g3; 		// r3
    int g30; 		// r30
    char * *g3_1; 		// r3
    union { int; unsigned char *x76; char *[] *; } g3_2; 		// r3{0}
    int g4; 		// r4
    union { int; unsigned char *x81; char *[] *; } g4_1; 		// r4{0}
    union { unsigned int; unsigned char *; char *[] *x74; } g5_1; 		// r5{29}
    union { unsigned int; unsigned char *; char *[] *x24; } g5_4; 		// r5{12}
    int g8; 		// r8

    if (argc <= 1) {
        if (argc >= 0) {
            if (argc <= 0) {
                g2 = *(unsigned char*)g5_4;
                if ((int) g2 == 47) {
bb0x1ba4:
                    g30 = 1;
                }
                else {
                    g3_1 = main(-61, (int) g2);
                    g3 = main(0, g3_1);
                    if (g3 != 0) {
                        goto bb0x1ba4;
                    }
                }
            }
            else {
bb0x1b5c:
                g3 = main(g3_2, g4_1);
                g30 = g3;
            }
        }
        else {
            if (argc >= -72) {
                if (argc >= -50) {
                    goto bb0x1b5c;
                }
                else {
                    g5_1 = *(unsigned char*)g5_4;
                    if (argv != (int) g5_1) {
                        goto bb0x1b5c;
                    }
                    else {
                        g4 = *(unsigned char*)(g5_4 + 31);
                        g8 = *(global_0x00002060 + 96);
                        *(int*)(global_0x00002060 + 96) = g8 - 1;
                        if (g8 >= 1) {
bb0x1afc:
                            g29 = *(global_0x00002060 + 88);
                            g0 = (int) g4 & 0xff;
                            *(unsigned char*)g29 = (char) (int) g4;
                            *(union { int; unsigned char *x2; char *[] *; }*)(global_0x00002060 + 88) = g29 + 1;
                        }
                        else {
                            g12 = *(global_0x00002060 + 112);
                            if (g8 - 1 < g12 || (int) g4 == 10) {
                                g3 = __swbuf();
                                g0 = g3;
                            }
                            else {
                                goto bb0x1afc;
                            }
                        }
                        g30 = g0;
                    }
                }
            }
            else {
                goto bb0x1b5c;
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
        main(-94, argc - 27);
        if (true) {
            g30 = 16;
        }
        else {
            if (argv > 12) {
                g30 = 9;
            }
            else {
                goto bb0x1b5c;
            }
        }
    }
    return g30;
}

