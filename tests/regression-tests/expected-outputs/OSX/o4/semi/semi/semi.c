int main(int argc, char *argv[]);


/** address: 0x00001d28 */
int main(int argc, char *argv[])
{
    int CR0; 		// r100
    int CR1; 		// r101
    int CR2; 		// r102
    int CR3; 		// r103
    int CR4; 		// r104
    int CR5; 		// r105
    int CR6; 		// r106
    __size32 CR7_1; 		// r107{47}
    __size32 CR7_4; 		// r107{11}
    __size32 CR7_6; 		// r107{24}
    __size32 CR7_7; 		// r107{49}
    __size32 CR7_8; 		// r107{36}
    __size32 LR; 		// r300
    int g3; 		// r3
    int g3_1; 		// r3{1}
    int g3_2; 		// r3{14}
    int g3_3; 		// r3{61}
    int g3_6; 		// r3{75}
    __size32 g4; 		// r4
    int local0; 		// g3_1{1}
    __size32 local1; 		// CR7_4{11}
    int local2; 		// g3_2{14}
    __size32 local3; 		// CR7_6{24}
    __size32 local4; 		// CR7_7{49}
    int local5; 		// g3{50}
    int local6; 		// argc{62}
    __size32 local7; 		// CR7_8{72}

    local6 = argc;
    local0 = argc;
    local7 = CR7_8;
    local1 = CR7_8;
    g4 = (CR0 << 28) + (CR1 << 24) + (CR2 << 20) + (CR3 << 16) + (CR4 << 12) + (CR5 << 8) + (CR6 << 4) + CR7_8;
    if (argc > 2) {
        do {
            CR7_4 = local1;
            local4 = CR7_4;
            g3_1 = local0;
            local5 = g3_1;
            local2 = g3_1;
            if (argc > 3 || argc <= 3) {
                g3_6 = putchar('9'); /* Warning: also results in g4 */
                local5 = g3_6;
bb0x1d68:
                CR7_7 = local4;
                local3 = CR7_7;
                g3 = local5;
                LR = 0x1d70;
                g3_3 = putchar('5'); /* Warning: also results in g4 */
                local2 = g3_3;
            }
            else {
                CR7_1 = (CR0 << 28) + (CR1 << 24) + (CR2 << 20) + (CR3 << 16) + (CR4 << 12) + (CR5 << 8) + (CR6 << 4) + CR7_8 >> 28 & 0xf;
                local4 = CR7_1;
                local3 = CR7_1;
                if (argc > 3) {
                    goto bb0x1d68;
                }
            }
            g3_2 = local2;
            local6 = g3_2;
            local0 = g3_2;
            CR7_6 = local3;
            local7 = CR7_6;
            local1 = CR7_6;
        } while (argc <= 3);
    }
    CR7_8 = local7;
    argc = local6;
    return 7;
}

