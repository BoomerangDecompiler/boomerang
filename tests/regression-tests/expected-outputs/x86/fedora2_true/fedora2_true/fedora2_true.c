int main(union { int; char *; } argc, char *argv[]);
void proc_0x08049e90(atexitfunc param1);
void proc_0x08049ac0(union { int x14; char *; FILE *; } param1, char param2[], char param3[], char param4[], __size32 param5);
void proc_0x08048a30(int param1);
void proc_0x080498b0(union { int x14; char *; FILE *; } param1, char param2[], char param3[], char param4[], __size32 *param5);


/** address: 0x08048b10 */
int main(union { int; char *; } argc, char *argv[])
{
    char *eax; 		// r24
    char * *ebx; 		// r27
    int ecx; 		// r25
    __size32 ecx_1; 		// r25{4}
    __size32 ecx_2; 		// r25{10}
    __size32 ecx_4; 		// r25{12}
    __size32 ecx_5; 		// r25{18}
    unsigned int *edi; 		// r31
    unsigned int *edi_1; 		// r31{6}
    unsigned int *edi_2; 		// r31{9}
    unsigned int *edi_4; 		// r31{14}
    unsigned int *edi_5; 		// r31{17}
    unsigned int *esi; 		// r30
    unsigned int *esi_1; 		// r30{5}
    unsigned int *esi_2; 		// r30{8}
    unsigned int *esi_4; 		// r30{13}
    unsigned int *esi_5; 		// r30{16}
    int esp; 		// r28
    union { int x14; char *; FILE *; } *esp_1; 		// r28{28}
    union { int x14; char *; FILE *; } *esp_4; 		// r28{34}
    void *esp_7; 		// r28{20}
    unsigned int *local10; 		// edi_4{14}
    union { int x14; char *; FILE *; } *local11; 		// esp{30}
    union { int x14; char *; FILE *; } *local12; 		// esp{40}
    __size32 local5; 		// ecx_1{4}
    unsigned int *local6; 		// esi_1{5}
    unsigned int *local7; 		// edi_1{6}
    __size32 local8; 		// ecx_4{12}
    unsigned int *local9; 		// esi_4{13}

    eax = *argv;
    *(char *[]*)0x804b928 = eax;
    setlocale(6, "");
    bindtextdomain("coreutils", "/usr/share/locale");
    textdomain("coreutils");
    esp_1 = proc_0x08049e90(0x8048c20);
    local11 = esp_1;
    if (argc == 2) {
        eax = getenv("POSIXLY_CORRECT"); /* Warning: also results in esp_4 */
        local12 = esp_4;
        local11 = esp_4;
        if (eax == 0) {
            eax = *(argv + 4);
            edi = 0x804a055;
            local7 = edi;
            esi = eax;
            local6 = esi;
            ecx = 7;
            local5 = ecx;
            do {
                edi_1 = local7;
                esi_1 = local6;
                ecx_1 = local5;
                tmpb = *esi_1 - *edi_1;
                esi_2 = esi_1 + ( (DF == 0) ? 1 : -1);
                local6 = esi_2;
                edi_2 = edi_1 + ( (DF == 0) ? 1 : -1);
                local7 = edi_2;
                ecx_2 = ecx_1 - 1;
                local5 = ecx_2;
            } while (ecx_1 != 1 && tmpb == 0);
            if (*esi_1 == *edi_1) {
                esp = (esp_7 - 48);
                local12 = esp;
                proc_0x08048a30(0);
                ebx = *(ebx + 4);
                *(char ***)(ebp - 16) = ebx;
            }
            esp = local12;
            local11 = esp;
            esi = eax;
            local9 = esi;
            edi = 0x804a05c;
            local10 = edi;
            ecx = 10;
            local8 = ecx;
            do {
                edi_4 = local10;
                esi_4 = local9;
                ecx_4 = local8;
                tmpb = *esi_4 - *edi_4;
                esi_5 = esi_4 + ( (DF == 0) ? 1 : -1);
                local9 = esi_5;
                edi_5 = edi_4 + ( (DF == 0) ? 1 : -1);
                local10 = edi_5;
                ecx_5 = ecx_4 - 1;
                local8 = ecx_5;
            } while (ecx_4 != 1 && tmpb == 0);
            if (*esi_4 == *edi_4) {
                *(__size32*)(esp + 20) = 0;
                *(__size32*)(esp + 16) = 0x804a066;
                eax = *0x804b7e0;
                *(__size32*)(esp + 12) = 0x804a073;
                *(__size32*)(esp + 8) = 0x804a079;
                *(__size32*)(esp + 4) = 0x804a087;
                *(union { int x14; char *; FILE *; }*)esp = eax;
                esp = proc_0x08049ac0(*esp, *(esp + 4), *(esp + 8), *(esp + 12), *(esp + 16));
                local11 = esp;
            }
        }
    }
    esp = local11;
    *(union { int x14; char *; FILE *; }*)esp = 0;
    exit(*esp);
    return;
}

/** address: 0x08049e90 */
void proc_0x08049e90(atexitfunc param1)
{
    void **eax_1; 		// r24{4}
    int edx; 		// r26

    edx = 0;
    if (eax_1 != 0) {
        edx = *eax_1;
    }
    __cxa_atexit(param1, 0, edx);
    return;
}

/** address: 0x08049ac0 */
void proc_0x08049ac0(union { int x14; char *; FILE *; } param1, char param2[], char param3[], char param4[], __size32 param5)
{
    proc_0x080498b0(param1, param2, param3, param4, &param5);
    return;
}

/** address: 0x08048a30 */
void proc_0x08048a30(int param1)
{
    __size32 eax; 		// r24
    char *eax_1; 		// r24{4}
    int edx; 		// r26

    eax_1 = dcgettext(0, "Usage: %s [ignored command line arguments]\n  or:  %s OPTION\nExit with a status code indicating success.\n\nThese option names may not be abbreviated.\n\n", 5);
    printf(eax_1);
    eax = dcgettext(0, "      --help     display this help and exit\n", 5);
    edx = *0x804b7e0;
    fputs_unlocked(eax, edx);
    eax = dcgettext(0, "      --version  output version information and exit\n", 5);
    edx = *0x804b7e0;
    fputs_unlocked(eax, edx);
    eax = dcgettext(0, "\nReport bugs to <%s>.\n", 5);
    printf(eax);
    exit(param1);
    return;
}

/** address: 0x080498b0 */
void proc_0x080498b0(union { int x14; char *; FILE *; } param1, char param2[], char param3[], char param4[], __size32 *param5)
{
    int eax; 		// r24
    unsigned int ebx; 		// r27
    int edx; 		// r26
    union { int; char *; } local0; 		// m[esp - 40]

    ebx = 0;
    edx = param5 + 4;
    eax = *param5;
    if (eax != 0) {
        do {
            ebx++;
            eax = *edx;
            edx++;
        } while (eax != 0);
    }
    if (param2 == 0) {
        fprintf(param1, "%s %s\n", param3, param4);
    }
    else {
        fprintf(param1, "%s (%s) %s\n", param2, param3, param4);
    }
    if (ebx > 9) {
        eax = 0x804a574;
bb0x8049940:
        local0 = eax;
        break;
    }
    else {
        switch(ebx) {
        case 0:
            abort();
        case 1:
            eax = 0x804a5bc;
            goto bb0x8049940;
        case 2:
            local0 = 0x804a5cc;
            break;
        case 3:
            eax = 0x804a5e3;
            goto bb0x8049940;
        case 4:
            eax = 0x804a478;
            goto bb0x8049940;
        case 5:
            eax = 0x804a498;
            goto bb0x8049940;
        case 6:
            local0 = 0x804a4bc;
            break;
        case 7:
            local0 = 0x804a4e4;
            break;
        case 8:
            eax = 0x804a510;
            goto bb0x8049940;
        case 9:
            eax = 0x804a540;
            goto bb0x8049940;
        }
    }
    %eax = dcgettext(0, local0, 5);
    vfprintf(param1, %eax, param5);
    eax = *(param1 + 20);
    if (eax >= *(param1 + 24)) {
        __overflow();
    }
    else {
        *(__size8*)eax = 10;
        *(__size32*)(param1 + 20)++;
    }
    eax = *0x804b7d0;
    fputs_unlocked(eax, param1);
    eax = *(param1 + 20);
    if (eax >= *(param1 + 24)) {
        __overflow();
    }
    else {
        *(__size8*)eax = 10;
        *(__size32*)(param1 + 20)++;
    }
    %eax = dcgettext(0, "This is free software; see the source for copying conditions.  There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n", 5);
    fputs_unlocked(%eax, param1);
    return;
}

