int main(int argc, char *argv[]);
void fib1();


/** address: 0x00010738 */
int main(int argc, char *argv[])
{
    __size32 i0; 		// r24
    char * *i1; 		// r25
    __size32 i2; 		// r26
    __size32 i3; 		// r27
    __size32 i4; 		// r28
    __size32 i5; 		// r29
    void *i6; 		// r30
    __size32 i7; 		// r31
    __size32 l0; 		// r16
    __size32 l1; 		// r17
    __size32 l2; 		// r18
    __size32 l3; 		// r19
    __size32 l4; 		// r20
    __size32 l5; 		// r21
    __size32 l6; 		// r22
    __size32 l7; 		// r23
    int local0; 		// m[o6 - 20]
    __size32 o1; 		// r9
    int o2; 		// r10
    int o2_1; 		// r10{10}
    int o3; 		// r11
    int o3_1; 		// r11{10}
    int o4; 		// r12
    int o4_1; 		// r12{10}
    int o5; 		// r13
    int o5_1; 		// r13{10}
    int o6; 		// r14
    int o7; 		// r15
    int o7_1; 		// r15{10}

    printf("Input number: ");
    o1 = scanf("%d", &local0); /* Warning: also results in o2_1, o3_1, o4_1, o5_1, o7_1 */
    fib1(0x10800, local0, o1, o2_1, o3_1, o4_1, o5_1, o7_1, 0, argv, o2, o3, o4, o5, o6, o7, l0, l1, l2, l3, l4, l5, l6, l7, l1, l2, l3, l4, l5, l6, l7, i0, i1, i2, i3, i4, i5, i6, i7, local0);
}

/** address: 0x000106f4 */
void fib1()
{
/* goto 0x10704 */
}

