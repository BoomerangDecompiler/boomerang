int main(int argc, __size32 argv);

/** address: 0x000106e0 */
int main(int argc, __size32 argv)
{
    __size32 fp; 		// r30
    int i0; 		// r24
    __size32 i1; 		// r25
    __size32 i2; 		// r26
    __size32 i3; 		// r27
    __size32 i4; 		// r28
    __size32 i5; 		// r29
    __size32 i7; 		// r31
    __size32 l0; 		// r16
    __size32 l1; 		// r17
    __size32 l2; 		// r18
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
    int local15; 		// m[o6 - 20]
    __size32 local2; 		// m[o6 + 12]
    __size32 local3; 		// m[o6 + 16]
    __size32 local4; 		// m[o6 + 20]
    __size32 local5; 		// m[o6 + 24]
    __size32 local6; 		// m[o6 + 28]
    __size32 local7; 		// m[o6 + 32]
    __size32 local8; 		// m[o6 + 36]
    __size32 local9; 		// m[o6 + 40]
    __size32 o0; 		// r8
    __size32 o1; 		// r9
    __size32 o2; 		// r10
    __size32 o3; 		// r11
    __size32 o4; 		// r12
    __size32 o5; 		// r13
    __size32 o6; 		// r14
    __size32 o7; 		// r15

    (*0x106a0)(0x106a0, argv, o2, o3, o4, o5, o7, l0, l1, l2, l3, l4, l5, l6, l7, argc, argv, o2, o3, o4, o5, o6, o7, l1, l2, l3, l4, l5, l6, l7, i0, i1, i2, i3, i4, i5, fp, i7, 0x106a0);
    *(__size32*)(fp - 20) = 0x106c0;
    o0 = *(fp - 20);
    (*o0)(o0, o1, o2, o3, o4, o5, o7, l0, l1, l2, l3, l4, l5, l6, l7, i0, i1, i2, i3, i4, i5, fp, i7, local0, local1, local2, local3, local4, local5, local6, local7, local8, local9, local10, local11, local12, local13, local14, local15, <all>);
    return 0;
}

