int main(int argc, union { __size32; char *[] *; } argv);

/** address: 0x00001c84 */
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
bb0x1d6c:
            }
            else {
                goto bb0x1d80;
            }
            goto bb0x1d6c;
        } while (argc <= 11);
bb0x1d80:
    }
    else {
        do {
            g3_3 = local0;
            argv = local1;
            local2 = g3_3;
            local3 = argv;
            if (argc <= 2) {
                if (argc <= 3) {
bb0x1d1c:
                    printf(/* machine specific */ (int) LR + 864);
bb0x1cf8:
                    g3 = printf(/* machine specific */ (int) LR + 860); /* Warning: also results in g4 */
                    local2 = g3;
                    local3 = g4;
                }
                else {
                    if (argc > 4) {
                        goto bb0x1cf8;
                    }
                    goto bb0x1d3c;
                }
            }
            else {
                goto bb0x1d1c;
            }
bb0x1d3c:
            g3_2 = local2;
            g4 = local3;
            local0 = g3_2;
            local1 = g4;
        } while (argc <= 5);
    }
    return 7;
}

