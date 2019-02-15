int main(int argc, char *argv[]);


/** address: 0x00001cf8 */
int main(int argc, char *argv[])
{
    void *g1; 		// r1
    __size32 g12; 		// r12
    __size32 g2; 		// r2
    __size32 g3; 		// r3
    __size32 g31; 		// r31
    __size32 g4; 		// r4
    int local0; 		// m[g1 + 8]
    __size32 local1; 		// m[g1 - 4]
    void *local2; 		// m[g1 - 80]

    g12 = *(/* machine specific */ (int) LR + 800);
    (*/* machine specific */ (int) CTR)(/* machine specific */ (int) LR, /* machine specific */ (int) LR, g12, /* machine specific */ (int) LR, /* machine specific */ (int) LR, g31, g1);
    g12 = *(g31 + 796);
    (*/* machine specific */ (int) CTR)(g31, g3, g4, g12, g31, <all>, local0, local1, local2);
    return 0;
}

