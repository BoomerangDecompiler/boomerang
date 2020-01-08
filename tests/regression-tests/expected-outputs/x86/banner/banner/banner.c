int main(int argc, char *argv[]);

union { int; unsigned char *; __size32 *x16; } glyphs[84];

/** address: 0x08048390 */
int main(int argc, char *argv[])
{
    unsigned char al; 		// r8
    int eax; 		// r24
    int edx; 		// r26
    int esp; 		// r28
    union { int; char *; __size32 *x260; } local0; 		// m[esp - 36]
    union { int; char *; __size32 *x281; } local1; 		// m[esp - 24]
    char local10; 		// m[esp - 124]
    size_t local11; 		// m[esp - 172]
    __size32 local12; 		// m[esp - 16]
    union { unsigned int; char *x155; char **; } local13; 		// m[esp - 20]
    union { int; char *; __size32 *x322; } local2; 		// m[esp - 28]
    union { int; char *; __size32 *x196; } local3; 		// m[esp - 40]
    union { int; unsigned char *; __size32 *x34; } local4; 		// m[esp - 32]
    union { int; char *; __size32 *x198; } local5; 		// m[esp - 132]
    union { int; char *; __size32 *x202; } local6; 		// m[esp - 144]
    union { unsigned int; char *; __size32 *x158; } local7; 		// m[esp - 128]
    union { unsigned int; char *; __size32 *x333; } local8; 		// m[esp - 136]
    union { int; char *; __size32 *x335; } local9; 		// m[esp - 140]

    eax = malloc(12);
    *(__size32*)(eax + 4) = 0x8049af9;
    local12 = 2;
    local13 = eax + 4;
    local12--;
    while (local12 != 0) {
        local11 = *local13;
        eax = strlen(local11);
        local0 = eax;
        if (eax > 10) {
            local0 = 10;
        }
        local1 = 0;
bb0x8048403:
        if (local1 <= 6) {
            local2 = 0;
            while (local2 < local0) {
                eax = local2 + *local13;
                eax = (int) *eax;
                local3 = eax - 32;
                if (eax < 32) {
                    local3 = 0;
                }
                local4 = 0;
bb0x8048448:
                if (local4 <= 6) {
                    eax = local2 * 8 + esp + local4 - 12;
                    local7 = eax - 112;
                    local5 = local3;
                    if (local3 < 0) {
                        local5 = local3 + 7;
                    }
                    edx = local1 + (local5 >> 3) * 7;
                    local8 = edx;
                    local9 = local3;
                    local6 = local3;
                    if (local3 < 0) {
                        local6 = local3 + 7;
                    }
                    al = *((local3 - (local6 >> 3) * 8) * 7 + local4 + glyphs[edx]);
                    *(unsigned char*)(eax - 112) = al;
                    local4++;
                    goto bb0x8048448;
                }
                *(__size8*)(esp + local2 * 8 - 117) = 32;
                local2++;
            }
            local2 = local0 * 8 - 1;
            while (local2 >= 0) {
                eax = esp + local2 - 124;
                tmpb = *eax - 32;
                if (*eax != 32) {
                    break;
                }
                *(__size8*)(esp + local2 - 124) = 0;
                local2--;
            }
            puts(&local10);
            local1++;
            goto bb0x8048403;
        }
        puts("");
        local13 += 4;
        local12--;
    }
    return 0;
}

