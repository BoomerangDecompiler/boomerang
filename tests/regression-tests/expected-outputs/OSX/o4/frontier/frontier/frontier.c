int main(int argc, char *argv[]);

/** address: 0x00001d7c */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int g3_1; 		// r3{0}
    int g3_4; 		// r3{0}
    int local0; 		// g3_1{0}
    int local1; 		// g3{0}

    local0 = argc;
    if (argc == 5) {
        do {
            g3_1 = local0;
            g3_4 = g3_1 - 1;
            local1 = g3_4;
            if (g3_1 <= 1) {
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
                    return 13;
                }
                goto bb0x1dc8;
            }
        } while (g3 > 0);
    }
    else {
        if ( ~(argc > 5 || argc != 2)) {
            do {
            } while (argc > 0);
        }
    }
    return 13;
}

