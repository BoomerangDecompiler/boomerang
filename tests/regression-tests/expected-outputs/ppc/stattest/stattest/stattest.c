int main(int argc, char *argv[]);
__size32 __stat();


/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    __size32 g0; 		// r0
    int g3; 		// r3
    int g31; 		// r31
    int g4; 		// r4
    int g5; 		// r5

    g3 = __stat();
    *(__size32*)(g31 + 112) = g3;
    g4 = *(g31 + 112);
    g5 = *(g31 + 60);
    printf("Stat returns %d; size of file is %d\n", g4, g5);
    g0 = *(g31 + 112);
    return g0;
}

/** address: 0x100005d0 */
__size32 __stat()
{
    int g3; 		// r3

    g3 = __xstat();
    return g3;
}

