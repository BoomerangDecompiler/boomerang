int main(int argc, char *argv[]);
__size32 foo1();
__size32 foo2();

/** address: 0x00001d50 */
int main(int argc, char *argv[])
{
    __size32 g31; 		// r31

    g31 = foo1();
    printf(g31 + 656);
    return 0;
}

/** address: 0x00001d24 */
__size32 foo1()
{
    __size32 g31; 		// r31
    __size32 g31_1; 		// r31{0}

    g31_1 = foo2();
    return g31; /* WARNING: Also returning: g31 := g31_1 */
}

/** address: 0x00001cb8 */
__size32 foo2()
{
    __size32 g31; 		// r31

    *(int*)(/* machine specific */ (int) LR + 856) = 12;
    printf(/* machine specific */ (int) LR + 800);
    return g31; /* WARNING: Also returning: g31 := /* machine specific */ (int) LR */
}

