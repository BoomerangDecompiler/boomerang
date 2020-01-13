int main(union { int; unsigned int *x2; char *[] *; } argc, union { int; unsigned int *x78; char *[] *; } argv);


/** address: 0x08048350 */
int main(union { int; unsigned int *x2; char *[] *; } argc, union { int; unsigned int *x78; char *[] *; } argv)
{
    unsigned int dl; 		// r10
    union { int; unsigned int *x51; char *[] *; } eax; 		// r24
    union { int; unsigned int *x368; char *[] *; } ebx; 		// r27
    char * *local14; 		// m[esp - 40]
    union { int; unsigned int *x3; char *[] *; } local15; 		// m[esp + 12]
    int local7; 		// m[esp - 44]

    if (argc <= 1) {
        if (argc < 0) {
            if (argc < -72) {
bb0x80483ba:
                eax = main(local7, local14);
            }
            else {
                if (argc >= -50) {
                    if (*local15 == 47) {
                        goto bb0x80483ba;
                    }
                    goto bb0x80483ba;
                }
                else {
                    eax = (int) *local15;
                    if (argv == eax) {
                        eax = (int) *(local15 + 31);
                        eax = putchar(eax);
                    }
                    else {
                        goto bb0x80483ba;
                    }
                }
            }
            ebx = eax;
        }
        else {
            if (argc <= 0) {
                dl = *local15;
                if (dl == 47) {
bb0x8048463:
                    ebx = 1;
                }
                else {
                    eax = main(-61, (int) dl);
                    eax = main(0, eax);
                    if (eax != 0) {
                        goto bb0x8048463;
                    }
                }
            }
            else {
                goto bb0x80483ba;
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
        eax = main(-94, argc - 27);
        if (eax != 0 && argc == 2) {
            if (argv <= 12) {
                goto bb0x80483ba;
            }
            else {
                ebx = 9;
            }
        }
        else {
            ebx = 16;
        }
    }
    return ebx;
}

