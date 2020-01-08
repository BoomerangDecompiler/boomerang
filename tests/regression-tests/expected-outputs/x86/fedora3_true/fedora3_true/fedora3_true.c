int main(int argc, char *argv[]);
__size32 proc_0x08048d64();
void proc_0x0804a0a0(atexitfunc param1);
void proc_0x08049ccd();
void proc_0x08048b68();
void proc_0x08049ac0();


/** address: 0x08048c4a */
int main(int argc, char *argv[])
{
    union { int; char *; } eax; 		// r24
    int ebp_1; 		// r29{23}
    int ebp_4; 		// r29{63}
    union { void * () *x44; int *; __size8 *; } ebx; 		// r27
    int ecx; 		// r25
    int ecx_2; 		// r25{5}
    int ecx_3; 		// r25{12}
    __size32 ecx_5; 		// r25{14}
    __size32 ecx_6; 		// r25{20}
    union { void * () *x91; unsigned int *; __size8 *; } edi; 		// r31
    union { void * () *x91; unsigned int *; __size8 *; } edi_1; 		// r31{16}
    union { void * () *x92; unsigned int *; __size8 *; } edi_2; 		// r31{19}
    int edx; 		// r26
    union { void * () *x152; unsigned int *; __size8 *; } esi; 		// r30
    union { void * () *x152; unsigned int *; __size8 *; } esi_1; 		// r30{15}
    union { void * () *x155; unsigned int *; __size8 *; } esi_2; 		// r30{18}
    int esp; 		// r28
    union { int; int *; } esp_1; 		// r28{32}
    union { int; int *; } esp_2; 		// r28{38}
    union { int; int *; } esp_5; 		// r28{62}
    union { int; int *; } esp_6; 		// r28{48}
    union { int; int *; } esp_7; 		// r28{0}
    union { int; int *; } local10; 		// esp_6{48}
    int local5; 		// ecx_2{5}
    __size32 local6; 		// ecx_5{14}
    union { void * () *x152; unsigned int *; __size8 *; } local7; 		// esi_1{15}
    union { void * () *x91; unsigned int *; __size8 *; } local8; 		// edi_1{16}
    union { int; int *; } local9; 		// esp{34}

    ebx = proc_0x08048d64();
    eax = *(ebp_1 + 12);
    edx = *eax;
    eax = *(ebx + 0x2ce3);
    *(__size32*)eax = edx;
    setlocale(6, ebx + 0x15d7);
    bindtextdomain(ebx + 0x1638, ebx + 0x15ee);
    textdomain(ebx + 0x1638);
    eax = *(ebx + 0x2cd7);
    esp_1 = proc_0x0804a0a0(eax);
    local9 = esp_1;
    if (*(ebp_1 + 8) == 2) {
        eax = getenv(ebx + 0x1600); /* Warning: also results in esp_2 */
        local10 = esp_2;
        local9 = esp_2;
        flags = LOGICALFLAGS32(eax);
        if (eax == 0) {
            ecx = *(ebp_1 + 12);
            ecx = *(ecx + 4);
            edi = ebx + 0x1610;
            *(union { int *; __size8 *; }*)(ebp_1 - 24) = ecx;
            esi = ecx;
            ecx = eax & ~0xff | 7;
            local5 = ecx;
            if ((eax & ~0xff | 7) != 0) {
                do {
                    ecx_2 = local5;
                    tmpb = *esi - *edi;
                    flags = SUBFLAGS8(*esi, *edi, tmpb);
                    esi +=  (DF == 0) ? 1 : -1;
                    edi +=  (DF == 0) ? 1 : -1;
                    ecx_3 = ecx_2 - 1;
                    local5 = ecx_3;
                } while (ecx_2 != 1 && tmpb == 0);
            }
            if (flags) {
                esp_5 = (esp_7 - 80);
                local10 = esp_5;
                proc_0x08048b68();
                eax = *(ebp_4 + 12);
                eax = *(eax + 4);
                *(int*)(ebp_4 - 24) = eax;
            }
            esp_6 = local10;
            local9 = esp_6;
            edi = ebx + 0x1617;
            local8 = edi;
            esi = *(esp_7 - 28);
            local7 = esi;
            ecx = 10;
            local6 = ecx;
            do {
                edi_1 = local8;
                esi_1 = local7;
                ecx_5 = local6;
                tmpb = *esi_1 - *edi_1;
                esi_2 = esi_1 + ( (DF == 0) ? 1 : -1);
                local7 = esi_2;
                edi_2 = edi_1 + ( (DF == 0) ? 1 : -1);
                local8 = edi_2;
                ecx_6 = ecx_5 - 1;
                local6 = ecx_6;
            } while (ecx_5 != 1 && tmpb == 0);
            if (*esi_1 == *edi_1) {
                *(__size32*)(esp_6 + 20) = 0;
                *(union { void * () *x57; int *; __size8 *; }*)(esp_6 + 16) = ebx + 0x1621;
                *(union { void * () *x63; int *; __size8 *; }*)(esp_6 + 12) = ebx + 0x162e;
                *(union { void * () *x69; int *; __size8 *; }*)(esp_6 + 8) = ebx + 0x1634;
                *(union { void * () *x75; int *; __size8 *; }*)(esp_6 + 4) = ebx + 0x1642;
                eax = *(ebx + 0x2ceb);
                eax = *eax;
                *(int*)esp_6 = eax;
                esp = proc_0x08049ccd();
                local9 = esp;
            }
        }
    }
    esp = local9;
    *(int*)esp = 0;
    exit(*esp);
    return;
}

/** address: 0x08048d64 */
__size32 proc_0x08048d64()
{
    __size32 local0; 		// m[esp]

    return local0;
}

/** address: 0x0804a0a0 */
void proc_0x0804a0a0(atexitfunc param1)
{
    void *eax; 		// r24
    void *eax_1; 		// r24{1}
    int edx; 		// r26
    void *local3; 		// eax{7}

    eax = 0;
    local3 = eax;
    if (edx != 0) {
        eax_1 = *edx;
        local3 = eax_1;
    }
    eax = local3;
    __cxa_atexit(param1, 0, eax);
    return;
}

/** address: 0x08049ccd */
void proc_0x08049ccd()
{
    proc_0x08048d64();
    proc_0x08049ac0();
    return;
}

/** address: 0x08048b68 */
void proc_0x08048b68()
{
    __size32 eax; 		// r24
    int eax_1; 		// r24
    int ebp; 		// r29
    __size32 ebx; 		// r27
    int edx; 		// r26
    FILE **esi; 		// r30

    ebx = proc_0x08048d64();
    eax = dcgettext(0, ebx + 0x15a6, 5);
    printf(eax);
    eax = dcgettext(0, ebx + 0x163e, 5);
    esi = *(ebx + 0x2dce);
    edx = *esi;
    fputs_unlocked(eax, edx);
    eax = dcgettext(0, ebx + 0x166e, 5);
    edx = *esi;
    fputs_unlocked(eax, edx);
    eax = dcgettext(0, ebx + 0x16a4, 5);
    printf(eax);
    eax_1 = *(ebp + 8);
    exit(eax_1);
    return;
}

/** address: 0x08049ac0 */
void proc_0x08049ac0()
{
    __size32 eax; 		// r24
    char *eax_1; 		// r24
    __size32 *eax_2; 		// r24{7}
    int ebp; 		// r29
    int ebx; 		// r27
    int ecx; 		// r25
    union { int x14; char *; FILE *; } edi; 		// r31
    int edx; 		// r26
    unsigned int esi; 		// r30
    __size32 *local4; 		// eax{8}

    esi = 0;
    proc_0x08048d64();
    edx = *(ebp + 24);
    edi = *(ebp + 8);
    ecx = *(ebp + 12);
    edx++;
    eax_2 = *(ebp + 24);
    local4 = eax_2;
    eax = local4;
    eax = *eax;
    while (eax != 0) {
        eax = edx;
        local4 = eax;
        esi++;
        edx++;
        eax = local4;
        eax = *eax;
    }
    if (ecx == 0) {
        fprintf(edi, ebx + 0xd16);
    }
    else {
        fprintf(edi, ebx + 0xcc7);
    }
    if (esi <= 9) {
/* goto (m[(ebx + (esi * 4)) + 0xd1f] + ebx) + 0x1e7f */
    }
    eax_1 = dcgettext(0, ebx + 0xaf7, 5);
    edx = *(ebp + 24);
    vfprintf(edi, eax_1, edx);
    eax_1 = *(edi + 20);
    if (eax_1 >= *(edi + 24)) {
        __overflow();
    }
    else {
        *(__size8*)eax_1 = 10;
        *(__size32*)(edi + 20)++;
    }
    eax = *(ebx + 0x1e63);
    eax_1 = *eax;
    fputs_unlocked(eax_1, edi);
    eax_1 = *(edi + 20);
    if (eax_1 >= *(edi + 24)) {
        __overflow();
    }
    else {
        *(__size8*)eax_1 = 10;
        *(__size32*)(edi + 20)++;
    }
    eax_1 = dcgettext(0, ebx + 0xb33, 5);
    fputs_unlocked(eax_1, edi);
    return;
}

