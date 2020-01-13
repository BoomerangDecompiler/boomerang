int main(int argc, char *argv[]);


/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    int CR0; 		// r100
    int CR1; 		// r101
    int CR2; 		// r102
    int CR3; 		// r103
    int CR4; 		// r104
    int CR5; 		// r105
    int CR6; 		// r106
    int CR6_1; 		// r106{2}
    __size32 CR7; 		// r107
    __size32 LR; 		// r300
    int g3; 		// r3
    __size32 g31; 		// r31
    int g3_1; 		// r3{63}
    int g3_2; 		// r3{27}
    int g3_3; 		// r3{14}
    __size32 g4; 		// r4
    __size32 g5; 		// r5
    int local0; 		// g3{3}
    int local1; 		// CR6{25}
    int local2; 		// g3_2{27}
    int local3; 		// g3_1{63}
    int local4; 		// argc{68}

    local4 = argc;
    local3 = argc;
    g4 = (CR0 << 28) + (CR1 << 24) + (CR2 << 20) + (CR3 << 16) + (CR4 << 12) + (CR5 << 8) + (CR6 << 4) + CR7;
    g5 = LR;
    if (argc > 11) {
        g31 = (CR0 << 28) + (CR1 << 24) + (CR2 << 20) + (CR3 << 16) + (CR4 << 12) + (CR5 << 8) + (CR6 << 4) + CR7;
        do {
            g3_1 = local3;
            local2 = g3_1;
            local0 = g3_1;
            if (argc <= 3 && argc > 3) {
                CR6 = ROTL(g31, 28) >> 24 & 0xf;
                local1 = CR6;
                g31 = ROTL(ROTL(g31, 28), 4);
                if (argc > 3) {
bb0x10000464:
                    g3 = local0;
                    CR6_1 = CR6;
                    local1 = CR6_1;
                    LR = 0x10000468;
                    g3_3 = putchar('5'); /* Warning: also results in g4, g5 */
                    local2 = g3_3;
                }
            }
            else {
                g3 = putchar('9'); /* Warning: also results in g4, g5 */
                local0 = g3;
                goto bb0x10000464;
            }
            g3_2 = local2;
            local4 = g3_2;
            local3 = g3_2;
            CR6 = local1;
        } while (argc <= 3);
    }
    argc = local4;
    return 7;
}

