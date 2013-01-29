// address: 10000468
int main(int argc, char *argv[], char *envp[]) {
    unsigned int g0; 		// r0
    void *g0_1; 		// r0
    void *g1; 		// r1
    __size32 g10; 		// r10
    __size32 g11; 		// r11
    __size32 g3; 		// r3
    int local0; 		// m[g1 - 112]
    int local1; 		// m[g1 - 120]
    int local2; 		// m[g1 - 124]
    char local3; 		// m[g1 - 96]
    __size32 local4; 		// m[g1 - 136]
    char[] **local5; 		// m[g1 - 132]
    int local6; 		// m[g1 - 116]
    int local7; 		// m[g1 - 128]
    __size32 local8; 		// m[g1 - 136]{241}

    malloc(12);
    *(__size32*)(g3 + 4) = 0x10001df4;
    local4 = 2;
    local5 = g3 + 4;
    local8 = local4;
    local4 = local8 - 1;
    while (local8 - 1 != 0) {
        g3 = *local5;
        strlen(g3);
        local6 = g3;
        if (g3 > 10) {
            local6 = 10;
        }
        local7 = 0;
L16:
        if (local7 <= 6) {
            local2 = 0;
            while (local2 < local6) {
                g0 = *(unsigned char*)(g9 + local2);
                g0 = (ROTL(g0) & 0xff) - 32;
                local0 = g0;
                if (g0 < 0) {
                    local0 = 0;
                }
                local1 = 0;
L2:
                if (local1 <= 6) {
                    g10 = (ROTL(local2) & 0xfffffff9) + g1 + local1 - 96;
                    g11 = (ROTL((rs * 7 + local7)) & 0xfffffffd) + 0x100120f4;
                    g0_1 = *g11;
                    g0 = *(unsigned char*)(rs * 7 + local1 + g0_1);
                    *(__size8*)g10 = (char) g0;
                    local1++;
                    goto L2;
                }
                *(__size8*)((ROTL(local2) & 0xfffffff9) + g1 - 89) = 32;
                local2++;
            }
            g0 = (ROTL(local6) & 0xfffffff9) - 1;
            local2 = g0;
            while (local2 >= 0) {
                g0 = *(unsigned char*)(g1 + local2 - 96);
                if ((ROTL(g0) & 0xff) != 32) {
                    break;
                }
                *(__size8*)(g1 + local2 - 96) = 0;
                local2 = local2 - 1;
            }
            puts(&local3);
            local7++;
            goto L16;
        }
        puts("");
        local5++;
        local8 = local4;
        local4 = local8 - 1;
    }
    return 0;
}

