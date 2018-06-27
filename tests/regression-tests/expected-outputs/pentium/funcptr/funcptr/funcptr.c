int main(int argc, char *argv[]);

/** address: 0x08048358 */
int main(int argc, char *argv[])
{
    __size32 eax; 		// r24
    __size32 ebp; 		// r29
    unsigned int esp; 		// r28
    __size32 esp_1; 		// r28{0}
    unsigned int local0; 		// m[esp - 16]
    int local1; 		// m[esp - 8]
    __size32 local2; 		// m[esp - 4]
    int local3; 		// m[esp + 4]
    char * *local4; 		// m[esp + 8]
    int local5; 		// m[esp - 12]

    (*0x8048328)(pc, 0x8048328, ebp, argc, argv, 0x8048328, esp - 4, SUBFLAGS32((esp - 12), 0, esp - 12), esp == 12, esp < 12);
    *(__size32*)(ebp - 4) = 0x8048340;
    eax = *(ebp - 4);
    (*eax)(local0, local1, local2, local3, local4, eax, ebp, <all>, flags, ZF, CF);
    return 0;
}

