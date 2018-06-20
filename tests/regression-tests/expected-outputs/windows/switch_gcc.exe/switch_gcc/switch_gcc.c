int global_0x00403024;
unsigned int global_0x00403000;
int global_0x00403024;
unsigned int global_0x00403000;
void proc_0x00401470(__size32 param1);
void _start(unsigned short param1);
__size32 proc_0x00401530(__size32 param1, __size32 param2, __size32 param3);
void proc_0x004016b0();
void proc_0x00401670(unsigned int param1, unsigned int param2, __size32 param3);

/** address: 0x00401470 */
void proc_0x00401470(__size32 param1)
{
    __size32 eax; 		// r24
    __size32 esp_1; 		// r28{0}
    __size32 esp_11; 		// r28{0}
    __size32 esp_12; 		// r28{0}
    __size32 esp_2; 		// r28{0}
    __size32 esp_5; 		// r28{0}
    __size32 esp_6; 		// r28{0}
    __size32 local0; 		// m[esp - 28]
    __size32 local3; 		// esp_11{0}
    __size32 local4; 		// esp_5{0}

    local0 = param1;
    eax = proc_0x00401530(param1, 0, (esp_12 - 4));
    esp_2 = esp_12 - 28;
    local3 = esp_2;
    local4 = esp_2;
    if (eax == 0) {
bb0x4014a3:
        esp_5 = local4;
        *(__size32*)(esp_5 - 168) = 0;
        *(__size32*)(esp_5 - 172) = esp_5 - 168;
        *(__size32*)(esp_5 - 176) = param1;
        proc_0x00401530(*(esp_5 - 176), *(esp_5 - 172), esp_12 - 4);
        esp_6 = esp_5 - 176;
        *(__size32*)(esp_5 - 176) = esp_5 - 168;
        local3 = esp_6;
bb0x40149d:
        esp_11 = local3;
        esp_1 = dll_crt0__FP11per_process();
        local4 = esp_1;
        goto bb0x4014a3;
    }
    *(__size32*)(esp_12 - 28) = 0;
    goto bb0x40149d;
}

/** address: 0x00401000 */
void _start(unsigned short param1)
{
    __size32 eax; 		// r24
    __size32 ebp; 		// r29
    int esp; 		// r28

    if (global_0x00403000 != 0) {
        __debugbreak();
    }
    eax = ((unsigned short) (param1) & ~0xf3f);
    proc_0x00401470(pc, 0x401080, (unsigned short) eax | 831, ebp, (unsigned short) eax | 831, eax | 831, esp - 4, (unsigned short) eax | 831, LOGICALFLAGS32(eax | 831), LOGICALFLAGS32(eax | 831), LOGICALFLAGS32(eax | 831));
    return;
}

/** address: 0x00401530 */
__size32 proc_0x00401530(__size32 param1, __size32 param2, __size32 param3)
{
    __size32 eax; 		// r24
    __size32 eax_1; 		// r24{0}
    int eax_4; 		// r24{0}
    __size32 ebx; 		// r27
    __size32 esp_1; 		// r28{0}
    union { __size32; __size32 *; } esp_4; 		// r28{0}
    union { __size32; __size32 *; } esp_5; 		// r28{0}
    __size32 esp_6; 		// r28{0}
    union { __size32 *; __size32; } local1; 		// esp_5{0}
    int local2; 		// eax{0}

    eax = 0;
    esp_1 = (esp_6 - 12);
    ebx = param2;
    local1 = esp_1;
    if (param2 == 0) {
        eax_1 = cygwin_internal(); /* Warning: also results in esp_4 */
        local1 = esp_4;
        eax_4 = 0;
        local2 = eax_4;
        if (eax_1 != -1) {
            ebx = eax_1;
            eax = 1;
bb0x401544:
            esp_5 = local1;
            *(__size32*)(ebx + 4) = 168;
            *(__size32*)(ebx + 8) = 1005;
            *(__size32*)(ebx + 12) = 9;
            *(int*)(ebx + 128) = 0;
            *(__size32*)(ebx + 132) = 112;
            *(__size32*)(ebx + 44) = 0x4017a0;
            *(__size32*)(ebx + 48) = 0x4017ac;
            *(__size32*)(ebx + 20) = 0x403020;
            if (eax == 0) {
                *(__size32*)(ebx + 16) = 0x403024;
            }
            else {
                eax = *(ebx + 164);
                global_0x00403024 = eax;
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
            *(__size32*)esp_5 = 0;
            eax = GetModuleHandleA(); /* Warning: also results in r[26] */
            *(__size32*)(ebx + 124) = eax;
            *(__size32*)(ebx + 52) = 0x402000;
            *(__size32*)(ebx + 56) = 0x402010;
            *(__size32*)(ebx + 60) = 0x403000;
            *(__size32*)(ebx + 64) = 0x403080;
            proc_0x004016b0();
            eax = 1;
            local2 = eax;
        }
    }
    else {
        goto bb0x401544;
    }
    eax = local2;
    return eax;
}

/** address: 0x004016b0 */
void proc_0x004016b0()
{
    proc_0x00401670(0x403000, 0x403000, 0x400000);
    return;
}

/** address: 0x00401670 */
void proc_0x00401670(unsigned int param1, unsigned int param2, __size32 param3)
{
    __size32 eax_1; 		// r24{0}
    __size32 eax_4; 		// r24{0}
    unsigned int ecx; 		// r25
    union { unsigned int; __size32 *; } ecx_1; 		// r25{0}
    unsigned int ecx_4; 		// r25{0}
    union { __size32 *; unsigned int; } local0; 		// ecx_1{0}

    ecx = param1;
    local0 = ecx;
    if (param1 < param2) {
        do {
            ecx_1 = local0;
            eax_1 = *(ecx_1 + 4);
            eax_4 = *ecx_1;
            ecx_4 = ecx_1 + 8;
            *(__size32*)(param3 + eax_1) += eax_4;
            local0 = ecx_4;
        } while (ecx_1 + 8 < param2);
    }
    return;
}

