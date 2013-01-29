// address: 10704
int main(int argc, char *argv[], char *envp[]) {
    int local0; 		// m[o6 - 32]
    int local1; 		// m[o6 - 36]
    int local2; 		// m[o6 - 44]
    char local3; 		// m[o6 - 128]
    __size32 local4; 		// m[o6 - 20]
    char[] **local5; 		// m[o6 - 24]
    int local6; 		// m[o6 - 40]
    int local7; 		// m[o6 - 28]
    __size32 local8; 		// m[o6 - 20]{307}
    int o0; 		// r8
    char *o0_1; 		// r8
    __size32 o1; 		// r9
    int o1_1; 		// r9
    int o2; 		// r10
    int o3; 		// r11
    int o4; 		// r12
    int o6; 		// r14

    malloc(12);
    *(__size32*)(o0 + 4) = 0x11f18;
    local4 = 2;
    local5 = o0 + 4;
    local8 = local4;
    local4 = local8 - 1;
    while (local8 - 1 != 0) {
        o0 = *local5;
        strlen(o0);
        local6 = o0;
        if (o0 > 10) {
            local6 = 10;
        }
        local7 = 0;
L16:
        if (local7 <= 6) {
            local0 = 0;
            while (local0 < local6) {
                o1 = *local5;
                o0 = *(unsigned char*)(o1 + local0);
                o0 = ((int)(o0 * 0x1000000) >> 24) - 32;
                local2 = o0;
                if (o0 < 0) {
                    local2 = 0;
                }
                local1 = 0;
L2:
                if (local1 <= 6) {
                    o1 = local0 * 8 + o6 - 16;
                    o3 = o1 + local1;
                    o4 = 0x220b4;
                    o1_1 = (int)(local2 + (local2 >> 31) / 0x20000000) >> 3;
                    o2 = (o1_1 * 7 + local7) * 4;
                    o1_1 = local2 - ((int)(local2 + (local2 >> 31) / 0x20000000) >> 3) * 8;
                    o0_1 = *(o2 + 0x220b4);
                    o0 = *(unsigned char*)(o1_1 * 7 + local1 + o0_1);
                    *(__size8*)(o1 + local1 - 112) = (char) o0;
                    local1++;
                    goto L2;
                }
                o1 = local0 * 8 + o6 - 16;
                *(__size8*)(o1 - 105) = 32;
                local0++;
            }
            local0 = local6 * 8 - 1;
            while (local0 >= 0) {
                o0 = *(unsigned char*)(o6 + local0 - 128);
                if ((int)(o0 * 0x1000000) >> 24 != 32) {
                    break;
                }
                *(__size8*)(o6 + local0 - 128) = 0;
                local0 = local0 - 1;
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

