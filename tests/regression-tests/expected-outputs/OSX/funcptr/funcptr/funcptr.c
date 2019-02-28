int main(int argc, char *argv[]);


/** address: 0x00001d34 */
int main(int argc, char *argv[])
{
    __size32 g0; 		// r0
    union { int; void *; } g1; 		// r1
    __size32 g12; 		// r12
    int g3; 		// r3
    union { int; void *; } g30; 		// r30
    __size32 g31; 		// r31
    char * *g4; 		// r4
    __size32 g9; 		// r9
    int local0; 		// m[g1 + 8]
    __size32 local1; 		// m[g1 - 4]
    __size32 local2; 		// m[g1 - 8]
    __size32 local3; 		// m[g1 - 32]
    union { int; void *; } local4; 		// m[g1 - 96]

    g0 = *(/* machine specific */ (int) LR + 728);
    (**(/* machine specific */ (int) LR + 728))(g0, /* machine specific */ (int) LR, g0, g1 - 96, /* machine specific */ (int) LR, /* machine specific */ (int) LR, g31, g30, g0, g1, argc, argv);
    g0 = *(g31 + 724);
    *(__size32*)(g30 + 64) = g0;
    g0 = *(g30 + 64);
    (**(g30 + 64))(g0, g31, g0, g30, g31, <all>, local0, local1, local2, local3, local4, g3, g4);
    return 0;
}

