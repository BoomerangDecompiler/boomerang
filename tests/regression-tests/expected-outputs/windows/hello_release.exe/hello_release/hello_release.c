int global3_405558 = 0;
int global3_405558 = 0;
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
__size32 proc_0x004010c0(__size32 param1);
__size32 proc_0x00401150(HINSTANCE param1);

/** address: 0x00401000 */
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    LRESULT eax; 		// r24
    __size32 eax_1; 		// r24{0}
    __size32 ebp; 		// r29
    __size32 ebx; 		// r27
    __size32 ecx; 		// r25
    int edi; 		// r31
    __size32 edx; 		// r26
    HINSTANCE esi; 		// r30
    int esp; 		// r28
    __size32 esp_1; 		// r28{0}
    union { __size32; unsigned int *; } esp_10; 		// r28{0}
    union { __size32; unsigned int *; } esp_11; 		// r28{0}
    union { __size32; unsigned int *; } esp_12; 		// r28{0}
    __size32 esp_13; 		// r28{0}
    __size32 esp_14; 		// r28{0}
    __size32 esp_15; 		// r28{0}
    __size32 esp_16; 		// r28{0}
    __size32 esp_17; 		// r28{0}
    __size32 esp_18; 		// r28{0}
    __size32 esp_19; 		// r28{0}
    __size32 esp_2; 		// r28{0}
    __size32 esp_20; 		// r28{0}
    __size32 esp_21; 		// r28{0}
    __size32 esp_22; 		// r28{0}
    __size32 esp_23; 		// r28{0}
    __size32 esp_24; 		// r28{0}
    __size32 esp_25; 		// r28{0}
    union { __size32; __size32 *; } esp_26; 		// r28{0}
    __size32 esp_27; 		// r28{0}
    __size32 esp_28; 		// r28{0}
    __size32 esp_29; 		// r28{0}
    __size32 esp_3; 		// r28{0}
    __size32 esp_30; 		// r28{0}
    __size32 esp_31; 		// r28{0}
    __size32 esp_32; 		// r28{0}
    union { unsigned int; void *; } esp_33; 		// r28{0}
    union { unsigned int; void *; } esp_34; 		// r28{0}
    __size32 esp_4; 		// r28{0}
    __size32 esp_5; 		// r28{0}
    __size32 esp_6; 		// r28{0}
    void *esp_7; 		// r28{0}
    void *esp_8; 		// r28{0}
    void *esp_9; 		// r28{0}
    HINSTANCE local0; 		// m[esp + 4]
    HINSTANCE local1; 		// m[esp + 8]
    unsigned int local10; 		// m[esp - 56]
    int local100; 		// m[esp_33 - 56]{0}
    int local101; 		// m[esp_33 - 56]{0}
    int local102; 		// m[esp_33 - 56]{0}
    int local103; 		// m[esp_33 - 56]{0}
    int local104; 		// m[esp_33 - 56]{0}
    int local105; 		// m[esp_33 - 56]{0}
    int local106; 		// m[esp_33 - 56]{0}
    int local107; 		// m[esp_33 - 56]{0}
    int local108; 		// m[esp_33 - 56]{0}
    int local109; 		// %flags{0}
    int local11; 		// m[esp_33 + 4]{0}
    int local110; 		// %flags{0}
    int local111; 		// %flags{0}
    int local112; 		// %flags{0}
    int local113; 		// %flags{0}
    int local114; 		// %flags{0}
    int local115; 		// %flags{0}
    int local116; 		// %flags{0}
    int local117; 		// %flags{0}
    int local118; 		// %flags{0}
    int local119; 		// %flags{0}
    int local12; 		// m[esp_33 + 4]{0}
    int local120; 		// %flags{0}
    int local121; 		// %flags{0}
    int local122; 		// %flags{0}
    int local123; 		// %flags{0}
    int local124; 		// %flags{0}
    int local125; 		// %flags{0}
    int local126; 		// %flags{0}
    int local127; 		// %flags{0}
    int local128; 		// %flags{0}
    int local129; 		// %ZF{0}
    int local13; 		// m[esp_33 + 4]{0}
    int local130; 		// %ZF{0}
    int local131; 		// %ZF{0}
    int local132; 		// %ZF{0}
    int local133; 		// %ZF{0}
    int local134; 		// %ZF{0}
    int local135; 		// %ZF{0}
    int local136; 		// %ZF{0}
    int local137; 		// %ZF{0}
    int local138; 		// %ZF{0}
    int local139; 		// %ZF{0}
    int local14; 		// m[esp_33 + 4]{0}
    int local140; 		// %ZF{0}
    int local141; 		// %ZF{0}
    int local142; 		// %ZF{0}
    int local143; 		// %ZF{0}
    int local144; 		// %ZF{0}
    int local145; 		// %ZF{0}
    int local146; 		// %ZF{0}
    int local147; 		// %ZF{0}
    int local148; 		// %ZF{0}
    int local149; 		// %CF{0}
    int local15; 		// m[esp_33 + 4]{0}
    int local150; 		// %CF{0}
    int local151; 		// %CF{0}
    int local152; 		// %CF{0}
    int local153; 		// %CF{0}
    int local154; 		// %CF{0}
    int local155; 		// %CF{0}
    int local156; 		// %CF{0}
    int local157; 		// %CF{0}
    int local158; 		// %CF{0}
    int local159; 		// %CF{0}
    int local16; 		// m[esp_33 + 4]{0}
    int local160; 		// %CF{0}
    int local161; 		// %CF{0}
    int local162; 		// %CF{0}
    int local163; 		// %CF{0}
    int local164; 		// %CF{0}
    int local165; 		// %CF{0}
    int local166; 		// %CF{0}
    int local167; 		// %CF{0}
    int local168; 		// %CF{0}
    int local169; 		// m[esp - 28]
    int local17; 		// m[esp_33 + 4]{0}
    __size32 local170; 		// eax{0}
    __size32 local171; 		// esp_27{0}
    int local172; 		// local111{0}
    int local173; 		// local131{0}
    int local174; 		// local151{0}
    __size32 local175; 		// esp_30{0}
    int local176; 		// local114{0}
    int local177; 		// local134{0}
    int local178; 		// local154{0}
    union { __size32 *; __size32; } local179; 		// esp{0}
    int local18; 		// m[esp_33 + 8]{0}
    int local19; 		// m[esp_33 + 8]{0}
    LPSTR local2; 		// m[esp + 12]
    int local20; 		// m[esp_33 + 8]{0}
    int local21; 		// m[esp_33 + 8]{0}
    int local22; 		// m[esp_33 + 8]{0}
    int local23; 		// m[esp_33 + 8]{0}
    int local24; 		// m[esp_33 + 8]{0}
    int local25; 		// m[esp_33 + 12]{0}
    int local26; 		// m[esp_33 + 12]{0}
    int local27; 		// m[esp_33 + 12]{0}
    int local28; 		// m[esp_33 + 12]{0}
    int local29; 		// m[esp_33 + 12]{0}
    int local3; 		// m[esp + 16]
    int local30; 		// m[esp_33 + 12]{0}
    int local31; 		// m[esp_33 + 12]{0}
    int local32; 		// m[esp_33 + 16]{0}
    int local33; 		// m[esp_33 + 16]{0}
    int local34; 		// m[esp_33 + 16]{0}
    int local35; 		// m[esp_33 + 16]{0}
    int local36; 		// m[esp_33 + 16]{0}
    int local37; 		// m[esp_33 + 16]{0}
    int local38; 		// m[esp_33 + 16]{0}
    int local39; 		// m[esp_33 - 32]{0}
    __size32 local4; 		// m[esp - 32]
    int local40; 		// m[esp_33 - 32]{0}
    int local41; 		// m[esp_33 - 32]{0}
    int local42; 		// m[esp_33 - 32]{0}
    int local43; 		// m[esp_33 - 32]{0}
    int local44; 		// m[esp_33 - 32]{0}
    int local45; 		// m[esp_33 - 32]{0}
    int local46; 		// m[esp_33 - 32]{0}
    int local47; 		// m[esp_33 - 32]{0}
    int local48; 		// m[esp_33 - 32]{0}
    int local49; 		// m[esp_33 - 36]{0}
    __size32 local5; 		// m[esp - 36]
    int local50; 		// m[esp_33 - 36]{0}
    int local51; 		// m[esp_33 - 36]{0}
    int local52; 		// m[esp_33 - 36]{0}
    int local53; 		// m[esp_33 - 36]{0}
    int local54; 		// m[esp_33 - 36]{0}
    int local55; 		// m[esp_33 - 36]{0}
    int local56; 		// m[esp_33 - 36]{0}
    int local57; 		// m[esp_33 - 36]{0}
    int local58; 		// m[esp_33 - 36]{0}
    int local59; 		// m[esp_33 - 40]{0}
    int local6; 		// m[esp - 40]
    int local60; 		// m[esp_33 - 40]{0}
    int local61; 		// m[esp_33 - 40]{0}
    int local62; 		// m[esp_33 - 40]{0}
    int local63; 		// m[esp_33 - 40]{0}
    int local64; 		// m[esp_33 - 40]{0}
    int local65; 		// m[esp_33 - 40]{0}
    int local66; 		// m[esp_33 - 40]{0}
    int local67; 		// m[esp_33 - 40]{0}
    int local68; 		// m[esp_33 - 40]{0}
    int local69; 		// m[esp_33 - 44]{0}
    int local7; 		// m[esp - 44]
    int local70; 		// m[esp_33 - 44]{0}
    int local71; 		// m[esp_33 - 44]{0}
    int local72; 		// m[esp_33 - 44]{0}
    int local73; 		// m[esp_33 - 44]{0}
    int local74; 		// m[esp_33 - 44]{0}
    int local75; 		// m[esp_33 - 44]{0}
    int local76; 		// m[esp_33 - 44]{0}
    int local77; 		// m[esp_33 - 44]{0}
    int local78; 		// m[esp_33 - 44]{0}
    int local79; 		// m[esp_33 - 48]{0}
    int local8; 		// m[esp - 48]
    int local80; 		// m[esp_33 - 48]{0}
    int local81; 		// m[esp_33 - 48]{0}
    int local82; 		// m[esp_33 - 48]{0}
    int local83; 		// m[esp_33 - 48]{0}
    int local84; 		// m[esp_33 - 48]{0}
    int local85; 		// m[esp_33 - 48]{0}
    int local86; 		// m[esp_33 - 48]{0}
    int local87; 		// m[esp_33 - 48]{0}
    int local88; 		// m[esp_33 - 48]{0}
    int local89; 		// m[esp_33 - 52]{0}
    HINSTANCE local9; 		// m[esp - 52]
    int local90; 		// m[esp_33 - 52]{0}
    int local91; 		// m[esp_33 - 52]{0}
    int local92; 		// m[esp_33 - 52]{0}
    int local93; 		// m[esp_33 - 52]{0}
    int local94; 		// m[esp_33 - 52]{0}
    int local95; 		// m[esp_33 - 52]{0}
    int local96; 		// m[esp_33 - 52]{0}
    int local97; 		// m[esp_33 - 52]{0}
    int local98; 		// m[esp_33 - 52]{0}
    int local99; 		// m[esp_33 - 56]{0}

    (*0x45de)(ebx, ebp, hInstance, 0x45de, hInstance, hPrevInstance, lpCmdLine, nCmdShow, esi, edi, 100, 0x4054f4, 103, hInstance, pc, SUBFLAGS32(esp_33, 28, (esp_33 - 28)), esp_33 - 28 == 0, esp_33 < (unsigned int)28);
    *(int*)(esp_1 - 4) = 100;
    *(__size32*)(esp_1 - 8) = 0x405490;
    *(int*)(esp_1 - 12) = 109;
    *(__size32*)(esp_1 - 16) = esi;
    (*edi)(ebx, ebp, esi, edi, local11, local18, local25, local32, local39, local49, local59, local69, local79, local89, local99, <all>, local109, local129, local149);
    *(__size32*)(esp_4 - 4) = esi;
    esp_7 = proc_0x004010c0(*(esp_4 - 4)); /* Warning: also results in esi */
    eax = *(esp_7 + 56);
    *(__size32*)(esp_7 - 4) = eax;
    *(__size32*)(esp_7 - 8) = esi;
    eax_1 = proc_0x00401150(*(esp_7 - 8)); /* Warning: also results in esp_10, esi */
    local170 = eax_1;
    if (eax_1 != 0) {
        *(int*)(esp_10 + 8) = 109;
        *(__size32*)(esp_10 + 4) = esi;
        eax = LoadAcceleratorsA(*(esp_10 + 4), *(esp_10 + 8)); /* Warning: also results in edx */
        *(int*)(esp_10 + 12LL - 4) = 0;
        *(int*)(esp_10 + 12LL - 8) = 0;
        *(int*)(esp_10 + 12LL - 12) = 0;
        *(__size32*)(esp_10 + 12LL - 16) = esp_10 + 12LL + 8;
        (*0x45bc)(eax, esp_10 + 12LL + 8, edx, ebx, ebp, eax, 0x45bc, local12, local19, local26, local33, local40, local50, local60, local70, local80, local90, local100, <all>, LOGICALFLAGS32(eax_1), LOGICALFLAGS32(eax_1), LOGICALFLAGS32(eax_1));
        local179 = esp_13;
        local115 = LOGICALFLAGS32(eax);
        local172 = local115;
        local173 = local135;
        local174 = local155;
        if (eax != 0) {
            *(__size32*)(esp_13 - 4) = ebx;
            ebx = 0x45a4;
            *(__size32*)(esp_13 - 8) = ebp;
            esp_16 = esp_13 - 8;
            ebp = 0x4590;
            local171 = esp_16;
            do {
                esp_27 = local171;
                local111 = local172;
                local131 = local173;
                local151 = local174;
                eax = *(esp_27 + 16);
                *(__size32*)(esp_27 - 4) = esp_27 + 16;
                *(__size32*)(esp_27 - 8) = esi;
                *(__size32*)(esp_27 - 12) = eax;
                (*ebx)(eax, ecx, esp_27 + 16, ebx, ebp, esi, edi, local13, local20, local27, local34, local42, local52, local62, local72, local82, local92, local102, <all>, local111, local131, local151);
                local175 = esp_17;
                local118 = LOGICALFLAGS32(eax);
                local176 = local118;
                local177 = local138;
                local178 = local158;
                if (eax == 0) {
                    *(__size32*)(esp_17 - 4) = esp_17 + 16;
                    (*ebp)(eax, esp_17 + 16, edx, ebx, ebp, esi, edi, local16, local23, local30, local37, local43, local53, local63, local73, local83, local93, local103, <all>, LOGICALFLAGS32(eax), LOGICALFLAGS32(eax), LOGICALFLAGS32(eax));
                    local176 = local122;
                    local177 = local142;
                    local178 = local162;
                    *(__size32*)(esp_20 - 4) = esp_20 + 16;
                    ecx = DispatchMessageA(*(esp_20 - 4)); /* Warning: also results in edx, esp_23 */
                    local175 = esp_23;
                }
                esp_30 = local175;
                local114 = local176;
                local134 = local177;
                local154 = local178;
                *(int*)(esp_30 - 4) = 0;
                *(int*)(esp_30 - 8) = 0;
                *(int*)(esp_30 - 12) = 0;
                *(__size32*)(esp_30 - 16) = esp_30 + 16;
                (*edi)(esp_30 + 16, ecx, edx, ebx, ebp, esi, edi, local16, local23, local30, local37, local41, local51, local61, local71, local81, local91, local101, <all>, local114, local134, local154);
                local171 = esp_26;
                local126 = LOGICALFLAGS32(eax);
                local172 = local126;
                local173 = local146;
                local174 = local166;
            } while (eax != 0);
            esp = esp_26 + 8;
            local179 = esp;
        }
        esp = local179;
        eax = *(esp + 16);
        local170 = eax;
    }
    eax = local170;
    return eax;
}

/** address: 0x004010c0 */
__size32 proc_0x004010c0(__size32 param1)
{
    ATOM eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26
    __size32 esi; 		// r30
    int esp; 		// r28
    __size32 esp_1; 		// r28{0}
    __size32 esp_2; 		// r28{0}
    __size32 esp_3; 		// r28{0}
    union { unsigned int; void *; } esp_4; 		// r28{0}
    union { unsigned int; void *; } esp_5; 		// r28{0}
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

    (*0x460e)(pc, param1, 107, esi, 48, 3, 0x4011b0, 0, 0, param1, param1, param1, 0x460e, SUBFLAGS32(esp_4, 48, esp_4 - 48), esp_4 - 48 == 0, esp_4 < (unsigned int)48);
    *(__size32*)(esp_1 - 4) = 0x7f00;
    *(int*)(esp_1 - 8) = 0;
    *(__size32*)(esp_1 + 28) = eax;
    eax = LoadCursorA(*(esp_1 - 8), *(esp_1 - 4)); /* Warning: also results in ecx, edx */
    *(HCURSOR*)(esp_1 + 12LL + 20) = eax;
    eax = *(esp_1 + 12LL + 12);
    *(int*)(esp_1 + 12LL - 16) = 108;
    *(__size32*)(esp_1 + 12LL - 20) = eax;
    *(__size32*)(esp_1 + 12LL + 24) = 6;
    *(__size32*)(esp_1 + 12LL + 28) = 109;
    *(__size32*)(esp_1 + 12LL + 32) = 0x405490;
    (*esi)(*(esp_4 - 64), *(esp_4 - 60), *(esp_4 - 56), *(esp_4 - 52), *(esp_4 - 48), *(esp_4 - 44), *(esp_4 - 40), *(esp_4 - 36), *(esp_4 - 32), *(esp_4 - 28), *(esp_4 + 4), eax, ecx, edx, esi, <all>, flags, ZF, CF);
    *(__size32*)(esp + 48) = eax;
    *(__size32*)(esp - 4) = esp + 4;
    RegisterClassExA(*(esp - 4));
    esi = *(esp + 8LL - 8);
    return esi;
}

/** address: 0x00401150 */
__size32 proc_0x00401150(HINSTANCE param1)
{
    BOOL eax; 		// r24
    HWND eax_1; 		// r24{0}
    __size32 esi; 		// r30
    int esp; 		// r28
    union { __size32; __size32 *; } esp_1; 		// r28{0}
    int local12; 		// eax{0}
    union { __size32 *; __size32; } local13; 		// esp{0}

    global3_405558 = param1;
    eax_1 = CreateWindowExA(0, "", "", 0xcf0000, 0x80000000, 0, 0x80000000, 0, 0, 0, param1, 0); /* Warning: also results in esp_1 */
    local12 = eax_1;
    local13 = esp_1;
    if (eax_1 != 0) {
        eax = *(esp + 52LL - 44);
        *(__size32*)(esp + 52LL - 60) = eax;
        *(HWND*)(esp + 52LL - 64) = eax_1;
        ShowWindow(*(esp + 52LL - 60), *(esp + 52LL - 64));
        *(HWND*)(esp + 52LL + 12LL - 72) = eax_1;
        UpdateWindow(*(esp + 52LL + 12LL - 72));
        eax = 1;
        esp = esp + 52LL + 12LL + 8LL - 72;
        local12 = eax;
        local13 = esp;
    }
    eax = local12;
    esp = local13;
    esi = *esp;
    return eax; /* WARNING: Also returning: esi := esi */
}

