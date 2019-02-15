int main(int argc, char *argv[]);


/** address: 0x10000488 */
int main(int argc, char *argv[])
{
    __size32 g0; 		// r0
    union { int; void *; } g1; 		// r1
    int g3; 		// r3
    union { int; void *; } g31; 		// r31
    char * *g4; 		// r4
    __size32 g9; 		// r9
    int local0; 		// m[g1 + 4]
    __size32 local1; 		// m[g1 - 4]
    int local2; 		// m[g1 - 24]
    union { int; void *; } local3; 		// m[g1 - 32]

    (*/* machine specific */ (int) CTR)(0x10000418, 0x10000000, g1 - 32, /* machine specific */ (int) LR, g31, 0x10000418, g1, argc, argv);
    *(__size32*)(g31 + 8) = 0x10000450;
    g0 = *(g31 + 8);
    (*/* machine specific */ (int) CTR)(g0, 0x10000000, g31, <all>, local0, local1, local2, local3, g3, g4);
    return 0;
}

