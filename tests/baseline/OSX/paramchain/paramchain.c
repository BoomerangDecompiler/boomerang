void passem(__size32 param2, __size32 param3, __size32 param4, __size32 *param4);
void addem(__size32 param2, __size32 param3, __size32 param4, __size32 *param4);

// address: 0x1d44
int main(int argc, char *argv[], char *envp[]) {
    __size32 local0; 		// m[g1 - 32]

    passem(5, 10, 40, &local0);
    printf(/* machine specific */ (int) LR + 656);
    return 0;
}

// address: 0x1cf8
void passem(__size32 param2, __size32 param3, __size32 param4, __size32 *param4) {
    addem(param2, param3, param4, param4);
    return;
}

// address: 0x1cb4
void addem(__size32 param2, __size32 param3, __size32 param4, __size32 *param4) {
    *(__size32*)param4 = param2 + param3 + param4;
    return;
}

