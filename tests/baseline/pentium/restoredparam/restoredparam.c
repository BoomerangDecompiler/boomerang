__size32 twice(__size32 param1);

// address: 0x804837c
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24

    eax = twice(argc);
    printf("Result is %d\n", eax);
    return 0;
}

// address: 0x8048396
__size32 twice(__size32 param1) {
    return param1 + param1;
}

