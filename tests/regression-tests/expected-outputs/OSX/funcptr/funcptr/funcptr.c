int main(int argc, char *argv[]);

/** address: 0x00001d34 */
int main(int argc, char *argv[])
{
    __size32 g0; 		// r0
    __size32 g1; 		// r1
    __size32 g12; 		// r12
    int g3; 		// r3
    __size32 g30; 		// r30
    __size32 g31; 		// r31
    union { char *[] *; __size32; } g4; 		// r4
    __size32 g9; 		// r9
    int local0; 		// m[g1 + 8]
    __size32 local1; 		// m[g1 + 112]
    __size32 local2; 		// m[g1 + 116]
    __size32 local3; 		// m[g1 - 32]
    __size32 local4; 		// m[g1 - 96]
    __size32 tmp + 30; 		// r[tmp + 30]

    g0 = *(/* machine specific */ (int) LR + 728);
    (*/* machine specific */ (int) CTR)(tmp + 30, g0, argc, argv, /* machine specific */ (int) LR, g0, g1 - 96, /* machine specific */ (int) LR, /* machine specific */ (int) LR, tmp + 30, tmp + 30, g0, g1);
    g0 = *(g31 + 724);
    *(__size32*)(g30 + 64) = g0;
    g0 = *(g30 + 64);
    (*/* machine specific */ (int) CTR)(tmp + 30, g0, g3, g4, g31, g0, g30, g31, local0, local1, local2, local3, local4, <all>);
    return 0;
}

