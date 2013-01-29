void passem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4);
void addem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4);

// address: 8048950
int main(int argc, char *argv[], char *envp[]) {
    int local0; 		// m[esp - 8]

    passem(5, 10, 40, &local0);
    printf("Fifty five is %d\n", local0);
    return 0;
}

// address: 8048938
void passem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4) {
    addem(param1, param2, param3, param4);
    return;
}

// address: 8048924
void addem(__size32 param1, __size32 param2, __size32 param3, __size32 *param4) {
    __size32 eax; 		// r24

    eax = param1 + param2;
    eax += param3;
    *(__size32*)param4 = eax;
    return;
}

