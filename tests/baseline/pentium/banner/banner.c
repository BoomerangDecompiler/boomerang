void * glyphs[84];

// address: 8048390
int main(int argc, char *argv[], char *envp[]) {
    unsigned char al; 		// r8
    __size32 eax; 		// r24
    union { __size32 * x11; int x12; } eax_1; 		// r24{115}
    unsigned int edx_1; 		// r26{148}
    int esp; 		// r28
    int local0; 		// m[esp - 28]
    int local1; 		// m[esp - 32]
    __size32 local10; 		// m[esp - 16]
    char[] **local11; 		// m[esp - 20]
    char *local12; 		// m[esp - 172]
    int local13; 		// m[esp - 36]
    int local14; 		// m[esp - 24]
    int local2; 		// m[esp - 40]
    union { __size32 * x11; int x12; } local3; 		// m[esp - 128]
    int local4; 		// m[esp - 132]
    unsigned int local5; 		// m[esp - 136]
    int local6; 		// m[esp - 140]
    int local7; 		// m[esp - 144]
    char local8; 		// m[esp - 124]

    malloc(12);
    *(__size32*)(eax + 4) = 0x8049af9;
    local10 = 2;
    local11 = eax + 4;
    local10 = local10 - 1;
    while (local10 != 0) {
        local12 = *local11;
        strlen(local12);
        local13 = eax;
        if (eax > 10) {
            local13 = 10;
        }
        local14 = 0;
L20:
        if (local14 <= 6) {
            local0 = 0;
            while (local0 < local13) {
                eax = local0 + *local11;
                eax = (int) *eax;
                local2 = eax - 32;
                if (eax - 32 < 0) {
                    local2 = 0;
                }
                local1 = 0;
L6:
                if (local1 <= 6) {
                    eax = local0 * 8 + esp - 12;
                    eax_1 = eax + local1;
                    local3 = eax_1 - 112;
                    local4 = local2;
                    if (local2 < 0) {
                        local4 = local2 + 7;
                    }
                    edx_1 = local14 + (local4 >> 3) * 7;
                    local5 = edx_1;
                    local6 = local2;
                    local7 = local2;
                    if (local2 < 0) {
                        local7 = local2 + 7;
                    }
                    eax = edx * 7 + local1;
                    al = *(eax + glyphs[edx_1]);
                    *(unsigned char*)(eax_1 - 112) = al;
                    local1++;
                    goto L6;
                }
                *(__size8*)(esp + local0 * 8 - 117) = 32;
                local0++;
            }
            local0 = local13 * 8 - 1;
            while (local0 >= 0) {
                eax = esp + local0 - 124;
                tmpb = *eax - 32;
                if (*eax != 32) {
                    break;
                }
                eax = esp + local0 - 124;
                *(__size8*)eax = 0;
                local0 = local0 - 1;
            }
            puts(&local8);
            local14++;
            goto L20;
        }
        puts("");
        local11++;
        local10 = local10 - 1;
    }
    return 0;
}

