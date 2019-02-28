int main(int argc, char *argv[]);
void hello();


/** address: 0x10000488 */
int main(int argc, char *argv[])
{
    __size32 g0; 		// r0
    void *g1; 		// r1
    __size32 g10; 		// r10
    __size32 g11; 		// r11
    __size32 g12; 		// r12
    __size32 g3; 		// r3
    int g31; 		// r31
    __size32 g4; 		// r4
    __size32 g5; 		// r5
    __size32 g6; 		// r6
    __size32 g7; 		// r7
    __size32 g8; 		// r8
    __size32 g9; 		// r9
    int local0; 		// m[g1 + 4]
    int local1; 		// m[g1 - 4]
    int local2; 		// m[g1 - 24]
    int local3; 		// m[g1 - 32]

    g3 = hello(); /* Warning: also results in g4, g5, g6, g7, g8, g10, g11, g12 */
    *(__size32*)(g31 + 8) = 0x10000450;
    g0 = *(g31 + 8);
    (**(g31 + 8))(g0, 0x10000000, g31, <all>, local0, local1, local2, local3, g3, g4, g5, g6, g7, g8, g10, g11, g12);
    return 0;
}

/** address: 0x10000418 */
void hello()
{
    printf("Hello, ");
    return;
}

