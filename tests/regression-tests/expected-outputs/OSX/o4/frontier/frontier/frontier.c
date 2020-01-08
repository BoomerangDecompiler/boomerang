int main(int argc, char *argv[]);


/** address: 0x00001d7c */
int main(int argc, char *argv[])
{
    __size32 g3; 		// r3
    int g3_1; 		// r3{2}
    int g3_2; 		// r3{6}
    int local0; 		// g3_1{5}
    int local1; 		// g3{10}

    local0 = argc;
    if (argc == 5) {
        do {
            g3_1 = local0;
            g3_2 = g3_1 - 1;
            local1 = g3_2;
            if (g3_1 <= 1) {
bb0x1dc0:
                if (g3_1 - 1 == 12) {
                    break;
                }
bb0x1dc8:
                g3 = local1;
                local0 = g3;
            }
            else {
                g3 = g3_1 - 2;
                local1 = g3;
                if (g3_1 - 1 > 2) {
                    break;
                }
                goto bb0x1dc8;
            }
            goto bb0x1dc0;
        } while (g3 > 0);
    }
    else {
        if (argc > 5 || argc != 2) {
        }
    }
    return 13;
}

