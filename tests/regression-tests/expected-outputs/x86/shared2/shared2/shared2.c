void gcc2_compiled.();
void gcc2_compiled.();


/** address: 0x080487a0 */
void gcc2_compiled.()
{
    unsigned char al; 		// r8
    void **eax; 		// r24
    union { unsigned int; void *; } ebp; 		// r29
    __size32 ebx; 		// r27
    int ecx; 		// r25
    __size32 edi; 		// r31
    int edx; 		// r26
    __size32 *esi; 		// r30
    int esp; 		// r28
    __size32 *esp_1; 		// r28{20}
    __size32 *esp_10; 		// r28{38}
    __size32 *esp_11; 		// r28{38}
    __size32 *esp_12; 		// r28{38}
    __size32 *esp_13; 		// r28{43}
    __size32 *esp_14; 		// r28{43}
    __size32 *esp_15; 		// r28{43}
    union { unsigned int; void *; } esp_16; 		// r28{0}
    union { unsigned int; void *; } esp_17; 		// r28{0}
    __size32 *esp_2; 		// r28{20}
    __size32 *esp_3; 		// r28{20}
    __size32 *esp_4; 		// r28{24}
    __size32 *esp_5; 		// r28{24}
    __size32 *esp_6; 		// r28{24}
    union { unsigned int; __size32 *; } esp_7; 		// r28{31}
    union { unsigned int; __size32 *; } esp_8; 		// r28{31}
    union { unsigned int; __size32 *; } esp_9; 		// r28{31}
    int local0; 		// m[esp - 44]
    __size32 local1; 		// m[esp - 4]
    int local10; 		// m[esp_16 - 4]{38}
    int local11; 		// m[esp_16 - 8]{20}
    int local12; 		// m[esp_16 - 8]{24}
    int local13; 		// m[esp_16 - 8]{31}
    int local14; 		// m[esp_16 - 8]{38}
    int local15; 		// m[esp_16 - 12]{20}
    int local16; 		// m[esp_16 - 12]{24}
    int local17; 		// m[esp_16 - 12]{31}
    int local18; 		// m[esp_16 - 12]{38}
    int local19; 		// m[esp_16 - 16]{20}
    __size32 local2; 		// m[esp - 8]
    int local20; 		// m[esp_16 - 16]{24}
    int local21; 		// m[esp_16 - 16]{31}
    int local22; 		// m[esp_16 - 16]{38}
    int local23; 		// m[esp_16 - 44]{20}
    int local24; 		// m[esp_16 - 44]{24}
    int local25; 		// m[esp_16 - 44]{31}
    int local26; 		// m[esp_16 - 44]{38}
    int local27; 		// m[esp_16 - 48]{20}
    int local28; 		// m[esp_16 - 48]{24}
    int local29; 		// m[esp_16 - 48]{31}
    __size32 local3; 		// m[esp - 12]
    int local30; 		// m[esp_16 - 48]{38}
    int local31; 		// %flags{20}
    int local32; 		// %flags{20}
    int local33; 		// %flags{38}
    int local34; 		// %flags{43}
    int local35; 		// %ZF{20}
    int local36; 		// %ZF{20}
    int local37; 		// %ZF{38}
    int local38; 		// %ZF{43}
    int local39; 		// %CF{20}
    __size32 local4; 		// m[esp - 16]
    int local40; 		// %CF{20}
    int local41; 		// %CF{38}
    int local42; 		// %CF{43}
    int local43; 		// %NF{20}
    int local44; 		// %NF{20}
    int local45; 		// %NF{38}
    int local46; 		// %NF{43}
    int local47; 		// %OF{20}
    int local48; 		// %OF{20}
    int local49; 		// %OF{38}
    void *local5; 		// m[esp - 44]
    int local50; 		// %OF{43}
    int local51; 		// m[esp - 40]
    unsigned int local6; 		// m[esp - 48]
    int local7; 		// m[esp_16 - 4]{20}
    int local8; 		// m[esp_16 - 4]{24}
    int local9; 		// m[esp_16 - 4]{31}

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
    *(void **)(eax + 16) = eax + 36;
    *(__size32*)(eax + 20) = 5;
    *(__size32*)(eax + 24) = 6;
    *(void **)eax = eax + 36;
    *(__size32*)(eax + 12) = 0x8049c00;
    *(__size32*)(eax + 28) = 7;
    *(__size32*)(eax + 44) = 0x8049c10;
    *(__size32*)(eax + 32) = 8;
    ecx = gcc2_compiled.(eax + 36, ecx, edx, eax + 16, esp_16 - 4, eax, edi, ebp, edi, esi, ebx, eax + 36, pc, SUBFLAGS32(esp_16 - 16, 24, (esp_16 - 40)), esp_16 == 40, esp_16 - 16 < 24, esp_16 < 40, esp_16 < 16 && esp_16 >= 40); /* Warning: also results in edx, ebx, esp_1, ebp, esi, edi */
    eax = *(esi + 12);
    *(__size32*)esp_1 = esi;
    (**(*(esi + 12) + 8))(eax, ecx, edx, ebx, ebp, esi, <all>, local31, local35, local39, local43, local47, edi, local7, local11, local15, local19, local23, local27);
    al =  (esi == 0) ? 1 : 0;
    edi = (al) - 1 & ebx;
    eax = *edi;
    edx = *(eax + 8);
    *(__size32*)esp_4 = eax;
    (**(*(eax + 8) + 8))(al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, LOGICALFLAGS32(edi), edi == 0, 0, edi < 0, 0, local8, local12, local16, local20, local24, local28);
    ebx = 0;
    if (esi != 0) {
        ebx = *esi;
    }
    eax = *(ebx + 8);
    *(__size32*)esp_7 = ebx;
    (**(*(ebx + 8) + 8))(al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, SUBFLAGS32(esp_7 + 16, 12, esp_7 + 4), esp_7 == -4, esp_7 + 16 < 12, esp_7 < -4, esp_7 < -16 && esp_7 >= -4, local9, local13, local17, local21, local25, local29);
    ebx = *edi;
    eax = *(ebx + 8);
    *(__size32*)esp_10 = ebx;
    (**(*(ebx + 8) + 8))(al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, local33, local37, local41, local45, local49, local10, local14, local18, local22, local26, local30);
    eax = *(ebx + 8);
    *(__size32*)esp_13 = ebx;
    (**(*(ebx + 8) + 8))(al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, local34, local38, local42, local46, local50, *(esp_16 - 4), *(esp_16 - 8), *(esp_16 - 12), *(esp_16 - 16), *(esp_16 - 44), *(esp_16 - 48));
    return;
}

/** address: 0x080488d0 */
void gcc2_compiled.()
{
}

