int main(int argc, char *argv[]);
__size32 cparam(int param1, __size32 param2);


/** address: 0x08048394 */
int main(int argc, char *argv[])
{
    int eax; 		// r24

    eax = cparam(argc - 3, 2);
    printf("Result is %d\n", eax);
    return 0;
}

/** address: 0x0804837c */
__size32 cparam(int param1, __size32 param2)
{
    __size32 eax; 		// r24
    __size32 local0; 		// m[esp + 8]
    __size32 local1; 		// param2{6}

    local1 = param2;
    if (param1 < 0) {
        local0 = 0;
        local1 = local0;
    }
    param2 = local1;
    eax = param2 + param1;
    return param2 + param1;
}

