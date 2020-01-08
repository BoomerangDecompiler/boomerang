int main(int argc, char *argv[]);


/** address: 0x00001c84 */
int main(int argc, char *argv[])
{
    int LR; 		// r300
    int g3; 		// r3
    int g3_2; 		// r3{13}
    int g3_3; 		// r3{28}
    char * *g4; 		// r4
    int local0; 		// g3_2{13}
    char * *local1; 		// argv{14}
    int local2; 		// g3_3{28}
    char * *local3; 		// g4{29}

    local0 = argc;
    local1 = argv;
    LR = 0x1c9c;
    if (argc <= 2) {
        do {
            if (argc != 11) {
            }
        } while (argc <= 11);
    }
    else {
        do {
            argv = local1;
            local3 = argv;
            g3_2 = local0;
            local2 = g3_2;
            if (argc <= 2) {
                if (argc <= 3) {
bb0x1d1c:
                    printf("9");
bb0x1cf8:
                    LR = 0x1d0c;
                    g3 = printf("5"); /* Warning: also results in g4 */
                    local3 = g4;
                    local2 = g3;
                }
                else {
                    if (argc > 4) {
                        goto bb0x1cf8;
                    }
                }
            }
            else {
                goto bb0x1d1c;
            }
            g3_3 = local2;
            local0 = g3_3;
            g4 = local3;
            local1 = g4;
        } while (argc <= 5);
    }
    return 7;
}

