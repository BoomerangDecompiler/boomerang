int main(int argc, char *argv[]);

/** address: 0x08048350 */
int main(int argc, char *argv[])
{
    union { unsigned int; char; } dl; 		// r10
    int eax; 		// r24
    int ebx; 		// r27
    union { union { __size8; char; unsigned int; } *; int; } local14; 		// m[esp + 12]
    int local7; 		// m[esp - 40]
    int local8; 		// m[esp - 44]

    if (argc <= 1) {
        if (argc < 0) {
            if (argc < -72) {
bb0x80483ba:
                eax = main(local8, local7);
            }
            else {
                if (argc >= -50) {
                    if (*local14 == 47) {
                    }
                    goto bb0x80483ba;
                }
                else {
                    eax = (int) *local14;
                    if (argv == eax) {
                        eax = (int) *(local14 + 31);
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
                dl = *local14;
                ebx = 0;
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

