int main(int argc, char *argv[]);
__size32 proc1(size_t param1, int param2, char *param3);


/** address: 0x080483cf */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    size_t local0; 		// m[esp - 40]
    char *local1; 		// m[esp - 24]

    local1 = *(argv + 4);
    eax = proc1(local0, argc, local1);
    printf("%d\n", eax);
    return 0;
}

/** address: 0x0804835c */
__size32 proc1(size_t param1, int param2, char *param3)
{
    size_t eax; 		// r24
    size_t eax_1; 		// r24{6}
    size_t eax_4; 		// r24{8}
    size_t local1; 		// m[esp + 4]
    size_t local2; 		// m[esp - 8]
    size_t local5; 		// param1{12}

    local5 = param1;
    if (param2 <= 2) {
        eax = strlen(param3);
        local1 = eax;
    }
    else {
        eax_1 = strlen(param3);
        local1 = eax_1;
        eax_4 = strlen(param3);
        local2 = eax_4;
        local5 = local2;
        printf("%d", eax_4 + eax_1);
    }
    param1 = local5;
    printf("%d, %d", local1, param1);
    return local1;
}

