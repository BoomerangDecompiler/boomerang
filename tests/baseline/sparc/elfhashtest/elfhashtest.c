int elf_hash(char *param1);

// address: 10678
int main(int argc, char *argv[], char *envp[]) {
    unsigned int o0_1; 		// r8

    o0_1 = elf_hash("main");
    printf("elf_hash(\"main\") is %x\n", o0_1);
    return 0;
}

// address: 106b4
int elf_hash(char *param1) {
    int o3; 		// r11
    int o4; 		// r12

    o3 = (int) *param1;
    o4 = 0;
    if (o3 != 0) {
        o4 = (o3 ^ (o3 & 0xf0000000) / 0x1000000) &  !(o3 & 0xf0000000);
        o3 = (int) *(param1 + 1);
    }
    return o4;
}

