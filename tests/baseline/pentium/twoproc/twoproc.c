__size32 proc1(__size32 param1, __size32 param2);

// address: 0x8048375
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24

    eax = proc1(11, 4);
    printf("%i\n", eax);
    return eax;
}

// address: 0x8048368
__size32 proc1(__size32 param1, __size32 param2) {
    return param1 - param2;
}

