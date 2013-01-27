__size32 program_name;// 4 bytes
__size32 stdout;// 4 bytes

void atexit(__size32 param1);
void version_etc();

// address: 0x8048b60
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    __size32 *ebx; 		// r27
    __size32 *ebx_1; 		// r27{126}
    __size32 ecx; 		// r25
    __size8 *edi; 		// r31
    __size8 *esi; 		// r30
    __size32 *esp_1; 		// r28{39}
    __size32 *esp_2; 		// r28{47}
    __size32 *esp_3; 		// r28{127}
    __size32 *esp_4; 		// r28{52}
    __size32 *local0; 		// m[esp - 20]
    __size32 *local1; 		// ebx{114}
    __size32 *local2; 		// esp_4{115}
    __size32 *local3; 		// esp_3{127}
    __size32 *local4; 		// ebx{152}

    ebx = argv;
    eax = *argv;
    program_name = eax;
    proc1();
    local1 = ebx;
    proc2();
    proc3();
    esp_1 = atexit(argv);
    local2 = esp_1;
    local3 = esp_1;
    if (argc != 2) {
        goto L0;
    }
    for(;;) {
        ebx_1 = ebx;
        esp_3 = local3;
        *(__size32*)esp_3 = 0x804a065;
        proc5();
        local1 = ebx_1;
        local2 = esp_4;
        local4 = ebx_1;
        flags = LOGICALFLAGS32(eax);
        if (eax != 0) {
            goto L0;
        }
        eax = *(ebx_1 + 4);
        edi = 0x804a075;
        esi = eax;
        ecx = 7;
        do {
            if (ecx == 0) {
                goto L10;
            }
            tmpb = *esi - *edi;
            flags = SUBFLAGS8(*esi, *edi, tmpb);
            esi +=  (DF == 0) ? 1 : -1;
            edi +=  (DF == 0) ? 1 : -1;
            ecx = ecx - 1;
        } while (tmpb == 0);
L10:
        if (flags) {
            *(__size32*)esp_3 = 0;
            proc7();
            ebx = *(ebx_1 + 4);
            local4 = ebx;
        }
        ebx = local4;
        esp_4 = esp_3;
        local0 = *(ebx_1 + 4);
        esi = local0;
        edi = 0x804a07c;
        ecx = 10;
        local1 = ebx;
        local2 = esp_4;
        do {
            if (ecx == 0) {
                goto L4;
            }
            tmpb = *esi - *edi;
            flags = SUBFLAGS8(*esi, *edi, tmpb);
            esi +=  (DF == 0) ? 1 : -1;
            edi +=  (DF == 0) ? 1 : -1;
            ecx = ecx - 1;
        } while (tmpb == 0);
L4:
        if (flags) {
            goto L0;
        }
        *(__size32*)(esp_3 + 20) = 0;
        *(__size32*)(esp_3 + 16) = 0x804a086;
        *(__size32*)(esp_3 + 12) = 0x804a093;
        *(__size32*)(esp_3 + 8) = 0x804a099;
        *(__size32*)(esp_3 + 4) = 0x804a0a7;
        *(__size32*)esp_3 = stdout;
        version_etc();
        proc6();
        return;
L0:
        ebx = local1;
        esp_4 = local2;
        *(__size32*)esp_4 = 0;
        proc4();
        local3 = esp_2;
    }
}

// address: 0x8049e90
void atexit(__size32 param1) {
    __size32 edx; 		// r26

    proc9();
    edx = *(param1 + 0x189e);
    if (edx != 0) {
    }
    proc10();
    return;
}

// address: 0x8049af0
void version_etc() {
    proc8();
    return;
}

