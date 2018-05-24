int main(int argc, char *argv[]);

/** address: 0x00010704 */
int main(int argc, char *argv[])
{
    int local0; 		// m[o6 - 32]
    int local1; 		// m[o6 - 36]
    int local2; 		// m[o6 - 44]
    char local3; 		// m[o6 - 128]
    int local4; 		// m[o6 - 20]
    union { __size32; union { char[] *; int; } *; } local5; 		// m[o6 - 24]
    int local6; 		// m[o6 - 40]
    int local7; 		// m[o6 - 28]
    int local8; 		// m[o6 - 20]{0}
    int o0; 		// r8
    void *o0_1; 		// r8
    int o0_2; 		// r8{0}
    unsigned int o0_3; 		// r8{0}
    int o0_4; 		// r8{0}
    unsigned int o0_5; 		// r8{0}
    int o1; 		// r9
    int o2; 		// r10
    int o3; 		// r11
    int o4; 		// r12
    int o6; 		// r14

    o0_1 = malloc(12);
    *(int*)(o0_1 + 4) = 0x11f18;
    local4 = 2;
    local5 = o0_1 + 4;
    local8 = local4;
    local4 = local8 - 1;
    while (local8 - 1 != 0) {
        o0 = *local5;
        o0_2 = strlen(o0); /* Warning: also results in o2, o3, o4 */
        local6 = o0_2;
        if (o0_2 > 10) {
            local6 = 10;
        }
        local7 = 0;
bb0x10794:
        if (local7 <= 6) {
            local0 = 0;
            while (local0 < local6) {
                o1 = *local5;
                o0_3 = *(unsigned char*)(o1 + local0);
                o0_4 = ((int)(o0_3 * 0x1000000) >> 24) - 32;
                local2 = o0_4;
                if (o0_4 < 0) {
                    local2 = 0;
                }
                local1 = 0;
bb0x10804:
                if (local1 <= 6) {
                    o1 = local0 * 8 + o6 - 16;
                    o3 = o1 + local1;
                    o4 = 0x220b4;
                    o2 = (((int)(local2 + (local2 >> 31) / 0x20000000) >> 3) * 7 + local7) * 4;
                    o0 = *(o2 + 0x220b4);
                    o0_5 = *(unsigned char*)((local2 - ((int)(local2 + (local2 >> 31) / 0x20000000) >> 3) * 8) * 7 + local1 + o0);
                    *(__size8*)(o1 + local1 - 112) = (char) o0_5;
                    local1++;
                    goto bb0x10804;
                }
                *(int*)(local0 * 8 + o6 - 121) = 32;
                local0++;
            }
            local0 = local6 * 8 - 1;
            while (local0 >= 0) {
                o0 = *(unsigned char*)(o6 + local0 - 128);
                if ((int)(o0 * 0x1000000) >> 24 != 32) {
                    goto bb0x10960;
                }
                *(int*)(o6 + local0 - 128) = 0;
                local0 = local0 - 1;
            }
bb0x10960:
            o2 = puts(&local3); /* Warning: also results in o3, o4 */
            local7++;
            goto bb0x10794;
        }
        puts(0x11f28);
        local5 += 4;
        local8 = local4;
        local4 = local8 - 1;
    }
    return 0;
}

