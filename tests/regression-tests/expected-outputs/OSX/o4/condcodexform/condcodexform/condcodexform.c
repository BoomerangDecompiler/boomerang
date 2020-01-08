int main(int argc, char *argv[]);

__size32 global_0x00002020;// 4 bytes
__size32 global_0x00002024;// 4 bytes
__size32 global_0x00002028;// 4 bytes
__size32 global_0x0000202c;// 4 bytes
__size32 global_0x00002030;// 4 bytes
__size32 global_0x00002034;// 4 bytes
__size32 global_0x00002038;// 4 bytes
__size32 global_0x0000203c;// 4 bytes

/** address: 0x00001c20 */
int main(int argc, char *argv[])
{
    int CR0; 		// r100
    int CR1; 		// r101
    int CR2; 		// r102
    int CR3; 		// r103
    int CR4; 		// r104
    int CR5; 		// r105
    int CR6; 		// r106
    __size32 CR7; 		// r107
    int g0; 		// r0
    int g0_1; 		// r0{24}
    __size32 g0_2; 		// r0{2}
    __size32 g0_3; 		// r0{26}
    __size32 g0_6; 		// r0{48}
    int g10; 		// r10
    int g11; 		// r11
    int g3; 		// r3
    int g8; 		// r8
    int g9; 		// r9
    __size32 local0; 		// g0_2{2}

    g0 = 1;
    if (argc <= 1) {
        g0 = 0;
    }
    g0_1 = g0;
    if (g0_1 == 0) {
        g0_6 = global_0x00002038;
        local0 = g0_6;
    }
    else {
        g0_3 = global_0x0000203c;
        local0 = g0_3;
    }
    g0_2 = local0;
    if (g0_1 == 0) {
        g11 = global_0x00002030;
    }
    else {
        g11 = global_0x00002034;
    }
    if (g0_1 == 0) {
        g10 = global_0x00002028;
    }
    else {
        g10 = global_0x0000202c;
    }
    if (g0_1 == 0) {
        g8 = global_0x00002020;
    }
    else {
        g8 = global_0x00002024;
    }
    if (g0_1 <= 0) {
        g3 = 0;
        if (g0_2 == global_0x00002038 && g11 == global_0x00002030 && g10 == global_0x00002028) {
            g9 = global_0x00002020;
bb0x1d34:
            g3 = 0;
            if (g8 == g9) {
                g3 = 1;
            }
        }
    }
    else {
        g3 = 0;
        if (g0_2 == global_0x0000203c && g11 == global_0x00002034 && g10 == global_0x0000202c) {
            g9 = global_0x00002024;
            goto bb0x1d34;
        }
    }
    if (g3 == 0) {
        g3 = "Failed!";
    }
    else {
        g3 = "Pass";
    }
    puts(g3);
    return ROTL(((CR0 << 28) + (CR1 << 24) + (CR2 << 20) + (CR3 << 16) + (CR4 << 12) + (CR5 << 8) + (CR6 << 4) + CR7), 19) & 0x1;
}

