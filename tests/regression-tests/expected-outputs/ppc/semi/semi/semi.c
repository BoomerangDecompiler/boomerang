int main(int argc, union { __size32; char *[] *; } argv);

/** address: 0x10000418 */
int main(int argc, union { __size32; char *[] *; } argv)
{
    int g3; 		// r3
    int g3_1; 		// r3{0}
    int g3_2; 		// r3{0}
    int g4; 		// r4
    int local0; 		// g3_2{0}
    union { __size32; char *[] *; } local1; 		// argv{0}
    int local2; 		// g3_1{0}
    union { __size32; char *[] *; } local3; 		// g4{0}

    local0 = argc;
    local1 = argv;
    if (argc <= 2) {
        do {
bb0x100004e0:
            if (argc != 11) {
            }
            else {
bb0x100004f4:
                goto bb0x100004f4;
            }
            goto bb0x100004e0;
        } while (argc <= 11);
    }
    else {
        do {
            g3_2 = local0;
            argv = local1;
            local2 = g3_2;
            local3 = argv;
            if (argc <= 2) {
                if (argc <= 3) {
                    printf(0x100008e0);
                    g3 = printf(0x100008dc); /* Warning: also results in g4 */
                    local2 = g3;
                    local3 = g4;
                }
                else {
bb0x10000470:
                    if (argc > 4) {
                        goto bb0x10000470;
                    }
bb0x1000049c:
                    goto bb0x1000049c;
                }
            }
            else {
bb0x10000458:
                goto bb0x10000458;
            }
            g3_1 = local2;
            g4 = local3;
            local0 = g3_1;
            local1 = g4;
        } while (argc <= 5);
    }
    return 7;
}

