int proc1(int param1, int param2, char param3[]);

// address: 0x80483cf
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    int local0; 		// m[esp - 40]
    char *local1; 		// m[esp - 24]

    local1 = *(argv + 4);
    eax = proc1(local0, argc, local1);
    printf("%d\n", eax);
    return 0;
}

// address: 0x804835c
int proc1(int param1, int param2, char param3[]) {
    __size32 eax; 		// r24
    int eax_1; 		// r24{18}
    int local1; 		// m[esp + 4]
    int local2; 		// m[esp - 8]
    int local5; 		// param1{100}

    local5 = param1;
    if (param2 <= 2) {
        strlen(param3);
        local1 = eax;
    } else {
        strlen(param3);
        local1 = eax_1;
        strlen(param3);
        local2 = eax;
        printf("%d", eax + eax_1);
        local5 = local2;
    }
    param1 = local5;
    printf("%d, %d", local1, param1);
    return local1;
}

