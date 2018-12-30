int main(int argc, char *argv[]);
void twofib(__size32 param3, __size32 param4, __size32 *param3, __size32 param4);

/** address: 0x080483f5 */
int main(int argc, char *argv[])
{
    int local0; 		// m[esp - 8]
    __size32 local1; 		// m[esp - 76]
    __size32 local2; 		// m[esp - 72]
    int local3; 		// m[esp - 20]

    printf("Enter number: ");
    scanf("%d", &local0);
    twofib(local1, local2, &local3, local0);
    printf("Fibonacci of %d is %d\n", local0, local3);
    return 0;
}

/** address: 0x0804839c */
void twofib(__size32 param3, __size32 param4, __size32 *param3, __size32 param4)
{
    void *esp; 		// r28
    __size32 local10; 		// m[esp - 88]
    __size32 local11; 		// m[esp - 52]
    __size32 local12; 		// m[esp - 48]
    int local2; 		// m[esp - 12]
    int local3; 		// m[esp - 8]
    __size32 local9; 		// m[esp - 92]

    if (param4 != 0) {
        twofib(local11, local12, esp - 12, param4 - 1, local9, local10);
        local2 = param4;
        local3 = param4 + param3;
    }
    else {
        local2 = 0;
        local3 = 1;
    }
    *(__size32*)param3 = local2;
    *(__size32*)(param3 + 4) = local3;
    return;
}

