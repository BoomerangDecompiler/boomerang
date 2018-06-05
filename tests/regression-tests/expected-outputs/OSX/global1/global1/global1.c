int main(int argc, char *argv[]);
__size32 foo1(__size32 param1, __size32 param2);
__size32 foo2(__size32 param1);

/** address: 0x00001d50 */
int main(int argc, char *argv[])
{
    __size32 g31; 		// r31

    g31 = foo1(/* machine specific */ (int) LR, /* machine specific */ (int) LR);
    printf(g31 + 656);
    return 0;
}

/** address: 0x00001d24 */
__size32 foo1(__size32 param1, __size32 param2)
{
    __size32 g31; 		// r31

    g31 = foo2(param2);
    return param1; /* WARNING: Also returning: g31 := g31 */
}

/** address: 0x00001ccc */
__size32 foo2(__size32 param1)
{
    *(int*)(/* machine specific */ (int) LR + 832) = 12;
    printf(/* machine specific */ (int) LR + 780);
    return param1; /* WARNING: Also returning: g31 := /* machine specific */ (int) LR */
}

