int main(int argc, char *argv[]);


/** address: 0x10000434 */
int main(int argc, char *argv[])
{
    int CR0; 		// r100
    int CR1; 		// r101
    int CR2; 		// r102
    int CR3; 		// r103
    int CR4; 		// r104
    int CR5; 		// r105
    int CR6; 		// r106
    int CR7; 		// r107
    __size32 g0; 		// r0
    __size32 g0_1; 		// r0{1}
    __size32 g0_2; 		// r0{27}
    int g10; 		// r10
    int g11; 		// r11
    int g8; 		// r8
    int g9; 		// r9
    __size32 local0; 		// g0{36}

    if (argc <= 1) {
        flags = SUBFLAGSNS(0, 0, CR7);
        g0 = 0x10000418;
        g11 = 0x10000420;
    }
    else {
        flags = SUBFLAGSNS(1, 0, CR7);
        g0 = 0x10000414;
        g11 = 0x1000041c;
    }
    g0_1 = g0;
    if (flags) {
        g10 = 0x10000424;
        if (flags) {
bb0x10000478:
            g8 = 0x10000430;
            if (flags) {
bb0x100004e0:
                g0 = 0;
                local0 = g0;
                local0 = g0;
                if (g11 == 0x1000041c) {
                    g9 = 0x1000042c;
                    if (g10 == 0x10000424) {
bb0x10000584:
                        g0 = 0;
                        local0 = g0;
                        if (g8 == g9) {
                            g0 = 1;
                            local0 = g0;
                        }
                    }
                }
bb0x10000508:
                g0 = local0;
                if (g0 == 0) {
bb0x1000049c:
                    puts("Failed!");
                }
                else {
                    puts("Pass");
                }
            }
            else {
bb0x10000484:
                g0_2 = 0;
                local0 = g0_2;
                if (g0_1 == 0x10000418) {
                    if (g11 == 0x10000420 && g10 == 0x10000428) {
                        g9 = 0x10000430;
                        goto bb0x10000584;
                    }
                    goto bb0x10000508;
                }
                else {
                    goto bb0x1000049c;
                }
            }
        }
        else {
bb0x100004d4:
            g8 = 0x1000042c;
            if (flags) {
                goto bb0x10000484;
            }
            else {
                goto bb0x100004e0;
            }
        }
    }
    else {
        g10 = 0x10000428;
        if (flags) {
            goto bb0x100004d4;
        }
        else {
            goto bb0x10000478;
        }
    }
    return ROTL(((CR0 << 28) + (CR1 << 24) + (CR2 << 20) + (CR3 << 16) + (CR4 << 12) + (CR5 << 8) + (CR6 << 4) + CR7), 19) & 0x1;
}

