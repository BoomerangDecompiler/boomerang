void proc1(int param1, char param2[], int param3);

// address: 0x10760
int main(int argc, char *argv[], char *envp[]) {
    int local0; 		// m[o6 - 132]
    int o1; 		// r9

    o1 = *(argv + 4);
    proc1(argc, o1, local0);
    printf("%d\n", argc);
    return 0;
}

// address: 0x106c4
void proc1(int param1, char param2[], int param3) {
    int local0; 		// m[o6 + 68]
    int local1; 		// m[o6 - 20]
    int local2; 		// param3{96}
    int o0_1; 		// r8
    int o0_2; 		// r8{35}

    local2 = param3;
    if (param1 <= 2) {
        strlen(param2);
        local0 = o0_1;
    } else {
        strlen(param2);
        local0 = o0_2;
        strlen(param2);
        local1 = o0_1;
        printf("%d", o0_2 + o0_1);
        local2 = local1;
    }
    param3 = local2;
    printf("%d, %d", local0, param3);
    return;
}

