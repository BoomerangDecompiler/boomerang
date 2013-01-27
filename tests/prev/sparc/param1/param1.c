__size32 cparam(int param1, __size32 param2);

// address: 0x106a0
int main(int argc, char *argv[], char *envp[]) {
    int o0; 		// r8

    o0 = cparam(argc - 3, 2);
    printf("Result is %d\n", o0);
    return 0;
}

// address: 0x10688
__size32 cparam(int param1, __size32 param2) {
    __size32 local0; 		// param2{9}
    __size32 o1; 		// r9

    local0 = param2;
    if (param1 < 0) {
        o1 = 0;
        local0 = o1;
    }
    param2 = local0;
    return param1 + param2;
}

