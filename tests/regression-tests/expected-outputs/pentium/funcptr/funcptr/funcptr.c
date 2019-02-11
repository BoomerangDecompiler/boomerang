int main(int argc, char *argv[]);
void hello();

/** address: 0x08048358 */
int main(int argc, char *argv[])
{
    __size32 eax; 		// r24
    int ebp; 		// r29
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    void *esp; 		// r28
    void *esp_1; 		// r28{1}
    int local0; 		// m[esp - 4]
    int local1; 		// m[esp - 8]
    int local2; 		// m[esp - 16]
    int local3; 		// m[esp + 4]
    char * *local4; 		// m[esp + 8]

    ecx = hello(); /* Warning: also results in edx, esp_1 */
    *(__size32*)(ebp - 4) = 0x8048340;
    eax = *(ebp - 4);
    (*eax)(eax, ebp, <all>, flags, ZF, CF, NF, OF, local0, local1, local2, ecx, edx, argc, argv);
    return 0;
}

/** address: 0x08048328 */
void hello()
{
    printf("Hello, ");
    return;
}

