int main(int argc, union { __size32; char *[] *; } argv);
__size32 proc1(__size32 param1, int param2, union { __size32; char[] *; } param3);

/** address: 0x00001d18 */
int main(int argc, union { __size32; char *[] *; } argv)
{
    __size32 g31; 		// r31
    int g4; 		// r4

    g4 = *(argv + 4);
    g31 = proc1(/* machine specific */ (int) LR, argc, g4);
    printf(g31 + 716);
    return 0;
}

/** address: 0x00001c74 */
__size32 proc1(__size32 param1, int param2, union { __size32; char[] *; } param3)
{
    if (param2 <= 2) {
        strlen(param3);
    }
    else {
        strlen(param3);
        strlen(param3);
        printf(/* machine specific */ (int) LR + 868);
    }
    printf(/* machine specific */ (int) LR + 872);
    return param1; /* WARNING: Also returning: g31 := /* machine specific */ (int) LR */
}

