// address: 8048364
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24

    printf("%08X\n", (0x87654321 >> 8 & 0xffffff | 18) & 0xffff00ff | 0x3400);
    return eax;
}

