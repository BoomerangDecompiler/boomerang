int main(int argc, char *argv[]);


/** address: 0x00010704 */
int main(int argc, char *argv[])
{
    union { int; char *; } local0; 		// m[o6 - 32]
    union { int; char *; } local1; 		// m[o6 - 36]
    union { int; char *; } local2; 		// m[o6 - 44]
    char local3; 		// m[o6 - 128]
    __size32 local4; 		// m[o6 - 20]
    union { int; char *; } *local5; 		// m[o6 - 24]
    union { int; char *; } local6; 		// m[o6 - 40]
    union { int; char *; } local7; 		// m[o6 - 28]
    __size32 local8; 		// m[o6 - 20]{10}
    int o0; 		// r8
    int o1; 		// r9
    int o2; 		// r10
    int o3; 		// r11
    int o4; 		// r12
    int o6; 		// r14

    o0 = malloc(12);
    *(__size32*)(o0 + 4) = 0x11f18;
    local4 = 2;
    local5 = o0 + 4;
    local8 = local4;
    local4 = local8 - 1;
    while (local8 != 1) {
        o0 = *local5;
        o0 = strlen(o0); /* Warning: also results in o2, o3, o4 */
        local6 = o0;
        if (o0 > 10) {
            local6 = 10;
        }
        local7 = 0;
bb0x10794:
        if (local7 <= 6) {
            local0 = 0;
            while (local0 < local6) {
                o1 = *local5;
                o0 = *(unsigned char*)(o1 + local0);
                o0 = (o0 << 24 >> 24) - 32;
                local2 = o0;
                if (o0 < 0) {
                    local2 = 0;
                }
                local1 = 0;
bb0x10804:
                if (local1 <= 6) {
                    o1 = local0 * 8 + o6 - 16;
                    o3 = o1 + local1;
                    o4 = 0x220b4;
                    o2 = ((local2 + ((unsigned int)(local2 >> 31) >> 29) >> 3) * 7 + local7) * 4;
                    o0 = *(o2 + 0x220b4);
                    o0 = *(unsigned char*)((local2 - (local2 + ((unsigned int)(local2 >> 31) >> 29) >> 3) * 8) * 7 + local1 + o0);
                    *(__size8*)(o1 + local1 - 112) = (char) o0;
                    local1++;
                    goto bb0x10804;
                }
                *(__size8*)(local0 * 8 + o6 - 121) = 32;
                local0++;
            }
            local0 = local6 * 8 - 1;
            while (local0 >= 0) {
                o0 = *(unsigned char*)(o6 + local0 - 128);
                if (o0 << 24 >> 24 == 32) {
                    *(__size8*)(o6 + local0 - 128) = 0;
                    local0--;
                }
            }
            o2 = puts(&local3); /* Warning: also results in o3, o4 */
            local7++;
            goto bb0x10794;
        }
        puts("");
        local5++;
        local8 = local4;
        local4 = local8 - 1;
    }
    return 0;
}

