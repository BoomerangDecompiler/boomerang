int main(int argc, char *argv[]);
void fib1();


/** address: 0x080483a0 */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    union { unsigned int; void *; } ebp; 		// r29
    int ecx; 		// r25
    int edx; 		// r26
    int esp; 		// r28
    int local0; 		// m[esp - 8]
    __size32 local3; 		// m[esp - 4]
    int local6; 		// m[esp + 4]
    char * *local7; 		// m[esp + 8]

    printf("Input number: ");
    ecx = scanf("%d", &local0); /* Warning: also results in edx */
    eax = fib1(0x804849f, ecx, edx, esp - 4, SUBFLAGS32((esp - 12), 12, esp - 24), esp == 24, esp - 12 < 12, esp < 24, esp < 12 && esp >= 24, ebp, local0, esp - 8, local0, pc, argc, argv); /* Warning: also results in esp, ebp */
    local7 = eax;
    local6 = *(ebp - 4);
    *(char **)esp = "fibonacci(%d) = %d\n";
    printf(*esp);
    return 0;
}

/** address: 0x0804835c */
void fib1()
{
}

