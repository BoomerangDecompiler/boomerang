union { __size32; unsigned char *; } glyphs[84];
int main(int argc, char *argv[]);

/** address: 0x08048390 */
int main(int argc, char *argv[])
{
    unsigned char al; 		// r8
    void *eax; 		// r24
    int eax_1; 		// r24
    int eax_2; 		// r24{0}
    int eax_5; 		// r24{0}
    int edx; 		// r26
    int esp; 		// r28
    int local0; 		// m[esp - 28]
    int local1; 		// m[esp - 32]
    __size32 local10; 		// m[esp - 16]
    union { __size32 *; int; } local11; 		// m[esp - 20]
    int local13; 		// m[esp - 36]
    int local14; 		// m[esp - 24]
    int local2; 		// m[esp - 40]
    int local3; 		// m[esp - 128]
    int local4; 		// m[esp - 132]
    unsigned int local5; 		// m[esp - 136]
    int local6; 		// m[esp - 140]
    int local7; 		// m[esp - 144]
    char local8; 		// m[esp - 124]
    size_t local9; 		// m[esp - 172]

    eax = malloc(12);
    *(__size32*)(eax + 4) = 0x8049af9;
    local10 = 2;
    local11 = eax + 4;
    local10 = local10 - 1;
    while (local10 != 0) {
        local9 = *local11;
        eax_2 = strlen(local9);
        local13 = eax_2;
        if (eax_2 > 10) {
            local13 = 10;
        }
        local14 = 0;
bb0x8048403:
        if (local14 <= 6) {
            local0 = 0;
            while (local0 < local13) {
                eax = local0 + *local11;
                eax_5 = (int) *eax;
                local2 = eax_5 - 32;
                if (eax_5 < 32) {
                    local2 = 0;
                }
                local1 = 0;
bb0x8048448:
                if (local1 <= 6) {
                    eax_1 = local0 * 8 + esp + local1 - 12;
                    local3 = eax_1 - 112;
                    local4 = local2;
                    if (local2 < 0) {
                        local4 = local2 + 7;
                    }
                    edx = local14 + (local4 >> 3) * 7;
                    local5 = edx;
                    local6 = local2;
                    local7 = local2;
                    if (local2 < 0) {
                        local7 = local2 + 7;
                    }
                    al = *((local2 - (local7 >> 3) * 8) * 7 + local1 + glyphs[edx]);
                    *(unsigned char*)(eax_1 - 112) = al;
                    local1++;
                    goto bb0x8048448;
                }
                *(__size8*)(esp + local0 * 8 - 117) = 32;
                local0++;
            }
            local0 = local13 * 8 - 1;
            while (local0 >= 0) {
                eax = esp + local0 - 124;
                tmpb = *eax - 32;
                if (*eax != 32) {
                    goto bb0x8048531;
                }
                *(__size8*)(esp + local0 - 124) = 0;
                local0 = local0 - 1;
            }
bb0x8048531:
            puts(&local8);
            local14++;
            goto bb0x8048403;
        }
        puts("");
        local11 += 4;
        local10 = local10 - 1;
    }
    return 0;
}

