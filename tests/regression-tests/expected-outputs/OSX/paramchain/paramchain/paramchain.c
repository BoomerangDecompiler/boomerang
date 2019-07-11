int main(int argc, char *argv[]);
void passem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4);
void addem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4);


/** address: 0x00001d44 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 32]

    passem(5, 10, 40, &local0);
    printf("Fifty five is %d\n", local0);
    return 0;
}

/** address: 0x00001cf8 */
void passem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4)
{
    addem(param1, param2, param3, param4);
    return;
}

/** address: 0x00001cb4 */
void addem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4)
{
    *(__size32*)param4 = param1 + param2 + param3;
    return;
}

