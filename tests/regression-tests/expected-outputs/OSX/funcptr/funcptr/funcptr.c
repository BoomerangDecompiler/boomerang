int main(int argc, char *argv[]);

void(*global_0x00002024)(void);

/** address: 0x00001d34 */
int main(int argc, char *argv[])
{
    __size32 CTR; 		// r301
    __size32 LR; 		// r300
    __size32 g0; 		// r0
    union { int; void *; } g1; 		// r1
    __size32 g12; 		// r12
    int g3; 		// r3
    union { int; void *; } g30; 		// r30
    __size32 g31; 		// r31
    char * *g4; 		// r4
    __size32 g9; 		// r9
    __size32 local0; 		// m[g1 + 8]
    __size32 local1; 		// m[g1 - 4]
    __size32 local2; 		// m[g1 - 8]
    int local3; 		// m[g1 - 32]
    union { int; void *; } local4; 		// m[g1 - 96]

    (*global_0x00002024)(global_0x00002024, 0x1d4c, global_0x00002024, g1 - 96, 0x1d4c, 0x1d6c, global_0x00002024, LR, g31, g30, global_0x00002024, g1, argc, argv);
    g0 = *(g31 + 724);
    *(__size32*)(g30 + 64) = g0;
    g0 = *(g30 + 64);
    (**(g30 + 64))(g0, g3, g31, g0, g30, g31, 0x1d88, g0, <all>, local0, local1, local2, local3, local4, g4);
    return 0;
}

