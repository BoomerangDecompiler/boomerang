int main(int argc, char *argv[]);
void atexit();
void version_etc(__size32 param1, __size32 param2, __size32 param3, __size32 param4);
void usage(int param1);
__size32 __i686.get_pc_thunk.bx();
void version_etc_va(union { FILE *; __size32; } param1, union { char[] *; __size32; } param2, union { char[] *; __size32; } param3, union { char[] *; __size32; } param4, union { __size32 *; __size32; } param5);

/** address: 0x08048b60 */
int main(int argc, char *argv[])
{
    char *eax; 		// r24
    int ecx; 		// r25
    __size32 edi; 		// r31
    __size32 esi; 		// r30

    eax = *argv;
    *(char *[]*)0x804b948 = eax;
    setlocale(6, "");
    bindtextdomain("coreutils", "/usr/share/locale");
    textdomain("coreutils");
    atexit();
    if (argc == 2) {
        eax = getenv("POSIXLY_CORRECT");
        flags = LOGICALFLAGS32(eax);
        if (eax == 0) {
            eax = *(argv + 4);
            edi = 0x804a075;
            esi = eax;
            ecx = 7;
            do {
                tmpb = *esi - *edi;
                flags = SUBFLAGS8(*esi, *edi, tmpb);
                esi +=  (DF == 0) ? 1 : -1;
                edi +=  (DF == 0) ? 1 : -1;
                ecx = ecx - 1;
            } while (tmpb == 0);
            if (flags) {
                usage(0);
            }
            if (r[25] != 0) {
                do {
                } while (ZF);
            }
            if ( ~flags) {
                *(__size32*)(r[28] + 20) = r[24];
                *(__size32*)(r[28] + 16) = r[31];
                *(__size32*)(r[28] + 12) = r[30];
                *(__size32*)(r[28] + 8) = r[27];
                *(__size32*)(r[28] + 4) = r[25];
                *(__size32*)r[28] = r[24];
                version_etc(*(r[28] + 4), *(r[28] + 8), *(r[28] + 12), *(r[28] + 16));
            }
        }
    }
    *(__size32*)r[28] = 0;
    exit(*(r[28] + 4));
    return;
}

/** address: 0x08049e90 */
void atexit()
{
    int ebx; 		// r27
    int edx; 		// r26

    __i686.get_pc_thunk.bx();
    edx = *(ebx + 0x189e);
    if (edx != 0) {
    }
    __cxa_atexit();
    return;
}

/** address: 0x08049af0 */
void version_etc(__size32 param1, __size32 param2, __size32 param3, __size32 param4)
{
    __size32 esp; 		// r28

    version_etc_va(param1, param2, param3, param4, (esp + 20));
    return;
}

/** address: 0x08048a80 */
void usage(int param1)
{
    __size32 eax; 		// r24
    char *eax_1; 		// r24{0}
    int edx; 		// r26

    eax_1 = dcgettext(0, "Usage: %s [ignored command line arguments]\n  or:  %s OPTION\nExit with a status code indicating success.\n\nThese option names may not be abbreviated.\n\n", 5);
    printf(eax_1);
    eax = dcgettext(0, "      --help     display this help and exit\n", 5);
    edx = *0x804b800;
    fputs_unlocked(eax, edx);
    eax = dcgettext(0, "      --version  output version information and exit\n", 5);
    edx = *0x804b800;
    fputs_unlocked(eax, edx);
    eax = dcgettext(0, "\nReport bugs to <%s>.\n", 5);
    printf(eax);
    exit(param1);
    return;
}

/** address: 0x08049e88 */
__size32 __i686.get_pc_thunk.bx()
{
    __size32 local0; 		// m[esp]

    return local0;
}

/** address: 0x080498d0 */
void version_etc_va(union { FILE *; __size32; } param1, union { char[] *; __size32; } param2, union { char[] *; __size32; } param3, union { char[] *; __size32; } param4, union { __size32 *; __size32; } param5)
{
    __size32 eax; 		// r24
    int ebx_1; 		// r27{0}
    __size32 ebx_2; 		// r27{0}
    int ebx_5; 		// r27{0}
    int ebx_8; 		// r27{0}
    int ebx_9; 		// r27{0}
    int edx; 		// r26
    char *local1; 		// m[esp - 40]
    int local11; 		// ebx_8{0}
    int local12; 		// ebx_1{0}
    union { char *; int; } local13; 		// local1{0}
    int local5; 		// m[esp - 40]

    ebx_5 = 0;
    edx = param5 + 4;
    eax = *param5;
    local11 = ebx_5;
    local12 = ebx_5;
    if (eax != 0) {
        do {
            ebx_8 = local11;
            ebx_9 = ebx_8 + 1;
            eax = *edx;
            edx += 4;
            local11 = ebx_9;
            local12 = ebx_9;
        } while (eax != 0);
    }
    ebx_1 = local12;
    if (param2 == 0) {
        fprintf(param1, "%s %s\n", param3, param4);
    }
    else {
        fprintf(param1, "%s (%s) %s\n", param2, param3, param4);
    }
    if ((unsigned int)ebx_1 > (unsigned int)9) {
        eax = 0x804a3f8;
bb0x8049951:
        local5 = eax;
        local13 = local5;
        goto bb0x8049955;
    }
    else {
        switch(ebx_2) {
        case 0:
            abort();
        case 1:
            eax = 0x804a607;
            goto bb0x8049951;
        case 2:
            local1 = 0x804a5f0;
            local13 = local1;
            goto bb0x8049955;
        case 3:
            eax = 0x804a5d4;
            goto bb0x8049951;
        case 4:
            eax = 0x804a5a8;
            goto bb0x8049951;
        case 5:
            eax = 0x804a584;
            goto bb0x8049951;
        case 6:
            local1 = 0x804a55c;
            local13 = local1;
            goto bb0x8049955;
        case 7:
            local1 = 0x804a530;
            local13 = local1;
            goto bb0x8049955;
        case 8:
            eax = 0x804a500;
            goto bb0x8049951;
        case 9:
            eax = 0x804a4cc;
            goto bb0x8049951;
        }
    }
bb0x8049955:
    local1 = local13;
    r[24] = dcgettext(0, local1, 5);
    vfprintf(param1, r[24], param5);
    eax = *(param1 + 20);
    if (eax >= *(param1 + 24)) {
        __overflow();
    }
    else {
        *(int*)eax = 10;
        *(__size32*)(param1 + 20)++;
    }
    eax = *0x804b7e8;
    fputs_unlocked(eax, param1);
    eax = *(param1 + 20);
    if (eax >= *(param1 + 24)) {
        __overflow();
    }
    else {
        *(int*)eax = 10;
        *(__size32*)(param1 + 20)++;
    }
    r[24] = dcgettext(0, "This is free software; see the source for copying conditions.  There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n", 5);
    fputs_unlocked(r[24], param1);
    return;
}

