int global21;

// address: 0x106a8
int main(int argc, char *argv[], union { char *[] * x9; char * x10; } envp) {
    int i0; 		// r24
    union { int x13; void * x14; } o0; 		// r8
    char * *o0_1; 		// r8
    __size8 *o0_2; 		// r8
    union { int x15; void * x16; } o1; 		// r9
    union { char *[] * x9; char * x10; } o2; 		// r10

    if (argc <= 1) {
        if (argc < 0) {
            if (argc < -72) {
L2:
                o0 = main(o0_1, o1, o2);
                i0 = o0;
            } else {
                o0 = (int) *envp;
                if (argc >= -50) {
                    goto L2;
                } else {
                    if (argv == o0) {
                        global21 = global21 - 1;
                        if (global21 - 1 < 0) {
                            __flsbuf();
                            i0 = o0;
                        } else {
                            o1 = *(unsigned char*)(envp + 31);
                            o0_2 = *0x20cf4;
                            i0 = o1;
                            *(__size8*)o0_2 = (char) o1;
                            *(void **)0x20cf4 = o0_2 + 1;
                        }
                    } else {
                        goto L2;
                    }
                }
            }
        } else {
            if (argc <= 0) {
                o1 = (int) *envp;
                i0 = 0;
                if (o1 == 47) {
L15:
                    i0 = 1;
                } else {
                    o0_1 = main(-61, o1, 0x10948);
                    o0 = main(0, o0_1, envp + 1);
                    if (o0 != 0) {
                        goto L15;
                    }
                }
            } else {
                goto L2;
            }
        }
    } else {
        if (argc <= 2) {
            o0 = main(-86, 0, envp + 1);
            o0 = main(-87, 1 - argv, envp + o0);
            main(-79, -13, envp + o0);
        }
        if (argc < argv) {
            main(argc + 1, argv, envp);
        }
        o0 = main(-94, argc - 27, envp);
        i0 = 16;
        if (argc == 2) {
            if (argv <= 12) {
                goto L2;
            } else {
                i0 = 9;
            }
        }
    }
    return i0;
}

