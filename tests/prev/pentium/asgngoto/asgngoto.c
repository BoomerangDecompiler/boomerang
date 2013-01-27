void atexit();

// address: 0x8048824
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    union { __size32 * x1; int x2; } ebp; 		// r29
    __size32 ebx; 		// r27
    int ecx; 		// r25
    int ecx_1; 		// r25{83}
    int edi; 		// r31
    int edx; 		// r26
    __size32 esi; 		// r30
    __size32 esi_1; 		// r30{82}
    int esp; 		// r28
    void *esp_1; 		// r28{62}
    void *esp_2; 		// r28{82}
    void *esp_3; 		// r28{116}
    unsigned int local0; 		// m[esp - 60]
    __size32 local1; 		// m[esp - 48]
    int local10; 		// m[esp + 8]
    void *local12; 		// esp_3{116}
    __size32 local2; 		// m[esp - 44]
    __size32 local3; 		// m[esp - 40]
    __size32 local4; 		// m[esp - 36]
    int local5; 		// m[esp - 32]
    __size32 local6; 		// m[esp - 28]
    __size32 local7; 		// m[esp - 24]
    __size32 local8; 		// m[esp - 4]
    int local9; 		// m[esp + 4]

    proc1();
    proc2();
    proc3();
    atexit();
    proc4();
    proc5();
    ebp = esp - 32;
    proc6();
    local12 = esp_1;
    eax = 0xc0000000;
    flags = SUBFLAGS32(0, 0xc0000000, 0x40000000);
    esi = 0;
    do {
        esp_3 = local12;
        (**(edx + esi * 4))(local0, local1, local2, local3, local4, local5, local6, local7, local8, local9, local10, envp, eax, ecx, edx, ebx, ebp, esi, edi, flags, ZF, CF);
        local12 = esp_2;
        ecx_1 = *(ebp - 16);
        esi = esi_1 + 1;
        ecx = ecx_1 - edi >> 2;
        tmp1 = esi_1 - (ecx_1 - edi >> 2) + 1;
        flags = SUBFLAGS32(esi_1 + 1, ecx_1 - edi >> 2, tmp1);
        edx = edi;
    } while (esi_1 + 1 < (unsigned int)(ecx_1 - edi >> 2));
    return eax;
}

// address: 0x8048904
void atexit() {
    __size32 edx; 		// r26

    if (edx != 0) {
    }
    proc7();
    return;
}

