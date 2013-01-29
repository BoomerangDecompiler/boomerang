unsigned int global2;

void proc1(__size32 param1);
__size32 proc2(__size32 param1, __size32 *param2, __size32 param3);
void proc4();
void proc6(unsigned int param1, unsigned int param2, __size32 param3);

// address: 401000
void _start(unsigned short param1) {
    unsigned int eax; 		// r24
    __size32 ebp; 		// r29
    __size32 esp; 		// r28

    if (global2 != 0) {
    }
    eax = ((unsigned short) (param1) & 0xfffff0c0);
    proc1(pc, 0x401080, (unsigned short) eax | 831, ebp, (unsigned short) eax | 831, eax | 831, esp - 4, (unsigned short) eax | 831, LOGICALFLAGS32(eax | 831), LOGICALFLAGS32(eax | 831), LOGICALFLAGS32(eax | 831));
    return;
}

// address: 401470
void proc1(__size32 param1) {
    __size32 eax; 		// r24
    void *esp; 		// r28
    void *esp_1; 		// r28{17}
    union { void * x7; int x8; } esp_2; 		// r28{26}
    union { void * x7; int x8; } esp_3; 		// r28{56}
    union { void * x7; int x8; } esp_4; 		// r28{39}
    union { void * x7; int x8; } esp_5; 		// r28{47}
    __size32 local0; 		// m[esp - 28]
    union { void * x7; int x8; } local3; 		// esp_5{47}
    union { void * x7; int x8; } local4; 		// esp_3{56}

L-1:
    local0 = param1;
    eax = proc2(param1, 0, (esp - 4));
L-1:
    esp_1 = esp - 28;
    local3 = esp_1;
    local4 = esp_1;
    if (eax == 0) {
L-1:
        esp_3 = local4;
        *(__size32*)(esp_3 - 168) = 0;
        *(union { void * x7; int x8; }*)(esp_3 - 172) = esp_3 - 168;
        *(__size32*)(esp_3 - 176) = param1;
        proc2(*(esp_3 - 176), *(esp_3 - 172), esp - 4);
L-1:
        esp_4 = esp_3 - 176;
        *(union { void * x7; int x8; }*)(esp_3 - 176) = esp_3 - 168;
        local3 = esp_4;
L-1:
        esp_5 = local3;
        dll_crt0__FP11per_process();
        local4 = esp_2;
        goto L-1;
    }
L-1:
    local0 = 0;
    goto L-1;
}

// address: 401530
__size32 proc2(__size32 param1, __size32 *param2, __size32 param3) {
    __size32 eax; 		// r24
    __size32 *eax_1; 		// r24{76}
    __size32 *ebx; 		// r27

    eax = 0;
    ebx = param2;
    if (param2 == 0) {
        cygwin_internal();
        eax = 0;
        if (eax_1 != -1) {
            ebx = eax_1;
            eax = 1;
L6:
            *(__size32*)(ebx + 4) = 168;
            *(__size32*)(ebx + 8) = 1005;
            *(__size32*)(ebx + 12) = 9;
            *(__size32*)(ebx + 128) = 0;
            *(__size32*)(ebx + 132) = 112;
            *(__size32*)(ebx + 44) = 0x4017a0;
            *(__size32*)(ebx + 48) = 0x4017ac;
            *(__size32*)(ebx + 20) = 0x403020;
            if (eax == 0) {
                *(__size32*)(ebx + 16) = 0x403024;
            } else {
                eax = *(ebx + 164);
                *(__size32*)0x403024 = eax;
            }
            *(__size32*)(ebx + 120) = 0;
            *(__size32*)(ebx + 72) = 0x401730;
            *(__size32*)(ebx + 76) = 0x401720;
            *(__size32*)(ebx + 40) = param1;
            *(__size32*)(ebx + 80) = 0x401710;
            *(__size32*)(ebx + 84) = 0x401700;
            *(__size32*)(ebx + 36) = 0x403028;
            *(__size32*)ebx = param3;
            *(__size32*)(ebx + 24) = 0x401510;
            *(__size32*)(ebx + 28) = 0x4014f0;
            *(__size32*)(ebx + 32) = 0x4016f0;
            *(__size32*)(ebx + 68) = 0x4016e0;
            GetModuleHandleA();
            *(__size32*)(ebx + 124) = eax;
            *(__size32*)(ebx + 52) = 0x402000;
            *(__size32*)(ebx + 56) = 0x402010;
            *(__size32*)(ebx + 60) = 0x403000;
            *(__size32*)(ebx + 64) = 0x403080;
            proc4();
            eax = 1;
        }
    } else {
        goto L6;
    }
    return eax;
}

// address: 4016b0
void proc4() {
    proc6(0x403000, 0x403000, 0x400000);
    return;
}

// address: 401670
void proc6(unsigned int param1, unsigned int param2, __size32 param3) {
    __size32 eax; 		// r24
    __size32 eax_1; 		// r24{16}
    union { __size32 * x3; unsigned int x4; } ecx; 		// r25
    union { __size32 * x3; unsigned int x4; } ecx_1; 		// r25{41}

    ecx = param1;
    if (param1 < param2) {
        do {
            ecx_1 = ecx;
            eax_1 = *(ecx_1 + 4);
            eax = *ecx_1;
            ecx = ecx_1 + 8;
            *(__size32*)(param3 + eax_1) += eax;
        } while (ecx_1 + 8 < param2);
    }
    return;
}

