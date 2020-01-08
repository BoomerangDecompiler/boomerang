int main(int argc, char *argv[]);


/** address: 0x100003f0 */
int main(int argc, char *argv[])
{
    int g3_1; 		// r3{7}
    int g3_2; 		// r3{11}
    int g3_5; 		// r3{13}
    int local0; 		// argc{2}
    int local1; 		// argc{3}
    int local2; 		// g3_1{10}

    local2 = argc;
    local1 = argc;
    local0 = argc;
    if (argc == 5) {
        do {
bb0x10000414:
            g3_1 = local2;
            g3_2 = g3_1 - 1;
            local2 = g3_2;
            local0 = g3_2;
            if (g3_1 - 1 <= 12) {
bb0x1000040c:
            }
            else {
                g3_5 = g3_1 - 2;
                local2 = g3_5;
                local1 = g3_5;
                local0 = g3_5;
                if (g3_1 > 2) {
                    goto bb0x10000404;
                }
                else {
                    if (g3_1 > 2) {
                        goto bb0x10000414;
                    }
                    argc = local1;
                    return 13;
                }
                argc = local1;
                return 13;
            }
            goto bb0x1000040c;
        } while (g3_1 - 1 > 12);
bb0x10000404:
        argc = local0;
        local1 = argc;
    }
    else {
        if (argc > 5 || argc != 2) {
            goto bb0x10000404;
        }
    }
    argc = local1;
    return 13;
}

