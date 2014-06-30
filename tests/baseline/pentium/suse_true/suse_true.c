__size32 program_name;// 4 bytes

void atexit();
void version_etc(FILE *param1, char param2[], char param3[], char param4[], char *param5);
void usage(int param1);
void version_etc_va(FILE *param1, char param2[], char param3[], char param4[], char **param5);
__size32 __i686.get_pc_thunk.bx();

// address: 0x8048b60
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    int ecx; 		// r25
    __size8 *edi; 		// r31
    __size8 *esi; 		// r30

    eax = *argv;
    program_name = eax;
    setlocale(6, "");
    bindtextdomain("coreutils", "/usr/share/locale");
    textdomain("coreutils");
    atexit();
    if (argc == 2) {
        getenv("POSIXLY_CORRECT");
        flags = LOGICALFLAGS32(eax);
        if (eax == 0) {
            eax = *(argv + 4);
            edi = 0x804a075;
            esi = eax;
            ecx = 7;
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
                usage(0);
            }
            do {
            } while (ecx != 0 && ZF);
            if ( !flags) {
                *(__size32*)(esp + 20) = eax;
                *(__size32*)(esp + 16) = edi;
                *(__size32*)(esp + 12) = esi;
                *(__size32*)(esp + 8) = ebx;
                *(__size32*)(esp + 4) = ecx;
                *(__size32*)esp = eax;
                version_etc(*(esp + 4), *(esp + 8), *(esp + 12), *(esp + 16), *(esp + 20));
            }
        }
    }
    *(__size32*)esp = 0;
    exit(*(esp + 4));
    return;
}

// address: 0x8049e90
void atexit() {
    int ebx; 		// r27
    __size32 edx; 		// r26

    __i686.get_pc_thunk.bx();
    edx = *(ebx + 0x189e);
    if (edx != 0) {
    }
    __cxa_atexit();
    return;
}

// address: 0x8049af0
void version_etc(FILE *param1, char param2[], char param3[], char param4[], char *param5) {
    version_etc_va(param1, param2, param3, param4, &param5);
    return;
}

// address: 0x8048a80
void usage(int param1) {
    __size32 eax; 		// r24
    int edx; 		// r26

    dcgettext(0, "Usage: %s [ignored command line arguments]\n  or:  %s OPTION\nExit with a status code indicating success.\n\nThese option names may not be abbreviated.\n\n", 5);
    printf(eax);
    dcgettext(0, "      --help     display this help and exit\n", 5);
    edx = *0x804b800;
    fputs_unlocked(eax, edx);
    dcgettext(0, "      --version  output version information and exit\n", 5);
    edx = *0x804b800;
    fputs_unlocked(eax, edx);
    dcgettext(0, "\nReport bugs to <%s>.\n", 5);
    printf(eax);
    exit(param1);
    return;
}

// address: 0x80498d0
void version_etc_va(FILE *param1, char param2[], char param3[], char param4[], char **param5) {
    union { unsigned int x3; __size8 * x4; } eax; 		// r24
    __size32 ebx; 		// r27
    char *local1; 		// m[esp - 40]

    eax = *param5;
    if (eax != 0) {
        do {
            eax = *(param5 + 4);
        } while (eax != 0);
    }
    if (param2 == 0) {
        fprintf(param1, "%s %s\n", param3, param4);
    } else {
        fprintf(param1, "%s (%s) %s\n", param2, param3, param4);
    }
    switch(ebx) {
    case 0:
        abort();
    case 1:
        eax = "Written by %s.\n";
L15:
        local1 = eax;
        break;
    case 2:
        local1 = "Written by %s and %s.\n";
        break;
    case 3:
        eax = "Written by %s, %s, and %s.\n";
        goto L15;
    case 4:
        eax = "Written by %s, %s, %s,\nand %s.\n";
        goto L15;
    case 5:
        eax = "Written by %s, %s, %s,\n%s, and %s.\n";
        goto L15;
    case 6:
        local1 = "Written by %s, %s, %s,\n%s, %s, and %s.\n";
        break;
    case 7:
        local1 = "Written by %s, %s, %s,\n%s, %s, %s, and %s.\n";
        break;
    case 8:
        eax = "Written by %s, %s, %s,\n%s, %s, %s, %s,\nand %s.\n";
        goto L15;
    case 9:
        eax = "Written by %s, %s, %s,\n%s, %s, %s, %s,\n%s, and %s.\n";
        goto L15;
    }
    dcgettext(0, local1, 5);
    vfprintf();
    eax = *(param1 + 20);
    if (eax >= *(param1 + 24)) {
        __overflow();
    } else {
        *(__size8*)eax = 10;
        *(__size32*)(param1 + 20)++;
    }
    eax = *0x804b7e8;
    fputs_unlocked(eax, param1);
    eax = *(param1 + 20);
    if (eax >= *(param1 + 24)) {
        __overflow();
    } else {
        *(__size8*)eax = 10;
        *(__size32*)(param1 + 20)++;
    }
    dcgettext(0, "This is free software; see the source for copying conditions.  There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n", 5);
    fputs_unlocked(eax, param1);
    return;
}

// address: 0x8049e88
__size32 __i686.get_pc_thunk.bx() {
    __size32 local0; 		// m[esp]

    return local0;
}

