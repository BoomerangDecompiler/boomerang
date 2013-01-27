void mid(__size32 param1);
void fst(__size32 param1);

// address: 0x10744
int main(int argc, char *argv[], char *envp[]) {
    int local0; 		// m[o6 - 24]
    unsigned char *local1; 		// m[o6 - 28]
    int local2; 		// m[o6 - 20]
    int o0; 		// r8

    local0 = 0;
    mid(0x20a50);
    fst(0x20a46);
    local1 = 0x20a50;
    local2 = 0;
    while (local2 <= 4) {
        o0 = *(unsigned char*)local1;
        local0 += (int)(o0 * 0x1000000) >> 24;
        local1++;
        local2++;
    }
    printf("Sum is %d\n", local0);
    return 0;
}

// address: 0x106cc
void mid(__size32 param1) {
    int o0; 		// r8

    o0 = *(unsigned char*)(param1 + 2);
    printf("Middle elment is %d\n", (int)(o0 * 0x1000000) >> 24);
    return;
}

// address: 0x10708
void fst(__size32 param1) {
    int o0; 		// r8

    o0 = *(unsigned char*)(param1 + 10);
    printf("First element is %d\n", (int)(o0 * 0x1000000) >> 24);
    return;
}

