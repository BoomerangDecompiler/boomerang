int main(int argc, char *argv[]);
void passem(__size32 param2, __size32 param3, __size32 param4, __size32 param4);
void addem(__size32 param2, __size32 param3, __size32 param4, union { __size32; __size32 *; } param4);

/** address: 0x00001d44 */
int main(int argc, char *argv[])
{
    __size32 g1; 		// r1

    passem(5, 10, 40, (g1 - 32));
    printf(/* machine specific */ (int) LR + 656);
    return 0;
}

/** address: 0x00001cf8 */
void passem(__size32 param2, __size32 param3, __size32 param4, __size32 param4)
{
    addem(param2, param3, param4, param4);
    return;
}

/** address: 0x00001cb4 */
void addem(__size32 param2, __size32 param3, __size32 param4, union { __size32; __size32 *; } param4)
{
    *(__size32*)param4 = param2 + param3 + param4;
    return;
}

