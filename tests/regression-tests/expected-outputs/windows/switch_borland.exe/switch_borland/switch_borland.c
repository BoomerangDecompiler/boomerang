int global54_40b59c = 0x40b570;
int global54_40b59c = 0x40b570;
int main(int argc, char *argv[]);
__size32 proc_0x004038e4(__size32 param1);
void proc_0x00403ad0(__size32 param1);
__size32 proc_0x00403a68(__size32 param1, unsigned int param2, unsigned int param3);
__size32 proc_0x00403a20(__size32 param1, __size32 param2);
void proc_0x00405090(unsigned long long param1, int param2, union { int *; __size32; } param3, unsigned long long param4, unsigned int param5, int param6);
void proc_0x00402250(union { int; char *; } param1);
void proc_0x0040487c(union { int; unsigned int *; } param1, unsigned int param2);
void proc_0x00403a9c(unsigned int param1, __size32 param2);
void proc_0x00404fc8();
void proc_0x00404fce();
void proc_0x00404d5a(unsigned long long param1, union { unsigned int; bool; } param2, unsigned int param3);
__size32 proc_0x00404c97(unsigned long long param1, unsigned long long param2, unsigned long long param3, unsigned int param4);

/** address: 0x00401150 */
int main(int argc, char *argv[])
{
    if ((unsigned int)argc > (unsigned int)7) {
        proc_0x004038e4(0x40a152);
bb0x4011da:
        goto bb0x4011da;
    }
bb0x40115b:
    switch(argc) {
    case 0:
        goto bb0x40115b;
    case 1:
        goto bb0x40115b;
    case 2:
        proc_0x004038e4(0x40a128);
bb0x40118c:
        goto bb0x40118c;
    case 3:
        proc_0x004038e4(0x40a12e);
bb0x401199:
        goto bb0x401199;
    case 4:
        proc_0x004038e4(0x40a136);
bb0x4011a6:
        goto bb0x4011a6;
    case 5:
        proc_0x004038e4(0x40a13d);
bb0x4011b3:
        goto bb0x4011b3;
    case 6:
        proc_0x004038e4(0x40a144);
bb0x4011c0:
        goto bb0x4011c0;
    case 7:
        proc_0x004038e4(0x40a14a);
bb0x4011cd:
        goto bb0x4011cd;
    }
    return 0;
}

/** address: 0x004038e4 */
__size32 proc_0x004038e4(__size32 param1)
{
    proc_0x00403ad0(0);
    return param1;
}

/** address: 0x00403ad0 */
void proc_0x00403ad0(__size32 param1)
{
    int ebx; 		// r27
    __size32 esi; 		// r30
    int local0; 		// m[r29 - 9]{0}
    __size8 local1; 		// m[r29 - 9]{0}
    __size8 local10; 		// m[r26]{0}
    __size16 local2; 		// m[r29 - 184]{0}
    __size8 local3; 		// m[r29 - 184]{0}
    __size8 local4; 		// m[r25]{0}
    __size16 local5; 		// m[r25]{0}
    int local6; 		// m[r26]{0}
    __size8 local8; 		// m[r26]{0}

    if (param1 == 0) {
    }
    else {
    }
    while ( ~flags) {
        if (flags) {
            if ( ~(flags || flags)) {
                *(__size32*)(r[28] - 4) = r[26];
                *(__size32*)(r[28] - 4) = r[27];
                proc_0x00403a68(*(r[28] + 8), r[25], r[26]);
            }
            *(__size32*)(r[28] - 4) = r[24];
            *(__size32*)(r[28] - 4) = r[27];
            proc_0x00403a68(*(r[28] + 8), r[25], r[26]);
        }
        else {
            if (flags) {
                *(__size32*)(r[29] - 20) = r[26];
                *(__size32*)(r[29] - 16) = r[26];
                local0 = 0;
                *(__size32*)(r[29] - 8) = r[26];
                *(__size32*)(r[29] - 4) = r[26];
                *(__size32*)(r[29] - 28) = r[25];
                do {
bb0x403b8b:
                    if (flags || flags) {
                        *(__size32*)(r[29] - 20)++;
                        while (flags) {
                            *(__size32*)(r[28] - 4) = r[26];
                            *(__size32*)(r[28] - 4) = r[24];
                            proc_0x00403a68(*(r[28] + 8), r[25], r[26]);
                            *(__size32*)(r[29] - 20)++;
                        }
                        goto bb0x4043e4;
                    }
                    else {
                    }
                    goto bb0x403b8b;
                } while (flags);
bb0x403bb9:
                switch(0 >> 8 & 0xffffff | *(unsigned char*)((0 >> 8 & 0xffffff | ((unsigned char) (ebx >> 8 & 0xffffff | *(unsigned char*)esi) - 32)) + 0x40b1fa)) {
                case 0:
bb0x403c4c:
                    if (flags) {
                        goto bb0x403c4c;
                    }
                    else {
bb0x403c54:
                        if (flags) {
                            goto bb0x403c54;
                        }
bb0x403c5e:
                        local1 = r[11];
                        goto bb0x403c5e;
                    }
                case 1:
bb0x403c2c:
                    if (flags) {
                        goto bb0x403c2c;
                    }
                    else {
bb0x403c34:
                        goto bb0x403c34;
                    }
                case 2:
                    *(__size32*)(r[29] + 28) += 4;
                    *(__size32*)(r[29] - 48) = r[25];
                    if (flags) {
bb0x403cd2:
                        if (flags) {
                            goto bb0x403cd2;
                        }
                        else {
bb0x403cdb:
                            *(__size32*)(r[29] - 8) = r[26];
                            goto bb0x403cdb;
                        }
                    }
                    else {
                        if (flags) {
                            *(__size32*)(r[29] - 4) = r[26];
                        }
                        else {
                            *(__size32*)(r[29] - 4) = r[24];
                        }
bb0x403cc8:
                        goto bb0x403cc8;
                    }
                case 3:
bb0x403c3c:
                    if (flags) {
                        goto bb0x403c3c;
                    }
                    else {
bb0x403c44:
                        goto bb0x403c44;
                    }
                case 4:
bb0x403ce7:
                    if (flags) {
                        goto bb0x403ce7;
                    }
                    else {
bb0x403cf0:
                        *(__size32*)(r[29] - 8)++;
                        goto bb0x403cf0;
                    }
                case 5:
                    if (flags) {
bb0x403d30:
                        if (flags) {
                            goto bb0x403d30;
                        }
                        else {
bb0x403d39:
                            *(__size32*)(r[29] - 8) = r[25];
                            goto bb0x403d39;
                        }
                    }
                    else {
bb0x403d05:
                        if (flags) {
bb0x403d1b:
                            *(__size32*)(r[29] - 4) = r[25];
                            goto bb0x403d1b;
                        }
                        else {
bb0x403d10:
                            *(__size32*)(r[29] - 4) = r[26];
                            goto bb0x403d10;
                        }
                        goto bb0x403d05;
                    }
                case 6:
bb0x403d4e:
                    goto bb0x403d4e;
                case 7:
bb0x403d5b:
                    goto bb0x403d5b;
                case 8:
bb0x403d6e:
                    goto bb0x403d6e;
                case 9:
bb0x403c80:
                    if (flags) {
                        goto bb0x403c80;
                    }
                    else {
bb0x403c84:
                        if (flags) {
                            goto bb0x403c84;
                        }
bb0x403c90:
                        goto bb0x403c90;
                    }
                case 10:
                    *(__size32*)(r[29] - 56) = 10;
                    if (flags) {
                        if (flags) {
                            if (flags) {
                                *(__size32*)(r[29] + 28) += 4;
                                *(__size32*)(r[29] - 48) = r[26];
                                if (flags) {
                                    *(__size32*)(r[29] - 40) = r[24];
                                    *(__size32*)(r[29] - 36) = r[26];
                                }
                                else {
                                    *(__size32*)(r[29] - 40) = r[24];
                                    *(__size32*)(r[29] - 36) = r[26];
                                }
                            }
                            else {
                                *(__size32*)(r[29] + 28) += 4;
                                *(__size16*)(r[29] - 50) = r[2];
                                if (flags) {
                                    *(__size32*)(r[29] - 40) = r[24];
                                    *(__size32*)(r[29] - 36) = r[26];
                                }
                                else {
                                    *(__size32*)(r[29] - 40) = r[24];
                                    *(__size32*)(r[29] - 36) = r[26];
                                }
                            }
                        }
                        else {
                            *(__size32*)(r[29] + 28) += 4;
                            *(__size32*)(r[29] - 44) = r[26];
                            if (flags) {
                                *(__size32*)(r[29] - 40) = r[24];
                                *(__size32*)(r[29] - 36) = r[26];
                            }
                            else {
                                *(__size32*)(r[29] - 40) = r[24];
                                *(__size32*)(r[29] - 36) = r[26];
                            }
                        }
                    }
                    else {
                        *(__size32*)(r[29] + 28) += 8;
                        *(__size32*)(r[29] - 40) = r[26];
                        *(__size32*)(r[29] - 36) = r[26];
                    }
                    *(__size32*)(r[29] - 24) = r[24];
                    if (flags || flags) {
                        *(__size32*)(r[28] - 4) = r[24];
                        *(__size32*)(r[28] - 4) = r[25];
                        *(__size32*)(r[28] - 4) = r[26];
                        *(__size32*)(r[28] - 4) = r[25];
                        *(__size32*)(r[28] - 4) = *(r[29] - 36);
                        *(__size32*)(r[28] - 4) = *(r[29] - 40);
                        proc_0x00405090(*(r[28] + 4), *(r[28] + 8), *(r[28] + 12), *(r[28] + 16), *(r[28] + 20), *(r[28] + 24));
                    }
                    else {
bb0x403ef9:
                        if (flags) {
                            goto bb0x403ef9;
                        }
                        else {
                            local6 = 0;
                        }
                    }
                    if (flags) {
                        if ( ~(flags || flags)) {
                            *(__size32*)(r[28] - 4) = r[26];
                            proc_0x00402250(*(r[28] + 4));
                            *(__size32*)(r[29] - 64) = r[24];
                            if ( ~flags) {
                                *(__size32*)(r[29] - 64) = *(r[29] - 64) - 1;
                            }
                            if ( ~flags) {
                                *(__size32*)(r[29] - 16) = r[26];
                            }
                        }
                        if ( ~( ~flags && flags)) {
                            if ( ~flags) {
                                *(__size32*)(r[29] - 24) = *(r[29] - 24) - 1;
                                local8 = r[9];
                            }
                            if ( ~flags) {
                                *(__size32*)(r[29] - 16) = *(r[29] - 16) - 1;
                            }
                        }
                        *(__size32*)(r[28] - 4) = r[24];
                        proc_0x00402250(*(r[28] + 4));
                        *(__size32*)(r[29] - 64) = r[24];
                    }
                    else {
                        *(__size32*)(r[28] - 4) = r[26];
                        proc_0x00402250(*(r[28] + 4));
                        *(__size32*)(r[29] - 60) = r[24];
                        *(__size32*)(r[29] - 64) = r[24];
                        if (flags) {
                            if ( ~flags) {
                                *(__size32*)(r[29] - 64)++;
                                *(__size32*)(r[29] - 24) = *(r[29] - 24) - 1;
                                local4 = r[8];
                            }
                        }
                        else {
                            *(__size32*)(r[29] - 60) = *(r[29] - 60) - 1;
                        }
                        if ( ~flags) {
                            *(__size32*)(r[29] - 16) = r[25];
                        }
                    }
                    if ( ~flags) {
                        if (flags) {
                            if ( ~( ~flags && flags)) {
                                *(__size32*)(r[29] - 4) = *(r[29] - 4) - 2;
                                *(__size32*)(r[29] - 16) = *(r[29] - 16) - 2;
                                if ( ~flags) {
                                    *(__size32*)(r[29] - 16) = r[26];
                                }
                            }
                        }
                        else {
                            if ( ~flags) {
                                *(__size32*)(r[29] - 16) = 1;
                            }
                        }
                    }
                    *(__size32*)(r[29] - 64) += r[25];
                    if ( ~(flags || flags)) {
                        do {
                            *(__size32*)(r[28] - 4) = r[26];
                            *(int*)(r[28] - 4) = 32;
                            proc_0x00403a68(*(r[28] + 8), r[25], r[26]);
                            *(__size32*)(r[29] - 4) = *(r[29] - 4) - 1;
                        } while (flags);
                    }
                    if ( ~flags) {
                        *(__size32*)(r[28] - 4) = r[24];
                        *(int*)(r[28] - 4) = 48;
                        proc_0x00403a68(*(r[28] + 8), r[25], r[26]);
                        *(__size32*)(r[28] - 4) = r[26];
                        *(__size32*)(r[28] - 4) = r[27];
                        proc_0x00403a68(*(r[28] + 8), r[25], r[26]);
                    }
                    if ( ~flags) {
                        *(__size32*)(r[29] - 64) = *(r[29] - 64) - r[25];
                        *(__size32*)(r[29] - 4) = *(r[29] - 4) - r[24];
                        if ( ~( ~(flags || flags) && flags)) {
                            *(__size32*)(r[28] - 4) = r[26];
                            *(__size32*)(r[29] - 24)++;
                            *(__size32*)(r[28] - 4) = r[24];
                            proc_0x00403a68(*(r[28] + 8), r[25], r[26]);
                            *(__size32*)(r[29] - 64) = *(r[29] - 64) - 1;
                            *(__size32*)(r[29] - 4) = *(r[29] - 4) - 1;
                        }
                        *(__size32*)(r[29] - 16) = *(r[29] - 16) - 1;
                        while (flags) {
                            *(__size32*)(r[28] - 4) = r[26];
                            *(int*)(r[28] - 4) = 48;
                            proc_0x00403a68(*(r[28] + 8), r[25], r[26]);
                            *(__size32*)(r[29] - 16) = *(r[29] - 16) - 1;
                        }
                    }
                    if ( ~flags) {
                        *(__size32*)(r[29] - 68) = r[24];
                        *(__size32*)(r[29] - 76) = r[26];
                        *(__size32*)(r[29] - 84) = r[25];
                        do {
bb0x40430b:
                            *(__size32*)(r[29] - 84) = *(r[29] - 84) - 1;
                            if (flags) {
                                *(__size32*)(r[29] - 68) += 2;
                                *(__size32*)(r[28] - 4) = r[26];
                                *(__size32*)(r[28] - 4) = r[25];
                                proc_0x0040487c(*(r[28] + 4), *(r[28] + 8));
                                *(__size32*)(r[29] - 80) = r[24];
                                if ( ~flags) {
                                    if (flags) {
                                        continue;
                                    }
                                    break;
                                }
                            }
                            *(__size32*)(r[29] - 24) = r[26];
                            *(__size32*)(r[29] - 64) = r[25];
                            goto bb0x404325;
                            local10 = r[11];
                            *(__size32*)(r[29] - 76)++;
                        } while (flags);
                        goto bb0x40430b;
                    }
bb0x404325:
                    if ( ~flags) {
                        *(__size32*)(r[29] - 4) = *(r[29] - 4) - r[24];
                        *(__size32*)(r[29] - 64) = *(r[29] - 64) - 1;
                        while (flags) {
                            *(__size32*)(r[28] - 4) = r[26];
                            *(__size32*)(r[29] - 24)++;
                            *(__size32*)(r[28] - 4) = r[24];
                            proc_0x00403a68(*(r[28] + 8), r[25], r[26]);
                            *(__size32*)(r[29] - 64) = *(r[29] - 64) - 1;
                        }
                    }
                    *(__size32*)(r[29] - 4) = *(r[29] - 4) - 1;
                    while (flags) {
                        *(__size32*)(r[28] - 4) = r[25];
                        *(int*)(r[28] - 4) = 32;
                        proc_0x00403a68(*(r[28] + 8), r[25], r[26]);
                        *(__size32*)(r[29] - 4) = *(r[29] - 4) - 1;
                    }
bb0x404374:
                    goto bb0x404374;
                case 11:
                    *(__size32*)(r[29] - 56) = 8;
bb0x403e23:
                    *(int*)(r[29] - 9) = 0;
                    goto bb0x403e23;
                case 12:
bb0x403e0d:
                    *(__size32*)(r[29] - 56) = 10;
                    goto bb0x403e0d;
                case 13:
bb0x403e16:
                    *(__size32*)(r[29] - 56) = 16;
                    *(__size8*)(r[29] - 29) = r[10];
                    goto bb0x403e16;
                case 14:
                    *(__size32*)(r[29] + 28) += 4;
                    *(__size32*)(r[29] - 24) = r[26];
                    *(__size32*)(r[28] - 4) = r[25];
                    *(__size32*)(r[28] - 4) = r[24];
                    proc_0x00403a9c(*(r[28] + 4), *(r[28] + 8));
bb0x403f97:
                    *(int*)(r[29] - 176) = 0;
                    *(__size32*)(r[29] - 24) = r[26];
                    goto bb0x403f97;
                case 15:
                    if ( ~flags) {
                        *(__size32*)(r[28] - 4) = r[24];
                        *(__size32*)(r[28] - 4) = r[26];
                        *(__size32*)(r[28] - 4) = r[27];
                        *(__size32*)(r[29] - 24) = r[25];
                        *(__size32*)(r[28] - 4) = r[25];
                        if ( ~flags) {
                            *(__size32*)(r[28] - 4) = r[24];
                            *(__size32*)(r[28] - 4) = r[26];
                            proc_0x00404fc8();
                        }
bb0x404120:
                        goto bb0x404120;
                    }
bb0x4040fc:
                    goto bb0x4040fc;
                case 16:
bb0x403fbd:
                    if (flags) {
bb0x403ffc:
                        *(__size32*)(r[29] + 28) += 4;
                        local3 = r[8];
                        *(int*)(r[29] - 183) = 0;
                        *(__size32*)(r[29] - 24) = r[26];
                        *(__size32*)(r[29] - 28) = r[25];
                        *(__size32*)(r[29] - 64) = 1;
                        goto bb0x403ffc;
                    }
                    else {
bb0x403fc5:
                        *(__size32*)(r[29] + 28) += 4;
                        local2 = r[0];
                        *(int*)(r[29] - 182) = 0;
                        *(__size32*)(r[29] - 28) = 1;
                        *(__size32*)(r[29] - 24) = r[26];
                        *(__size32*)(r[29] - 64) = 1;
                        goto bb0x403fc5;
                    }
                    goto bb0x403fbd;
                case 17:
                    if (flags) {
                        *(__size32*)(r[29] + 28) += 4;
                        *(__size32*)(r[29] - 24) = r[24];
                        *(__size32*)(r[29] - 28) = r[26];
                        if ( ~flags) {
                            *(__size32*)(r[29] - 24) = 0x40b1e4;
                        }
                    }
                    else {
                        *(__size32*)(r[29] + 28) += 4;
                        *(__size32*)(r[29] - 24) = r[26];
                        *(__size32*)(r[29] - 28) = 1;
                        if ( ~flags) {
                            *(__size32*)(r[29] - 24) = 0x40b1ec;
                        }
                    }
bb0x404082:
                    if (flags) {
                        if (flags) {
                        }
                        else {
                        }
                        *(__size32*)(r[29] - 64) = r[25];
bb0x4040db:
                        while ( ~flags) {
                            if ( ~flags) {
bb0x4040e8:
                                goto bb0x4040e8;
                            }
                            *(__size32*)(r[29] - 64)++;
bb0x4040db:
                        }
                        goto bb0x4040db;
                    }
                    else {
                        if (flags) {
                        }
                        else {
                        }
                        *(__size32*)(r[29] - 64) = r[25];
bb0x4040a9:
                        while ( ~flags) {
                            if ( ~flags) {
bb0x4040b7:
                                goto bb0x4040b7;
                            }
                            *(__size32*)(r[29] - 64)++;
bb0x4040a9:
                        }
                        goto bb0x4040a9;
                    }
                    goto bb0x404082;
                case 18:
bb0x403fb2:
                    if ( ~flags) {
bb0x403fba:
                        goto bb0x403fba;
                    }
                    goto bb0x403fb2;
                case 19:
bb0x40402d:
                    if ( ~flags) {
bb0x404035:
                        goto bb0x404035;
                    }
                    goto bb0x40402d;
                case 20:
bb0x404379:
                    *(__size32*)(r[29] + 28) += 4;
                    *(__size32*)(r[29] - 24) = r[25];
                    if (flags) {
bb0x40439e:
                        if (flags) {
bb0x4043b8:
                            *(__size32*)r[26] = r[25];
                            goto bb0x4043b8;
                        }
                        else {
bb0x4043a6:
                            local5 = r[0];
                            goto bb0x4043a6;
                        }
                        goto bb0x40439e;
                    }
                    else {
bb0x40438e:
                        *(__size32*)r[24] = r[26];
                        goto bb0x40438e;
                    }
                    goto bb0x404379;
                case 21:
                    goto bb0x403bb9;
                case 22:
                    goto bb0x403bb9;
                case 23:
                    goto bb0x403bb9;
                case 24:
bb0x403c66:
                    goto bb0x403c66;
                case 25:
bb0x403c73:
                    goto bb0x403c73;
                case 26:
                    if (flags || flags) {
                        if (flags || flags) {
                            if (flags || flags) {
bb0x403dea:
                                if (flags) {
                                    goto bb0x403dea;
                                }
bb0x403df3:
                                goto bb0x403df3;
                            }
                            else {
bb0x403dd1:
                                goto bb0x403dd1;
                            }
                        }
                        else {
bb0x403db0:
                            goto bb0x403db0;
                        }
                    }
                    else {
bb0x403d8c:
                        goto bb0x403d8c;
                    }
                }
            }
            else {
bb0x403b2f:
                goto bb0x403b2f;
            }
        }
    }
bb0x4043e4:
    *(__size32*)(r[28] - 4) = r[25];
    proc_0x00403a20(*(r[28] + 4), r[26]);
    if (flags) {
    }
    else {
    }
    return;
}

/** address: 0x00403a68 */
__size32 proc_0x00403a68(__size32 param1, unsigned int param2, unsigned int param3)
{
    unsigned char al; 		// r8
    union { unsigned int *; int; } eax; 		// r24
    __size32 ebp; 		// r29
    __size32 ebx; 		// r27
    unsigned int ecx; 		// r25
    unsigned int edx; 		// r26
    __size32 esp; 		// r28
    __size32 esp_1; 		// r28{0}
    __size32 esp_4; 		// r28{0}
    unsigned int local3; 		// param2{0}
    unsigned int local4; 		// param3{0}
    union { __size32 *; __size32; } local5; 		// esp{0}
    unsigned int local6; 		// edx{0}
    unsigned int local7; 		// ecx{0}

    ebp = (esp_4 - 4);
    esp_1 = (esp_4 - 8);
    ebx = param1;
    local3 = param2;
    local4 = param3;
    local5 = esp_1;
    if (*(param1 + 80) >= 80) {
        edx = proc_0x00403a20(param1, param3); /* Warning: also results in ebx, esp, ebp */
        local4 = edx;
        ecx = *esp;
        esp += 4;
        local3 = ecx;
        local5 = esp;
    }
    param2 = local3;
    param3 = local4;
    esp = local5;
    eax = *(ebx + 100);
    al = (unsigned char) eax;
    local6 = param3;
    local7 = param2;
    if (eax == 0) {
        edx = local6;
        ecx = *(ebx + 80);
        al = *(ebp + 8);
        eax = eax >> 8 & 0xffffff | (al);
        *(unsigned char*)(ebx + ecx) = al;
        *(__size32*)(ebx + 80)++;
        local7 = ecx;
    }
    else {
bb0x403a83:
        edx = *eax;
        local6 = edx;
        if (edx > *(ebx + 92)) {
            goto bb0x403a83;
        }
    }
    ecx = local7;
    *(__size32*)(ebx + 92)++;
    ebx = *esp;
    ebp = *(esp + 4);
    return eax; /* WARNING: Also returning: al := al, ecx := ecx, edx := edx, ebx := ebx, ebp := ebp */
}

/** address: 0x00403a20 */
__size32 proc_0x00403a20(__size32 param1, __size32 param2)
{
    int eax; 		// r24
    __size32 ebp; 		// r29
    __size32 ebx; 		// r27
    unsigned int ecx; 		// r25
    union { unsigned int *; int; } edx; 		// r26
    __size32 esp; 		// r28
    __size32 esp_1; 		// r28{0}
    __size32 esp_4; 		// r28{0}
    __size32 local8; 		// param2{0}
    union { __size32 *; __size32; } local9; 		// esp{0}

    esp_1 = esp_4 - 8;
    eax = 1;
    local8 = param2;
    local9 = esp_1;
    if (*(param1 + 80) != 0) {
        edx = *(param1 + 100);
        flags = LOGICALFLAGS32(edx);
        if (edx != 0) {
            ecx = *edx;
            tmp1 = ecx - *(param1 + 92);
            flags = SUBFLAGS32(ecx, *(param1 + 92), tmp1);
            if (ecx <= *(param1 + 92)) {
                eax = 0;
                flags = LOGICALFLAGS32(0);
            }
        }
        edx = *(param1 + 88);
        ecx = *(param1 + 80);
        (**(param1 + 84))(pc, param1, ecx, edx, eax, ebx, ebp, param1, eax, ecx, edx, param1, esp_4 - 4, flags, ZF, CF);
        local8 = edx;
        esp += 16;
        local9 = esp;
        if (eax == 0) {
            *(__size32*)(ebx + 96) = 1;
        }
        eax = 0;
        *(int*)(ebx + 80) = 0;
    }
    param2 = local8;
    esp = local9;
    ebx = *esp;
    ebp = *(esp + 4);
    return eax; /* WARNING: Also returning: edx := param2, ebx := ebx, ebp := ebp */
}

/** address: 0x00405090 */
void proc_0x00405090(unsigned long long param1, int param2, union { int *; __size32; } param3, unsigned long long param4, unsigned int param5, int param6)
{
    unsigned int al; 		// r8
    unsigned long long eax; 		// r24
    unsigned long long eax_2; 		// r24{0}
    long long eax_3; 		// r24{0}
    int eax_6; 		// r24{0}
    unsigned long long eax_9; 		// r24{0}
    int ebx; 		// r27
    int ebx_1; 		// r27{0}
    int ebx_2; 		// r27{0}
    int edx; 		// r26
    __size32 esi; 		// r30
    int esp; 		// r28
    unsigned long long local0; 		// m[esp + 4]
    int local1; 		// m[esp + 8]
    unsigned long long local11; 		// param1{0}
    int local12; 		// param2{0}
    unsigned long long local13; 		// local0{0}
    int local14; 		// local1{0}
    unsigned long long local15; 		// eax_9{0}
    int local16; 		// ebx_1{0}
    unsigned long long local4; 		// m[esp + 4]{0}
    __size8 local8; 		// m[esi]{0}
    int local9; 		// m[esi]{0}

    esi = param3;
    local11 = param1;
    local11 = param1;
    local11 = param1;
    local12 = param2;
    local12 = param2;
    local12 = param2;
    if ( ~((long long)param4 < 2 || (long long)param4 > 36)) {
        if (param2 != 0) {
            if (param2 < 0) {
                if (param5 != 0) {
                    *(int*)param3 = 45;
                    esi = param3 + 1;
                    local4 = 0 - param1;
                    local1 = 0 - param2 + ((unsigned int)0 < param1);
                    local11 = local4;
                    local12 = local1;
                }
            }
        }
        else {
            if (param1 < (unsigned long long)0) {
bb0x4050bd:
                goto bb0x4050bd;
            }
        }
        param1 = local11;
        param2 = local12;
        ebx = (esp - 72);
        local13 = param1;
        local14 = param2;
        do {
            local0 = local13;
            local1 = local14;
            proc_0x00404d5a(param4,  ((long long)param4 < 0) ? -1 : 0, local1);
            *(__size8*)ebx = (unsigned char) local0;
            ebx++;
            eax_2 = proc_0x00404c97(param4,  ((long long)param4 < 0) ? -1 : 0, local0, local1); /* Warning: also results in edx */
            local15 = eax_2;
            local16 = ebx;
            local0 = eax_2;
            local1 = edx;
            local13 = local0;
            local14 = local1;
        } while (edx != 0 || eax_2 != 0);
        eax_9 = local15;
        ebx_1 = local16;
        while (ebx_1 != esp - 72) {
            ebx_2 = ebx_1 - 1;
            al = *(ebx_1 - 1);
            eax_6 = eax_9 >> 8 & 0xffffff | (al);
            local16 = ebx_2;
            local16 = ebx_2;
            if ((int)al < 10) {
bb0x40511d:
                eax_3 = eax_6 + 48;
                local8 = (unsigned char) eax_6 + 48;
                esi++;
                local15 = eax_3;
                goto bb0x40511d;
            }
            al = (al + param6);
            eax = (eax_6 >> 8 & 0xffffff | (al)) >> 8 & 0xffffff | (al - 10);
            local9 = al - 10;
            esi++;
            local15 = eax;
            eax_9 = local15;
            ebx_1 = local16;
        }
    }
    *(int*)esi = 0;
    return;
}

/** address: 0x00402250 */
void proc_0x00402250(union { int; char *; } param1)
{
    union { char *; int * x590; int; } eax; 		// r24
    int edx; 		// r26

bb0x402250:
    eax = param1;
    tmpb = (unsigned char) param1 & 0x3;
    if (((unsigned char) param1 & 0x3) != 0) {
        tmpb = *param1 & 0xff;
        if (tmpb == 0) {
        }
        else {
bb0x40228e:
            tmpb = *(param1 + 1) & 0xff;
            if (tmpb == 0) {
                goto bb0x40228e;
            }
            else {
bb0x402294:
                tmpb = *(param1 + 2) & 0xff;
                if (tmpb == 0) {
                    goto bb0x402294;
                }
                else {
                    eax = param1 + 3 >> 8 & 0xffffff | ((unsigned char) param1 + 3 & ~0x3);
                    do {
bb0x402258:
                        edx = *eax;
                        eax += 4;
                        if ((edx - 0x1010101 & 0x80808080) == 0) {
                            goto bb0x402258;
                        }
                        else {
                            edx = edx - 0x1010101 & 0x80808080 &  ~edx;
                        }
                    } while (edx == 0);
bb0x402273:
                    if ((unsigned char) edx != 0) {
                        goto bb0x402273;
                    }
                    else {
bb0x402277:
                        if ((edx >> 8 & 0xff) != 0) {
                            goto bb0x402277;
                        }
                        else {
bb0x40227b:
                            if ((edx & 0xff0000) != 0) {
                                goto bb0x40227b;
                            }
                            else {
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        goto bb0x402250;
    }
    return;
}

/** address: 0x0040487c */
void proc_0x0040487c(union { int; unsigned int *; } param1, unsigned int param2)
{
    __size32 eax; 		// r24

bb0x40487c:
    if (param1 != 0) {
        if (*(global54_40b59c + 8) == 0) {
            eax = WideCharToMultiByte();
            if (eax == 0) {
            }
            else {
            }
        }
        else {
            if (param2 <= (unsigned int)255) {
                *(unsigned int*)param1 = param2;
            }
            else {
            }
        }
    }
    else {
        goto bb0x40487c;
    }
    return;
}

/** address: 0x00403a9c */
void proc_0x00403a9c(unsigned int param1, __size32 param2)
{
    unsigned char dl; 		// r10
    __size32 eax; 		// r24
    int ecx; 		// r25
    int ecx_1; 		// r25{0}
    unsigned int esi; 		// r30

    ecx = 7;
    esi = param1;
    eax = param2 + 7;
    do {
        ecx_1 = ecx;
        if ((esi & 0xf) >= 10) {
            dl = (unsigned char) (esi & 0xf) + 55;
            *(unsigned char*)eax = (unsigned char) (esi & 0xf) + 55;
        }
        else {
            dl = (unsigned char) (esi & 0xf) + 48;
            *(unsigned char*)eax = (unsigned char) (esi & 0xf) + 48;
        }
        esi = esi / 16;
        ecx = ecx_1 - 1;
        eax = eax - 1;
    } while (ecx_1 - 1 >= 0);
    return;
}

/** address: 0x00404fc8 */
void proc_0x00404fc8()
{
/* goto m[0x40b780] */
}

/** address: 0x00404fce */
void proc_0x00404fce()
{
/* goto m[0x40b784] */
}

/** address: 0x00404d5a */
void proc_0x00404d5a(unsigned long long param1, union { unsigned int; bool; } param2, unsigned int param3)
{
    if (param2 == 0 && (param3 == 0 || param1 == 0)) {
    }
    else {
        if ( ~(ROTLC(0) < param2 || ROTLC(0) <= param2 && ROTLC(0) < param1)) {
        }
    }
    return;
}

/** address: 0x00404c97 */
__size32 proc_0x00404c97(unsigned long long param1, unsigned long long param2, unsigned long long param3, unsigned int param4)
{
    unsigned long long eax; 		// r24
    int edx; 		// r26

    if (param2 == 0 && (param4 == 0 || param1 == 0)) {
        eax = ((param4) << 32 | (param3)) / param1;
        edx = 0;
    }
    else {
        eax = param3 * 2;
        edx = ROTLC(param4);
        if ( ~(ROTLC(0) < param2 || ROTLC(0) <= param2 && ROTLC(0) < param1)) {
            eax = param3 * 2 + 1;
        }
    }
    return eax; /* WARNING: Also returning: edx := edx */
}

