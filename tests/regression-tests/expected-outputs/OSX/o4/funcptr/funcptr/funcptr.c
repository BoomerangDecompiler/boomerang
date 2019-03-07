int main(int argc, char *argv[]);

void(*global_0x00002024)(void);

/** address: 0x00001cf8 */
int main(int argc, char *argv[])
{
    __size32 CTR; 		// r301
    __size32 LR; 		// r300
    void *g1; 		// r1
    __size32 g12; 		// r12
    __size32 g2; 		// r2
    __size32 g3; 		// r3
    __size32 g31; 		// r31
    __size32 g4; 		// r4
    __size32 local0; 		// m[g1 + 8]
    __size32 local1; 		// m[g1 - 4]
    void *local2; 		// m[g1 - 80]

    (*global_0x00002024)(0x1d04, LR, global_0x00002024, 0x1d04, pc + 4, global_0x00002024, LR, g31, g1);
    g12 = *(g31 + 796);
    (**(g31 + 796))(g31, g3, g4, g12, g31, pc + 4, g12, <all>, local0, local1, local2);
    return 0;
}

