int main(int argc, union { __size32; char *[] *; } argv);

/** address: 0x10000418 */
int main(int argc, union { __size32; char *[] *; } argv)
{
    int g3; 		// r3
    int g3_2; 		// r3{0}
    int g3_3; 		// r3{0}
    int g4; 		// r4
    int local0; 		// g3_3{0}
    union { __size32; char *[] *; } local1; 		// argv{0}
    int local2; 		// g3_2{0}
    union { __size32; char *[] *; } local3; 		// g4{0}

    local0 = argc;
    local1 = argv;
    if (argc <= 2) {
        do {
            if (argc != 11) {
            }
        } while (argc <= 11);
    }
    else {
        do {
            g3_3 = local0;
            argv = local1;
            local2 = g3_3;
            local3 = argv;
            if (argc <= 2) {
                if (argc <= 3) {
bb0x100004a8:
                    printf("9");
bb0x10000484:
                    g3 = printf("5"); /* Warning: also results in g4 */
                    local2 = g3;
                    local3 = g4;
                }
                else {
                    if (argc > 4) {
                        goto bb0x10000484;
                    }
                    goto bb0x100004c8;
                }
            }
            else {
                goto bb0x100004a8;
            }
bb0x100004c8:
            g3_2 = local2;
            g4 = local3;
            local0 = g3_2;
            local1 = g4;
        } while (argc <= 5);
    }
    return 7;
}

