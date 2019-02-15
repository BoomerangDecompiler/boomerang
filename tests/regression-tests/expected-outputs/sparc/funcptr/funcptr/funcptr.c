int main(int argc, char *argv[]);
void hello();


/** address: 0x000106e0 */
int main(int argc, char *argv[])
{
    __size32 g0; 		// r0
    __size32 i0; 		// r24
    __size32 i1; 		// r25
    __size32 i2; 		// r26
    __size32 i3; 		// r27
    __size32 i4; 		// r28
    __size32 i5; 		// r29
    int i6; 		// r30
    __size32 i7; 		// r31
    __size32 l0; 		// r16
    __size32 l1; 		// r17
    __size32 l2; 		// r18
    __size32 l3; 		// r19
    __size32 l4; 		// r20
    __size32 l5; 		// r21
    __size32 l6; 		// r22
    __size32 l7; 		// r23
    int local0; 		// m[o6 + 4]
    int local1; 		// m[o6 + 8]
    int local10; 		// m[o6 + 44]
    int local11; 		// m[o6 + 48]
    int local12; 		// m[o6 + 52]
    int local13; 		// m[o6 + 56]
    int local14; 		// m[o6 + 60]
    int local15; 		// m[o6 - 20]
    int local2; 		// m[o6 + 12]
    int local3; 		// m[o6 + 16]
    int local4; 		// m[o6 + 20]
    int local5; 		// m[o6 + 24]
    int local6; 		// m[o6 + 28]
    int local7; 		// m[o6 + 32]
    int local8; 		// m[o6 + 36]
    int local9; 		// m[o6 + 40]
    __size32 o0; 		// r8
    __size32 o1; 		// r9
    __size32 o2; 		// r10
    __size32 o3; 		// r11
    __size32 o4; 		// r12
    __size32 o5; 		// r13
    void *o6; 		// r14
    __size32 o7; 		// r15

    g0 = hello();
    *(__size32*)(i6 - 20) = 0x106c0;
    o0 = *(i6 - 20);
    (**(i6 - 20))(o0, i0, i1, i2, i3, i4, i5, i6, i7, <all>, o1, o2, o3, o4, o5, o7, l0, l1, l2, l3, l4, l5, l6, l7, local0, local1, local2, local3, local4, local5, local6, local7, local8, local9, local10, local11, local12, local13, local14, local15, g0);
    return 0;
}

/** address: 0x000106a0 */
void hello()
{
    printf("Hello, ");
    return;
}

