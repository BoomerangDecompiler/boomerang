void __thunk_36_foo__1D();

// address: 0x80487a0
int main(int argc, char *argv[], char *envp[]) {
    __size8 al; 		// r8
    void **eax; 		// r24
    __size32 ebp; 		// r29
    __size32 ebx; 		// r27
    int ecx; 		// r25
    __size32 edi; 		// r31
    int edx; 		// r26
    __size32 esi; 		// r30
    int esp; 		// r28
    __size32 *esp_1; 		// r28
    int local0; 		// m[esp - 44]
    unsigned int local1; 		// m[esp - 48]
    int local10; 		// m[esp + 4]{43}
    int local11; 		// m[esp + 4]{49}
    int local12; 		// m[esp + 4]{65}
    int local13; 		// m[esp + 4]{84}
    int local14; 		// m[esp + 8]{43}
    int local15; 		// m[esp + 8]{49}
    int local16; 		// m[esp + 8]{65}
    int local17; 		// m[esp + 8]{84}
    int local18; 		// m[esp + 12]{43}
    int local19; 		// m[esp + 12]{49}
    void *local2; 		// m[esp - 44]
    int local20; 		// m[esp + 12]{65}
    int local21; 		// m[esp + 12]{84}
    int local22; 		// m[esp - 4]{43}
    int local23; 		// m[esp - 4]{49}
    int local24; 		// m[esp - 4]{65}
    int local25; 		// m[esp - 4]{84}
    int local26; 		// m[esp - 8]{43}
    int local27; 		// m[esp - 8]{49}
    int local28; 		// m[esp - 8]{65}
    int local29; 		// m[esp - 8]{84}
    __size32 local3; 		// m[esp - 16]
    int local30; 		// m[esp - 12]{43}
    int local31; 		// m[esp - 12]{49}
    int local32; 		// m[esp - 12]{65}
    int local33; 		// m[esp - 12]{84}
    int local34; 		// m[esp - 16]{43}
    int local35; 		// m[esp - 16]{49}
    int local36; 		// m[esp - 16]{65}
    int local37; 		// m[esp - 16]{84}
    int local38; 		// m[esp - 44]{43}
    int local39; 		// m[esp - 44]{49}
    __size32 local4; 		// m[esp - 12]
    int local40; 		// m[esp - 44]{65}
    int local41; 		// m[esp - 44]{84}
    int local42; 		// m[esp - 48]{43}
    int local43; 		// m[esp - 48]{49}
    int local44; 		// m[esp - 48]{65}
    int local45; 		// m[esp - 48]{84}
    int local46; 		// %flags{43}
    int local47; 		// %flags{84}
    int local48; 		// %flags{91}
    int local49; 		// %ZF{43}
    __size32 local5; 		// m[esp - 8]
    int local50; 		// %ZF{84}
    int local51; 		// %ZF{91}
    int local52; 		// %CF{43}
    int local53; 		// %CF{84}
    int local54; 		// %CF{91}
    int local55; 		// m[esp - 40]
    __size32 local6; 		// m[esp - 4]
    int local7; 		// m[esp + 4]
    int local8; 		// m[esp + 8]
    int local9; 		// m[esp + 12]

    __builtin_new(56);
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
    esi = __thunk_36_foo__1D(pc, eax + 36, ebx, esi, edi, ebp, argc, argv, envp, eax + 36, ecx, edx, eax + 16, esp - 4, eax, edi, SUBFLAGS32(esp - 16, 24, (esp - 40)), esp - 40 == 0, (unsigned int)(esp - 16) < 24); /* Warning: also results in al, ecx, edx, ebx, esp, ebp, edi */
    eax = *(esi + 12);
    *(__size32*)esp = esi;
    (**(*(esi + 12) + 8))(local42, local38, local34, local30, local26, local22, local10, local14, local18, al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, local46, local49, local52);
    al =  (esi == 0) ? 1 : 0;
    edi = (0 >> 8 & 0xffffff | (al)) - 1 & ebx;
    eax = *edi;
    edx = *(eax + 8);
    *(__size32*)esp = eax;
    (**(*(eax + 8) + 8))(local43, local39, local35, local31, local27, local23, local11, local15, local19, al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, LOGICALFLAGS32(edi), LOGICALFLAGS32(edi), LOGICALFLAGS32(edi));
    ebx = 0;
    if (esi != 0) {
        ebx = *esi;
    }
    eax = *(ebx + 8);
    *(__size32*)esp = ebx;
    (**(*(ebx + 8) + 8))(local44, local40, local36, local32, local28, local24, local12, local16, local20, al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, SUBFLAGS32(esp + 16, 12, esp + 4), esp + 4 == 0, esp + 16 < 12);
    ebx = *edi;
    eax = *(ebx + 8);
    *(__size32*)esp = ebx;
    (**(*(ebx + 8) + 8))(local45, local41, local37, local33, local29, local25, local13, local17, local21, al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, local47, local50, local53);
    eax = *(ebx + 8);
    *(__size32*)esp = ebx;
    (**(*(ebx + 8) + 8))(local1, local0, local3, local4, local5, local6, local7, local8, local9, al, eax, ecx, edx, ebx, ebp, esi, edi, <all>, local48, local51, local54);
    return 0;
}

// address: 0x80488d0
void __thunk_36_foo__1D() {
    __ls__7ostreamPCc();
    hex__FR3ios();
    __ls__7ostreamPCv();
    __ls__7ostreamPCc();
    endl__FR7ostream();
    return;
}

