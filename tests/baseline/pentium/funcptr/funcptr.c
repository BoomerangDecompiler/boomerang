// address: 8048358
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    __size32 ebp; 		// r29
    unsigned int esp; 		// r28
    void *esp_1; 		// r28
    unsigned int local0; 		// m[esp - 16]
    int local1; 		// m[esp - 8]
    __size32 local2; 		// m[esp - 4]
    int local3; 		// m[esp + 4]
    int local4; 		// m[esp + 8]
    int local5; 		// m[esp + 12]
    int local6; 		// m[esp - 12]

    (*0x8048328)(pc, 0x8048328, ebp, argc, argv, envp, 0x8048328, esp - 4, SUBFLAGS32((esp - 12), 0, esp - 12), esp - 12 == 0, (unsigned int)(esp - 12) < 0);
    *(__size32*)(ebp - 4) = 0x8048340;
    eax = *(ebp - 4);
    (*eax)(local0, local1, local2, local3, local4, local5, eax, ebp, <all>, flags, ZF, CF);
    return 0;
}

