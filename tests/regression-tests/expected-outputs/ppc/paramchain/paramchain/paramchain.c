int main(int argc, char *argv[]);
void passem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4);
void addem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4);


/** address: 0x100004b0 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 24]

    passem(5, 10, 40, &local0);
    printf("Fifty five is %d\n", local0);
    return 0;
}

/** address: 0x10000460 */
void passem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4)
{
    addem(param1, param2, param3, param4);
    return;
}

/** address: 0x10000418 */
void addem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4)
{
    *(__size32*)param4 = param1 + param2 + param3;
    return;
}

