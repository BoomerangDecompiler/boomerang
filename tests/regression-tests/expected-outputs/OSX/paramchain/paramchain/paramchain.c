int main(int argc, char *argv[]);
__size32 passem(__size32 param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6);
__size32 addem(__size32 param1, __size32 param2, __size32 param3, __size32 param4, union { __size32; __size32 *; } param5);

/** address: 0x00001d44 */
int main(int argc, char *argv[])
{
    __size32 g1; 		// r1
    __size32 g31; 		// r31

    g31 = passem(/* machine specific */ (int) LR, 5, 10, 40, (g1 - 32), /* machine specific */ (int) LR);
    printf(g31 + 656);
    return 0;
}

/** address: 0x00001cf8 */
__size32 passem(__size32 param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6)
{
    __size32 g31; 		// r31

    g31 = addem(param6, param2, param3, param4, param5);
    return param1; /* WARNING: Also returning: g31 := g31 */
}

/** address: 0x00001cb4 */
__size32 addem(__size32 param1, __size32 param2, __size32 param3, __size32 param4, union { __size32; __size32 *; } param5)
{
    *(__size32*)param5 = param2 + param3 + param4;
    return param1; /* WARNING: Also returning: g31 := param1 */
}

