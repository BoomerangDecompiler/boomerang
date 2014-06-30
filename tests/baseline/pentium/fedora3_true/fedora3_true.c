__size32 proc2();
void proc3();
void proc4(FILE *param1, __size32 param2, __size32 param3);
void proc5(int param1);
void proc6(FILE *param1, __size32 param2, __size32 *param3);

// address: 0x8048c4a
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    __size32 ebx; 		// r27
    int ecx; 		// r25
    __size8 *edi; 		// r31
    __size32 edx; 		// r26
    __size8 *esi; 		// r30

    ebx = proc2();
    edx = *argv;
    eax = *(ebx + 0x2ce3);
    *(__size32*)eax = edx;
    setlocale(6, ebx + 0x15d7);
    bindtextdomain(ebx + 0x1638, ebx + 0x15ee);
    textdomain(ebx + 0x1638);
    proc3();
    if (argc == 2) {
        getenv(ebx + 0x1600);
        flags = LOGICALFLAGS32(eax);
        if (eax == 0) {
            ecx = *(argv + 4);
            edi = ebx + 0x1610;
            esi = ecx;
            ecx = eax >> 8 & 0xffffff | 7;
            do {
                if (ecx == 0) {
                    goto L7;
                }
                tmpb = *esi - *edi;
                flags = SUBFLAGS8(*esi, *edi, tmpb);
                esi +=  (DF == 0) ? 1 : -1;
                edi +=  (DF == 0) ? 1 : -1;
                ecx = ecx - 1;
            } while (tmpb == 0);
L7:
            if (flags) {
                proc5(0);
            }
            do {
            } while (ecx != 0 && ZF);
            if ( !flags) {
                *(__size32*)(esp + 20) = 0;
                *(__size32*)(esp + 16) = eax;
                *(__size32*)(esp + 12) = eax;
                *(__size32*)(esp + 8) = eax;
                *(__size32*)(esp + 4) = eax;
                *(__size32*)esp = eax;
                proc4(*(esp + 4), *(esp + 8), *(esp + 20));
            }
        }
    }
    *(__size32*)esp = 0;
    exit(*(esp + 4));
    return;
}

// address: 0x8048d64
__size32 proc2() {
    __size32 local0; 		// m[esp]

    return local0;
}

// address: 0x804a0a0
void proc3() {
    __size32 edx; 		// r26

    if (edx != 0) {
    }
    __cxa_atexit();
    return;
}

// address: 0x8049ccd
void proc4(FILE *param1, __size32 param2, __size32 param3) {
    proc2();
    proc6(param1, param2, &param3);
    return;
}

// address: 0x8048b68
void proc5(int param1) {
    __size32 eax; 		// r24
    __size32 ebx; 		// r27
    int edx; 		// r26
    FILE **esi; 		// r30

    ebx = proc2();
    dcgettext(0, ebx + 0x15a6, 5);
    printf(eax);
    dcgettext(0, ebx + 0x163e, 5);
    esi = *(ebx + 0x2dce);
    edx = *esi;
    fputs_unlocked(eax, edx);
    dcgettext(0, ebx + 0x166e, 5);
    edx = *esi;
    fputs_unlocked(eax, edx);
    dcgettext(0, ebx + 0x16a4, 5);
    printf(eax);
    exit(param1);
    return;
}

// address: 0x8049ac0
void proc6(FILE *param1, __size32 param2, __size32 *param3) {
    union { unsigned int x3; __size8 * x4; } eax; 		// r24
    __size32 *eax_1; 		// r24{28}
    int ebx; 		// r27
    __size32 edx; 		// r26
    unsigned int esi; 		// r30

    esi = 0;
    proc2();
    edx = param3 + 4;
    eax_1 = param3;
    eax = *eax_1;
    while (eax != 0) {
        eax_1 = edx;
        esi++;
        edx++;
        eax = *eax_1;
    }
    if (param2 == 0) {
        fprintf(param1, ebx + 0xd16);
    } else {
        fprintf(param1, ebx + 0xcc7);
    }
    if (esi <= 9) {
/* goto (m[(ebx + (esi * 4)) + 0xd1f] + ebx) + 0x1e7f*/
    }
    dcgettext(0, ebx + 0xaf7, 5);
    vfprintf();
    eax = *(param1 + 20);
    if (eax >= *(param1 + 24)) {
        __overflow();
    } else {
        *(__size8*)eax = 10;
        *(__size32*)(param1 + 20)++;
    }
    eax = *(ebx + 0x1e63);
    eax = *eax;
    fputs_unlocked(eax, param1);
    eax = *(param1 + 20);
    if (eax >= *(param1 + 24)) {
        __overflow();
    } else {
        *(__size8*)eax = 10;
        *(__size32*)(param1 + 20)++;
    }
    dcgettext(0, ebx + 0xb33, 5);
    fputs_unlocked(eax, param1);
    return;
}

