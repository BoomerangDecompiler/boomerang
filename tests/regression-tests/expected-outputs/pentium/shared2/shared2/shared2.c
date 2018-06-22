void gcc2_compiled.();
void gcc2_compiled.();

/** address: 0x080487a0 */
void gcc2_compiled.()
{
    unsigned char al; 		// r8
    union { __size32; void *; } *eax; 		// r24
    __size32 ebp; 		// r29
    __size32 ebx; 		// r27
    int ecx; 		// r25
    __size32 edi; 		// r31
    int edx; 		// r26
    __size32 esi; 		// r30
    int esp; 		// r28
    union { __size32; __size32 *; } esp_1; 		// r28{0}
    union { __size32; __size32 *; } esp_10; 		// r28{0}
    union { __size32; __size32 *; } esp_11; 		// r28{0}
    union { __size32; __size32 *; } esp_12; 		// r28{0}
    union { __size32; __size32 *; } esp_13; 		// r28{0}
    union { __size32; __size32 *; } esp_14; 		// r28{0}
    union { __size32; __size32 *; } esp_15; 		// r28{0}
    unsigned int esp_16; 		// r28{0}
    unsigned int esp_17; 		// r28{0}
    union { __size32; __size32 *; } esp_2; 		// r28{0}
    union { __size32; __size32 *; } esp_3; 		// r28{0}
    union { __size32; __size32 *; } esp_4; 		// r28{0}
    union { __size32; __size32 *; } esp_5; 		// r28{0}
    union { __size32; __size32 *; } esp_6; 		// r28{0}
    union { __size32; int *; } esp_7; 		// r28{0}
    union { __size32; int *; } esp_8; 		// r28{0}
    union { __size32; int *; } esp_9; 		// r28{0}
    int local0; 		// m[esp - 44]
    unsigned int local1; 		// m[esp - 48]
    int local10; 		// m[esp_16 - 4]{0}
    int local11; 		// m[esp_16 - 8]{0}
    int local12; 		// m[esp_16 - 8]{0}
    int local13; 		// m[esp_16 - 8]{0}
    int local14; 		// m[esp_16 - 8]{0}
    int local15; 		// m[esp_16 - 12]{0}
    int local16; 		// m[esp_16 - 12]{0}
    int local17; 		// m[esp_16 - 12]{0}
    int local18; 		// m[esp_16 - 12]{0}
    int local19; 		// m[esp_16 - 16]{0}
    void *local2; 		// m[esp - 44]
    int local20; 		// m[esp_16 - 16]{0}
    int local21; 		// m[esp_16 - 16]{0}
    int local22; 		// m[esp_16 - 16]{0}
    int local23; 		// m[esp_16 - 44]{0}
    int local24; 		// m[esp_16 - 44]{0}
    int local25; 		// m[esp_16 - 44]{0}
    int local26; 		// m[esp_16 - 44]{0}
    int local27; 		// m[esp_16 - 48]{0}
    int local28; 		// m[esp_16 - 48]{0}
    int local29; 		// m[esp_16 - 48]{0}
    __size32 local3; 		// m[esp - 16]
    int local30; 		// m[esp_16 - 48]{0}
    int local31; 		// %flags{0}
    int local32; 		// %flags{0}
    int local33; 		// %flags{0}
    int local34; 		// %flags{0}
    int local35; 		// %ZF{0}
    int local36; 		// %ZF{0}
    int local37; 		// %ZF{0}
    int local38; 		// %ZF{0}
    int local39; 		// %CF{0}
    __size32 local4; 		// m[esp - 12]
    int local40; 		// %CF{0}
    int local41; 		// %CF{0}
    int local42; 		// %CF{0}
    int local43; 		// m[esp - 40]
    __size32 local5; 		// m[esp - 8]
    __size32 local6; 		// m[esp - 4]
    int local7; 		// m[esp_16 - 4]{0}
    int local8; 		// m[esp_16 - 4]{0}
    int local9; 		// m[esp_16 - 4]{0}

    eax = __builtin_new(56); /* Warning: also results in ecx, edx */
    *(__size32*)(eax + 36) = 100;
    *(__size32*)(eax + 44) = 0x8049c40;
    *(__size32*)(eax + 40) = 101;
    *(__size32*)(eax + 48) = 1;
    *(__size32*)(eax + 52) = 2;
    *(__size32*)(eax + 12) = 0x8049c30;
    *(__size32*)(eax + 4) = 3;
    *(__size32*)(eax + 44) = 0x8049c20;
    *(__size32*)(eax + 8) = 4;
    *(union { void *; __size32; }*)(eax + 16) = eax + 36;
    *(__size32*)(eax + 20) = 5;
    *(__size32*)(eax + 24) = 6;
    *(union { void *; __size32; }*)eax = eax + 36;
    *(__size32*)(eax + 12) = 0x8049c00;
    *(__size32*)(eax + 28) = 7;
    *(__size32*)(eax + 44) = 0x8049c10;
    *(__size32*)(eax + 32) = 8;
    ecx = gcc2_compiled.(pc, eax + 36, ebx, esi, edi, ebp, eax + 36, ecx, edx, eax + 16, esp_16 - 4, eax, edi, SUBFLAGS32(esp_16 - 16, 24, (esp_16 - 40)), esp_16 == 40, (unsigned int)(esp_16 - 16) < (unsigned int)24); /* Warning: also results in edx, ebx, esp_1, ebp, esi, edi */
    eax = *(esi + 12);
    *(__size32*)esp_1 = esi;
    (**(*(esi + 12) + 8))(local27, local23, local19, local15, local11, local7, eax, ecx, edx, ebx, ebp, esi, edi, <all>, local31, local35, local39);
    al =  (esi == 0) ? 1 : 0;
    edi = (0 >> 8 & 0xffffff | (al)) - 1 & ebx;
    eax = *edi;
    edx = *(eax + 8);
    *(__size32*)esp_4 = eax;
    (**(*(eax + 8) + 8))(local28, local24, local20, local16, local12, local8, al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, LOGICALFLAGS32(edi), LOGICALFLAGS32(edi), LOGICALFLAGS32(edi));
    ebx = 0;
    if (esi != 0) {
        ebx = *esi;
    }
    eax = *(ebx + 8);
    *(int*)esp_7 = ebx;
    (**(*(ebx + 8) + 8))(local29, local25, local21, local17, local13, local9, al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, SUBFLAGS32(esp_7 + 16, 12, esp_7 + 4), esp_7 == -4, esp_7 + 16 < (unsigned int)12);
    ebx = *edi;
    eax = *(ebx + 8);
    *(__size32*)esp_10 = ebx;
    (**(*(ebx + 8) + 8))(local30, local26, local22, local18, local14, local10, al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, local33, local37, local41);
    eax = *(ebx + 8);
    *(__size32*)esp_13 = ebx;
    (**(*(ebx + 8) + 8))(*(esp_16 - 48), *(esp_16 - 44), *(esp_16 - 16), *(esp_16 - 12), *(esp_16 - 8), *(esp_16 - 4), al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, local34, local38, local42);
    return;
}

/** address: 0x080488d0 */
void gcc2_compiled.()
{
    __ls__7ostreamPCc();
    hex__FR3ios();
    __ls__7ostreamPCv();
    __ls__7ostreamPCc();
    endl__FR7ostream();
    return;
}

