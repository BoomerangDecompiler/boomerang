int main(int argc, char *argv[]);

/** address: 0x08048358 */
int main(int argc, char *argv[])
{
    __size32 eax; 		// r24
    union { unsigned int; void *; } ebp; 		// r29
    union { unsigned int; void *; } esp; 		// r28
    void *esp_1; 		// r28{1}
    __size32 local0; 		// m[esp - 4]
    int local1; 		// m[esp - 8]
    unsigned int local2; 		// m[esp - 16]
    int local3; 		// m[esp + 4]
    char * *local4; 		// m[esp + 8]
    int local5; 		// m[esp - 12]

    (*0x8048328)(0x8048328, esp - 4, SUBFLAGS32((esp - 12), 0, esp - 12), esp == 12, esp < 12, esp < 12, esp < 12 && esp >= 12, ebp, 0x8048328, pc, argc, argv);
    *(__size32*)(ebp - 4) = 0x8048340;
    eax = *(ebp - 4);
    (*eax)(eax, ebp, <all>, flags, ZF, CF, NF, OF, local0, local1, local2, local3, local4);
    return 0;
}

