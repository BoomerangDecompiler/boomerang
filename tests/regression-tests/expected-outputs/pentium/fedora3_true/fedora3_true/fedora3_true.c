int main(int argc, char *argv[]);
__size32 proc_0x08048d64();
void proc_0x0804a0a0(union { atexitfunc; __size32; } param1);
void proc_0x08049ccd(__size32 param1, __size32 param2);
void proc_0x08048b68(int param1);
void proc_0x08049ac0(union { FILE *; __size32; } param1, __size32 param2, union { va_list; __size32; } param3);

/** address: 0x08048c4a */
int main(int argc, char *argv[])
{
    char *eax; 		// r24
    __size32 ebx; 		// r27
    int ecx; 		// r25
    __size32 edi; 		// r31
    int edx; 		// r26
    __size32 esi; 		// r30

    ebx = proc_0x08048d64();
    edx = *argv;
    eax = *(ebx + 0x2ce3);
    *(char *[]*)eax = edx;
    setlocale(6, ebx + 0x15d7);
    bindtextdomain(ebx + 0x1638, ebx + 0x15ee);
    textdomain(ebx + 0x1638);
    eax = *(ebx + 0x2cd7);
    proc_0x0804a0a0(eax);
    if (argc == 2) {
        eax = getenv(ebx + 0x1600);
        flags = LOGICALFLAGS32(eax);
        if (eax == 0) {
            ecx = *(argv + 4);
            edi = ebx + 0x1610;
            esi = ecx;
            ecx = eax & ~0xff | 7;
            if ((eax & ~0xff | 7) != 0) {
                do {
                    tmpb = *esi - *edi;
                    flags = SUBFLAGS8(*esi, *edi, tmpb);
                    esi +=  (DF == 0) ? 1 : -1;
                    edi +=  (DF == 0) ? 1 : -1;
                    ecx--;
                } while (tmpb == 0);
            }
            if (flags) {
                proc_0x08048b68(0);
            }
            if (%ecx != 0) {
                do {
                } while (ZF);
            }
            if ( ~flags) {
                *(__size32*)(%esp + 20) = 0;
                *(__size32*)(%esp + 16) = %eax;
                *(__size32*)(%esp + 12) = %eax;
                *(__size32*)(%esp + 8) = %eax;
                *(__size32*)(%esp + 4) = %eax;
                *(__size32*)%esp = %eax;
                proc_0x08049ccd(*(%esp + 4), *(%esp + 8));
            }
        }
    }
    *(__size32*)%esp = 0;
    exit(*(%esp + 4));
    return;
}

/** address: 0x08048d64 */
__size32 proc_0x08048d64()
{
    __size32 local0; 		// m[esp]

    return local0;
}

/** address: 0x0804a0a0 */
void proc_0x0804a0a0(union { atexitfunc; __size32; } param1)
{
    int eax; 		// r24
    int edx; 		// r26

    eax = 0;
    if (edx != 0) {
        eax = *edx;
    }
    __cxa_atexit(param1, 0, eax);
    return;
}

/** address: 0x08049ccd */
void proc_0x08049ccd(__size32 param1, __size32 param2)
{
    __size32 esp; 		// r28

    proc_0x08048d64();
    proc_0x08049ac0(param1, param2, (esp + 20));
    return;
}

/** address: 0x08048b68 */
void proc_0x08048b68(int param1)
{
    __size32 eax; 		// r24
    char *eax_1; 		// r24{4}
    __size32 ebx; 		// r27
    int edx; 		// r26
    union { __size32; __size32 *; } esi; 		// r30

    ebx = proc_0x08048d64();
    eax_1 = dcgettext(0, ebx + 0x15a6, 5);
    printf(eax_1);
    eax = dcgettext(0, ebx + 0x163e, 5);
    esi = *(ebx + 0x2dce);
    edx = *esi;
    fputs_unlocked(eax, edx);
    eax = dcgettext(0, ebx + 0x166e, 5);
    edx = *esi;
    fputs_unlocked(eax, edx);
    eax = dcgettext(0, ebx + 0x16a4, 5);
    printf(eax);
    exit(param1);
    return;
}

/** address: 0x08049ac0 */
void proc_0x08049ac0(union { FILE *; __size32; } param1, __size32 param2, union { va_list; __size32; } param3)
{
    __size32 eax; 		// r24
    int ebx; 		// r27
    int edx; 		// r26
    unsigned int esi; 		// r30

    esi = 0;
    proc_0x08048d64();
    edx = param3 + 4;
    eax = param3;
    eax = *eax;
    while (eax != 0) {
        eax = edx;
        esi++;
        edx += 4;
        eax = *eax;
    }
    if (param2 == 0) {
        fprintf(param1, ebx + 0xd16);
    }
    else {
        fprintf(param1, ebx + 0xcc7);
    }
    if (esi <= 9) {
/* goto (m[(ebx + (esi * 4)) + 0xd1f] + ebx) + 0x1e7f */
    }
    %eax = dcgettext(0, ebx + 0xaf7, 5);
    vfprintf(param1, %eax, param3);
    eax = *(param1 + 20);
    if (eax >= *(param1 + 24)) {
        __overflow();
    }
    else {
        *(__size8*)eax = 10;
        *(__size32*)(param1 + 20)++;
    }
    eax = *(ebx + 0x1e63);
    eax = *eax;
    fputs_unlocked(eax, param1);
    eax = *(param1 + 20);
    if (eax >= *(param1 + 24)) {
        __overflow();
    }
    else {
        *(__size8*)eax = 10;
        *(__size32*)(param1 + 20)++;
    }
    %eax = dcgettext(0, ebx + 0xb33, 5);
    fputs_unlocked(%eax, param1);
    return;
}

