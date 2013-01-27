HINSTANCE global3 = 0;

__size32 proc1(__size32 param1);
HWND proc2(HINSTANCE param1, int param2);

// address: 0x401000
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    __size32 eax; 		// r24
    __size32 eax_1; 		// r24{48}
    __size32 ebp; 		// r29
    __size32 ebx; 		// r27
    __size32 ecx; 		// r25
    __size32 edi; 		// r31
    __size32 edx; 		// r26
    __size32 esi; 		// r30
    void *esp; 		// r28
    void *esp_1; 		// r28
    union { __size32 * x67; int x68; } esp_2; 		// r28{95}
    union { int x1; __size32 * x2; } esp_3; 		// r28{138}
    __size32 local0; 		// m[esp + 4]
    int local1; 		// m[esp + 8]
    unsigned int local10; 		// m[esp - 56]
    int local11; 		// m[esp + 4]{21}
    int local12; 		// m[esp + 4]{33}
    int local13; 		// m[esp + 4]{233}
    int local14; 		// m[esp + 4]{232}
    int local15; 		// m[esp + 8]{21}
    int local16; 		// m[esp + 8]{33}
    int local17; 		// m[esp + 8]{87}
    int local18; 		// m[esp + 12]{21}
    int local19; 		// m[esp + 12]{33}
    int local2; 		// m[esp + 12]
    int local20; 		// m[esp + 12]{87}
    int local21; 		// m[esp + 16]{21}
    int local22; 		// m[esp + 16]{33}
    int local23; 		// m[esp + 16]{87}
    unsigned int local24; 		// m[esp_1 - 12]{46}
    void *local25; 		// m[esp_1 - 12]{81}
    int local26; 		// m[esp - 32]{21}
    int local27; 		// m[esp - 32]{33}
    int local28; 		// m[esp - 32]{87}
    int local29; 		// m[esp - 36]{21}
    int local3; 		// m[esp + 16]
    int local30; 		// m[esp - 36]{33}
    int local31; 		// m[esp - 36]{87}
    int local32; 		// m[esp - 40]{21}
    int local33; 		// m[esp - 40]{33}
    int local34; 		// m[esp - 40]{87}
    int local35; 		// m[esp - 44]{21}
    int local36; 		// m[esp - 44]{33}
    int local37; 		// m[esp - 44]{87}
    int local38; 		// m[esp - 48]{21}
    int local39; 		// m[esp - 48]{33}
    __size32 local4; 		// m[esp - 32]
    int local40; 		// m[esp - 48]{87}
    int local41; 		// m[esp - 52]{21}
    int local42; 		// m[esp - 52]{33}
    int local43; 		// m[esp - 52]{87}
    int local44; 		// m[esp - 56]{21}
    int local45; 		// m[esp - 56]{33}
    int local46; 		// m[esp - 56]{87}
    int local47; 		// %flags{21}
    int local48; 		// %flags{89}
    int local49; 		// %flags{110}
    __size32 local5; 		// m[esp - 36]
    int local50; 		// %flags{118}
    int local51; 		// %flags{176}
    int local52; 		// %flags{118}
    int local53; 		// %flags{188}
    int local54; 		// %flags{140}
    int local55; 		// %ZF{21}
    int local56; 		// %ZF{89}
    int local57; 		// %ZF{110}
    int local58; 		// %ZF{118}
    int local59; 		// %ZF{177}
    int local6; 		// m[esp - 40]
    int local60; 		// %ZF{118}
    int local61; 		// %ZF{189}
    int local62; 		// %ZF{140}
    int local63; 		// %CF{21}
    int local64; 		// %CF{89}
    int local65; 		// %CF{110}
    int local66; 		// %CF{118}
    int local67; 		// %CF{178}
    int local68; 		// %CF{118}
    int local69; 		// %CF{190}
    int local7; 		// m[esp - 44]
    int local70; 		// %CF{140}
    int local71; 		// m[esp - 28]
    union { int x1; __size32 * x2; } local72; 		// esp_3{171}
    int local73; 		// local51{176}
    int local74; 		// local59{177}
    int local75; 		// local67{178}
    int local76; 		// local53{188}
    int local77; 		// local61{189}
    int local78; 		// local69{190}
    int local8; 		// m[esp - 48]
    __size32 local9; 		// m[esp - 52]

    (*0x45de)(hInstance, 0x45de, hInstance, hPrevInstance, lpCmdLine, nCmdShow, esi, edi, 100, 0x4054f4, 103, hInstance, pc, SUBFLAGS32(esp, 28, (esp - 28)), esp - 28 == 0, esp < 28);
    *(__size32*)(esp_1 - 4) = 100;
    *(__size32*)(esp_1 - 8) = 0x405490;
    *(__size32*)(esp_1 - 12) = 109;
    *(__size32*)(esp_1 - 16) = esi;
    (*edi)(eax, ecx, edx, ebx, ebp, esi, edi, local11, local15, local18, local21, local26, local29, local32, local35, local38, local41, local44, <all>, local47, local55, local63);
    *(__size32*)(esp_1 - 4) = esi;
    esi = proc1(*(esp_1 - 4)); /* Warning: also results in esp_1 */
    eax = *(esp_1 + 56);
    *(__size32*)(esp_1 - 4) = eax;
    *(__size32*)(esp_1 - 8) = esi;
    eax_1 = proc2(*(esp_1 - 4), *(esp_1 - 8));
    if (eax_1 != 0) {
        *(LPCSTR*)esp_1 = 109;
        *(__size32*)(esp_1 - 4) = esi;
        LoadAcceleratorsA(*(esp_1 - 4), *esp_1);
        *(LPCSTR*)esp = 0;
        *(__size32*)(esp - 4) = 0;
        *(__size32*)(esp - 8) = 0;
        local25 = esp + 12;
        (*0x45bc)(eax, esp + 12, edx, ebx, ebp, eax, 0x45bc, local12, local16, local19, local22, local27, local30, local33, local36, local39, local42, local45, <all>, LOGICALFLAGS32(eax_1), LOGICALFLAGS32(eax_1), LOGICALFLAGS32(eax_1));
        local48 = LOGICALFLAGS32(eax);
        local73 = local48;
        local74 = local56;
        local75 = local64;
        if (eax != 0) {
            *(__size32*)(esp - 4) = ebx;
            ebx = 0x45a4;
            *(__size32*)(esp - 8) = ebp;
            esp_2 = esp - 8;
            ebp = 0x4590;
            local72 = esp_2;
            do {
                esp_3 = local72;
                local51 = local73;
                local59 = local74;
                local67 = local75;
                eax = *(esp_3 + 16);
                *(union { int x1; __size32 * x2; }*)(esp_3 - 4) = esp_3 + 16;
                *(__size32*)(esp_3 - 8) = esi;
                *(__size32*)(esp_3 - 12) = eax;
                (*ebx)(eax, ecx, esp_3 + 16, ebx, ebp, esi, edi, local14, local17, local20, local23, local28, local31, local34, local37, local40, local43, local46, <all>, local51, local59, local67);
                local49 = LOGICALFLAGS32(eax);
                local76 = local49;
                local77 = local57;
                local78 = local65;
                if (eax == 0) {
                    *(void **)(esp - 4) = esp + 16;
                    (*ebp)(eax, esp + 16, edx, ebx, ebp, esi, edi, local0, local1, local2, local3, local4, local5, local6, local7, local8, local9, local10, <all>, LOGICALFLAGS32(eax), LOGICALFLAGS32(eax), LOGICALFLAGS32(eax));
                    local76 = local50;
                    local77 = local58;
                    local78 = local66;
                    *(void **)(esp - 4) = esp + 16;
                    DispatchMessageA(*(esp - 4));
                }
                local53 = local76;
                local61 = local77;
                local69 = local78;
                *(__size32*)(esp - 4) = 0;
                *(__size32*)(esp - 8) = 0;
                *(__size32*)(esp - 12) = 0;
                *(void **)(esp - 16) = esp + 16;
                (*edi)(esp + 16, ecx, edx, ebx, ebp, esi, edi, *(esp + 4), *(esp + 8), *(esp + 12), *(esp + 16), *(esp - 32), *(esp - 36), *(esp - 40), *(esp - 44), *(esp - 48), *(esp - 52), *(esp - 56), <all>, local53, local61, local69);
                local72 = esp_3;
                local54 = LOGICALFLAGS32(eax);
                local73 = local54;
                local74 = local62;
                local75 = local70;
            } while (eax != 0);
            esp = esp_3 + 8;
        }
        eax_1 = *(esp + 16);
    } else {
    }
    return eax_1;
}

// address: 0x4010c0
__size32 proc1(__size32 param1) {
    __size32 eax; 		// r24
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 esi; 		// r30
    int esp; 		// r28
    void *esp_1; 		// r28
    unsigned int local0; 		// m[esp - 64]
    __size32 local1; 		// m[esp - 60]
    __size32 local10; 		// m[esp + 4]
    int local2; 		// m[esp - 56]
    __size32 local3; 		// m[esp - 52]
    int local4; 		// m[esp - 48]
    int local5; 		// m[esp - 44]
    int local6; 		// m[esp - 40]
    int local7; 		// m[esp - 36]
    int local8; 		// m[esp - 32]
    __size32 local9; 		// m[esp - 28]

    (*0x460e)(pc, param1, 107, esi, 48, 3, 0x4011b0, 0, 0, param1, param1, param1, 0x460e, SUBFLAGS32(esp, 48, esp - 48), esp - 48 == 0, esp < 48);
    *(__size32*)(esp_1 - 4) = 0x7f00;
    *(__size32*)(esp_1 - 8) = 0;
    *(__size32*)(esp_1 + 28) = eax;
    LoadCursorA(*(esp_1 - 8), *(esp_1 - 4));
    *(__size32*)(esp + 32) = eax;
    eax = *(esp + 24);
    *(__size32*)(esp - 4) = 108;
    *(__size32*)(esp - 8) = eax;
    *(__size32*)(esp + 36) = 6;
    *(__size32*)(esp + 40) = 109;
    *(__size32*)(esp + 44) = 0x405490;
    (*esi)(local0, local1, local2, local3, local4, local5, local6, local7, local8, local9, local10, eax, ecx, edx, esi, <all>, flags, ZF, CF);
    *(__size32*)(esp + 48) = eax;
    *(void **)(esp - 4) = esp + 4;
    RegisterClassExA(*(esp - 4));
    esi = *esp;
    return esi;
}

// address: 0x401150
HWND proc2(HINSTANCE param1, int param2) {
    __size32 eax; 		// r24

    global3 = param1;
    CreateWindowExA(0, "", "", 0xcf0000, 0x80000000, 0, 0x80000000, 0, 0, 0, param1, 0);
    if (eax != 0) {
        ShowWindow(eax, param2);
        UpdateWindow(eax);
        eax = 1;
    } else {
    }
    return eax;
}

