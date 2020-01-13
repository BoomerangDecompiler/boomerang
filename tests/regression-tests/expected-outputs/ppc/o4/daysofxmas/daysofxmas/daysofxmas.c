int main(union { int; char *[] *; } argc, union { int; char *[] *; } argv);


/** address: 0x10000444 */
int main(union { int; char *[] *; } argc, union { int; char *[] *; } argv)
{
    union { int; char *[] *; } g3; 		// r3
    union { int; char *[] *; } g31; 		// r31
    union { int; char *[] *; } g3_1; 		// r3{0}
    union { int; char *[] *; } g3_2; 		// r3{12}
    int g4; 		// r4
    union { int; char *[] *; } g4_1; 		// r4{0}
    int g5; 		// r5
    unsigned int g5_1; 		// r5

    if (argc <= 1) {
        if (argc < 0) {
            if (argc < -72) {
bb0x10000564:
                g3 = main(g3_1, g4_1);
            }
            else {
                if (argc >= -50) {
                    goto bb0x10000564;
                }
                else {
                    g5_1 = *(unsigned char*)g5;
                    if (argv == g5_1) {
                        g3 = _IO_putc();
                    }
                    else {
                        goto bb0x10000564;
                    }
                }
            }
        }
        else {
            if (argc > 0) {
                goto bb0x10000564;
            }
            else {
                g4 = *(unsigned char*)g5;
                if (g4 == 47) {
bb0x10000518:
                    g3 = 1;
                }
                else {
                    g3 = main(-61, g4);
                    g3 = main(0, g3);
                    if (g3 == 0) {
bb0x100004a4:
                        g3 = g31;
                    }
                    else {
                        goto bb0x10000518;
                    }
                }
            }
        }
    }
    else {
        if (argc <= 2) {
            main(-86, 0);
            main(-87, 1 - argv);
            main(-79, -13);
            if (argc < argv) {
bb0x100005e8:
                main(argc + 1, argv);
            }
        }
        else {
            if (argc < argv) {
                goto bb0x100005e8;
            }
        }
        g3_2 = main(-94, argc - 27);
        if (g3_2 != 0) {
            if (g3_2 != 0) {
bb0x100004a0:
                g31 = 16;
                goto bb0x100004a4;
            }
            else {
                g31 = 9;
                if (argv > 12) {
                    goto bb0x100004a4;
                }
                else {
                    goto bb0x10000564;
                }
            }
        }
        else {
            goto bb0x100004a0;
        }
    }
    return g3;
}

