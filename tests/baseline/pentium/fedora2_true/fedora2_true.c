__size32 global4;// 4 bytes

void proc2();
void proc3(char *param1, FILE *param2, char param3[], char param4[], char param5[], __size32 param6);
void proc4(int param1);
void proc5(char *param1, FILE *param2, char param3[], char param4[], char param5[], __size32 *param6);

// address: 0x8048b10
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    int ecx; 		// r25
    __size8 *edi; 		// r31
    __size8 *esi; 		// r30

    eax = *argv;
    global4 = eax;
    setlocale(6, "");
    bindtextdomain("coreutils", "/usr/share/locale");
    textdomain("coreutils");
    proc2();
    if (argc == 2) {
        getenv("POSIXLY_CORRECT");
        flags = LOGICALFLAGS32(eax);
        if (eax == 0) {
            eax = *(argv + 4);
            edi = 0x804a055;
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
                proc4(0);
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
                proc3(*(esp - 72), *(esp + 4), *(esp + 8), *(esp + 12), *(esp + 16), *(esp + 20));
            }
        }
    }
    *(__size32*)esp = 0;
    exit(*(esp + 4));
    return;
}

// address: 8049e90
void proc2() {
    __size32 eax; 		// r24

    if (eax != 0) {
    }
    __cxa_atexit();
    return;
}

// address: 0x8049ac0
void proc3(char *param1, FILE *param2, char param3[], char param4[], char param5[], __size32 param6) {
    proc5(param1, param2, param3, param4, param5, &param6);
    return;
}

// address: 0x8048a30
void proc4(int param1) {
    __size32 eax; 		// r24
    int edx; 		// r26

    dcgettext(0, "Usage: %s [ignored command line arguments]\n  or:  %s OPTION\nExit with a status code indicating success.\n\nThese option names may not be abbreviated.\n\n", 5);
    printf(eax);
    dcgettext(0, "      --help     display this help and exit\n", 5);
    edx = *0x804b7e0;
    fputs_unlocked(eax, edx);
    dcgettext(0, "      --version  output version information and exit\n", 5);
    edx = *0x804b7e0;
    fputs_unlocked(eax, edx);
    dcgettext(0, "\nReport bugs to <%s>.\n", 5);
    printf(eax);
    exit(param1);
    return;
}

// address: 0x80498b0
void proc5(char *param1, FILE *param2, char param3[], char param4[], char param5[], __size32 *param6) {
    __size32 eax; 		// r24
    __size32 ebx; 		// r27

    eax = *param6;
    if (eax != 0) {
        do {
            eax = *(param6 + 4);
        } while (eax != 0);
    }
    if (param3 == 0) {
        fprintf(param2, "%s %s\n", param4, param5);
    } else {
        fprintf(param2, "%s (%s) %s\n", param3, param4, param5);
    }
    switch(ebx) {
    case 0:
    case 1:
L16:
L15:
        dcgettext(0, param1, 5);
        vfprintf();
        eax = *(param2 + 20);
        if (eax < *(param2 + 24)) {
            *(__size8*)eax = 10;
            *(__size32*)(param2 + 20)++;
L9:
            eax = *0x804b7d0;
            fputs_unlocked(eax, param2);
            eax = *(param2 + 20);
            if (eax < *(param2 + 24)) {
                *(__size8*)eax = 10;
                *(__size32*)(param2 + 20)++;
L4:
                dcgettext(0, "This is free software; see the source for copying conditions.  There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n", 5);
                fputs_unlocked(eax, param2);
                return;
            }
            __overflow();
            goto L4;
        }
        __overflow();
        goto L9;
    case 2:
        goto L15;
    case 3:
        goto L16;
    case 4:
        goto L16;
    case 5:
        goto L16;
    case 6:
        goto L15;
    case 7:
        goto L15;
    case 8:
        goto L16;
    case 9:
        goto L16;
    }
    abort();
    return;
}

