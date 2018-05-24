int main(int argc, union { __size32; char *[] *; } argv);

/** address: 0x00010f58 */
int main(int argc, union { __size32; char *[] *; } argv)
{
    __size32 fp; 		// r30
    int i0; 		// r24
    __size32 i1; 		// r25
    __size32 i2; 		// r26
    __size32 i3; 		// r27
    __size32 i4; 		// r28
    __size32 i5; 		// r29
    __size32 i7; 		// r31
    union { int *; __size32; } l0; 		// r16
    __size32 l1; 		// r17
    union { void *; __size32; } l2; 		// r18
    __size32 l3; 		// r19
    __size32 l4; 		// r20
    __size32 l5; 		// r21
    __size32 l6; 		// r22
    __size32 l7; 		// r23
    __size32 local0; 		// m[o6 + 4]
    __size32 local1; 		// m[o6 + 8]
    __size32 local10; 		// m[o6 + 44]
    __size32 local11; 		// m[o6 + 48]
    __size32 local12; 		// m[o6 + 52]
    __size32 local13; 		// m[o6 + 56]
    __size32 local14; 		// m[o6 + 60]
    int local15; 		// m[o6 + 4]{0}
    int local16; 		// m[o6 + 4]{0}
    int local17; 		// m[o6 + 4]{0}
    int local18; 		// m[o6 + 4]{0}
    int local19; 		// m[o6 + 8]{0}
    __size32 local2; 		// m[o6 + 12]
    int local20; 		// m[o6 + 8]{0}
    int local21; 		// m[o6 + 8]{0}
    int local22; 		// m[o6 + 8]{0}
    int local23; 		// m[o6 + 12]{0}
    int local24; 		// m[o6 + 12]{0}
    int local25; 		// m[o6 + 12]{0}
    int local26; 		// m[o6 + 12]{0}
    int local27; 		// m[o6 + 16]{0}
    int local28; 		// m[o6 + 16]{0}
    int local29; 		// m[o6 + 16]{0}
    __size32 local3; 		// m[o6 + 16]
    int local30; 		// m[o6 + 16]{0}
    int local31; 		// m[o6 + 20]{0}
    int local32; 		// m[o6 + 20]{0}
    int local33; 		// m[o6 + 20]{0}
    int local34; 		// m[o6 + 20]{0}
    int local35; 		// m[o6 + 24]{0}
    int local36; 		// m[o6 + 24]{0}
    int local37; 		// m[o6 + 24]{0}
    int local38; 		// m[o6 + 24]{0}
    int local39; 		// m[o6 + 28]{0}
    __size32 local4; 		// m[o6 + 20]
    int local40; 		// m[o6 + 28]{0}
    int local41; 		// m[o6 + 28]{0}
    int local42; 		// m[o6 + 28]{0}
    int local43; 		// m[o6 + 32]{0}
    int local44; 		// m[o6 + 32]{0}
    int local45; 		// m[o6 + 32]{0}
    int local46; 		// m[o6 + 32]{0}
    int local47; 		// m[o6 + 36]{0}
    int local48; 		// m[o6 + 36]{0}
    int local49; 		// m[o6 + 36]{0}
    __size32 local5; 		// m[o6 + 24]
    int local50; 		// m[o6 + 36]{0}
    int local51; 		// m[o6 + 40]{0}
    int local52; 		// m[o6 + 40]{0}
    int local53; 		// m[o6 + 40]{0}
    int local54; 		// m[o6 + 40]{0}
    int local55; 		// m[o6 + 44]{0}
    int local56; 		// m[o6 + 44]{0}
    int local57; 		// m[o6 + 44]{0}
    int local58; 		// m[o6 + 44]{0}
    int local59; 		// m[o6 + 48]{0}
    __size32 local6; 		// m[o6 + 28]
    int local60; 		// m[o6 + 48]{0}
    int local61; 		// m[o6 + 48]{0}
    int local62; 		// m[o6 + 48]{0}
    int local63; 		// m[o6 + 52]{0}
    int local64; 		// m[o6 + 52]{0}
    int local65; 		// m[o6 + 52]{0}
    int local66; 		// m[o6 + 52]{0}
    int local67; 		// m[o6 + 56]{0}
    int local68; 		// m[o6 + 56]{0}
    int local69; 		// m[o6 + 56]{0}
    __size32 local7; 		// m[o6 + 32]
    int local70; 		// m[o6 + 56]{0}
    int local71; 		// m[o6 + 60]{0}
    int local72; 		// m[o6 + 60]{0}
    int local73; 		// m[o6 + 60]{0}
    int local74; 		// m[o6 + 60]{0}
    __size32 local8; 		// m[o6 + 36]
    __size32 local9; 		// m[o6 + 40]
    int o0; 		// r8
    int *o0_1; 		// r8
    int o1; 		// r9
    __size32 o1_1; 		// r9{0}
    int o2; 		// r10
    int o2_1; 		// r10{0}
    int o2_2; 		// r10{0}
    int o2_3; 		// r10{0}
    __size32 o2_4; 		// r10{0}
    __size32 o2_5; 		// r10{0}
    __size32 o2_6; 		// r10{0}
    __size32 o2_7; 		// r10{0}
    __size32 o2_8; 		// r10{0}
    int o3; 		// r11
    int o3_1; 		// r11{0}
    int o4; 		// r12
    int o5; 		// r13
    __size32 o5_1; 		// r13{0}
    __size32 o5_2; 		// r13{0}
    __size32 o5_3; 		// r13{0}
    __size32 o5_4; 		// r13{0}
    __size32 o5_5; 		// r13{0}
    int o6; 		// r14
    int o7; 		// r15
    int o7_1; 		// r15{0}
    int o7_2; 		// r15{0}
    int o7_3; 		// r15{0}
    __size32 o7_4; 		// r15{0}
    __size32 o7_5; 		// r15{0}

    o0_1 = _Znwj(52);
    *(int*)(o0_1 + 36) = 100;
    *(int*)(o0_1 + 48) = 2;
    *(int*)(o0_1 + 40) = 101;
    *(int*)(o0_1 + 44) = 1;
    *(int*)(o0_1 + 32) = 0x217d8;
    o2_1 = *0x21740;
    *(int*)o0_1 = o2_1;
    o1_1 = *0x21744;
    o3_1 = *(o2_1 - 12);
    *(__size32*)(o0_1 + o3_1) = o1_1;
    *(int*)(o0_1 + 8) = 4;
    *(int*)(o0_1 + 4) = 3;
    o2_4 = *0x21748;
    *(__size32*)(o0_1 + 12) = o2_4;
    o7_1 = *(o2_4 - 12);
    o5_1 = *0x2174c;
    *(__size32*)(o0_1 + o7_1 + 12) = o5_1;
    *(int*)(o0_1 + 16) = 5;
    *(int*)(o0_1 + 20) = 6;
    *(int*)(o0_1 + 32) = 0x217c8;
    *(int*)(o0_1 + 12) = 0x217b8;
    *(int*)o0_1 = 0x217a4;
    *(int*)(o0_1 + 28) = 8;
    *(int*)(o0_1 + 24) = 7;
    o1 = *0x217a8;
    (*o1)(o0_1, o1, 8, 0x217a4, 0x21740, o5_1, o7_1, o0_1, l1, o0_1 + 12, l3, l4, l5, l6, l7, 0, argv, o2_7, o3, o4, o5_4, o6, o7_4, l1, l2, l3, l4, l5, l6, l7, i0, i1, i2, i3, i4, i5, fp, i7);
    o2 = *l0;
    o1 = *o2;
    (*o1)(l0, o1, o2, o3, o4, o5, o7, l0, l1, l2, l3, l4, l5, l6, l7, i0, i1, i2, i3, i4, i5, fp, i7, local15, local19, local23, local27, local31, local35, local39, local43, local47, local51, local55, local59, local63, local67, local71, <all>);
    o2 = *(l0 + 12);
    o1 = *o2;
    (*o1)(l2, o1, o2, o3, o4, o5, o7, l0, l1, l2, l3, l4, l5, l6, l7, i0, i1, i2, i3, i4, i5, fp, i7, local16, local20, local24, local28, local32, local36, local40, local44, local48, local52, local56, local60, local64, local68, local72, <all>);
    o0 = *l0;
    o1 = *(o0 - 12);
    o2 = *(l0 + o1);
    o3 = *o2;
    (*o3)(l0 + o1, o1, o2, o3, o4, o5, o7, l0, l1, l2, l0 + o1, l4, l5, l6, l7, i0, i1, i2, i3, i4, i5, fp, i7, local17, local21, local25, local29, local33, local37, local41, local45, local49, local53, local57, local61, local65, local69, local73, <all>);
    o0 = *(l0 + 12);
    l1 = *(o0 - 12);
    o1 = *(l2 + l1);
    o2 = *o1;
    (*o2)(l2 + l1, o1, o2, o3, o4, o5, o7, l0, l1, l2, l2 + l1, l4, l5, l6, l7, i0, i1, i2, i3, i4, i5, fp, i7, local18, local22, local26, local30, local34, local38, local42, local46, local50, local54, local58, local62, local66, local70, local74, <all>);
    o3 = *(l2 + l1);
    o1 = *o3;
    (*o1)(l3, o1, o2, o3, o4, o5, o7, l0, l1, l2, l3, l4, l5, l6, l7, i0, i1, i2, i3, i4, i5, fp, i7, local0, local1, local2, local3, local4, local5, local6, local7, local8, local9, local10, local11, local12, local13, local14, <all>);
    return i0;
}

