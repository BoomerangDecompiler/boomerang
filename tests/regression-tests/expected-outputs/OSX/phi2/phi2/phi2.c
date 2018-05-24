int main(int argc, union { __size32; char *[] *; } argv);
__size32 proc1(int param2, union { __size32; char[] *; } param2);

/** address: 0x00001d18 */
int main(int argc, union { __size32; char *[] *; } argv)
{
    __size32 g31; 		// r31
    int g4; 		// r4

    g4 = *(argv + 4);
    g31 = proc1(argc, g4);
    printf(g31 + 716);
    return 0;
}

/** address: 0x00001c74 */
__size32 proc1(int param2, union { __size32; char[] *; } param2)
{
    __size32 g31; 		// r31

    if (param2 <= 2) {
        strlen(param2);
    }
    else {
        strlen(param2);
        strlen(param2);
        printf(/* machine specific */ (int) LR + 868);
    }
    printf(/* machine specific */ (int) LR + 872);
    return g31; /* WARNING: Also returning: g31 := /* machine specific */ (int) LR */
}

